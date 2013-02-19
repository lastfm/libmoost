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

#ifndef MOOST_THREAD_ASYNC_WORKER_HPP__
#define MOOST_THREAD_ASYNC_WORKER_HPP__

#include <vector>
#include <stdexcept>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "xtime_util.hpp"

namespace moost { namespace thread {

/// @brief Thrown by async_worker when there is a timeout while trying to enqueue more work.
struct enqueue_timeout : public std::runtime_error
{
  enqueue_timeout() : std::runtime_error("enqueue timed out") {}
};

/** @brief async_worker is a virtual class that simplifies the mechanics of doing work asynchronously.
 *
 * To use it, you must inherit from it and provide a template parameter, then implement the do_work method.
 */
template<typename TWork>
class async_worker
{
private:

  std::vector< boost::shared_ptr<boost::thread> > m_pworker_threads;
  std::vector< boost::shared_ptr< TWork > >       m_work;
  size_t                                          m_max_queue;
  size_t                                          m_enqueue_timeout_ms;
  bool                                            m_working;

  boost::mutex                                    m_work_mutex;
  boost::condition                                m_work_to_do;
  boost::condition                                m_work_done;

  /// @brief Entry point for worker threads.
  void work_loop()
  {
    for (;;)
    {
      boost::shared_ptr< TWork > work;

      {
        boost::mutex::scoped_lock lock(m_work_mutex);

        while (m_work.empty())
        {
          if (!m_working)
            break;
          m_work_to_do.wait(lock);
        }
        if (m_work.empty()) // can only be true if we were told to stop and we have nothing left to do
          break;

        // inform anyone waiting to enqueue that a spot has freed up
        m_work_done.notify_one();
        work = m_work.front();
        m_work.erase(m_work.begin());
      }

      try
      {
        do_work(*work);
      }
      catch (const std::exception & e)
      {
        report_error(e);
      }
      catch (...)
      {
        report_error(std::runtime_error("async_worker: unknown exception in worker"));
      }
    }
  }

protected:

  /// @brief Override this method and do your work here.
  virtual void do_work(TWork & work) = 0;

  /// @brief Optionally override this method for custom error logging.
  virtual void report_error(const std::exception &) {}

public:

  /** @brief Constructs an async_worker.
   * @param num_threads the number of worker threads to launch
   * @param max_queue the maximum length the queue may grow before further enqueue's begin to wait (0 means allow infinite queue lengths)
   * @param enqueue_timeout the longest amount of time (ms) an enqueue may wait (0 means wait forever)
   */
  async_worker(size_t num_threads = 1,
               size_t    max_queue = 0,
               size_t    enqueue_timeout_ms = 0)
  : m_pworker_threads(num_threads),
    m_max_queue(max_queue),
    m_enqueue_timeout_ms(enqueue_timeout_ms),
    m_working(false)
  {
    start();
  }

  /**
   * destroys async_worker
   *
   * note: it's unsafe to destroy an async worker that's running!
   */
  virtual ~async_worker()
  {
    // it would be a bug to put stop() here and allow the user the assume async_workers are safe to destroy
    // without stopping.  what happens is the implementing class destroys first, and if you're unlucky,
    // the implementing class's do_work has now disappeared but the worker thread is still running
    // very bad things happen as a result
    // if you want this behavior, put stop() into the implementing class' dtor
  }

  /**
   * @brief Enqueues some work, and wakes up worker threads as necessary.
   * @param work is the work to be done
   */
  void enqueue(const TWork & work)
  {
    boost::mutex::scoped_lock lock(m_work_mutex);

    if (!m_working)
      throw std::runtime_error("can't enqueue when not working");

    while (m_max_queue > 0 && m_work.size() == static_cast<size_t>(m_max_queue))
    {
      // too much work to do!
      if (m_enqueue_timeout_ms == 0)
        m_work_done.wait(lock);
      else
      {
        if (!m_work_done.timed_wait(lock, xtime_util::add_ms(xtime_util::now(), m_enqueue_timeout_ms)))
          throw enqueue_timeout();
      }
    }

    m_work.push_back(boost::shared_ptr<TWork>(new TWork(work)));

    // wake up, you got work to do!
    // notice we notify even if queue wasn't empty
    // this way we ensure we're running as many
    // parallel workers as possible
    // TODO: some people say it's less efficient to notify while you still own the lock
    // i don't believe this, but it might be worth testing
    m_work_to_do.notify_one();
  }

  // @brief starts all worker threads
  void start()
  {
    {
      boost::mutex::scoped_lock lock(m_work_mutex);
      if (m_working)
        return;
      m_working = true;
    }
    for (std::vector< boost::shared_ptr<boost::thread> >::iterator it = m_pworker_threads.begin(); it != m_pworker_threads.end(); ++it)
      it->reset(new boost::thread(boost::bind(&async_worker::work_loop, this)));
  }

  // @brief stops all worker threads, and waits for them to finish all enqueued work
  void stop()
  {
    {
      boost::mutex::scoped_lock lock(m_work_mutex);
      if (!m_working)
        return;
      m_working = false;
    }
    m_work_to_do.notify_all();
    for (std::vector< boost::shared_ptr<boost::thread> >::iterator it = m_pworker_threads.begin(); it != m_pworker_threads.end(); ++it)
      (*it)->join();
  }

};

}} // moost::thread

#endif // MOOST_THREAD_ASYNC_WORKER_HPP__
