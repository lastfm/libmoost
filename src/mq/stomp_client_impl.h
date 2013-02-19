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

#ifndef MOOST_MQ_STOMP_CLIENT_IMPL_H_
#define MOOST_MQ_STOMP_CLIENT_IMPL_H_

#include <string>
#include <csignal>
#include <map>
#include <csignal>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include "stream_manager.h"
#include "../../include/moost/mq/error.h"
#include "../../include/moost/mq/stomp_client.h"

namespace moost { namespace mq {

class stomp_client::impl : public boost::enable_shared_from_this<stomp_client::impl>
{
public:
   typedef boost::function<void (const boost::system::error_code&, const std::string&)> error_cb_t;

   impl(size_t consumer_pool_size,
        const boost::posix_time::time_duration& keepalive_interval,
        const boost::posix_time::time_duration& reconnect_interval);
   ~impl();

   void connect(const std::string& hostname, int port, error_cb_t error_cb);
   void disconnect();

   void subscribe(const std::string& topic, stream::message_cb_t message_cb, stomp_client::ack::type ack_type,
                  const boost::posix_time::time_duration& max_msg_interval);
   void unsubscribe(const std::string& topic);

   void send(const std::string& topic, const std::string& message);

   bool is_connected() const
   {
      return m_state != state::disconnected;
   }

   bool is_online() const
   {
      return m_state == state::online;
   }

   uint64_t get_num_processed() const
   {
      return m_streams.get_num_processed();
   }

   size_t get_num_pending() const
   {
      return m_streams.get_num_pending();
   }

private:
   typedef std::map<std::string, std::string> header_map;

   struct state
   {
      enum type
      {
         disconnected,
         connecting,
         online,
         reconnecting
      };
   };

   struct protocol
   {
      enum type
      {
         undefined,
         version10,
         version11
      };
   };

   boost::system::error_code connect();
   void reconnect();
   void subscribe(const std::string& topic, stomp_client::ack::type ack_type);

   boost::system::error_code send_to_queue(const std::string& command, const std::string& body = std::string());
   boost::system::error_code send_to_queue(const std::string& command, const header_map& headers, const std::string& body = std::string());
   void send_to_queue_async(const std::string& command, const std::string& body = std::string());
   void send_to_queue_async(const std::string& command, const header_map& headers, const std::string& body = std::string());
   void send_to_queue(const std::string& command, const header_map& headers, const std::string& body, boost::system::error_code *ec);

   void receive_from_queue(std::string& command, header_map& headers, std::string& body);

   void handle_keepalive(const boost::system::error_code& err);
   void handle_recv(const boost::system::error_code& err);
   void handle_reconnect(const boost::system::error_code& err);
   void handle_write(boost::shared_ptr<boost::asio::streambuf>, const boost::system::error_code& err);
   void handle_stomp_error(const header_map& headers, const std::string& body);
   void handle_dead_conn(const boost::system::error_code& err);

   void recv_more();
   void keepalive();
   void dead_conn_detect();

   void on_message(const header_map& headers, const std::string& msg);

   static boost::system::error_code make_error_code(error::type ec);

   const boost::posix_time::time_duration m_keepalive_interval;
   const boost::posix_time::time_duration m_reconnect_interval;

   std::string m_hostname;
   int m_port;
   error_cb_t m_error_cb;

   boost::asio::io_service m_ios;
   boost::asio::ip::tcp::socket m_socket;
   boost::asio::streambuf m_response;
   boost::asio::deadline_timer m_keepalive_timer;
   boost::asio::deadline_timer m_reconnect_timer;
   boost::asio::deadline_timer m_dead_conn_timer;
   boost::shared_ptr<boost::asio::io_service::work> m_ios_work;
   boost::thread m_ios_thread;

   stream_manager m_streams;

   volatile sig_atomic_t m_state;
   volatile sig_atomic_t m_proto;
};

}}

#endif
