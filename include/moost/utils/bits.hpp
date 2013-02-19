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

// A (growing) collection of bit manipulation utils

#ifndef MOOST_UTILS_NEXT_POWER_OF_TWO_HPP__
#define MOOST_UTILS_NEXT_POWER_OF_TWO_HPP__

#include <stdexcept>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/cstdint.hpp>

namespace moost { namespace utils {

template<typename unsigned_intT>
unsigned_intT next_power_of_two(unsigned_intT num)
{
   // Only allow unsigned types (sign types get all messy!)
   BOOST_STATIC_ASSERT(boost::is_unsigned<unsigned_intT>::value);

   // Only allow types that are 8, 16, 32 or 64 bits
   BOOST_STATIC_ASSERT(
      sizeof(unsigned_intT) == 8 ||
      sizeof(unsigned_intT) == 4 ||
      sizeof(unsigned_intT) == 2 ||
      sizeof(unsigned_intT) == 1
      );

   // Make sure int_max_t is big enough to cope
   BOOST_STATIC_ASSERT(sizeof(boost::intmax_t) == 8);

   // Convert to intmax so we can bitshift without warning for
   // types that have less bits that we are going to manupulate
   boost::intmax_t intmax = num;

   --intmax;

   switch(sizeof(unsigned_intT))
   {
   case 8: // 64 bit
      intmax |= intmax >> 32;
   case 4: // 32 bit
      intmax |= intmax >> 16;
   case 2: // 16 bit
      intmax |= intmax >> 8;
   case 1: // 08 bit
      intmax |= intmax >> 4;
      intmax |= intmax >> 2;
      intmax |= intmax >> 1;
      break;
   }

   return static_cast<unsigned_intT>(++intmax);
}

template<typename unsigned_intT>
bool is_power_of_two(unsigned_intT num)
{
   // Only allow unsigned types
   BOOST_STATIC_ASSERT(boost::is_unsigned<unsigned_intT>::value);

   return (0 == num) || (0 == ((num - 1) & num));
}

template<typename unsigned_intT>
signed char msb_set(unsigned_intT num)
{
   // Only allow unsigned types
   BOOST_STATIC_ASSERT(boost::is_unsigned<unsigned_intT>::value);

   signed char cnt = -1; // No bits set
   while(num) { ++cnt; num >>= 1; }

   return cnt; // LSB is 0 and MSB is N-1 where N is the number of bits
}

}}

#endif // MOOST_UTILS_NEXT_POWER_OF_TWO_HPP__
