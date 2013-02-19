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

#include <algorithm>

#include <boost/date_time/posix_time/time_formatters.hpp>

#include "../../include/moost/logging.hpp"
#include "../../include/moost/utils/stringify.hpp"

#include "error_category.h"
#include "stomp_client_impl.h"

namespace moost { namespace mq {

boost::system::error_code stomp_client::impl::make_error_code(error::type ec)
{
   return boost::system::error_code(ec, mq_error_category());
}

stomp_client::impl::impl(size_t consumer_pool_size,
                         const boost::posix_time::time_duration& keepalive_interval,
                         const boost::posix_time::time_duration& reconnect_interval)
   : m_keepalive_interval(keepalive_interval)
   , m_reconnect_interval(reconnect_interval)
   , m_socket(m_ios)
   , m_keepalive_timer(m_ios)
   , m_reconnect_timer(m_ios)
   , m_dead_conn_timer(m_ios)
   , m_ios_work(new boost::asio::io_service::work(m_ios))
   , m_ios_thread(boost::bind(&boost::asio::io_service::run, &m_ios))
   , m_streams(consumer_pool_size)
   , m_state(state::disconnected)
   , m_proto(protocol::undefined)
{
}

stomp_client::impl::~impl()
{
   try
   {
      m_ios_work.reset();
      m_ios_thread.join();
   }
   catch (...)
   {
   }
}

void stomp_client::impl::connect(const std::string& hostname, int port, error_cb_t error_cb)
{
   if (is_connected())
   {
      throw std::runtime_error("already connected to " + m_hostname + ":" + boost::lexical_cast<std::string>(m_port));
   }

   m_hostname = hostname;
   m_port = port;
   m_error_cb = error_cb;
   m_state = state::connecting;
   reconnect();
}

boost::system::error_code stomp_client::impl::connect()
{
   using namespace boost::asio::ip;

   boost::system::error_code error;

   tcp::resolver resolver(m_ios);
   tcp::resolver::query query(m_hostname, boost::lexical_cast<std::string>(m_port));
   tcp::resolver::iterator it = resolver.resolve(query, error);
   tcp::resolver::iterator end;

   if (!error)
   {
      while (it != end)
      {
         m_socket.connect(*it++, error);

         if (!error)
         {
            break;
         }
      }

      if (!error)
      {
         m_socket.set_option(tcp::acceptor::keep_alive(true));

         header_map headers;
         headers["accept-version"] = "1.0,1.1";
         headers["host"] = m_hostname;

         MLOG_CLASS_DEBUG("connecting to stomp queue at " << m_hostname << ":" << m_port);

         error = send_to_queue("CONNECT", headers);

         if (!error)
         {
            boost::asio::read_until(m_socket, m_response, char(0), error);

            if (!error)
            {
               std::string command, body;

               headers.clear();
               receive_from_queue(command, headers, body);

               MLOG_CLASS_TRACE("got " << command << " from stomp queue");

               if (command != "CONNECTED")
               {
                  return make_error_code(error::connect_cmd_failed);
               }

               MLOG_CLASS_TRACE("connected headers: " << moost::utils::stringify(headers));

               header_map::const_iterator it = headers.find("version");
               m_proto = it != headers.end() && it->second == "1.1" ? protocol::version11 : protocol::version10;

               m_state = state::online;

               recv_more();
               keepalive();
            }
         }
      }
   }

   return error;
}

void stomp_client::impl::disconnect()
{
   m_keepalive_timer.cancel();
   m_reconnect_timer.cancel();
   m_dead_conn_timer.cancel();

   send_to_queue("DISCONNECT");

   m_state = state::disconnected;
   m_proto = protocol::undefined;

   m_socket.close();
   m_streams.clear();
}

void stomp_client::impl::reconnect()
{
   if (is_connected())
   {
      MLOG_CLASS_TRACE("attempting to connect");

      boost::system::error_code ec = connect();

      if (!ec)
      {
         std::vector<stream_manager::topic_stream_pair> topics;
         m_streams.get_list(topics);

         for (std::vector<stream_manager::topic_stream_pair>::const_iterator it = topics.begin(); it != topics.end(); ++it)
         {
            subscribe(it->first, it->second->ack_type());
            it->second->reset_interval_timer();
         }

         dead_conn_detect();
      }
      else
      {
         MLOG_CLASS_TRACE("connect failed: " << ec.message());

         if (m_state == state::connecting)
         {
            m_state = state::reconnecting;
            m_error_cb(ec, "connect failed");
         }

         m_reconnect_timer.expires_from_now(m_reconnect_interval);
         m_reconnect_timer.async_wait(boost::bind(&stomp_client::impl::handle_reconnect, shared_from_this(), boost::asio::placeholders::error));
      }
   }
}

void stomp_client::impl::subscribe(const std::string& topic, stream::message_cb_t message_cb, stomp_client::ack::type ack_type,
                                   const boost::posix_time::time_duration& max_msg_interval)
{
   if (!is_connected())
   {
      throw std::runtime_error("not connected");
   }

   if (!m_streams.insert(topic, message_cb, ack_type, max_msg_interval))
   {
      throw std::runtime_error("already subscribed to " + topic);
   }

   subscribe(topic, ack_type);
}

void stomp_client::impl::subscribe(const std::string& topic, stomp_client::ack::type ack_type)
{
   if (is_online())
   {
      header_map headers;
      headers["destination"] = topic;
      headers["receipt"] = "subscribe:" + topic;
      switch (ack_type)
      {
         case stomp_client::ack::client:
            headers["ack"] = "client";
            break;

         default:
            headers["ack"] = "auto";
            break;
      }
      send_to_queue_async("SUBSCRIBE", headers);
   }
}

void stomp_client::impl::unsubscribe(const std::string& topic)
{
   if (!is_connected())
   {
      throw std::runtime_error("not connected");
   }

   if (!m_streams.erase(topic))
   {
      throw std::runtime_error("not subscribed to " + topic);
   }

   if (is_online())
   {
      header_map headers;
      headers["destination"] = topic;
      send_to_queue_async("UNSUBSCRIBE", headers);
   }
}

void stomp_client::impl::send(const std::string& topic, const std::string& message)
{
   if (!is_connected())
   {
      throw std::runtime_error("not connected");
   }

   header_map headers;
   headers["destination"] = topic;
   send_to_queue_async("SEND", headers, message);
}

boost::system::error_code stomp_client::impl::send_to_queue(const std::string& command, const std::string& body)
{
   boost::system::error_code ec;
   header_map headers;
   send_to_queue(command, headers, body, &ec);
   return ec;
}

boost::system::error_code stomp_client::impl::send_to_queue(const std::string& command, const header_map& headers, const std::string& body)
{
   boost::system::error_code ec;
   send_to_queue(command, headers, body, &ec);
   return ec;
}

void stomp_client::impl::send_to_queue_async(const std::string& command, const std::string& body)
{
   header_map headers;
   send_to_queue(command, headers, body, 0);
}

void stomp_client::impl::send_to_queue_async(const std::string& command, const header_map& headers, const std::string& body)
{
   send_to_queue(command, headers, body, 0);
}

void stomp_client::impl::send_to_queue(const std::string& command, const header_map& headers, const std::string& body, boost::system::error_code *ec)
{
   boost::shared_ptr<boost::asio::streambuf> request(new boost::asio::streambuf);
   std::ostream os(request.get());

   os << command << "\n";

   for (header_map::const_iterator i = headers.begin(); i != headers.end(); ++i)
   {
      os << i->first << ":" << i->second << "\n";
   }

   os << "\n" << body << char(0);

   MLOG_CLASS_TRACE("sending " << command << " to stomp queue (headers: " << moost::utils::stringify(headers) << ") [async=" << (ec == 0) << "]");

   if (ec)
   {
      boost::asio::write(m_socket, *request, boost::asio::transfer_all(), *ec);
   }
   else
   {
      boost::asio::async_write(m_socket, *request, boost::asio::transfer_all(),
         boost::bind(&stomp_client::impl::handle_write, shared_from_this(), request, boost::asio::placeholders::error));
   }
}

void stomp_client::impl::receive_from_queue(std::string& command, header_map& headers, std::string& body)
{
   std::istream is(&m_response);

   while (std::getline(is, command) && command.empty())
   {
   }

   std::string header;

   while (std::getline(is, header) && !header.empty())
   {
      size_t sep = header.find(':');
      const std::string& key = header.substr(0, sep);

      if (headers.count(key) == 0)
      {
         headers[key] = header.substr(sep + 1);
      }
   }

   std::getline(is, body, '\0');
}

void stomp_client::impl::keepalive()
{
   m_keepalive_timer.expires_from_now(m_keepalive_interval);
   m_keepalive_timer.async_wait(boost::bind(&stomp_client::impl::handle_keepalive, shared_from_this(), boost::asio::placeholders::error));
}

void stomp_client::impl::dead_conn_detect()
{
   m_dead_conn_timer.expires_from_now(boost::posix_time::milliseconds(500));
   m_dead_conn_timer.async_wait(boost::bind(&stomp_client::impl::handle_dead_conn, shared_from_this(), boost::asio::placeholders::error));
}

void stomp_client::impl::handle_write(boost::shared_ptr<boost::asio::streambuf>, const boost::system::error_code& err)
{
   if (err)
   {
      MLOG_CLASS_INFO("error while writing to stomp queue: " << err.message());
      m_error_cb(err, "error writing to stomp queue");
   }
}

void stomp_client::impl::handle_keepalive(const boost::system::error_code& err)
{
   if (err != boost::asio::error::operation_aborted)
   {
      MLOG_CLASS_TRACE("sending keepalive connect to stomp queue");
      send_to_queue_async("CONNECT");
      keepalive();
   }
}

void stomp_client::impl::handle_reconnect(const boost::system::error_code& err)
{
   if (err != boost::asio::error::operation_aborted)
   {
      reconnect();
   }
}

void stomp_client::impl::handle_dead_conn(const boost::system::error_code& err)
{
   if (err != boost::asio::error::operation_aborted)
   {
      if (m_streams.max_msg_interval_exceeded())
      {
         MLOG_CLASS_INFO("max message interval exceeded, forcing reconnect");
         m_error_cb(make_error_code(error::connection_lost), "max message interval exceeded");

         // this will trigger a reconnect
         m_state = state::reconnecting;
         m_socket.close();
      }
      else
      {
         dead_conn_detect();
      }
   }
}

void stomp_client::impl::handle_stomp_error(const header_map& headers, const std::string& /*body*/)
{
   header_map::const_iterator msg = headers.find("message");

   if (msg != headers.end())
   {
      header_map::const_iterator rcpt = headers.find("receipt-id");

      if (rcpt != headers.end())
      {
         if (rcpt->second.compare(0, 10, "subscribe:") == 0)
         {
            const std::string& topic = rcpt->second.substr(10);
            m_streams.erase(topic);
            m_error_cb(make_error_code(error::subscribe_failed), topic);
            return;
         }
      }

      m_error_cb(make_error_code(error::queue_error), msg->second);
   }
   else
   {
      m_error_cb(make_error_code(error::queue_error), "unknown queue error");
   }
}

void stomp_client::impl::handle_recv(const boost::system::error_code& err)
{
   if (!is_connected())
   {
      return;
   }

   if (err)
   {
      if (m_state != state::reconnecting)
      {
         MLOG_CLASS_INFO("error while receiving from stomp queue: " << err.message());
         m_error_cb(make_error_code(error::connection_lost), err.message());
         m_state = state::reconnecting;
      }

      m_keepalive_timer.cancel();
      m_dead_conn_timer.cancel();
      reconnect();

      return;
   }

   std::string command, body;
   header_map headers;
   receive_from_queue(command, headers, body);

   MLOG_CLASS_TRACE("got " << command << " from stomp queue (headers: " << moost::utils::stringify(headers) << ")");

   if (command == "MESSAGE")
   {
      on_message(headers, body);
   }
   else if (command == "CONNECTED")
   {
      // ok, this is the normal keepalive response
   }
   else if (command == "ERROR")
   {
      handle_stomp_error(headers, body);
   }
   else if (command == "RECEIPT")
   {
      // there's no need to handle this, all went well
   }
   else
   {
      MLOG_CLASS_WARN("got unexpected " << command << " from stomp queue");
   }

   recv_more();
}

void stomp_client::impl::on_message(const header_map& headers, const std::string& msg)
{
   header_map::const_iterator hit = headers.find("destination");

   if (hit != headers.end())
   {
      stomp_client::ack::type ack_type;

      if (m_streams.push_message(hit->second, msg, ack_type))
      {
         if (ack_type == stomp_client::ack::client)
         {
            hit = headers.find("message-id");

            if (hit != headers.end())
            {
               header_map ack_headers;
               ack_headers.insert(*hit);
               send_to_queue_async("ACK", ack_headers);
            }
         }
      }
      else
      {
         MLOG_CLASS_DEBUG("no stream found for topic: " + hit->second);
      }
   }
   else
   {
      MLOG_CLASS_WARN("skipping message without destination");
   }
}

void stomp_client::impl::recv_more()
{
   boost::asio::async_read_until(m_socket, m_response, char(0),
                                 boost::bind(&stomp_client::impl::handle_recv, shared_from_this(),
                                             boost::asio::placeholders::error));
}

}}
