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

#ifndef MOOST_ALGORITHM_PARTITIONER_HPP__
#define MOOST_ALGORITHM_PARTITIONER_HPP__

namespace moost { namespace algorithm {

/** @brief A partitioner can assign templatized types to a specified number of buckets
 *
 * This is useful, for example, for spreading items across servers.  partitioner is a virtual class,
 * and the partition method must be implemented by subclasses.
 */
template <typename T>
class partitioner
{
private:

  size_t m_num_buckets;

public:

  /// Constructs a partitioner.
  /// @param num_buckets the number of buckets to partition into.
  partitioner(size_t num_buckets)
  : m_num_buckets(num_buckets) {}

  virtual ~partitioner() {}

  /// Returns a bucket for the given key, from 0 to num_buckets - 1.
  /// Must be overridden by implementing partitioners.
  virtual size_t partition(const T & key) const = 0;

  /// Return the number of buckets the partitioner will assign keys to.
  size_t num_buckets() const
  {
    return m_num_buckets;
  }
};

}} // moost::partition

#endif // MOOST_ALGORITHM_PARTITIONER_HPP__
