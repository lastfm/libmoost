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

#ifndef MOOST_THREAD_ASYNC_BATCH_PROCESSOR
#define MOOST_THREAD_ASYNC_BATCH_PROCESSOR

#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "../utils/foreach.hpp"

#include "worker_group.hpp"

namespace moost { namespace thread {

/**
* @brief Allow a batch of jobs to be processed asyncronously whilst you wait
*
* There are times when you have a batch of data that needs to be processed and
* you want to process things asynchronously to maximise CPU usage.
*
* The premise behind how it works is pretty simple; you create a collection of
* jobs where a job is a functor loaded with your data and logic to process the
* data. You then dispatch threads to process the jobs. The jobs will then be
* processed asyncronously but the dispatcher will block until all the jobs are
* processed.
*
* It's like you are sending out a bunch of workers to dig a hole for you whilst
* you sit down and have a nice cup of tea :)
*
*/
class async_batch_processor
{
private:
   class batch_state
   {
   public:
      batch_state(size_t todo)
         : m_todo(todo)
      {
      }

      void one_done()
      {
         {
            boost::lock_guard<boost::mutex> lock(m_mx);
            --m_todo;
         }

         m_cond.notify_one();
      }

      void wait_for_all_done()
      {
         boost::unique_lock<boost::mutex> lock(m_mx);

         while(m_todo > 0) // when m_todo == 0 all jobs are done
         {
            // wait for workers to signal when they're done
            m_cond.wait(lock);
         }
      }

   private:
      volatile size_t m_todo;
      boost::mutex m_mx;
      boost::condition_variable m_cond;
   };

public:
   typedef boost::function0<void> job_t;
   typedef std::vector<job_t> jobs_t;

   /**
    * @brief Construct a new processor
    *
    * @param threadcnt
    *    the number of threads that will be used by the dispatcher
    */
   async_batch_processor(size_t num_threads)
      : m_wg(num_threads)
   {
   }

   /**
    * @brief Dispatch worker threads to process jobs
    *
    * The dispatcher will dispatch threads to process your jobs and then
    * wait until they are all done. Only then will it return to the caller.
    *
    * @param jobs
    *    A collection of jobs to be processed
    */
   void dispatch(jobs_t & jobs)
   {
      if (!jobs.empty())
      {
         boost::shared_ptr<batch_state> state(new batch_state(jobs.size()));

         // dispatch jobs
         foreach (job_t& job, jobs)
         {
            m_wg.add_job(boost::bind(&async_batch_processor::handler, boost::ref(job), state));
         }

         // and wait until all are done
         state->wait_for_all_done();
      }
   }

private:
   static void handler(job_t& job, boost::shared_ptr<batch_state> state)
   {
      // do job
      job();

      // and tell the state object we're done
      state->one_done();
   }

   worker_group m_wg;
};

}}

#endif // MOOST_THREAD_ASYNC_BATCH_PROCESSOR
