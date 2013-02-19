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
* @file option.hpp
* @brief A Moost C++ wrapper that represents a LibCurl option
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*/

#ifndef FM_LAST_MOOST_MURCL_OPTION_HPP__
#define FM_LAST_MOOST_MURCL_OPTION_HPP__

#include <vector>

#include <curl/curl.h>

#include <boost/shared_ptr.hpp>

#include "option_setter.hpp"

namespace moost { namespace murcl {

/**
* @brief This is a polymorphic base class for options
*
* Each option has to be stashed. Although LibCurl can take a copy of each
* option's value this is not probably not the most efficient thing to do
* and since we already have the option anyway we just need to shove it in
* to a container. Problem: each option is a different type. Solution: each
* option derived from this trivial base, which means we can then store a
* polymorphic pointer in our generic container (a vector) and; thus, preserve
* the value of the option for the required lifetime in a simple manner.
*/
class option_base
{
public:
   virtual ~option_base() {}
};

/**
* @brief This is our option stash, used to store the options.
*/
typedef std::vector<boost::shared_ptr<option_base> > option_stash;

/**
* @brief This class is an abstract representation of a LibCurl option.
*        Upon construction it requires an interface handle and a value,
*        which it will use to set the appropriate LibCurl option.
*
*  @param OptTraits
*     Represents the traits of the option being set
*/
template <typename OptTraits>
class option : public option_base
{
public:
   typedef OptTraits option_traits;

   /*
   * @brief Construct and set a LibCurl option
   *
   * @param handle
   *     a valid LibCurl interface handle
   * @param val
   *     the value being set for this option
   */
   option(
      typename option_traits::handle_type handle,
      typename option_traits::value_type const & val
      )
      : val_(val)
   {
      option_setter<typename option_traits::curl_option,
         typename option_traits::value_type>::set(handle, val_);
   }

private:
   typename option_traits::value_type val_;
};

}}

#endif //FM_LAST_MOOST_MURCL_OPTION_HPP__
