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

#ifndef MOOST_REMOTE_WATCHER_HPP__
#define MOOST_REMOTE_WATCHER_HPP__

#include <map>
#include <string>
#include <sstream>
#include <limits>
#include <functional>

#include <boost/asio.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/functional/hash.hpp>

#include "../thread/xtime_util.hpp"

/**
 * \namespace moost::io
 * \brief IO-related routines used commonly everywhere
 */
namespace moost { namespace io {

/**
 * \brief Asynchronously watches a remote file, then notifies a callback function
 *        when a change occurs.
 *
 * After a remote_watcher is instantiated, you must call remote_watcher::start() to
 * start up the asynchronous thread.
 */
class remote_watcher
{
public:

  /// The actions that remote_watcher can monitor
  enum file_action
  {
     CREATED = 0, ///< File creation
     CHANGED = 1, ///< File modification
     DELETED = 2
  };

  typedef boost::function<void(file_action action, const std::string & url)> callback_t;

private:

  /// The map that associates watched files to the callbacks that should be launched
  std::map<std::string, callback_t > m_file_callback;
  /// The map that associates watched files with their last known modification times
  /** We actually track the last \em two modification times. Ideally, we want
   * to send a notification only when the file was modified \em and the modifying
   * process already closed the file, therefore we fire a \c CHANGED event only
   * if the previous previous modification time is \em different from the
   * previous modification time \em but the previous modification time equals
   * the modification time we read from the file. Otherwise we just "shift"
   * the pair (drop the previous previous modification, move the previous
   * to the first position that is empty and push the file's last modification
   * obtained from the filesystem to the second element of the pair).
   */
  std::map<std::string, std::string > m_file_modified;

  /** \brief Mutex to make modifications to remote_watcher::m_file_callback and
   *  remote_watcher::m_file_modified thread-safe */
  boost::mutex m_file_mutex;

  /// Shared pointer to the thread that is constantly checking the files for modifications
  boost::shared_ptr< boost::thread> m_pthread;

  // m_run is thread-safe: it's mutex/conditioned to coordinate with the async watcher thread
  // when false, and the cond is signalled, the watcher thread will wake up and exit

  /// Whether the watcher thread is running
  bool m_run;
  /// Mutex that must be locked when we are changing remote_watcher::m_run
  /** This is because remote_watcher::m_run can be modified from the watcher thread
   * itself and also from the thread that originally constructed this
   * remote_watcher.
   */
  boost::mutex m_run_mutex;
  /// Condition to notify threads blocking on the remote_watcher watcher thread
  boost::condition m_run_cond;

  int m_sleep_ms;
  int m_timeout_ms;

  std::time_t get_time()
  {
     std::time_t ltime;
     time(&ltime);
     return ltime;
  }

  /// we cannot assume that if a file exists, it will exist in the following moment that we check its file size
  std::string last_write_time(const std::string & url)
  {
     using boost::asio::ip::tcp;
     tcp::iostream s;
     if ( !connect(s, url, m_timeout_ms) )
        return "";

     std::string response_line;
     size_t pos;
     const std::string lastModified = "Last-Modified: ";
     const std::string contentLength = "Content-Length: ";
     std::string returned;

     for ( int i = 0; !s.eof(); ++i )
     {
        std::getline(s, response_line);

        if ( s.bad() )
          throw std::runtime_error("Bad stream! (Timeout?)");

        boost::algorithm::trim(response_line);

        if ( response_line.empty() )
          break;

        if ( i == 0 && response_line != "HTTP/1.0 200 OK" )
          return ""; // not found!

        pos = response_line.find(lastModified);
        if ( pos != std::string::npos )
          return response_line.substr(lastModified.size()); // ah: last modified is the best case
        pos = response_line.find(contentLength);
        if ( pos != std::string::npos )
          returned = response_line.substr(contentLength.size());
     }

     if ( returned.empty() && s.good() )
     {
        // ah nothing found. Let's parse the entire page and hash it!
        size_t hash = 0;
        for ( int i = 0; !s.eof(); ++i )
        {
           std::getline(s, response_line);
           if ( s.bad() )
              throw std::runtime_error("Bad stream! (Timeout?)");
           boost::algorithm::trim(response_line);
           boost::hash_combine(hash, response_line);
        }

        std::ostringstream oss;
        oss << hash;
        returned = oss.str();
     }

     return returned;
  }

public:

