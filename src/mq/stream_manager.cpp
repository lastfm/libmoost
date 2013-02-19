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

#include "stream_manager.h"

namespace moost { namespace mq {

stream_manager::stream_manager(size_t consumer_pool_size)
   : m_running(1)
{
   for (size_t i = 0; i < consumer_pool_size; ++i)
   {
      m_consumer_threads.create_thread(boost::bind(&stream_manager::consumer_thread, this));
   }
}

stream_manager::~stream_manager()
{
   try
   {
      m_running = 0;
      m_cond_messages_list.notify_all();
      m_consumer_threads.join_all();
   }
   catch (...)
   {
   }
}

bool stream_manager::insert(const std::string& topic, stream::message_cb_t message_cb, stomp_client::ack::type ack_type,
                            const boost::posix_time::time_duration& max_msg_interval)
{
   boost::mutex::scoped_lock lock(m_mx_streams);

   if (m_streams.count(topic))
   {
      return false;
   }

   m_streams[topic].reset(new stream(message_cb, ack_type, max_msg_interval));

   return true;
}

bool stream_manager::erase(const std::string& topic)
{
   boost::mutex::scoped_lock lock(m_mx_streams);

   stream_map::iterator it = m_streams.find(topic);

   if (it == m_streams.end())
   {
      return false;
   }

   m_streams.erase(it);

   return true;
}

void stream_manager::clear()
{
   boost::mutex::scoped_lock lock(m_mx_streams);
   m_streams.clear();
}

void stream_manager::get_list(std::vector<topic_stream_pair>& topics) const
{
   boost::mutex::scoped_lock lock(m_mx_streams);
   std::copy(m_streams.begin(), m_streams.end(), std::back_inserter(topics));
}

bool stream_manager::push_message(const std::string& topic, const std::string& message, stomp_client::ack::type& ack_type)
{
   {
      boost::mutex::scoped_lock lock(m_mx_num_processed);
      ++m_num_processed;
   }

   stream_ptr sp;

   {
      boost::mutex::scoped_lock lock(m_mx_streams);
      stream_map::const_iterator it = m_streams.find(topic);

      if (it != m_streams.end())
      {
         sp = it->second;
      }
      else
      {
         return false;
      }
   }

   {
      boost::mutex::scoped_lock lock(m_mx_messages_list);
      m_messages_list.push_back(std::make_pair(sp, message));
   }

   m_cond_messages_list.notify_one();

   ack_type = sp->ack_type();

   return true;
}

void stream_manager::consumer_thread()
{
   while (m_running)
   {
      stream_message_pair smp;

      {
         boost::mutex::scoped_lock lock(m_mx_messages_list);

         if (m_messages_list.empty())
         {
            m_cond_messages_list.wait(lock);
         }

         if (m_messages_list.empty())
         {
            continue;
         }

         smp = m_messages_list.front();
         m_messages_list.pop_front();
      }

      smp.first->invoke(smp.second);
   }
}

bool stream_manager::max_msg_interval_exceeded() const
{
   boost::mutex::scoped_lock lock(m_mx_streams);

   for (stream_map::const_iterator it = m_streams.begin(); it != m_streams.end(); ++it)
   {
      if (it->second->max_msg_interval_exceeded())
      {
         return true;
      }
   }

   return false;
}

}}
