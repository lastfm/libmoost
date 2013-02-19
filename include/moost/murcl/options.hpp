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
* @file options.hpp
* @brief A Moost C++ wrapper for LibCurl's options
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_OPTIONS_HPP__
#define FM_LAST_MOOST_MURCL_OPTIONS_HPP__

#include <map>

#include <curl/curl.h>

#include "option_traits.hpp"

namespace moost { namespace murcl {

/**
* @brief A type-safe representaion of all the supported LibCurl options
*/

struct easyopt
{
   typedef option_traits<curl_easyopt<CURLOPT_VERBOSE>, bool> verbose;
   typedef option_traits<curl_easyopt<CURLOPT_HEADER>, bool> header;
   typedef option_traits<curl_easyopt<CURLOPT_NOPROGRESS>, bool> noprogress;
   typedef option_traits<curl_easyopt<CURLOPT_NOSIGNAL>, bool> nosignal;

   typedef option_traits<curl_easyopt<CURLOPT_URL>, std::string> uri;
   typedef option_traits<curl_easyopt<CURLOPT_POSTFIELDS>, std::string> postfields;

   typedef option_traits<curl_easyopt<CURLOPT_READFUNCTION>, curl_read_callback> readcb;
   typedef option_traits<curl_easyopt<CURLOPT_READDATA>, void *> readdata;

   typedef option_traits<curl_easyopt<CURLOPT_WRITEFUNCTION>, curl_write_callback> writecb;
   typedef option_traits<curl_easyopt<CURLOPT_WRITEDATA>, void *> writedata;
};


}}

#endif //FM_LAST_MOOST_MURCL_OPTIONS_HPP__
