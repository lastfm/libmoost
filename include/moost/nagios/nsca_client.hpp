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

#ifndef MOOST_NAGIOS_NSCA_CLIENT_HPP__
#define MOOST_NAGIOS_NSCA_CLIENT_HPP__

#include <string>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "detail/nsca_init_packet.hpp"
#include "detail/nsca_data_packet.hpp"
#include "detail/nsca_config.hpp"
#include "detail/nsca_crc32.hpp"
#include "detail/nsca_crypto.hpp"
#include "detail/nsca_enctype.hpp"
#include "detail/nsca_encpass.hpp"

namespace moost { namespace nagios {

   class nsca_client
   {
   public:
      // nsca service states
      struct service_state {
         enum type {
            OK,
            WARNING,
            CRITICAL,
            UNKNOWN
         };
      };

      nsca_client(nsca_config const & cfg)
         : cfg_(new nsca_config(cfg))
      {
      }

      nsca_client(boost::shared_ptr<nsca_config> cfg)
         : cfg_(cfg)
      {
      }

      // Payload format
      // "{HOSTNAME}\n{SERVICEDESC}\n{SERVICESTATEID}\n{PLUGINGOUTPUT}"
      void send(std::string payload) const
      {
         nsca_data_packet send_packet;
         init_packet(send_packet);

         std::istringstream ss(payload);
         ss.exceptions(std::ios::failbit); // simplify life.
         ss >> send_packet;

         send(send_packet);
      }

      void send(
         std::string hostname,
         std::string svc_description,
         boost::int16_t return_code,
         std::string plugin_output) const
      {
         std::stringstream ss;

         ss
            << hostname << "\n"
            << svc_description << "\n"
            << return_code << "\n"
            << plugin_output << "\n";

         ss.exceptions(std::ios::badbit); // simplify life.

         nsca_data_packet send_packet;
         init_packet(send_packet);
         ss >> send_packet;

         send(send_packet);
      }

   private:
      typedef boost::shared_ptr<nsca_crypto> crypto_ptr;

      typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

      void init_packet(nsca_data_packet & send_packet) const
      {
         nsca_data_packet::randomize(send_packet);
      }

      // create a connection to the nsca server
      socket_ptr connect(boost::asio::io_service & io_service) const
      {
         using namespace boost::asio::ip;

         tcp::resolver resolver(io_service);
         tcp::resolver::query query(tcp::v4(), cfg_->nsca_svr_host.c_str(),
            cfg_->nsca_svr_port.c_str());
         tcp::resolver::iterator iterator = resolver.resolve(query);

         socket_ptr psock(new tcp::socket(io_service));
         psock->connect(*iterator);

         setsockopt(psock->native(), SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<char const *>(&cfg_->recv_timeout), sizeof(cfg_->recv_timeout));

         setsockopt(psock->native(), SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<char const *>(&cfg_->send_timeout), sizeof(cfg_->send_timeout));

         return psock;
      }

      // recv the server initialisation packet from the socket
      void recv(socket_ptr psock, nsca_init_packet & packet) const
      {
         if(boost::asio::read(*psock, boost::asio::buffer(&packet, sizeof(packet))) != sizeof(packet))
         {
            throw std::runtime_error("failed to recv packet from server");
         }

         using namespace boost::asio::detail::socket_ops;
         packet.timestamp = network_to_host_long(packet.timestamp);
      }

      // send the data packet to the socket
      void send(socket_ptr psock, nsca_data_packet & packet) const
      {
         if(boost::asio::write(*psock, boost::asio::buffer(&packet, sizeof(packet))) != sizeof(packet))
         {
            throw std::runtime_error("failed to send packet to server");
         }
      }

      // send the status update to the server
      void send(nsca_data_packet const & send_packet) const
      {
         using namespace boost::asio::detail::socket_ops;

         // a copy of send_packet with the fields converted to network byte order
         nsca_data_packet hton_send_packet;

         // memcpy used as we need a binary copy (the padding is important) =/
         memcpy(&hton_send_packet, &send_packet, sizeof(hton_send_packet));

         // socket service
         boost::asio::io_service io_service;
         socket_ptr psock;

         // get the initialisation packet from the server
         nsca_init_packet init_packet;

         psock = connect(io_service);
         recv(psock, init_packet);

         // send the server the status update
         hton_send_packet.packet_version= host_to_network_short(nsca_const::NSCA_PACKET_VERSION);
         hton_send_packet.return_code = host_to_network_short(send_packet.return_code);
         hton_send_packet.timestamp = host_to_network_long(init_packet.timestamp);
         hton_send_packet.crc32_value = 0;
         hton_send_packet.crc32_value = host_to_network_long(crc32_.calculate(hton_send_packet));

         crypto_ptr pcrypto(
            new nsca_crypto(
               init_packet.iv,
               cfg_->enctype,
               cfg_->encpass
            ));

         pcrypto->encrypt(hton_send_packet);


         if(!psock) { psock = connect(io_service); }
         send(psock, hton_send_packet);

         if(pcrypto)
         {
            pcrypto->decrypt(hton_send_packet);
         }

      }

   private:
      boost::shared_ptr<nsca_config> cfg_;
      nsca_crc32 crc32_;
   };

}}

#endif
