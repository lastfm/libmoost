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
* @file easy.hpp
* @brief A Moost C++ wrapper for LibCurl's easy interface
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_EASY_HPP__
#define FM_LAST_MOOST_MURCL_EASY_HPP__

#include <stdexcept>
#include <sstream>

#include <boost/bind.hpp>

#include <curl/curl.h>

#include "global.hpp"
#include "option.hpp"

namespace moost { namespace murcl {

/**
* @brief An abstract represenation of the LibCurl easy interface
*/
class easy
{
public:

   /**
   * @brief Shared pointer for a murcl_easy instance.
   *
   * The rest of murcl expects an easy::ptr to avoid issues of ownership.
   */
   typedef boost::shared_ptr<easy> ptr;

   /**
   * @brief Go make me an easy API object; simples!
   */
   easy() : handle_(0)
   {
      // if the global env is not configured all bets are off
      if(!global_singleton::instance().isok())
      {
         throw std::runtime_error("Failed to initialise the LibCurl library");
      }

      // attempt to create an interface handle
      handle_ = curl_easy_init();
      if(0 == handle_)
      {
         throw std::runtime_error("Failed to initialise the LibCurl 'easy' API");
      }
   }

   /**
   * @brief Set a LibCurl option
   *
   * @param OptionTraits
   *     This template parameter defines the option being set
   * @param val
   *     The value being set. Its type is defined by OptionTraits::value_type
   */
   template <typename OptionTraits>
   void set_option(typename OptionTraits::value_type const & val)
   {
      optstash_.push_back(
         boost::shared_ptr<option<OptionTraits> > (
            new option<OptionTraits>(handle_, val)
            )
         );
   }

   /**
   * @brief Clean up after ourselves - it's only polite to do so!
   */
   ~easy()
   {
      curl_easy_cleanup(handle_);
   }

   /**
   * @brief Performs a request as defined by the options that were set
   */
   void perform() const
   {
      CURLcode code = curl_easy_perform(handle_);
      validate_result(code, "Unable to perform request");
   }

private:

   // general function to simplify error checking
   void validate_result(CURLcode const code, std::string const & msg) const
   {
      if(CURLE_OK == code) { return; }

      std::ostringstream oss;
      oss
         << msg
         << ": "
         << curl_easy_strerror(code);

      throw std::runtime_error(oss.str());
   }

   CURL * handle_;
   option_stash optstash_;
};

;

}}

#endif //FM_LAST_MOOST_MURCL_EASY_HPP__
