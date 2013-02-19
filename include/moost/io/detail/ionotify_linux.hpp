/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * Copyright © 2008-2013 Last.fm Limited
 *
 * This file is part of libmoost.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// This is worth reading if you are new to inotify
// http://www.linuxjournal.com/article/8478?page=0,0

#ifndef MOOST_IO_NOTIFY_LINUX_HPP__
#define MOOST_IO_NOTIFY_LINUX_HPP__

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <sys/inotify.h>

#include <string>
#include <vector>
#include <map>

#include <iostream>
#include <stdio.h>
#include <poll.h>

namespace moost { namespace io {

   class ionotify
   {
   public:
      ionotify(
         bool stop_immediately = false   // Terminate as soon as the event processor quits, don't flush handlers
         ):
            fd_(inotify_init()),
            stop_immediately_(stop_immediately)
      {
         if(fd_ < 0)
         {
            throw std::runtime_error("failed to initialise inotify");
         }
      }

      ~ionotify()
      {
         try
         {
            stop();
            close(fd_); // And we're done
         }
         catch(...)
         {
            // swallowed.
         }
      }

      enum file_action
      {
         CREATED = 0, ///< File creation (here to be backwards compatible with file_watcher, but not directly supported by inotify!)
         CHANGED = 1, ///< File modification
         DELETED = 2  ///< File deletion
      };

      typedef boost::function<void(file_action fa, const std::string & path)> callback_t;

   private:
      typedef std::pair<callback_t, std::string> event_t;
      typedef std::map<int,  event_t> event_map_t;
      typedef std::map<std::string, int> path_map_t;
      typedef boost::shared_ptr<boost::thread> pthread_t;

      static const int EVENT_MASK = IN_ATTRIB | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM;

   public:
      void insert(std::string const & path, callback_t const & callback, bool call_now = false)
      {
         boost::unique_lock<boost::shared_mutex> ul(smtx_event_);

         // Add a new watch to the event notifier

         int wd = inotify_add_watch(
            fd_, path.c_str(),
            EVENT_MASK
            );

         if(wd < 0)
         {
            throw std::runtime_error("failed to add watch");
         }

         path_map_[path] = wd;
         event_map_[wd] = event_t(callback, path);

         // If we've been asked to fire now, well maybe we should :)
         if(call_now) { callback(CHANGED, path); }
      }

      void erase(std::string const & path)
      {
         boost::unique_lock<boost::shared_mutex> ul(smtx_event_);

         // Find an event and if we do remove the watch
         path_map_t::iterator itr  = path_map_.find(path);

         if(itr != path_map_.end())
         {
            inotify_rm_watch(itr->second, EVENT_MASK); // If this fails it fails... not much we can do really!
            event_map_.erase(itr->second);
            path_map_.erase(itr);
         }
      }

      void start()
      {
         boost::unique_lock<boost::mutex> ul(mtx_stop_start_);

         // Start event handler queue processor
         if(!pthread_handler)
         {
            // we only have one thread because we want events to be processed in order, we're
            // using asio to do this so we don't block the event processng queue whilst we wait
            // for a slow user callback to finish.
            spwork_.reset(new boost::asio::io_service::work(ioservice_));
            pthread_handler.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &ioservice_)));
         }

