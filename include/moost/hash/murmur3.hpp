/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       murmur3.hpp
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

#ifndef MOOST_HASH_MURMUR3_HPP
#define MOOST_HASH_MURMUR3_HPP

#include <vector>

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pod.hpp>

namespace moost { namespace hash {

/**
 * Fast murmur3 hash implementation
 *
 * This class implements the 32-bit murmur3 hash algorithm and is
 * in fact derived from the original code:
 *
 *   https://github.com/PeterScott/murmur3
 *
 * The main difference is that the template function compute32()
 * can be more easily optimised by the compiler. Especially if
 * you're hashing constant-size types (even more so if they're
 * a multiple of 32 bits in size), the code will be significantly
 * faster than the original implementation.
 */
class murmur3
{
private:
   static uint32_t fmix(uint32_t h)
   {
     h ^= h >> 16;
     h *= 0x85ebca6b;
     h ^= h >> 13;
     h *= 0xc2b2ae35;
     h ^= h >> 16;

     return h;
   }

   static uint32_t getblock(const uint32_t *p, int i)
   {
     return p[i];
   }

   static uint32_t rotl32 (uint32_t x, int8_t r)
   {
     return (x << r) | (x >> (32 - r));
   }

public:
   static uint32_t compute32(const void *key, size_t len, uint32_t seed)
   {
      const uint8_t *data = reinterpret_cast<const uint8_t *>(key);
      const int nblocks = len/4;

      uint32_t h1 = seed;

      const uint32_t c1 = 0xcc9e2d51;
      const uint32_t c2 = 0x1b873593;

      //----------
      // body

      const uint32_t *blocks = reinterpret_cast<const uint32_t *>(data + nblocks*4);

      for(int i = -nblocks; i; i++)
      {
        uint32_t k1 = getblock(blocks, i);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1*5+0xe6546b64;
      }

      //----------
      // tail

      const uint8_t *tail = reinterpret_cast<const uint8_t*>(data + nblocks*4);

      uint32_t k1 = 0;

      switch (len & 3)
      {
         case 3: k1 ^= tail[2] << 16;
         case 2: k1 ^= tail[1] << 8;
         case 1: k1 ^= tail[0];
                 k1 *= c1; k1 = rotl32(k1, 15); k1 *= c2; h1 ^= k1;
      }

      //----------
      // finalization

      h1 ^= len;

      return fmix(h1);
   }

   template <typename T>
   static uint32_t compute32(const T& key, uint32_t seed)
   {
      BOOST_STATIC_ASSERT(boost::is_pod<T>::value);
      return compute32(&key, sizeof(key), seed);
   }

   static uint32_t compute32(const std::string& key, uint32_t seed)
   {
      return compute32(key.data(), key.size(), seed);
   }

   template <typename T>
   static uint32_t compute32(const std::vector<T>& key, uint32_t seed)
   {
      BOOST_STATIC_ASSERT(boost::is_pod<T>::value);
      return compute32(&key[0], sizeof(T)*key.size(), seed);
   }

   template <typename T, unsigned Seed = 0U>
   struct hash32
   {
      size_t operator() (const T& key) const
      {
         return compute32(key, Seed);
      }
   };
};

}}

#endif
