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
* @file uri_params.hpp
* @brief Represents Uri (and post) parameters encoded according to RFC-2396
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*
*/

#ifndef FM_LAST_MOOST_MURCL_URI_PARAMS_HPP__
#define FM_LAST_MOOST_MURCL_URI_PARAMS_HPP__

#include <string>
#include <sstream>
#include <map>

#include <curl/curl.h>

#include <boost/lexical_cast.hpp>

#include "uri_encoder.hpp"

namespace moost { namespace murcl {

/**
* @brief This class is an object representaion of uri/post parameters encoded
*/
class uri_params
{
public:
   typedef std::map<std::string, std::string> map_t;

   /**
   * @brief construct a set of uri params from a map
   *
   * @param map
   *     a map of key/value string pairs
   */
   uri_params(map_t const & map)
   {
      std::ostringstream oss;

      for(map_t::const_iterator itr = map.begin(); itr != map.end(); ++itr)
      {
         if(itr != map.begin())
         {
            oss << '&';
         }

         oss
            << uri_encoder::encode(itr->first)    // key
            << '='
            << uri_encoder::encode(itr->second);  // val
      }

      params_ = oss.str();
   }

   /**
   * @brief Get string representation of uri params
   *
   * @return
   *     uri param string
   */
   std::string const & get() const
   {
      return params_;
   }

   /**
   * @brief Implicit convertor to string, used by generic options handler
   *
   * @return
   *     uri param string
   */
   operator std::string const & () const
   {
      return get();
   }

private:
   std::string params_;
};

}}


#endif // FM_LAST_MOOST_MURCL_URI_PARAMS_HPP__