         // start the event processer
         if(!pthread_event)
         {
            pthread_event.reset(new boost::thread(boost::bind(&ionotify::thread_proc, this)));
         }
      }

      void stop()
      {
         boost::unique_lock<boost::mutex> ul(mtx_stop_start_);

         // Stop event handler queue processor
         if(pthread_event)
         {
            // Wait for the event processor to be done
            pthread_event->interrupt();
            pthread_event->join();
            pthread_event.reset();
         }

         // Stop the event processer
         if(pthread_handler)
         {
            // If we're not interested in flushing the event queue stop now!
            if(stop_immediately_)
            {
               ioservice_.stop(); // Stop!!!!!
            }

            // Wait for the event queue to be done
            spwork_.reset();
            pthread_handler->join();
         }
      }

   private:

      bool is_file_action(int mask, int eventid)
      {
         // I faculitate mask matching :)
         return ((mask & eventid) == eventid);
      }

      bool get_file_action(int mask, file_action & fa)
      {
         // Here we're mapping inotify masl values to our file action enum
         if(is_file_action(mask, IN_DELETE)) { fa = DELETED; return true; }
         if(is_file_action(mask, IN_DELETE_SELF)) { fa = DELETED; return true; }
         if(is_file_action(mask, IN_MODIFY)) { fa = CHANGED; return true; }
         if(is_file_action(mask, IN_MOVE_SELF)) { fa = DELETED; return true; }
         if(is_file_action(mask, IN_MOVED_FROM)) { fa = DELETED; return true; }

         return false;
      }

      void thread_proc()
      {
         size_t const EVENT_SIZE = ( sizeof (inotify_event) );
         size_t const BUF_LEN    = ( 1024 * ( EVENT_SIZE + 16 ) );
         std::vector<char> vbuf(BUF_LEN);

         struct pollfd pfd;
         int ret;

         pfd.fd = fd_;
         pfd.events = POLLIN;

         for(;;)
         {
            boost::this_thread::interruption_point(); // Thread interupt point

            // Every second poll() will timeout and give us a chance to interrupt
            pfd.revents = 0;
            ret = ::poll(&pfd, 1, 1000);

            boost::this_thread::interruption_point(); // Thread interupt point

            if (ret < 0)
            {
               // error - um, whoops! Ignored.
            }
            else if (!ret)
            {
               // timed out - we don't care. Ignored.
            }
            else if (pfd.revents & POLLIN)
            {
               // signaled - let's start reading the event sequence
               int elen = 0;
               int len = read (fd_, &vbuf[0], vbuf.size());

               if (len < 0)
               {
                  // Hmmm... the read returned an error state

                  if (errno == EINTR)
                  {
                     continue; // More is coming... go fetch
                  }
                  else
                  {
                     // Read error.  Ignored. Let's try and process what we can.
                  }
               }
               else if (!len)
               {
                  // buffer is to small, let's increase it by BUF_LEN - at most allow BUF_LEN * 10

                  if(vbuf.size() < (BUF_LEN * 10))
                  {
                     vbuf.resize(vbuf.size() + BUF_LEN);
                  }
                  else
                  {
                     // Ok, we have a bit of a problem... this really should never happen so ignored!
                  }
               }

               // Whilst there are events to process...
               while (len && elen < len)
               {
                  boost::this_thread::interruption_point(); // Therad interupt point

                  // Bytes to event object please
                  inotify_event *this_event = (inotify_event *) &vbuf[elen];
                  elen += EVENT_SIZE + this_event->len;

                  boost::shared_lock<boost::shared_mutex> sl(smtx_event_);
                  event_map_t::iterator itrEvent = event_map_.find(this_event->wd);

                  // These aren't the droids you're looking for - but is it an event we care about?
                  if(itrEvent != event_map_.end())
                  {
                     try
                     {
                        // Great, get the corresponding file action and hand the event to asio to fire
                        file_action fa;
                        if(get_file_action(this_event->mask, fa))
                        {
                           // new events add a handler to the event hander queue, which is processed by asio
                           ioservice_.post(
                              boost::bind(
                                    itrEvent->second.first, fa, itrEvent->second.second
                                 )
                              );
                        }
                     }
                     catch(...)
                     {
                        // swallow exceptions, we don't want our thread to die due to callback error
                     }
                  }
               }
            }
         }
      }

   private:
      int fd_;
      event_map_t event_map_;
      path_map_t path_map_;
      pthread_t pthread_event;
      boost::shared_mutex smtx_event_;
      boost::mutex mtx_stop_start_;
      boost::asio::io_service ioservice_;
      boost::shared_ptr<boost::asio::io_service::work> spwork_;
      pthread_t pthread_handler;
      bool stop_immediately_;
   };

}} // moost::io

#endif // MOOST_IO_NOTIFY_LINUX_HPP__