  /** Constructs an empty remote_watcher that is not running and not watching anything
   * @param sleep_ms: the number of milliseconds to sleep in between checking files
   */
  remote_watcher(int sleep_ms = 10000, int timeout_ms = 2000) :
    m_run(false), m_sleep_ms(sleep_ms), m_timeout_ms(timeout_ms)
  {
  }

  /// Destructor.
  /**
   * This method takes care of properly setting \c remote_watcher::m_run to \c false using
   * the remote_watcher::m_run_mutex mutex, and it also waits until the watcher
   * thread exits.
   */
  ~remote_watcher()
  {
    stop();
  }

  /// Adds \a path to the list of monitored paths.
  /** \param  url     the url to monitor
   *  \param  callback  the callback to notify if the file at url changes
   */
  void insert( const std::string & url,
               const callback_t & callback)
  {
    boost::mutex::scoped_lock lock(m_file_mutex);
    m_file_callback[url] = callback;

    //boost::filesystem::path p(path);
    std::string lw = last_write_time(url);
    if ( !lw.empty() )
      m_file_modified[url] = lw;
    else
      m_file_modified.erase(url);
  }

  /// clear the whole list
  void clear()
  {
     this->stop();
     boost::mutex::scoped_lock lock(m_file_mutex);
     m_file_modified.clear();
     m_file_callback.clear();
  }

  /// Removes \a path from the list of monitored paths.
  /**
   * \param  path   the path to stop monitoring
   */
  void erase(const std::string & url)
  {
    boost::mutex::scoped_lock lock(m_file_mutex);
    m_file_callback.erase(url);
    m_file_modified.erase(url);
  }

  /// Starts the asynchronous monitor thread.
  /**
   * \throw  std::invalid_argument  if the thread is already started
   * \important start will \b NOT call the callback function straight away!!!
   */
  void start()
  {
    boost::mutex::scoped_lock lock(m_run_mutex);
    if (m_run)
      return;
    m_run = true;
    m_pthread.reset(new boost::thread(boost::bind(&remote_watcher::run, this)));
  }

  /// Stops the asynchronous monitor thread
  /**
   * \throw  std::invalid_argument  if the thread is already stopped
   */
  void stop()
  {
    {
      boost::mutex::scoped_lock lock(m_run_mutex);
      if (!m_run)
        return;
      m_run = false;
      // Notify the asynchronous monitor thread that it should wake up
      // if it's currently waiting on m_run_mutex
      m_run_cond.notify_one();
    }
    // can only join once we've released the mutex, so run() loop can finish up
    if ( m_pthread )
      m_pthread->join();
  }

  /// Connects to the given url. Will put the connection in the passed stream
  /**
  * \return false if the connection did not succeed
  */
  static bool connect( boost::asio::ip::tcp::iostream& s, std::string url,
                       int timeout_ms,  bool skip_header = false )
  {
     std::string protocol = "http";
     std::string protocolUrl = protocol + "://";

     std::string host;
     std::string page = "/";

     if ( url.size() > protocolUrl.size() &&
          url.substr(0, protocolUrl.size()) == protocolUrl )
     {
        url = url.substr(protocolUrl.size());
     }

     size_t posCol = url.find(':');
     size_t posPage = url.find('/');
     if ( posCol != std::string::npos && posCol != 0 && ++posCol < url.size())
     {
        host = url.substr(0, posCol-1);
        if ( posPage == std::string::npos )
        {
           protocol = url.substr(posCol); // i.e. http://www.myHost.com:8080
        }
        else
        {
           protocol = url.substr(posCol, posPage-(posCol)); // i.e. http://www.myHost.com:8080/page
           page = url.substr(posPage);
        }
     }
     else if ( posPage != std::string::npos && posPage != 0)
     {
        host = url.substr(0, posPage); // i.e. http://www.myHost.com/page
        page = url.substr(posPage);
     }
     else
        host = url; // i.e. http://www.myHost.com

     s.connect(host, protocol);
     if ( !s )
        return false;

     struct timeval r = { (int)(timeout_ms/1000),
                          (int)((timeout_ms%1000)*1000) };

     // native!
     setsockopt(s.rdbuf()->native(), SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&r), sizeof(r));
     setsockopt(s.rdbuf()->native(), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&r), sizeof(r));

