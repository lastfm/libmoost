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

#ifndef MOOST_NAGIOS_NSCA_CLIENT_NSCA_ENCTYPE_HPP__
#define MOOST_NAGIOS_NSCA_CLIENT_NSCA_ENCTYPE_HPP__

#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/regex.hpp>

#include "nsca_common.hpp"

#define MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, enc) \
   if(boost::regex_match(str, boost::regex(#enc, boost::regex::perl|boost::regex::icase))) \
   return nsca_encryption_method::ENCRYPT_ ## enc

namespace moost { namespace nagios {


   struct nsca_enctype
   {
      nsca_enctype(std::string const & enc)
         : enc(validate_encryption_method(str2enc(enc)))
      {
      }

      nsca_enctype(int const enc)
         : enc(validate_encryption_method(enc))
      {
      }

      nsca_enctype(nsca_encryption_method::type const enc =
            nsca_encryption_method::ENCRYPT_NONE)
         : enc(enc)
      {
      }

      nsca_encryption_method::type enc;

      operator nsca_encryption_method::type() const
      {
         return enc;
      }

      operator int() const
      {
         return enc;
      }

      static nsca_encryption_method::type validate_encryption_method(int encryption_method)
      {
         switch(encryption_method)
         {
         case nsca_encryption_method::ENCRYPT_NONE:
         case nsca_encryption_method::ENCRYPT_XOR:
            break;

#ifdef HAVE_LIBMCRYPT
         // these methods require mcrypt
         case nsca_encryption_method::ENCRYPT_DES:
         case nsca_encryption_method::ENCRYPT_3DES:
         case nsca_encryption_method::ENCRYPT_CAST128:
         case nsca_encryption_method::ENCRYPT_CAST256:
         case nsca_encryption_method::ENCRYPT_XTEA:
         case nsca_encryption_method::ENCRYPT_3WAY:
         case nsca_encryption_method::ENCRYPT_BLOWFISH:
         case nsca_encryption_method::ENCRYPT_TWOFISH:
         case nsca_encryption_method::ENCRYPT_LOKI97:
         case nsca_encryption_method::ENCRYPT_RC2:
         case nsca_encryption_method::ENCRYPT_ARCFOUR:
         case nsca_encryption_method::ENCRYPT_RIJNDAEL128:
         case nsca_encryption_method::ENCRYPT_RIJNDAEL192:
         case nsca_encryption_method::ENCRYPT_RIJNDAEL256:
         case nsca_encryption_method::ENCRYPT_WAKE:
         case nsca_encryption_method::ENCRYPT_SERPENT:
         case nsca_encryption_method::ENCRYPT_ENIGMA:
         case nsca_encryption_method::ENCRYPT_GOST:
         case nsca_encryption_method::ENCRYPT_SAFER64:
         case nsca_encryption_method::ENCRYPT_SAFER128:
         case nsca_encryption_method::ENCRYPT_SAFERPLUS:
            break;
#endif
         default:
            throw std::invalid_argument("unknown encryption method");
         }

         return nsca_encryption_method::type(encryption_method);
      }

      // just returns a string of all the possible encryption options for use in "help"
      static std::string get_enc_helpstr()
      {
         char const delim = '|';
         std::stringstream ss;

         ss << "none" << delim;
         ss << "xor";

#ifdef HAVE_LIBMCRYPT
         ss << delim;
         ss << "xor" << delim;
         ss << "des" << delim;
         ss << "3des" << delim;
         ss << "cast128" << delim;
         ss << "cast256" << delim;
         ss << "xtea" << delim;
         ss << "3way" << delim;
         ss << "blowfish" << delim;
         ss << "twofish" << delim;
         ss << "loki97" << delim;
         ss << "rc2" << delim;
         ss << "arcfour" << delim;
         ss << "rijndael128" << delim;
         ss << "rijndael192" << delim;
         ss << "rijndael256" << delim;
         ss << "wake" << delim;
         ss << "serpent" << delim;
         ss << "enigma" << delim;
         ss << "gost" << delim;
         ss << "safer64" << delim;
         ss << "safer128" << delim;
         ss << "saferplus";
#endif

         return ss.str();
      }

      // convert str into the relevant enum value
      static int str2enc(std::string const & str)
      {
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, NONE);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, XOR);

#ifdef HAVE_LIBMCRYPT
         // these are only avaliable if mcrypt is installed
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, DES);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, 3DES);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, CAST128);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, CAST256);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, XTEA);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, 3WAY);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, BLOWFISH);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, TWOFISH);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, LOKI97);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, RC2);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, ARCFOUR);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, RIJNDAEL128);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, RIJNDAEL192);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, RIJNDAEL256);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, WAKE);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, SERPENT);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, ENIGMA);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, GOST);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, SAFER64);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, SAFER128);
         MOOST_NAGIOS_NSCA_CONFIG_STR2ENC__(str, SAFERPLUS);
#endif
         // sorry, who are you?
         throw std::invalid_argument("unknown encryption method");
      }
   };

}}

#endif
