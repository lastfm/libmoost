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

/**
 * @file fixed_interval_timer.hpp
 * @brief Notify after a fixed period of time
 * @author Ricky Cormier
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-02-26
 */
#ifndef MOOST_FIXED_INTERVAL_TIMER_TIMER_HPP__
#define MOOST_FIXED_INTERVAL_TIMER_TIMER_HPP__

#include <csignal>
#include <utility>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/ref.hpp>

namespace moost { namespace utils {

   /**
    * @brief A fixed interval timer
    *
    * Notifies either via a callback function or future once a fixed interval
    * of time has passed. The frame of reference is taken from the time the
    * notification request is made.
    *
    * If the notification is to be via a callback function this function shall
    * be called once the interval period has been surpassed.
    *
    * If the notification is to be via a future the future shall be set once
    * the notification period has been surpassed. It should be noted that the
    * actual value of the set future is undefined. Once the notification period
    * has passed a called to "is_ready" or "has_value" shall return true.
    *
    * Since all notifications have to wait the same fixed period it should be
    * clear the the sequence of notification is non-anachronistici; they will
    * always trigger in the order they were registered. This property means we
    * can use a FIFO queue to store and process the notification requests.
    *
    * The astute reader will notice there may be a problem with this model. As
    * the callback is fired in the same thread as that processing the queue it
    * is imperative that each callback is lightweight and should not do any
    * thing other than trip a signal or invoke a seperate asynchronous task.
    * Failure to follow this criterian will lead to notification backlog.
    *
    * The reason d'entre of this timer is to provide a simple and low cost way
    * to generate fixed interval notifications. If you need something more
    * complex than than this is not the timer for you and you should consider
    * using a different timer (maybe the boost::asio::deadline_timer).
    *
    */
   class fixed_interval_timer
   {
      private:
         typedef boost::promise<int> promise_t;
         typedef boost::shared_ptr<promise_t> promise_ptr;
         typedef std::sig_atomic_t volatile sig_atomic_t;
         typedef boost::shared_ptr<sig_atomic_t> signal_ptr;

      public:
         typedef boost::function0<void> callback_t;
         typedef std::pair<boost::posix_time::ptime, callback_t> process_arg;

         /**
          * @brief Construct a new fixed interval timer
          *
          * @param interval : interval between queuing and calling a callback
          *
          */
         fixed_interval_timer(boost::posix_time::time_duration const & interval)
            : interval_(interval)
            , thread_(boost::bind(&boost::asio::io_service::run, &io_service_))
            , pwork_(new boost::asio::io_service::work(io_service_))
         {
         }

         /**
          * @brief Stop work and re-join the worker thread
          */
         ~fixed_interval_timer()
         {
            try
            {
               // right, down tools chaps
               pwork_.reset();
               thread_.join();
            }
            catch(...)
            {
               // oh, crap :(
            }
         }

         /**
          * @brief Register a callback for notification
          *
          * @param cb : this will be called when the interval passes
          */
         void notify(callback_t const & cb)
         {
            // notify me when this time has passed
            boost::posix_time::ptime pt =
               boost::posix_time::microsec_clock::universal_time() + interval_;

            // queue the notification request
            io_service_.post(boost::bind(
               &fixed_interval_timer::process, this,
               std::make_pair(pt, cb)));
         }

         /**
          * @brief This future is set when the notification interval has passed
          */
         typedef boost::unique_future<int> future_t;

         /**
          * @brief Register a future for notification
          *
          * @param f_sig : this is set when the interval passes
          */
         void notify(future_t & f_sig)
         {
            // signal me when this time has passed
            boost::posix_time::ptime pt =
               boost::posix_time::microsec_clock::universal_time() + interval_;

            // I promise to notify you in the future
            promise_ptr p_sig(new promise_t);
            f_sig = p_sig->get_future();

            // queue the notification request
            io_service_.post(boost::bind(
               &fixed_interval_timer::process, this,
               std::make_pair(pt, boost::bind(
                  &fixed_interval_timer::fulfil_promise, this, p_sig))
               ));
         }


         /**
          * @brief Represents an atomic signal
          */
         class signal_t
         {
            friend class fixed_interval_timer;

            public:
               signal_t()
                  : psig_(new sig_atomic_t(0)) {}

               bool is_ready() const
               {
                  return *psig_ == 1;
               }

            private:
               signal_ptr psig_;
         };

         /**
          * @brief Register a signal for notificatino
          *
          * @param sig : this is set when the interval passes
          */
         void notify(signal_t & sig)
         {
            // signal me when this time has passed
            boost::posix_time::ptime pt =
               boost::posix_time::microsec_clock::universal_time() + interval_;

            // queue the signal request
            io_service_.post(boost::bind(
               &fixed_interval_timer::process, this,
               std::make_pair(pt, boost::bind(
                  &fixed_interval_timer::set_signal, this, sig.psig_))
               ));
         }


      private:
         // process the current queue item
         void process(process_arg const & arg) const
         {
            // sleep until it's time to call the callback
            boost::thread::sleep(arg.first);

            // call the callback
            arg.second();
         }

         // make good on our promise
         void fulfil_promise(promise_ptr p_sig) const
         {
            // a promise should never be broken!
            p_sig->set_value(1);
         }

         // signal that sucker
         void set_signal(signal_ptr psig) const
         {
            *psig = 1;
         }

      private:
         boost::posix_time::time_duration interval_;
         boost::asio::io_service io_service_;
         boost::thread thread_;
         boost::shared_ptr<boost::asio::io_service::work> pwork_;
   };

}}

#endif
