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
* @file option_trait.hpp
* @brief A traits class for LIbCurls options
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_OPTION_TRAITS_HPP__
#define FM_LAST_MOOST_MURCL_OPTION_TRAITS_HPP__

#include <map>
#include <stdexcept>
#include <sstream>
#include <curl/curl.h>

namespace moost { namespace murcl {

/**
* @brief Base class for the option abstractions that implements common behaviour
*/
template <CURLoption optval>
class curl_opt_base
{
   protected:
      curl_opt_base() {}

   template <typename T>
   static void error(
      T const & val,
      std::string const & opttype,
      std::string const & msg
      )
   {
      std::ostringstream oss;
      oss
         << "Error setting " << opttype
         << " option '" << optval
         << "' to value '" << val
         << "': " << msg;

     throw std::runtime_error(oss.str());
   }
};

/**
* @brief Abstraction of the LibCurl easy interface options
*/
template <CURLoption optval>
struct curl_easyopt: curl_opt_base<optval>
{
   typedef CURL * handle_type;
   typedef CURLoption option_type;
   typedef CURLcode result_type;
   static const option_type option_value = optval;

   template <typename T>
   static void set(handle_type handle, T const & val)
   {
      result_type code = curl_easy_setopt(handle, option_value, val);

      if(code != CURLE_OK)
      {
         curl_opt_base<optval>::error(val, "easy", curl_easy_strerror(code));
      }
   }
};

/**
* @brief Abstraction of the LibCurl multi interface options
*/
template <CURLMoption optval>
struct curl_multiopt: curl_opt_base<optval>
{
   typedef CURLM * handle_type;
   typedef CURLMoption option_type;
   typedef CURLMcode result_type;
   static const option_type option_value = optval;

   template <typename T>
   static void set(handle_type handle, T const & val)
   {
      result_type code = curl_multi_setopt(handle, option_value, val);

      if(code != CURLM_OK)
      {
         curl_opt_base<optval>::error(val, "multi", curl_multi_strerror(code));
      }
   }
};

/**
* @brief Abstraction of the libCurl share interface options
*/
template <CURLSHoption optval>
struct curl_shareopt: curl_opt_base<optval>
{
   typedef CURLSH * handle_type;
   typedef CURLSHoption option_type;
   typedef CURLSHcode result_type;
   static const option_type option_value = optval;

   template <typename T>
   static void set(handle_type handle, T const & val)
   {
      result_type code = curl_easy_setopt(handle, option_value, val);

      if(code != CURLSHE_OK)
      {
         curl_opt_base<optval>::error(val, "share", curl_share_strerror(code));
      }
   }
};

/**
* @brief A traits class to abstract out the differences between the different
*        interfaces, options and option values. Each supported option has an
*        associated traits class that defines the options compile-time traits.
*/
template <typename O, typename T>
struct option_traits
{
   typedef O curl_option;
   typedef typename curl_option::handle_type handle_type;
   typedef typename curl_option::option_type option_type;
   typedef typename curl_option::result_type result_type;
   static const option_type option_value = curl_option::option_value;
   typedef T value_type;
};

}}

#endif //FM_LAST_MOOST_MURCL_OPTION_TRAITS_HPP__
