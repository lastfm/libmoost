/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       log2.hpp
 * \author     Marcus Holland-Moritz (marcus@last.fm)
 * \copyright  Copyright © 2008-2013 Last.fm Limited
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

#ifndef MOOST_MATH_INTEGER_LOG2_HPP
#define MOOST_MATH_INTEGER_LOG2_HPP

#include <cassert>
#include <climits>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_unsigned.hpp>

namespace moost { namespace math { namespace integer {

#ifdef __GNUC__
namespace detail {

inline int count_leading_zeroes(unsigned arg)
{
   return __builtin_clz(arg);
}

inline int count_leading_zeroes(unsigned long arg)
{
   return __builtin_clzl(arg);
}

inline int count_leading_zeroes(boost::ulong_long_type arg)
{
   return __builtin_clzll(arg);
}

}
#endif

template <typename T>
inline unsigned log2_compat(T arg)
{
   BOOST_STATIC_ASSERT(boost::is_unsigned<T>::value);

   // This could be optimised using binary search, but I doubt
   // it'll ever get used... ;)
   unsigned r = 0;

   while (arg > static_cast<T>(1))
   {
      arg >>= 1;
      ++r;
   }

   return r;
}

/**
 * Fast base-2 integer logarithm implementation
 *
 * There is no run-time check catching the case of passing in zero
 * and the result is undefined if you actually do so.
 *
 * There is, however, an assertion that checks arg > 0 for debug
 * builds.
 *
 * \param arg                An unsigned integer argument.
 *
 * \returns The (truncated) base-2 logarithm of \p arg.
 */
template <typename T>
inline unsigned log2(T arg)
{
   BOOST_STATIC_ASSERT(boost::is_unsigned<T>::value);

   assert(arg > 0U);

#ifdef __GNUC__
   return (8U*unsigned(sizeof(T)) - 1U) - detail::count_leading_zeroes(arg);
#else
   return log2_compat(arg);
#endif
}

}}}

#endif
