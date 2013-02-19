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
* @file user_agent.hpp
* @brief Generate a simple user agent
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_USER_AGENT_HPP__
#define FM_LAST_MOOST_MURCL_USER_AGENT_HPP__

#include <string>
#include <sstream>

#include <curl/curl.h>

namespace moost { namespace murcl {

/**
* @brief Generates a simple LibCurl user agent
*
* The user agent generated is based on examples show here:
*    http://www.useragentstring.com/pages/curl/
*/
struct user_agent
{
   /**
   * @brief Gets a user agent string
   *
   * @return
   *     A string representation of a user agent
   */
   static std::string get()
   {
      // according to the docs for LibCurl, this does not need to be free'd.
      // http://curl.haxx.se/libcurl/c/curl_version_info.html
      curl_version_info_data * ver = curl_version_info(CURLVERSION_NOW);

      std::ostringstream oss;

      oss
         << "curl/"
         << ver->version
         << '(' << ver->host << ')'
         << " libcurl " << ver->version
         << '(' << ver->ssl_version << ')';

      if((ver->features & CURL_VERSION_IPV6) == CURL_VERSION_IPV6)
      {
         oss
            << " (ipv6 enabled)";
      }

      return oss.str();
   }
};

}}

#endif // FM_LAST_MOOST_MURCL_USER_AGENT_HPP__
