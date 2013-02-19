/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       worker_group.hpp
 * \author     Marcus Holland-Moritz (marcus@last.fm)
 * \copyright  Copyright © 2008-2013 Last.fm Limited
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

#ifndef MOOST_THREAD_WORKER_GROUP_HPP
#define MOOST_THREAD_WORKER_GROUP_HPP

#include <queue>
#include <csignal>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace moost { namespace thread {

/**
 * A group of worker threads
 *
 * This is an easy to use, multithreaded work dispatcher.
 * You can add jobs at any time and they will be dispatched
 * to the next available worker thread.
 */
class worker_group : public boost::noncopyable
{
public:
   typedef boost::function0<void> job_t;

   /**
    * Create a worker group
    *
    * \param num_workers     Number of worker threads.
    */
   explicit worker_group(size_t num_workers = 1)
      : m_running(1)
   {
      if (num_workers < 1)
      {
         throw std::runtime_error("invalid number of worker threads");
      }

      for (size_t i = 0; i < num_workers; ++i)
      {
         m_workers.create_thread(boost::bind(&worker_group::work, this));
      }
   }

   /**
    * Stop and destroy a worker group
    */
   ~worker_group()
   {
      try
      {
         stop();
      }
      catch (...)
      {
      }
   }

   /**
    * Stop a worker group
    */
   void stop()
   {
      if (m_running)
      {
         {
            boost::mutex::scoped_lock lock(m_mx);
            m_running = 0;
         }
         m_cond.notify_all();
         m_workers.join_all();
      }
   }

   /**
    * Check whether the worker group is still running
    */
   bool running() const
   {
      return m_running;
   }

   /**
    * Add a new job to the worker group
    *
    * The new job will be dispatched to the first available worker thread.
    *
    * \param job             The job to add to the dispatcher.
    */
   bool add_job(job_t job)
   {
      bool added = m_running != 0;

      if (added)
      {
         {
            boost::mutex::scoped_lock lock(m_mx);
            m_jobs.push(job);
         }

         m_cond.notify_one();
      }

      return added;
   }

   /**
    * Return the number of worker threads
    *
    * \returns The number of worker threads.
    */
   size_t size() const
   {
      return m_workers.size();
   }

   /**
    * Return the number of queued jobs
    *
    * \returns The number of queued jobs.
    */
   size_t queued_jobs() const
   {
      boost::mutex::scoped_lock lock(m_mx);
      return m_jobs.size();
   }

private:
   typedef std::queue<job_t> jobs_t;

   void work()
   {
      for (;;)
      {
         job_t job;

         {
            boost::mutex::scoped_lock lock(m_mx);

            while (m_jobs.empty() && m_running)
            {
               m_cond.wait(lock);
            }

            if (m_jobs.empty())
            {
               if (m_running)
                  continue;
               else
                  break;
            }

            job = m_jobs.front();
            m_jobs.pop();
         }

         job();
      }
   }

   boost::thread_group m_workers;
   jobs_t m_jobs;
   boost::condition_variable m_cond;
   mutable boost::mutex m_mx;
   volatile sig_atomic_t m_running;
};

}}

#endif
