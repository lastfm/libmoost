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

#ifndef MOOST_ALGORITHM_KETAMA_PARTITIONER_HPP__
#define MOOST_ALGORITHM_KETAMA_PARTITIONER_HPP__

#include <boost/random/mersenne_twister.hpp>

#include "partitioner.hpp"

#include <vector>
#include <algorithm>
#include <limits>

namespace moost { namespace algorithm {

/// @brief ketama_partitioner implements consistent hashing, such that the addition or removal of buckets does not
/// significantly change the mapping of keys to buckets.  By using consistent hashing, only K/n keys need to be
/// remapped on average, where K is the number of keys, and n is the number of buckets.
template<typename T>
class ketama_partitioner: public partitioner<T>
{
private:

  /* erikf raised a concern about boost::mt19937 potentially changing and thus
   * included a copy of mersenne_twister.hpp in moost. Not only is this ugly,
   * it has also been the source of link-level errors with recent versions of
   * boost due to refactoring of the implementation.
   *
   * The algorithm itself is well-defined and extremely unlikely to ever be
   * changed. The only thing that's slightly more probable to change (even
   * though I don't think it ever will) is the default seed, which is why we
   * keep a copy of the "original" seed here and explicitly seed the RNGs.
   */
  static uint32_t default_seed()
  {
     return 5489u;
  }

  struct bucket_hash
  {
    bucket_hash(size_t bucket_, size_t hash_) :
      bucket(bucket_), hash(hash_)
    {
    }
    size_t bucket;
    size_t hash;
    bool operator <(const bucket_hash & other) const
    {
      return hash < other.hash;
    }
  };

  std::vector<bucket_hash> m_bhashes;

  // stolen from http://isthe.com/chongo/tech/comp/fnv/
  unsigned int fnv_hash(const void *key, size_t len) const
  {
    const unsigned char *p = (const unsigned char*) key;
    unsigned int h = 2166136261UL;

    for (size_t i = 0; i != len; i++)
      h = (h * 16777619) ^ p[i];

    return h;
  }

public:

  ketama_partitioner(size_t num_buckets, size_t num_hashes = 4096)
  : partitioner<T> (num_buckets)
  {
    boost::mt19937 gen(default_seed());

    // for each bucket
    for (size_t i = 0; i != num_buckets; ++i)
    {
      // for each hash
      for (size_t j = 0; j != num_hashes; ++j)
      {
        // add it to the ring
        m_bhashes.push_back(bucket_hash(i, gen()));
      }
    }

    // now sort the whole darn thing
    std::sort(m_bhashes.begin(), m_bhashes.end());
  }

  template <typename Y>
  ketama_partitioner( const std::vector<Y>& buckets, size_t num_hashes = 4096)
  : partitioner<T>( buckets.size() )
  {
     boost::mt19937 gen(default_seed());
     typename std::vector<Y>::const_iterator it;
     // for each bucket
     size_t i = 0;
     for ( it = buckets.begin(); it != buckets.end(); ++it, ++i )
     {
        gen.seed( fnv_hash( &(*it), sizeof(Y)) );
        // for each hash
        for (size_t j = 0; j != num_hashes; ++j)
        {
           // add it to the ring
           m_bhashes.push_back(bucket_hash(i, gen()));
        }
     }

     // now sort the whole darn thing
     std::sort(m_bhashes.begin(), m_bhashes.end());
  }

  /// Specialization for bucket of strings
  ketama_partitioner( const std::vector<std::string>& buckets, size_t num_hashes = 4096)
     : partitioner<T>( buckets.size() )
  {
     boost::mt19937 gen(default_seed());
     std::vector<std::string>::const_iterator it;
     // for each bucket
     size_t i = 0;
     for ( it = buckets.begin(); it != buckets.end(); ++it, ++i )
     {
        gen.seed( fnv_hash(it->c_str(), it->length()) );
        // for each hash
        for (size_t j = 0; j != num_hashes; ++j)
        {
           // add it to the ring
           m_bhashes.push_back(bucket_hash(i, gen()));
        }
     }

     // now sort the whole darn thing
     std::sort(m_bhashes.begin(), m_bhashes.end());
  }

  size_t partition(const T & key) const
  {
    typename std::vector<bucket_hash>::const_iterator it = std::lower_bound(m_bhashes.begin(), m_bhashes.end(),
        bucket_hash(0, fnv_hash(&key, sizeof(T))));
    if (it == m_bhashes.end())
      it = m_bhashes.begin();
    return it->bucket;
  }
};

/// Specialization for string key
template <>
inline size_t ketama_partitioner<std::string>::partition(const std::string& key) const
{
   std::vector<bucket_hash>::const_iterator it = std::lower_bound(m_bhashes.begin(), m_bhashes.end(),
      bucket_hash(0, fnv_hash(key.c_str(), key.length())));
   if (it == m_bhashes.end())
      it = m_bhashes.begin();
   return it->bucket;
}

}} // moost::algorithm

#endif // MOOST_ALGORITHM_KETAMA_PARTITIONER_HPP__
