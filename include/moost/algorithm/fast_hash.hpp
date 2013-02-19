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

#ifndef MOOST_ALGORITHM_FAST_HASH_HPP
#define MOOST_ALGORITHM_FAST_HASH_HPP

/**
 * \file fast_hash.h
 * A really fast hash function.
 * can be used either as free function (fast_hash) or as function FastHash
 * Examples:
\code
  // simple hash function
  int toHash = 10;
  size_t hash = moost::algorithm::fast_hash( &toHash, sizeof(int) );

  // or as functor
  boost::dense_hash_map<
       int, // the key
       int, // the value
       moost::algorithm::FastHash // the hashing function
     > aMap;

  aMap[34341] = 3;
\endcode
* If you plan to use a non-pod type (be VERY careful!) for the key
* you can disable the static check by declaring MOOST_FASTHASH_NO_ISPOD_CHECK
\note This is \b sdbm from from http://www.cs.yorku.ca/~oz/hash.html
 */

#include <functional>
#include <string>

#include <boost/type_traits/is_pod.hpp>
#include <boost/static_assert.hpp>

namespace moost { namespace algorithm {

//////////////////////////////////////////////////////////////////////////////
// *** BIG IMPORTANT WARNING!!! ***
// If you plan to use fast_hash to hash a custom type, including structs you
// need to understand that they can and probably will contain padding. Since
// the compiler is not at liberty to initialise padding, if you create two
// instances of a struct and initialise each member with the same value there
// is a very good change when you hash each instance you will generate
// different digests. The only way to make this safe is to use memset to zero
// out the whole object first. Note that struct foo = {0} does NOT set all
// bytes in the objects memory to 0, it does a member by member initialisation
// and thus padding is still uninitialised.
// Also, this should NOT be used for non-POD types, since the memory layout
// of a non-POD type is compiler specific and there can be no assumptions
// made. If you need to hash a custom object you are better off making an
// array of bytes out of the various member values that should form part
// of the hash and then send that to fast_hash. For example, if we have
// a non-POD class that contains of key/value pairs and the key is made up of
// 2 integer values (a composite key) this would be one safe way to create
// a hash from the key...
//       int32_t i32s[] = {key.first, key.second};
//       size_ t h = moost::algorithm::fast_hash(i32s, sizeof(i32s));

//////////////////////////////////////////////////////////////////////////////

static const size_t DEFAULT_SEED = 5381;

//#define moost_fasthash_default_seed__ 5381

/// free hash function
inline size_t fast_hash(const void* data_in, size_t size, size_t seed = DEFAULT_SEED)
{
   const unsigned char*    data = (const unsigned char*) data_in;

// [13/6/2011 ricky] Not really sure why this is unsigned int (it's not portable)
//                   but I've added the explicit cast to shut up Windows!
#ifdef WIN32
   unsigned int h = (unsigned int) seed;
#else
   unsigned int h = seed;
#endif

   while (size > 0) {
      size--;
      h = (h << 16) + (h << 6) - h + (unsigned) data[size];
   }
   return h;
}

/// functor that uses fast_hash
template <size_t TSeed = DEFAULT_SEED>
struct FastHashFunctor
{
   template <typename T>
   size_t operator()(const T& p) const
   {
#ifndef MOOST_FASTHASH_NO_ISPOD_CHECK
      BOOST_STATIC_ASSERT((boost::is_pod<T>::value));
#endif

      return fast_hash(&p, sizeof(T), TSeed);
   }

   size_t operator()( const void* key, size_t size ) const
   { return fast_hash(key, size, TSeed); }

   // overrides default seed
   size_t operator()( const void* key, size_t size, size_t seed ) const
   { return fast_hash(key, size, seed); }

   // specialization for strings
   size_t operator()( const std::string& str) const
   { return fast_hash( str.data(), str.size(), TSeed ); }

   // specialization for strings, override seed
   size_t operator()( const std::string& str, size_t seed) const
   { return fast_hash( str.data(), str.size(), seed ); }
};

//#undef moost_fasthash_default_seed__

typedef FastHashFunctor<> FastHash;

//////////////////////////////////////////////////////////////////////////

}} // end of namespaces

#endif // MOOST_ALGORITHM_FAST_HASH_HPP
