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

#ifndef FM_LAST_MOOST_PROCESS_SLEEPER_H_
#define FM_LAST_MOOST_PROCESS_SLEEPER_H_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
 * \file sleeper.hpp
 *
 * moost::process::sleeper uses boost::asio to implement an interruptible sleeper, i.e.
 * code that sleeps until it is explicitly woken up. Creating a moost::process::sleeper
 * instance is cheap, as all expensive setup and object allocation only takes place if
 * you ever call the sleep() method. This method call will not return until awaken()
 * is called later on.
 */

namespace moost { namespace process {

class sleeper : public boost::noncopyable
{
private:
   // This helper class avoids creating all the io_service and deadline_timer cruft right away.
   // It might be a user of moost::process::sleeper doesn't even go beyond creating an instance.
   class impl : public boost::enable_shared_from_this<impl>, public boost::noncopyable
   {
   public:
      impl()
        : m_sleeping(false)
        , m_ios()
        , m_timer(m_ios)
      {
      }

      void sleep()
      {
         if (!m_sleeping)
         {
            m_sleeping = true;
            on_timer();
            m_ios.run();
         }
      }

      void awaken()
      {
         m_sleeping = false;
         m_timer.cancel();
      }

   private:
      void on_timer()
      {
         if (m_sleeping)
         {
            m_timer.expires_from_now(boost::posix_time::time_duration(boost::posix_time::pos_infin));
            m_timer.async_wait(boost::bind(&impl::on_timer, shared_from_this()));
         }
      }

      bool m_sleeping;
      boost::asio::io_service m_ios;
      boost::asio::deadline_timer m_timer;
   };

public:
   sleeper()
   {
   }

   void sleep()
   {
      if (!m_impl)
      {
         m_impl.reset(new impl());
      }

      m_impl->sleep();
   }

   void awaken()
   {
      if (m_impl)
      {
         m_impl->awaken();
      }
   }

private:
   boost::shared_ptr<impl> m_impl;
};

} }

#endif
