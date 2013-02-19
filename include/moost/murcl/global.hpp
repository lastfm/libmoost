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
* @file murcl.hpp
* @brief A Moost C++ wrapper for LibCurl
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*
*/

#ifndef FM_LAST_MOOST_MURCL_GLOBAL_HPP__
#define FM_LAST_MOOST_MURCL_GLOBAL_HPP__

#include <stdexcept>

#include <curl/curl.h>

#include <boost/shared_ptr.hpp>
#include "../utils/singleton.hpp"

namespace moost { namespace murcl {

/**
* @brief This class initialialises the Curl 'program environment'
*/
class global
{
template <typename T> friend
   class moost::utils::singleton_default<T>::friend_type;

private:
   /**
   * @brief Inialises the LibCurl global space and is constructed as a singleton
   */
   global()
      : code_(CURLE_FAILED_INIT)
   {
      // This bad-boy is instantiated as a singleton so it
      // absolutely must not throw during construction!
      try
      {
         // Since this call is to a C API is shouldn't throw...
         code_ = curl_global_init(CURL_GLOBAL_ALL);
      }
      catch(...)
      {
         // ...but just in case!
      }
   }

public:

   /**
   * @brief Checks if LibCurl was correctly initialised. This should be
   *        checked before using the murcl API (not mandatory but wise).
   *
   * @return
   *  true if LibCurl is ready for use, else false (in which case panic!)
   */
   bool isok() const
   {
      return (code_ == CURLE_OK);
   }

   /**
   * @brief Get the status of initialisation
   *
   * @return Curl status code
   */
   CURLcode status_code() const
   {
      return code_;
   }

   /**
   * @brief The d_tor cleans up the LibCurl global space
   */
   ~global()
   {
      curl_global_cleanup();
   }

private:
   CURLcode code_;
};

/**
* @brief A factory object for creating other interface instances
*
* It is imperative that the LibCurl library is initialised before any of the
* interface members are called. To ensure this is the case interface objects
* can only be constructed from this singleton factory object.
*/

typedef moost::utils::singleton_default<global> global_singleton;

}}

#endif // FM_LAST_MOOST_MURCL_GLOBAL_HPP__
