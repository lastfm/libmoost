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
* @file response.hpp
* @brief Represents an abstraction of a LibCurl response
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_RESPONSE_HPP__
#define FM_LAST_MOOST_MURCL_RESPONSE_HPP__

#include<string>

#include <curl/curl.h>

namespace moost { namespace murcl {

// This is a SHA1 of the string "response", used to validate a cast from void *
char const RESPONSE_SHA1[] = "0ec6d150549780250a9772c06b619bcc46a0e560";
size_t const RESPONSE_SHA1_LEN = sizeof(RESPONSE_SHA1)/sizeof(RESPONSE_SHA1[0]);

/**
* @brief This class is an object representation of a LibCurl response
*/
class response
{
public:
   /**
   * @brief Represents the userdata object to the request callback
   */
   struct userdata // This class MUST be a POD type (yes, Ricky, A POD!)
   {
      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      /// This member MUST be first;it is used to validate the cast from void*
      /// The C++ standard guarantees a pointer to a POD-struct object, suitably
      /// converted using a reinterpret_cast, points to its initial member
      char sha1_[RESPONSE_SHA1_LEN];
      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

      std::string * pdata; // this is a pointer... pointers are PODs!
   };

   /**
   * @brief Construct a response object and an suitably initialised userdata POD
   */
   response()
   {
      strncpy(userdata_.sha1_, RESPONSE_SHA1, sizeof(userdata_.sha1_));
      userdata_.pdata = &data_;
   }

   /**
   * @brief
   *
   * @param ptr
         pointer to data that needs to be written
   * @param size
   *     size of a single datum to be written
   * @param nmemb
   *     the number of data to be written
   * @param userdata
   *     a user definable object (here it is set to 'this')
   *
   * @return
   *     The amount of data successfully stored
   *
   * This callback needs to be registered with LibCurl. If this class is used
   * in conjunction with murcl::request the registration is done automatically.
   */
   static size_t callback( char *ptr, size_t size, size_t nmemb, void *userdata)
   {
      response::userdata * prud =
         reinterpret_cast<response::userdata *>(userdata);

      // make sure we have a valid object to play with
      if(!validate(prud))
      {
         // eek, something has gone pretty badly wrong - abort, abort abort!
         return CURL_READFUNC_ABORT;
      }

      // Not majorly efficient, but simple and given this is happening as a
      // callback in a network connection with latency I think simple trumps!
      size *= nmemb;
      prud->pdata->append(ptr, size);

      return size;
   }

   /**
   * @brief get the response string
   *
   * @return
   *     the response string
   */
   std::string const & get() const
   {
      return data_;
   }

   /**
   * @brief A function operator to return a pointer to user data
   *
   * @return
   *     a pointer to user data
   */
   userdata * operator()()
   {
      return &userdata_;
   }

private:
   static bool validate(userdata * prud)
   {
      // check we have a pointer to a valid response object
      return 0 == memcmp(prud, RESPONSE_SHA1, RESPONSE_SHA1_LEN);
   }

   std::string data_;
   userdata userdata_;

};

}}

#endif // FM_LAST_MOOST_MURCL_RESPONSE_HPP__
