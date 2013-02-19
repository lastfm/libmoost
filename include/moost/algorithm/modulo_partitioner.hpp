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

#ifndef MOOST_ALGORITHM_MODULO_PARTITIONER_HPP__
#define MOOST_ALGORITHM_MODULO_PARTITIONER_HPP__

#include <boost/functional/hash.hpp>
#include "partitioner.hpp"

namespace moost { namespace algorithm {

/// @brief modulo_partitioner hashes the key then modulos the result against the number of buckets.
/// This results in an even spread given a reasonable hasher.
template <typename T>
class modulo_partitioner : public partitioner<T>
{
private:

  boost::hash<T> m_hasher;

public:

  /// constructs a basic_partitioner
  /// @param num_buckets the number of buckets to partition into.
  basic_partitioner(size_t num_buckets)
  : partitioner<T>(num_buckets)
  {
  }

  /// return a bucket for the given key
  size_t partition(const T & key) const
  {
    return m_hasher(key) % this->getNumBuckets();
  }
};

}} // moost::partition

#endif // MOOST_ALGORITHM_MODULO_PARTITIONER_HPP__
