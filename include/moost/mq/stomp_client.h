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

#ifndef MOOST_MQ_STOMP_CLIENT_H_
#define MOOST_MQ_STOMP_CLIENT_H_

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/cstdint.hpp>

namespace moost { namespace mq {

/**
 * STOMP client implementation
 *
 * This class implements a multi-stream, multi-thread STOMP client.
 */
class stomp_client
{
public:
   /**
    * Acknowledge mechanism to use
    */
   struct ack
   {
      enum type
      {
         automatic,
         client
      };
   };

   /**
    * Create a new client
    *
    * \param consumer_pool_size    The number of threads from which messages
    *                              shall be dispatched. This can be useful if
    *                              the actual message processing is quite CPU
    *                              intensive.
    *
    * \param keepalive_interval    The interval in which keepalive packets will
    *                              be sent to the server.
    *
    * \param reconnect_interval    The interval in which reconnection attempts
    *                              will be made after a failed connection attempt.
    */
   stomp_client(size_t consumer_pool_size = 1,
                const boost::posix_time::time_duration& keepalive_interval = boost::posix_time::seconds(30),
                const boost::posix_time::time_duration& reconnect_interval = boost::posix_time::seconds(1));

   /**
    * Destroy a client
    */
   ~stomp_client();

   /**
    * Connect to queue server
    *
    * \param hostname              Host name of the queue server.
    *
    * \param port                  Port number of the queue server.
    *
    * \param error_cb              Callback for asynchronous errors.
    */
   void connect(const std::string& hostname, int port,
                boost::function<void (const boost::system::error_code&, const std::string&)> error_cb);

   /**
    * Disconnect from queue server
    */
   void disconnect();

   /**
    * Subscribe to a topic
    *
    * \param topic                 Topic (destination) to subscribe to.
    *
    * \param message_cb            Callback for received messages for this topic.
    *
    * \param ack_type              Which acknowledgement protocol to use.
    *
    * \param max_msg_interval      The maximum expected interval between any two
    *                              messages. If this interval is exceeded, the
    *                              client will initiate a reconnect to the server.
    */
   void subscribe(const std::string& topic, boost::function<void (const std::string&)> message_cb, ack::type ack_type = ack::automatic,
                  const boost::posix_time::time_duration& max_msg_interval = boost::posix_time::pos_infin);

   /**
    * Unsubscribe from a topic
    *
    * \param topic                 Topic (destination) to unsubscribe from.
    */
   void unsubscribe(const std::string& topic);

   /**
    * Send message to the queue server
    *
    * \param topic                 Topic (destination) of the message.
    *
    * \param message               Message content.
    */
   void send(const std::string& topic, const std::string& message);

   /**
    * Check if the client object is connected to the server
    *
    * Note that "being connected" is a state of the object. The object may be
    * connected even though the server is offline, in which case the object
    * will attempt to periodically reconnect to the server.
    *
    * \return Boolean indicating whether the client object is connected.
    */
   bool is_connected() const;

   /**
    * Check if the connected server is online
    *
    * Use this method to actually check if the connection between the client
    * and server is healthy.
    *
    * \return Boolean indicating whether the server is online.
    */
   bool is_online() const;

   /**
    * Get number of messages processed by this object
    *
    * \return Number of processed messages.
    */
   uint64_t get_num_processed() const;

   /**
    * Get number of messages received but currently pending
    *
    * \return Number of pending messages.
    */
   size_t get_num_pending() const;

private:
   class impl;
   boost::shared_ptr<impl> m_impl;
};

}}

#endif
