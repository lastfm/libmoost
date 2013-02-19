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

#ifndef MOOST_NAGIOS_NSCA_CLIENT_NSCA_ENCPASS_HPP__
#define MOOST_NAGIOS_NSCA_CLIENT_NSCA_ENCPASS_HPP__

#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/regex.hpp>

#include "nsca_common.hpp"

namespace moost { namespace nagios {


   struct nsca_encpass
   {
      nsca_encpass(std::string const & passwd = std::string())
         : passwd(validate_encryption_password(passwd))
      {
      }

      static std::string validate_encryption_password(
            std::string const & password)
      {
         if(password.size() > nsca_const::MAX_PASSWORD_LENGTH)
         {
            throw std::invalid_argument(
                  "Password is too long for NSCA to handle");
         }

         return password;
      }

      operator std::string const & () const
      {
         return passwd;
      }

      std::string passwd;
   };

}}

#endif
