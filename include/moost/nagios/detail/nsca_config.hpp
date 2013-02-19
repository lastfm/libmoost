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

#ifndef MOOST_NAGIOS_NSCA_CLIENT_NSCA_CONFIG_HPP__
#define MOOST_NAGIOS_NSCA_CLIENT_NSCA_CONFIG_HPP__

#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/regex.hpp>

#include "nsca_enctype.hpp"
#include "nsca_encpass.hpp"
#include "nsca_init_packet.hpp"
#include "nsca_data_packet.hpp"

namespace moost { namespace nagios {


   struct nsca_config
   {
      nsca_config(
         std::string const & host = nsca_const::DEFAULT_HOST,
         boost::uint16_t port = nsca_const::DEFAULT_PORT,
         boost::uint16_t recv_timeout = nsca_const::DEFAULT_RECV_TIMEOUT_MS,
         boost::uint16_t send_timeout = nsca_const::DEFAULT_SEND_TIMEOUT_MS,
         nsca_enctype const & enctype = nsca_enctype(),
         nsca_encpass const & encpass = nsca_encpass()
         )
         : nsca_svr_host(host)
         , nsca_svr_port(boost::lexical_cast<std::string>(port))
         , recv_timeout(recv_timeout)
         , send_timeout(send_timeout)
         , enctype(enctype)
         , encpass(encpass)
      {

      }

      // these are public, get over it.
      std::string nsca_svr_host;
      std::string nsca_svr_port;
      boost::uint16_t recv_timeout;
      boost::uint16_t send_timeout;
      nsca_enctype enctype;
      nsca_encpass encpass;
   };
}}

#endif
