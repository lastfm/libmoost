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

// [ricky 7/13/2011 ] Mostly ripped from the nsca_send common.h file

#ifndef MOOST_NAGIOS_NSCA_CLIENT_NSCA_COMMON_HPP__
#define MOOST_NAGIOS_NSCA_CLIENT_NSCA_COMMON_HPP__

#include <boost/cstdint.hpp>

namespace moost { namespace nagios {

// some constants that can be reused
namespace nsca_const
{
   // default config values (these are lfm specific, obviously)
   static const char * const DEFAULT_HOST = "xenu.sov.last.fm";
   static const size_t DEFAULT_PORT = 5667;
   static const int DEFAULT_RECV_TIMEOUT_MS = 1000;
   static const int DEFAULT_SEND_TIMEOUT_MS = 1000;
   static const char * const DEFAULT_ENCTYPE = "xor";

   // default values for nsca internals
   static const size_t TRANSMITTED_IV_SIZE = 128;
   static const size_t MAX_HOSTNAME_LENGTH = 64;
   static const size_t MAX_DESCRIPTION_LENGTH = 128;
   static const size_t MAX_PLUGINOUTPUT_LENGTH = 512;
   static const size_t MAX_PASSWORD_LENGTH = 512;
   static const boost::int16_t NSCA_PACKET_VERSION = 3;
}

// supported encryption methods
struct nsca_encryption_method
{
   enum type
   {
      ENCRYPT_NONE = 0,
      ENCRYPT_XOR = 1

#ifdef HAVE_LIBMCRYPT
      ,
      ENCRYPT_DES = 2,
      ENCRYPT_3DES = 3,
      ENCRYPT_CAST128 = 4,
      ENCRYPT_CAST256 = 5,
      ENCRYPT_XTEA = 6,
      ENCRYPT_3WAY = 7,
      ENCRYPT_BLOWFISH = 8,
      ENCRYPT_TWOFISH = 9,
      ENCRYPT_LOKI97 = 10,
      ENCRYPT_RC2 = 11,
      ENCRYPT_ARCFOUR = 12,
      ENCRYPT_RC6 = 13,
      ENCRYPT_RIJNDAEL128 = 14,
      ENCRYPT_RIJNDAEL192 = 15,
      ENCRYPT_RIJNDAEL256 = 16,
      ENCRYPT_MARS = 17,
      ENCRYPT_PANAMA = 18,
      ENCRYPT_WAKE = 19,
      ENCRYPT_SERPENT = 20,
      ENCRYPT_IDEA = 21,
      ENCRYPT_ENIGMA = 22,
      ENCRYPT_GOST = 23,
      ENCRYPT_SAFER64 = 24,
      ENCRYPT_SAFER128 = 25,
      ENCRYPT_SAFERPLUS = 26
#endif
   };
};

}}

#endif
