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

#ifndef MOOST_TRANSACTION_HANDLER_HPP__
#define MOOST_TRANSACTION_HANDLER_HPP__

// A very simple transaction handler, which queues jobs and attempts to commit
// them using a user defined commit functor. If this functor returns false or
// throws an exception the commit is deemed to have failed so the job is added
// back to the job queue and retried later.

// Tip: For a persisted queue take a look a moost/transaction/queue.hpp

#include <csignal>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace moost { namespace transaction {

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Async transaction handler
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// queueT has the following interface requirements:
//
//   Functions...
//      void push_back(value_type const &);
//      void pop_front();
//      value_type & front();
//      size_t size() const;
//      bool empty() const;
//
//   Typedefs...
//      value_type
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

template <
   typename queueT, // See above for interface requirements
   typename commitFunctorT
   >
class TransactionHandler : boost::noncopyable
{
public:
   TransactionHandler(queueT & queue, commitFunctorT commitFunctor) :
      m_sigRunning(0),m_queue(queue), m_commitFunctor(commitFunctor)
   {
      size_t qsize = m_queue.size();

      ///////////////////////////////////////////////////////////////////////////
      // Start transaction service
      m_spWork.reset(new boost::asio::io_service::work(m_ioService));
      m_spThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &m_ioService)));
      ///////////////////////////////////////////////////////////////////////////

      m_sigRunning = 1;

      // Post jobs for currently queued items
      postJob(qsize);
   }

   ~TransactionHandler()
   {
      // Stop backup service
      //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      // The service will try to flush all items in the queue and will requeue
      // any that fail; however, it will only try and flush what was in the queue
      // when it was signalled to stop, failed items put to the back of the queue
      // will not be retried, so they'll be lost unless queue is fully persisted.

      m_sigRunning = 0;
      m_spWork.reset();
      m_spThread->join();
   }

   typedef typename queueT::value_type value_type;

   void post(value_type const data)
   {
      // Add data item to the queue
      {
         unique_lock_t rl(m_mtx);
         m_queue.push_back(data);
      }

      postJob();
    }

private:
   void postJob(size_t postCnt = 1)
   {
      // Tell asio we have jobs to process as long as the service is running
      for(size_t post = 0 ; m_sigRunning && (post < postCnt) ; ++post)
      {
         m_ioService.post(
            boost::bind(&TransactionHandler<queueT, commitFunctorT>::CommitHandler, this)
         );
      }
   }

   void CommitHandler()
   {
      // TODO: Should this drain the queue? Currently it'll process one
      //       item at a time, relying on the number of asio posts to
      //       be in sync with the number of queued items. Would forcing
      //       the full queue to be processed each post be better? It does
      //       mean the remaining asio posts will be noops, potentially,
      //       but it also means the queue will get fully processed.
      bool bCommited = false;

      // Take a copy of the data item so we can unlock mutex asap
      value_type data;
      {
         read_lock_t l(m_mtx);
         data = m_queue.front();
      }

      try
      {
         // Attempt commit
         bCommited = m_commitFunctor(data);
      }
      catch(...) { }

      // If there was an issue during the commit requeue
      if(!bCommited) { post(data); }

      // Remove current item from the front
      {
         unique_lock_t rl(m_mtx);
         m_queue.pop_front();
      }
   }

private:
   std::sig_atomic_t volatile m_sigRunning;
   queueT & m_queue;
   commitFunctorT m_commitFunctor;

   boost::asio::io_service m_ioService;
   boost::shared_ptr<boost::asio::io_service::work> m_spWork;
   boost::shared_ptr<boost::thread> m_spThread;

   typedef boost::shared_lock<boost::shared_mutex> read_lock_t;
   typedef boost::upgrade_lock<boost::shared_mutex> upgrade_lock_t;
   typedef boost::upgrade_to_unique_lock<boost::shared_mutex> upgrade_to_unique_lock;
   typedef boost::unique_lock<boost::shared_mutex> unique_lock_t;
   boost::shared_mutex m_mtx;
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

}}

#endif // MOOST_TRANSACTION_HANDLER_HPP__
