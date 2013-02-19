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

#ifndef MOOST_ALGORITHM_VARIABLE_LENGTH_ENCODING_HPP__
#define MOOST_ALGORITHM_VARIABLE_LENGTH_ENCODING_HPP__

#include <stdexcept>
#include <limits>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace moost { namespace algorithm {

/// MIDI-style variable length encoding, inspired from:
/// http://www.borg.com/~jglatt/tech/midifile/vari.htm
struct variable_length_encoding
{
   // DEPRECATED! Use the one below!
   template <class T, class InputIterator>
   inline static void read(T& value, InputIterator & p_in)
   {
      BOOST_STATIC_ASSERT(
            sizeof(T) == 4 &&
            std::numeric_limits<T>::is_integer &&
            std::numeric_limits<T>::is_signed
         );

      value = *p_in & 0x7F; // 1

      // unroll the loop since we know a VLE uint can occupy at most 5 bytes!
      if (*p_in & 0x80)
      {
         ++p_in;
         value = ((value << 7) | (*p_in & 0x7F)); // 2
         if (*p_in & 0x80)
         {
            ++p_in;
            value = ((value << 7) | (*p_in & 0x7F)); // 3
            if (*p_in & 0x80)
            {
               ++p_in;
               value = ((value << 7) | (*p_in & 0x7F)); // 4
               if (*p_in & 0x80)
               {
                  ++p_in;
                  value = ((value << 7) | (*p_in & 0x7F)); // 5
                  if (*p_in & 0x80)
                     throw std::overflow_error("overflow");
               }
            }
         }
      }
      ++p_in;
   }


  template <class InputIterator>
  inline static boost::int32_t read(InputIterator & p_in)
  {
    boost::int32_t value = *p_in & 0x7F; // 1

    // unroll the loop since we know a VLE uint can occupy at most 5 bytes!
    if (*p_in & 0x80)
    {
      ++p_in;
      value = ((value << 7) | (*p_in & 0x7F)); // 2
      if (*p_in & 0x80)
      {
        ++p_in;
        value = ((value << 7) | (*p_in & 0x7F)); // 3
        if (*p_in & 0x80)
        {
          ++p_in;
          value = ((value << 7) | (*p_in & 0x7F)); // 4
          if (*p_in & 0x80)
          {
            ++p_in;
            value = ((value << 7) | (*p_in & 0x7F)); // 5
            if (*p_in & 0x80)
              throw std::overflow_error("overflow");
          }
        }
      }
    }
    ++p_in;
    return value;
  }

  template <class OutputIterator>
  inline static void write(boost::int32_t value, OutputIterator & p_out)
  {
    if ((value & 0xFFFFFF80) == 0) // only occupying lower 7 bits?
    {
      *p_out = static_cast<char>(value); ++p_out;
    }
    else if ((value & 0xFFFFC000) == 0) // 14?
    {
      *p_out = static_cast<char>(value >> 7 | 0x80); ++p_out;
      *p_out = static_cast<char>(value & 0x7F);      ++p_out;
    }
    else if ((value & 0xFFE00000) == 0) // 21!?!
    {
      *p_out = static_cast<char>(value >> 14 | 0x80); ++p_out;
      *p_out = static_cast<char>(value >> 7  | 0x80); ++p_out;
      *p_out = static_cast<char>(value & 0x7F);       ++p_out;
    }
    else if ((value & 0xF0000000) == 0) // 28!?!??!?
    {
      *p_out = static_cast<char>(value >> 21 | 0x80); ++p_out;
      *p_out = static_cast<char>(value >> 14 | 0x80); ++p_out;
      *p_out = static_cast<char>(value >> 7  | 0x80); ++p_out;
      *p_out = static_cast<char>(value & 0x7F);       ++p_out;
    }
    else
    {
      *p_out = static_cast<char>(value >> 28 | 0x80); ++p_out;
      *p_out = static_cast<char>(value >> 21 | 0x80); ++p_out;
      *p_out = static_cast<char>(value >> 14 | 0x80); ++p_out;
      *p_out = static_cast<char>(value >> 7  | 0x80); ++p_out;
      *p_out = static_cast<char>(value & 0x7F);       ++p_out;
    }
  }
};

}} // moost::algorithm

#endif // MOOST_ALGORITHM_VARIABLE_LENGTH_ENCODING_HPP__
