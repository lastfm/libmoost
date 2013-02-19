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

#ifndef MOOST_IO_FILE_WATCHER_HPP__
#define MOOST_IO_FILE_WATCHER_HPP__

#include <map>
#include <string>
#include <limits>
#include <functional>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

// for last_write_time and exists
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include "../thread/xtime_util.hpp"

/**
 * \namespace moost::io
 * \brief IO-related routines used commonly everywhere
 */
namespace moost { namespace io {

/**
 * \brief Asynchronously watches a path for changes, then notifies a callback function
 *        when a change occurs.
 *
 * file_watcher will notify you if a path is created, changed, or deleted.
 *
 * After a file_watcher is instantiated, you must call file_watcher::start() to
 * start up the asynchronous thread.
 */
class file_watcher
{
public:

  /// The actions that file_watcher can monitor
  enum file_action
  {
    CREATED = 0, ///< File creation
    CHANGED = 1, ///< File modification
    DELETED = 2  ///< File deletion
  };

  typedef boost::function<void(file_action action, const std::string & path)> callback_t;

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
  std::map<std::string, std::pair<time_t, time_t> > m_file_modified;

  /** \brief Mutex to make modifications to file_watcher::m_file_callback and
   *  file_watcher::m_file_modified thread-safe */
  boost::mutex m_file_mutex;

  /// Shared pointer to the thread that is constantly checking the files for modifications
  boost::shared_ptr< boost::thread> m_pthread;

  // m_run is thread-safe: it's mutex/conditioned to coordinate with the async watcher thread
  // when false, and the cond is signalled, the watcher thread will wake up and exit

  /// Whether the watcher thread is running
  bool m_run;
  /// Mutex that must be locked when we are changing file_watcher::m_run
  /** This is because file_watcher::m_run can be modified from the watcher thread
   * itself and also from the thread that originally constructed this
   * file_watcher.
   */
  boost::mutex m_run_mutex;
  /// Condition to notify threads blocking on the file_watcher watcher thread
  boost::condition m_run_cond;

  int m_sleep_ms;

  /// we cannot assume that if a file exists, it will exist in the following moment that we check its file size
  std::time_t last_write_time(const boost::filesystem::path & p)
  {
    try
    {
       if (!boost::filesystem::exists(p))
          return 0;
       return boost::filesystem::last_write_time(p);
    }
    catch (const boost::filesystem::filesystem_error &)
    {
       return 0; // we were very unlucky!
    }
    catch (const std::exception&)
    {
       return 0; // todo: handle differently
    }
  }

public:

  /** Constructs an empty file_watcher that is not running and not watching anything
   * @param sleep_ms: the number of milliseconds to sleep in between checking files
   */
  file_watcher(int sleep_ms = 500) :
    m_run(false), m_sleep_ms(sleep_ms)
  {
  }

  /// Destructor.
  /**
   * This method takes care of properly setting \c file_watcher::m_run to \c false using
   * the file_watcher::m_run_mutex mutex, and it also waits until the watcher
   * thread exits.
   */
  ~file_watcher()
  {
    stop();
  }

  /// Adds \a path to the list of monitored paths.
  /** \param  path      the path to monitor
   *  \param  callback  the callback to notify if the path changes
   *  \param  call_now  will invoke the callback now
   */
  void insert(const std::string & path,
              const callback_t & callback,
              bool call_now = false )
  {
    boost::mutex::scoped_lock lock(m_file_mutex);

    m_file_callback[path] = callback;

    boost::filesystem::path p(path);
    std::time_t lw = last_write_time(p);
    if (lw != 0)
    {
      m_file_modified[path] = std::make_pair(lw, lw);
      if ( call_now )
        callback(CHANGED, path);
    }
    else
    {
      m_file_modified.erase(path);
      if ( call_now )
        callback(DELETED, path);
    }
  }

  /// Removes \a path from the list of monitored paths.
  /**
   * \param  path   the path to stop monitoring
   */
  void erase(const std::string & path)
  {
    boost::mutex::scoped_lock lock(m_file_mutex);
    m_file_callback.erase(path);
    m_file_modified.erase(path);
  }

  /// Starts the asynchronous monitor thread
  /**
   * \throw  std::invalid_argument  if the thread is already started
   */
  void start()
  {
    boost::mutex::scoped_lock lock(m_run_mutex);
    if (m_run)
      return;
    m_run = true;
    m_pthread.reset(new boost::thread(boost::bind(&file_watcher::run, this)));
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
    m_pthread->join();
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
          boost::filesystem::path p(it->first);

          // Does the path exist?
          std::time_t lw = last_write_time(p);
          if (lw != 0)
          {
            // Check its last modification time and compare it with what we had earlier
            std::map< std::string, std::pair<time_t, time_t> >::iterator it_mod = m_file_modified.find(it->first);

            if (it_mod == m_file_modified.end())
            {
              // We haven't seen this file so far, so insert it into the
              // map and add a creation event that will be fired
              m_file_modified[it->first] = std::make_pair(lw, lw);
              notifications.push_back(std::make_pair(it->second, std::make_pair(CREATED, it->first)));
            }
            else
            {
              // only case we consider a real modification: prev prev mod != prev mod,
              // but this mod == prev mod
              // the idea is that we want to capture a write to a file,
              // but only notify when the write is finished

              /**
               * \todo This could cause problems with frequent writing.
               *       We should really use boost.interprocess file locking
               *       instead (when we get 1.36 everywhere)
               */
              if (lw == it_mod->second.second && it_mod->second.first != it_mod->second.second)
                notifications.push_back(std::make_pair(it->second, std::make_pair(CHANGED, it->first)));
              it_mod->second.first = it_mod->second.second;
              it_mod->second.second = lw;
            }
          }
          else
          {
            // The path does not exist. Did we have it before? If so, fire
            // a deletion event.
            std::map< std::string, std::pair<time_t, time_t> >::iterator it_mod = m_file_modified.find(it->first);
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

#endif // MOOST_IO_FILE_WATCHER_HPP__