     s << "GET " << page << " HTTP/1.0\r\n"
       << "Host: " << host << "\r\n"
       << "\r\n"
       << std::flush;

     if ( !s )
        return false;

     if ( skip_header )
     {
        std::string response_line;
        for ( int i = 0; !s.eof(); ++i )
        {
           std::getline(s, response_line);
           boost::algorithm::trim(response_line);
           if ( response_line.empty() )
              break;
        }
     }

     return true;
  }

private:

  /// Entry point for the asynchronous monitor thread
  void run()
  {
    bool run = true;
    typedef std::pair< callback_t , std::pair< file_action, std::string> > notification;
    std::vector< notification> notifications;

    for (;;)
    {
      {
        // Lock and check m_run; if it was set to false, we must return ASAP
        boost::mutex::scoped_lock lock(m_run_mutex);
        run = m_run;
        if (!run)
          return;
        // We release the lock, block the thread until one second
        m_run_cond.timed_wait(lock, moost::thread::xtime_util::add_ms(moost::thread::xtime_util::now(), m_sleep_ms));
      }
      // If we are not running (e.g. when the destructor woke us up), return
      if (!run)
        return;

      // Clear the notifications vector where we will collect the events
      // that will be fired
      notifications.clear();
      {
        // Lock m_file_mutex while we are working on m_file_callback
        boost::mutex::scoped_lock lock(m_file_mutex);
        for (std::map<std::string, callback_t>::iterator it = m_file_callback.begin(); it != m_file_callback.end(); ++it)
        {
          // Does the path exist?
          std::string lw = last_write_time(it->first);
          if ( !lw.empty() )
          {
            // Check its last modification time and compare it with what we had earlier
            std::map< std::string, std::string >::iterator it_mod = m_file_modified.find(it->first);

            if (it_mod == m_file_modified.end())
            {
              // We haven't seen this file so far, so insert it into the
              // map and add a creation event that will be fired
              m_file_modified[it->first] = lw;
              notifications.push_back(std::make_pair(it->second, std::make_pair(CREATED, it->first)));
            }
            else
            {
              if (lw != it_mod->second)
                notifications.push_back(std::make_pair(it->second, std::make_pair(CHANGED, it->first)));
              it_mod->second = lw;
            }
          }
          else
          {
            // The path does not exist. Did we have it before? If so, fire
            // a deletion event.
            std::map< std::string, std::string >::iterator it_mod = m_file_modified.find(it->first);
            if (it_mod != m_file_modified.end())
            {
              m_file_modified.erase(it_mod);
              notifications.push_back(std::make_pair(it->second, std::make_pair(DELETED, it->first)));
            }
          }
        }
      }

      // okay!  we've released our lock on m_file_callback and m_file_modified
      // so it's time to send off our notifications
      for (std::vector<notification>::iterator it = notifications.begin(); it != notifications.end(); ++it)
      {
        try
        {
          it->first(it->second.first, it->second.second);
        }
        catch (...)
        {
          // \todo  can we do better here than silently ignoring the exception?
        }
      }
    }
  }
};

}} // moost::io

#endif // MOOST_IO_REMOTE_WATCHER_HPP__
