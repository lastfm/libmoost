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

#ifndef MOOST_MQ_STREAM_H_
#define MOOST_MQ_STREAM_H_

#include <string>

#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "../../include/moost/mq/stomp_client.h"

namespace moost { namespace mq {

class stream
{
public:
   typedef boost::function<void (const std::string&)> message_cb_t;

   stream(const message_cb_t& cb, stomp_client::ack::type ack_type,
          const boost::posix_time::time_duration& max_msg_interval)
      : m_callback(cb)
      , m_last_invoke(boost::posix_time::microsec_clock::universal_time())
      , m_ack_type(ack_type)
      , m_max_msg_interval(max_msg_interval)
   {
   }

   void invoke(const std::string& message)
   {
      reset_interval_timer();
      m_callback(message);
   }

   void reset_interval_timer()
   {
      // we can save a call if we never need the value in m_last_invoke

      if (!m_max_msg_interval.is_pos_infinity())
      {
         m_last_invoke = boost::posix_time::microsec_clock::universal_time();
      }
   }

   stomp_client::ack::type ack_type() const
   {
      return m_ack_type;
   }

   const boost::posix_time::time_duration& max_msg_interval()
   {
      return m_max_msg_interval;
   }

   bool max_msg_interval_exceeded() const
   {
      if (m_max_msg_interval.is_pos_infinity())
      {
         return false;
      }

      const boost::posix_time::ptime& now = boost::posix_time::microsec_clock::universal_time();
      return now - m_last_invoke > m_max_msg_interval;
   }

private:
   message_cb_t m_callback;
   boost::posix_time::ptime m_last_invoke;

   const stomp_client::ack::type m_ack_type;
   const boost::posix_time::time_duration m_max_msg_interval;
};

}}

#endif
