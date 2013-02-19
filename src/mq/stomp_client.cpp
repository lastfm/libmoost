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

#include "../../include/moost/mq/stomp_client.h"
#include "stomp_client_impl.h"

namespace moost { namespace mq {

stomp_client::stomp_client(size_t consumer_pool_size,
                           const boost::posix_time::time_duration& keepalive_interval,
                           const boost::posix_time::time_duration& reconnect_interval)
   : m_impl(new impl(consumer_pool_size, keepalive_interval, reconnect_interval))
{
}

stomp_client::~stomp_client()
{
   try
   {
      disconnect();
   }
   catch (...)
   {
   }
}

void stomp_client::connect(const std::string& hostname, int port, boost::function<void (const boost::system::error_code&, const std::string&)> error_cb)
{
   m_impl->connect(hostname, port, error_cb);
}

void stomp_client::disconnect()
{
   if (is_connected())
   {
      m_impl->disconnect();
   }
}

void stomp_client::subscribe(const std::string& topic, boost::function<void (const std::string&)> message_cb, ack::type ack_type,
                             const boost::posix_time::time_duration& max_msg_interval)
{
   m_impl->subscribe(topic, message_cb, ack_type, max_msg_interval);
}

void stomp_client::unsubscribe(const std::string& topic)
{
   m_impl->unsubscribe(topic);
}

void stomp_client::send(const std::string& topic, const std::string& message)
{
   m_impl->send(topic, message);
}

bool stomp_client::is_connected() const
{
   return m_impl->is_connected();
}

bool stomp_client::is_online() const
{
   return m_impl->is_online();
}

uint64_t stomp_client::get_num_processed() const
{
   return m_impl->get_num_processed();
}

size_t stomp_client::get_num_pending() const
{
   return m_impl->get_num_pending();
}

}}
