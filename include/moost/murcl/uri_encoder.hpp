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

/**
* @file uri_encoder.hpp
* @brief Encodes Uris in accordence with RFC-2396
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*
*/

#ifndef FM_LAST_MOOST_MURCL_URI_ENCODER_HPP__
#define FM_LAST_MOOST_MURCL_URI_ENCODER_HPP__

#include <string>

#include <curl/curl.h>

#include "../utils/scope_exit.hpp"

#include <boost/cast.hpp>

namespace moost { namespace murcl {

/**
* @brief This class can be used to uri encode a string representing a uri
*/
struct uri_encoder
{
   typedef moost::utils::scope_exit::type<char *>
      ::call_free_function_with_val encoded_t;

   /**
   * @brief Encode a string that represents a uri
   *
   * @param uri
   *     The string representation of a uri to be encoded
   *
   * @return
   *     A uri encoded string
   */
   static std::string encode(std::string const & uri)
   {
      std::string encoded_uri;

      if(!uri.empty())
      {
         encoded_t encoded(
            curl_escape(uri.c_str(),
            boost::numeric_cast<int>(uri.size())),
            curl_free);

         encoded_uri = encoded->get();
      }

      return encoded_uri;
   }
};

}}


#endif // FM_LAST_MOOST_MURCL_URI_ENCODER_HPP__
