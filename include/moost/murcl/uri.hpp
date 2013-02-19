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
* @file uri.hpp
* @brief Represents an url encoded according to RFC-2396
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*
*/

#ifndef FM_LAST_MOOST_MURCL_URI_HPP__
#define FM_LAST_MOOST_MURCL_URI_HPP__

#include <string>
#include <sstream>

#include <curl/curl.h>

#include <boost/lexical_cast.hpp>

#include "uri_elements.hpp"

namespace moost { namespace murcl {

/**
* @brief This class is an object representation of a RFC-2396 encoded uri
*/
class uri
{
public:

   /**
   * @brief Constructs a uri string from a uri elements object
   *
   * @param elements
   *     A uri elements object to be converted to an encoded RFC-2396 string
   */
   uri(uri_elements const & elements)
   {
      std::ostringstream oss;
      oss
         << elements.get_scheme()
         << elements.get_host()
         << ':' << elements.get_port()
         << '/' << elements.get_path();

      if(!elements.get_params().empty())
      {
         oss << "?" << elements.get_params();
      }

      uri_ =  oss.str();
   }

   /**
   * @brief Get uri encoded string
   *
   * @return
   *     url encoded string
   */
   std::string const & get() const
   {
      return uri_;
   }

   /**
   * @brief Implicit conversion to a string, used by generic options handler
   *
   * @return
   *     uri encoded string
   */
   operator std::string const & () const
   {
      return get();
   }

private:
   std::string uri_;
};

}}


#endif // FM_LAST_MOOST_MURCL_URI_HPP__
