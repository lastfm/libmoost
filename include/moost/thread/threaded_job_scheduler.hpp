/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       threaded_job_scheduler.hpp
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

#ifndef MOOST_THREAD_THREADED_JOB_SCHEDULER_HPP
#define MOOST_THREAD_THREADED_JOB_SCHEDULER_HPP

#include <cassert>
#include <queue>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>

#include "job_batch.hpp"
#include "worker_group.hpp"

namespace moost { namespace thread {

/**
 * A job batch that runs jobs in a set of worker threads
 *
 * This implementation of a job_batch runs jobs in a set of
 * worker threads provided by the threaded_job_scheduler class.
 */
class threaded_job_batch : public job_batch
                         , public boost::noncopyable
                         , public boost::enable_shared_from_this<threaded_job_batch>
{
public:
   threaded_job_batch()
      : m_todo(0)
      , m_count(0)
   {
   }

   virtual void add(job_t job)
   {
      {
         boost::mutex::scoped_lock lock(m_mx);
         m_jobs.push(job);
         ++m_todo;
      }

      m_cond.notify_one();
   }

   void run(worker_group& wg)
   {
      while (!done())
      {
         do_one(wg);
      }
   }

   /**
    * Get the number of jobs executed in this batch
    *
    * \returns The total number of jobs.
    */
   size_t count() const
   {
      boost::mutex::scoped_lock lock(m_mx);
      return m_count;
   }

   /**
    * Get errors that have been caught
    *
    * Get all errors that have been caught while running the jobs
    * in this batch.
    *
    * This method is only safe to call after all jobs in this batch
    * have completed. It will throw if there are any unprocessed jobs
    * in the batch.
    *
    * \returns A vector of error strings.
    */
   const std::vector<std::string>& errors() const
   {
      boost::mutex::scoped_lock lock(m_mx);

      if (m_todo > 0)
      {
         throw std::runtime_error("cannot call errors() when there are unfinished jobs");
      }

      return m_errors;
   }

private:
   typedef std::queue<job_t> jobs_t;

   /**
    * Enqueue one job to worker threads
    *
    * This method is supposed to be called by threaded_job_scheduler
    * only. It pops the next job off the job queue and enqueues it to
    * run in the next available worker thread.
    */
   void do_one(worker_group& wg)
   {
      job_t job;

      {
         boost::mutex::scoped_lock lock(m_mx);
         assert(!m_jobs.empty());
         job = m_jobs.front();
         m_jobs.pop();
      }

      wg.add_job(boost::bind(&threaded_job_batch::run_one, shared_from_this(), job));
   }

   /**
    * Check if all jobs are done
    *
    * This method is supposed to be called by threaded_job_scheduler
    * only. Does more or less what it says on the tin, though it'll
    * block if the job queue is empty. This is to ensure that new
    * jobs can still be added while other jobs are being run.
    */
   bool done() const
   {
      boost::mutex::scoped_lock lock(m_mx);

      while (m_todo > 0 && m_jobs.empty())
      {
         m_cond.wait(lock);
      }

      return m_todo == 0;
   }

   /**
    * Runs a single job
    *
    * This method runs in the various worker threads provided by
    * the scheduler.
    */
   void run_one(job_t& job)
   {
      try
      {
         job();  // JFDI! :)
      }
      catch (const std::exception& e)
      {
         boost::mutex::scoped_lock lock(m_mx_errors);
         m_errors.push_back(e.what());
      }
      catch (...)
      {
         boost::mutex::scoped_lock lock(m_mx_errors);
         m_errors.push_back("unknown exception caught");
      }

      {
         boost::mutex::scoped_lock lock(m_mx);
         --m_todo;
         ++m_count;
      }

      m_cond.notify_one();
   }

   jobs_t m_jobs;
   mutable boost::condition_variable m_cond;
   mutable boost::mutex m_mx;
   volatile size_t m_todo;
   volatile size_t m_count;
   std::vector<std::string> m_errors;
   mutable boost::mutex m_mx_errors;
};

/**
 * A scheduler to run job batches in multiple worker threads
 *
 * The scheduler mainly provides a set of worker threads, the
 * actual distribution of jobs is done by threaded_job_batch.
 */
class threaded_job_scheduler : public boost::noncopyable
{
public:
   typedef threaded_job_batch job_batch_type;

   /**
    * Create a threaded job scheduler
    *
    * \param num_workers     Number of worker threads used to run jobs.
    */
   explicit threaded_job_scheduler(size_t num_workers = 1)
      : m_workers(num_workers)
   {
   }

   /**
    * Dispatch a batch of jobs
    *
    * \param batch           The batch of jobs that need to be run.
    */
   void dispatch(boost::shared_ptr<job_batch_type> batch)
   {
      batch->run(m_workers);
   }

private:
   worker_group m_workers;
};

}}

#endif
