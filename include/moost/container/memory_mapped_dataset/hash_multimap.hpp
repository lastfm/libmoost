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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_HASH_MULTIMAP_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_HASH_MULTIMAP_HPP__

#include <string>
#include <vector>
#include <stdexcept>
#include <iterator>

#include <boost/type_traits/is_pod.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>

#include "section_writer_base.hpp"
#include "pod_pair.hpp"

namespace moost { namespace container {

/**
 * Memory-mapped dataset section representing a POD hash multimap
 *
 * This is a little more versatile than the mmd_vector if you're really
 * looking for fast lookup by a key instead of lookup by index.
 *
 * It is supposed to offer faster lookup than the non-hashed map at
 * the expense of not having the key-value pairs in a deterministic
 * order when iterating over the map.
 */
template <typename Key, typename T, class HashFcn = MMD_DEFAULT_HASH_FCN<Key>, typename IndexType = boost::uint64_t>
class mmd_hash_multimap : public boost::noncopyable
{
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<Key>::value, "mmd_hash_multimap<> template can only handle POD key types");
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<T>::value, "mmd_hash_multimap<> template can only handle POD value types");

public:
   static const size_t MMD_HASH_ALIGNMENT = 16;
   static const size_t MMD_HASH_BITS = 10;   // you'll usually want to pick this larger for huge tables

   typedef Key key_type;
   typedef T mapped_type;
   typedef pod_pair<Key, T> value_type;

   typedef const value_type *const_iterator;
   typedef size_t size_type;

   typedef const T& const_reference;
   typedef const T* const_pointer;

   typedef IndexType index_type;

   typedef std::bidirectional_iterator_tag iterator_category;

private:
   static bool compare(const value_type& a, const value_type& b)
   {
      return a.first < b.first;
   }

public:
   class writer : public mmd_section_writer_base
   {
   public:
      writer(memory_mapped_dataset::writer& wr, const std::string& name, size_t hash_bits = MMD_HASH_BITS, size_t alignment = MMD_HASH_ALIGNMENT)
         : mmd_section_writer_base(wr, name, "mmd_hash_multimap", alignment)
         , m_size(0)
         , m_hash_mask((1 << hash_bits) - 1)
      {
         if (hash_bits < 1 || hash_bits > 8*sizeof(size_type))
         {
            throw std::runtime_error("invalid value for hash_bits");
         }

         m_values.resize(1 << hash_bits);

         setattr("key_size", sizeof(key_type));
         setattr("mapped_size", sizeof(mapped_type));
         setattr("elem_size", sizeof(value_type));
         setattr("index_elem_size", sizeof(index_type));
         setattr("hash_bits", hash_bits);
      }

      writer& operator<< (const value_type& e)
      {
         insert(e);
         return *this;
      }

      void insert(const value_type& e)
      {
         m_values[HashFcn()(e.first) & m_hash_mask].push_back(e);
         ++m_size;
      }

      size_type size() const
      {
         return m_size;
      }

   protected:
      void pre_commit()      // all the writing actually happens here
      {
         std::vector<index_type> index;

         index.reserve(m_values.size() + 1);
         index.push_back(0);

         for (typename std::vector< std::vector<value_type> >::iterator vi = m_values.begin(); vi != m_values.end(); ++vi)
         {
            std::sort(vi->begin(), vi->end(), compare);
            write(*vi);
            index.push_back(index.back() + vi->size());
         }

         write(index);

         setattr("size", m_size);
      }

   private:
      size_type m_size;
      const size_type m_hash_mask;
      std::vector< std::vector<value_type> > m_values;
   };

   mmd_hash_multimap()
      : m_index(0)
      , m_hash_mask(0)
      , m_begin(0)
      , m_end(0)
      , m_hash_bits(0)
   {
   }

   mmd_hash_multimap(const memory_mapped_dataset& mmd, const std::string& name)
   {
      set(mmd, name);
   }

   void set(const memory_mapped_dataset& mmd, const std::string& name)
   {
      const memory_mapped_dataset::section_info& info = mmd.find(name, "mmd_hash_multimap");

      if (info.getattr<size_t>("key_size") != sizeof(key_type))
      {
         throw std::runtime_error("wrong key size for hash_multimap " + name + " in dataset " + mmd.description());
      }

      if (info.getattr<size_t>("mapped_size") != sizeof(mapped_type))
      {
         throw std::runtime_error("wrong mapped size for hash_multimap " + name + " in dataset " + mmd.description());
      }

      if (info.getattr<size_t>("elem_size") != sizeof(value_type))
      {
         throw std::runtime_error("wrong element size for hash_multimap " + name + " in dataset " + mmd.description());
      }

      if (info.getattr<size_t>("index_elem_size") != sizeof(index_type))
      {
         throw std::runtime_error("wrong index element size for hash_multimap " + name + " in dataset " + mmd.description());
      }

      m_hash_bits = info.getattr<size_type>("hash_bits");
      m_hash_mask = (1 << m_hash_bits) - 1;

      size_type index_size = (1 << m_hash_bits) + 1;
      size_type table_size = info.getattr<size_type>("size");

      m_begin = mmd.data<value_type>(info.offset(), table_size);
      m_end = m_begin + table_size;
      m_index = mmd.data<index_type>(info.offset() + sizeof(value_type)*table_size, index_size);
   }

   size_type hash_bits() const
   {
      return m_hash_bits;
   }

   void warm_cache() const
   {
      memory_mapped_dataset::warm_cache(m_begin, m_end);
   }

   const_iterator begin() const
   {
      return m_begin;
   }

   const_iterator end() const
   {
      return m_end;
   }

   size_type size() const
   {
      return m_end - m_begin;
   }

   bool empty() const
   {
      return size() == 0;
   }

   const_iterator lower_bound(const key_type& x) const
   {
      if (m_index)
      {
         size_t hash = HashFcn()(x) & m_hash_mask;
         value_type search;
         search.first = x;
         return std::lower_bound(&m_begin[m_index[hash]], &m_begin[m_index[hash + 1]], search, compare);
      }
      else
      {
         return m_end;
      }
   }

   // TODO: anyone feel free to add more methods for better compatibility with std::multimap<> :-)

private:
   const index_type *m_index;
   size_t m_hash_mask;
   const_iterator m_begin;
   const_iterator m_end;
   size_type m_hash_bits;
};

}}

#endif
