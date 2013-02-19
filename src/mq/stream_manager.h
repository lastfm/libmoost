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

#ifndef MOOST_MQ_STREAM_MANAGER_H_
#define MOOST_MQ_STREAM_MANAGER_H_

#include <deque>
#include <string>
#include <csignal>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "stream.hpp"

namespace moost { namespace mq {

class stream_manager
{
public:
   typedef boost::shared_ptr<stream> stream_ptr;
   typedef std::pair<std::string, stream_ptr> topic_stream_pair;

   stream_manager(size_t consumer_pool_size);
   ~stream_manager();

   bool insert(const std::string& topic, stream::message_cb_t message_cb, stomp_client::ack::type ack_type,
               const boost::posix_time::time_duration& max_msg_interval);
   bool erase(const std::string& topic);
   void clear();

   void get_list(std::vector<topic_stream_pair>& topics) const;

   bool push_message(const std::string& topic, const std::string& message, stomp_client::ack::type& ack_type);

   uint64_t get_num_processed() const
   {
      boost::mutex::scoped_lock lock(m_mx_num_processed);
      return m_num_processed;
   }

   size_t get_num_pending() const
   {
      boost::mutex::scoped_lock lock(m_mx_messages_list);
      return m_messages_list.size();
   }

   bool max_msg_interval_exceeded() const;

private:
   typedef std::map<std::string, stream_ptr> stream_map;
   typedef std::pair<stream_ptr, std::string> stream_message_pair;

   void consumer_thread();

   stream_map m_streams;
   mutable boost::mutex m_mx_streams;

   volatile uint64_t m_num_processed;
   mutable boost::mutex m_mx_num_processed;

   mutable boost::mutex m_mx_messages_list;
   boost::condition_variable m_cond_messages_list;
   std::deque<stream_message_pair> m_messages_list;

   boost::thread_group m_consumer_threads;

   volatile sig_atomic_t m_running;
};

}}

#endif
