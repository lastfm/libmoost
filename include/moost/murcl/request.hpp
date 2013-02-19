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
* @file request.hpp
* @brief Represents an abstraction of a LibCurl request
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_REQUEST_HPP__
#define FM_LAST_MOOST_MURCL_REQUEST_HPP__

#include <stdexcept>
#include <iostream>
#include <boost/bind.hpp>

#include <curl/curl.h>

#include "easy.hpp"
#include "response.hpp"

namespace moost { namespace murcl {

class request
{
public:
   /**
   * @brief This class represents a LibCurl request
   *
   * @param peasy
   *     a pointer to a LibCurl easy interface handle
   */
   request(easy::ptr peasy)
      : peasy_(peasy)
   {
   }

   /**
   * @brief Performs a LibCurl request and generates a response
   *
   * @param resp
   *     a reference to a response object
   */
   void perform(response & resp)
   {
      peasy_->set_option<easyopt::writedata>(resp());
      peasy_->set_option<easyopt::writecb>(response::callback);
      peasy_->perform();
   }

private:
   easy::ptr peasy_;
};

}}

#endif // FM_LAST_MOOST_MURCL_REQUEST_HPP__
