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
* @file option_setter.hpp
* @brief A typesafe option setter
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_OPTION_SETTER_HPP__
#define FM_LAST_MOOST_MURCL_OPTION_SETTER_HPP__

#include <curl/curl.h>
#include "../utils/foreach.hpp"
#include "options.hpp"

namespace moost { namespace murcl {

/**
* @brief A generic option setter to abstract details of both the option value
*        types and the LibCurl interface being used (easy, multi or share).
*/
template <typename CurlOption, typename ValueType>
struct option_setter
{
   typedef CurlOption option;
   typedef ValueType value_type;

   static void set(
      typename option::handle_type handle,
      value_type const & parameter
      )
   {
      option::set(handle, parameter);
   }
};

/**
* @brief Specialisation of the generic option setter for std::string type values
*/
template <typename CurlOption>
struct option_setter<CurlOption, std::string>
{
   typedef CurlOption option;
   typedef std::string value_type;

   static void set(
      typename option::handle_type handle,
      value_type const & parameter
      )
   {
      option::set(handle, parameter.c_str());
   }
};

}}

#endif //FM_LAST_MOOST_MURCL_OPTION_SETTER_HPP__
