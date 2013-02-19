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

#ifndef MOOST_CONTAINER_BIT_FILTER_HPP_
#define MOOST_CONTAINER_BIT_FILTER_HPP_

#include <vector>
#include <iostream>

#include <bm/bm.h>
#include <bm/bmserial.h>

namespace moost { namespace container {

namespace bit_filter_types
{

/*!
 * Used as the default hash function when no other is provided. Basically
 * does absolutely nothing and should be optimized away by the compiler.
 */

struct default_hash
{
   template <typename itemT>
   size_t operator()(itemT const & t) const
   {
      return t;
   }
};

/*!
 * Represents a buffer for serialisation and deserialisation of a bit_filter
 */
typedef std::vector<unsigned char> serial_buffer_t;

}

template <typename itemT, typename hashT = bit_filter_types::default_hash>
class bit_filter
{
public:
   typedef itemT item_type;
   typedef hashT hash_type;
   typedef bit_filter<item_type, hash_type> this_type;
   typedef bit_filter_types::serial_buffer_t serial_buffer_t;

   /*!
    * c_tor
    */
   bit_filter(
      size_t const size,
      hash_type const & ht = hash_type()
      ) : size_(size), ht_(ht)
   {
   }

   /*!
    * c_tor
    */
   template <typename inputIteratorT>
   bit_filter(
      size_t const size,
      inputIteratorT beg,
      inputIteratorT end,
      bool boptimise = false,
      hash_type const & ht = hash_type()
      ) : size_(size), ht_(ht)
   {
      insert(beg, end, boptimise);
   }

   /*!
    * cc_tor
    */
   bit_filter(bit_filter const & rhs)
       : size_(rhs.size_), bv_(rhs.bv_), ht_(rhs.ht_)
   {
   }

   /*!
    * Assignment
    */
   bit_filter & operator = (bit_filter const & rhs)
   {
      if(this != &rhs)
      {
         size_ = rhs.size_;
         bv_ = rhs.bv_;
         ht_ = rhs.ht_;
      }

      return *this;
   }

   /*!
    * Insert an item in to the filter
    */
   void insert(item_type const & t, bool boptimise = false)
   {
      if(!find(t))
      {
         bv_.set(ht_(t) % size_);
      }

      if(boptimise) { optimize(); }
   }

   /*!
    * Insert items in to the filter using iterators of a container, finding items are preserved
    */
   template <typename inputIteratorT>
   void insert(inputIteratorT beg, inputIteratorT end, bool boptimise = false)
   {
      while(beg != end)
      {
         insert(*beg);
         ++beg;
      }

      if(boptimise) { optimize(); }
   }

   /*!
    * Insert items in to the filter using iterators of a container, finding items are cleared first
    */
   template <typename inputIteratorT>
   void replace(inputIteratorT beg, inputIteratorT end)
   {
      clear();
      insert(beg, end);
   }

   /*!
    * Unset all the bits in the filter
    */
   void clear()
   {
      bv_.clear();
   }

   /*!
    * The size (number of bits) of the filter
    */
   size_t size() const
   {
      return size_;
   }

   /*!
    * Checks for the findance of a single item
    */
   bool find(item_type const & t) const
   {
      return bv_.test(ht_(t) % size_);
   }

   /*!
    * Checks for the findance of multiple items in a container, returning a
    * count of the number of items that match
    */
   template <typename inputIteratorT>
   size_t find(inputIteratorT beg, inputIteratorT end) const
   {
      size_t cnt = 0;
      while(beg != end)
      {
         if(find(*beg))
         {
            ++cnt;
         }

         ++beg;
      }

      return cnt;
   }

   /*!
    * Checks for the findance of multiple items in a container, returning a
    * count of the number of items that match as well as adding all matching
    * items to an output container using the output iterator
    */
   template <typename inputIteratorT, typename outputIteratorT>
   size_t find(inputIteratorT beg, inputIteratorT end, outputIteratorT out) const
   {
      size_t cnt = 0;
      while(beg != end)
      {
         if(find(*beg))
         {
            ++cnt;
            *out = *beg;
            ++out;
         }

         ++beg;
      }

      return cnt;
   }

   /*!
    * Checks if any bits in rhs match
    */
   bool find(this_type const & rhs) const
   {
      return (this == &rhs) ? true : (bm::any_and(bv_, rhs.bv_) != 0);
   }

   /*!
    * If possible will optimize the internal representation of the bitset, saving memory
    */
   void optimize()
   {
      bv_.optimize();
   }

   /*!
    * Serialise the bit filter to a vector of bytes.
    * Optionally, optimize before serialisation.
    * Returns the number of bytes used by serialisation.
    */
   size_t serialize(serial_buffer_t & buf, bool boptimise = true)
   {
      // Since we can't deallocate vector memory and since we initially
      // allocate far more than we'll need we use a temporary that can be
      // thrown away after, so we only used as much memory as needed.

      if(boptimise) { optimize(); }

      bm::bvector<>::statistics st;
      bm::serializer<bm::bvector<> > bvs;

      // Optimize things
      bvs.byte_order_serialization(false); // We don't care about supporting different endianess
      bvs.gap_length_serialization(false); // We don't care about GAP levels
      bvs.set_compression_level(4);         // We want maximum compression

      bv_.calc_stat(&st);

      serial_buffer_t tmp_buf(st.max_serialize_mem);
      size_t const used = bvs.serialize(bv_, &tmp_buf[0], tmp_buf.size());
      buf.resize(used);
      buf.insert(buf.begin(), tmp_buf.begin(), tmp_buf.begin() + used);

      return used;
   }

   /*!
    * Deserialise the bit filter from a vector of bytes.
    * Optionally, once serialised the filter is cleared.
    */
   void deserialize(serial_buffer_t const & buf, bool bclear = true)
   {
      if(bclear) { bv_.clear(); }
      if(!buf.empty())
      {
         bm::deserialize(bv_, &buf[0]);
      }
   }

   /*!
    * Equality operator for the bit filter
    */
   bool operator == (this_type const & rhs) const
   {
      return bv_ == rhs.bv_;
   }

   /*!
    * Inequality operator for the bit filter
    */
   bool operator != (this_type const & rhs) const
   {
      return bv_ != rhs.bv_;
   }

   /*!
    * The amount of memory being used by the internal filter (in bytes).
    * Note that a call to optimise is likely to improve memory usage.
    */
   size_t memory() const
   {
      bm::bvector<>::statistics st;
      bv_.calc_stat(&st);
      return st.memory_used;
   }

   /*!
    * Gets the count of the number of bits actually set.
    */
   size_t count () const
   {
      return bv_.count();
   }

private:
   size_t size_;
   bm::bvector<> bv_;
   hash_type ht_;
};

template <typename itemT, typename hashT>
bit_filter<itemT, hashT> &  operator >> (
   bit_filter<itemT, hashT> & bf,
   typename bit_filter<itemT, hashT>::serial_buffer_t & buf
   )
{
   bf.serialize(buf);
   return bf;
}

template <typename itemT, typename hashT>
bit_filter<itemT, hashT> & operator << (
   bit_filter<itemT, hashT> & bf,
   typename bit_filter<itemT, hashT>::serial_buffer_t const & buf
   )
{
   bf.deserialize(buf);
   return bf;
}

}}

#endif
