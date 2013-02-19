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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_DENSE_HASH_MAP_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_DENSE_HASH_MAP_HPP__

#include <string>
#include <vector>
#include <stdexcept>
#include <iterator>

#include <boost/type_traits/is_pod.hpp>
#include <boost/noncopyable.hpp>

#include "section_writer_base.hpp"
#include "pod_pair.hpp"

namespace moost { namespace container {

/**
 * Memory-mapped dataset section representing a POD dense hash map
 *
 * This section provides extremely fast lookups at the expense
 * of consuming more memory. It is suitable for medium sized
 * high-performance hash maps. Thanks to some simplifications
 * that could be made due to the read-only character of the
 * memory-mapped dense hash map, read access is about 30% faster
 * than using google's dense hash map implementation.
 *
 * This is not a multimap, so there can only be one value per key.
 * Just sayin' in case that wasn't obvious. ;)
 */
template <typename Key, typename T, class HashFcn = MMD_DEFAULT_HASH_FCN<Key> >
class mmd_dense_hash_map : public boost::noncopyable
{
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<Key>::value, "mmd_dense_hash_map<> template can only handle POD key types");
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<T>::value, "mmd_dense_hash_map<> template can only handle POD value types");

   friend class const_iterator;

public:
   static const size_t MMD_HASH_ALIGNMENT = 16;

   static float MAX_POPULATION_RATIO()
   {
      // lower values result in faster lookups and higher memory usage
      return 0.8;
   }

   typedef Key key_type;
   typedef T mapped_type;
   typedef pod_pair<Key, T> value_type;

   typedef size_t size_type;

   typedef const value_type& const_reference;
   typedef const value_type* const_pointer;

   typedef std::forward_iterator_tag iterator_category;

   class const_iterator
   {
      friend class mmd_dense_hash_map;

   public:
      const_reference operator* () const
      {
         return *m_it;
      }

      const_pointer operator-> () const
      {
         return &(operator*());
      }

      const_iterator& operator++ ()
      {
         ++m_it;
         skip();
         return *this;
      }

      const_iterator operator++ (int)
      {
         const_iterator tmp(*this);
         ++*this;
         return tmp;
      }

      bool operator== (const const_iterator& it) const
      {
         return m_it == it.m_it;
      }

      bool operator!= (const const_iterator& it) const
      {
         return !(*this == it);
      }

   private:
      const_iterator(const mmd_dense_hash_map& map, const value_type *it, const value_type *end)
         : m_it(it)
         , m_end(end)
         , m_map(map)
      {
         skip();
      }

      void skip()
      {
         while (m_it != m_end && m_it->first == m_map.empty_key())
         {
            ++m_it;
         }
      }

      const value_type *m_it;
      const value_type *m_end;
      const mmd_dense_hash_map& m_map;
   };

   struct HashingPolicy
   {
      HashingPolicy(const key_type& k, size_type size)
         : m_size(size)
         , m_hash_mask(size - 1)
         , m_index(HashFcn()(k) & m_hash_mask)
         , m_iter(0)
      {
      }

      HashingPolicy& operator++()
      {
         if (++m_iter < m_size)
         {
            m_index = (m_index + m_iter) & m_hash_mask;
         }
         else
         {
            m_index = m_size;
         }

         return *this;
      }

      size_type operator() () const
      {
         return m_index;
      }

      size_type iter() const
      {
         return m_iter;
      }

   private:
      const size_type m_size;
      const size_type m_hash_mask;
      size_type m_index;
      size_type m_iter;
   };

public:
   class writer : public mmd_section_writer_base
   {
   public:
      writer(memory_mapped_dataset::writer& wr, const std::string& name, const key_type& empty_key, float max_population_ratio = MAX_POPULATION_RATIO(), size_t alignment = MMD_HASH_ALIGNMENT)
         : mmd_section_writer_base(wr, name, "mmd_dense_hash_map", alignment)
         , m_empty_key(empty_key)
         , m_max_pop_ratio(max_population_ratio)
      {
         if (max_population_ratio < 0.009999 || max_population_ratio > 0.990001)
         {
            rollback();
            throw std::runtime_error("invalid max_population_ratio (must be in [0.01, 0.99])");
         }
         setattr("key_size", sizeof(key_type));
         setattr("mapped_size", sizeof(mapped_type));
         setattr("elem_size", sizeof(value_type));
      }

      writer& operator<< (const value_type& e)
      {
         insert(e);
         return *this;
      }

      writer& operator<< (const std::pair<Key, T>& e)
      {
         insert(e);
         return *this;
      }

      void insert(const value_type& e)
      {
         if (e.first == m_empty_key)
         {
            throw std::runtime_error("attempt to insert empty key");
         }
         m_values.push_back(e);
      }

      void insert(const std::pair<Key, T>& e)
      {
         value_type v;
         v.first = e.first;
         v.second = e.second;
         insert(v);
      }

      size_type size() const
      {
         return m_values.size();
      }

   protected:
      void pre_commit()      // all the writing actually happens here
      {
         setattr("population", size());

         std::vector<value_type> map;
         build_dense_hash_map(map);
         write(map);

         value_type empty;
         empty_value(empty);
         write(empty);

         setattr("size", map.size());
      }

   private:
      size_type get_optimum_table_size(size_type pop) const
      {
         if (pop <= 1)
         {
            return pop;
         }

         pop /= m_max_pop_ratio;
         size_type opt = 1;

         while (opt < pop)
         {
            opt <<= 1;
         }

         return opt;
      }

      size_type find(const key_type& key, const value_type *begin, size_type size) const
      {
         HashingPolicy hash(key, size);

         for (;;)
         {
            const value_type& target = begin[hash()];

            if (target.first == m_empty_key)
            {
               break;
            }

            if (target.first == key)
            {
               return size;
            }

            ++hash;
         }

         return hash();
      }

      void empty_value(value_type& val) const
      {
         std::memset(&val, 0, sizeof(value_type));
         val.first = m_empty_key;
      }

      void build_dense_hash_map(std::vector<value_type>& target) const
      {
         // TODO: priority sorting; if we insert high priority items first, we will find those again much faster

         std::vector<value_type> rv;

         size_type size = get_optimum_table_size(this->size());

         rv.resize(size);
         value_type empty;
         empty_value(empty);
         std::fill(rv.begin(), rv.end(), empty);

         for (typename std::vector<value_type>::const_iterator it = m_values.begin(); it != m_values.end(); ++it)
         {
            size_type index = find(it->first, &rv[0], rv.size());

            if (index == rv.size())
            {
               throw std::runtime_error("duplicate key detected");
            }

            rv[index] = *it;
         }

         target.swap(rv);
      }

      key_type m_empty_key;
      float m_max_pop_ratio;
      std::vector<value_type> m_values;
   };

   mmd_dense_hash_map()
      : m_size(0)
      , m_population(0)
      , m_begin(0)
   {
   }

   mmd_dense_hash_map(const memory_mapped_dataset& mmd, const std::string& name)
   {
      set(mmd, name);
   }

   void set(const memory_mapped_dataset& mmd, const std::string& name)
   {
      const memory_mapped_dataset::section_info& info = mmd.find(name, "mmd_dense_hash_map");

      if (info.getattr<size_t>("key_size") != sizeof(key_type))
      {
         throw std::runtime_error("wrong key size for dense_hash_map " + name + " in dataset " + mmd.description());
      }

      if (info.getattr<size_t>("mapped_size") != sizeof(mapped_type))
      {
         throw std::runtime_error("wrong mapped size for dense_hash_map " + name + " in dataset " + mmd.description());
      }

      if (info.getattr<size_t>("elem_size") != sizeof(value_type))
      {
         // shouldn't happen unless we run into an alignment mismatch
         throw std::runtime_error("wrong element size for dense_hash_map " + name + " in dataset " + mmd.description());
      }

      m_size = info.getattr<size_type>("size");
      m_population = info.getattr<size_type>("population");
      m_begin = mmd.data<value_type>(info.offset(), m_size + 1);
      m_empty_key = m_begin[m_size].first;
   }

   void warm_cache() const
   {
      memory_mapped_dataset::warm_cache(m_begin, m_begin + m_size);
   }

   const_iterator begin() const
   {
      return const_iterator(*this, m_begin, m_begin + m_size);
   }

   const_iterator end() const
   {
      return const_iterator(*this, m_begin + m_size, m_begin + m_size);
   }

   size_type size() const
   {
      return m_population;
   }

   size_type capacity() const
   {
      return m_size;
   }

   bool empty() const
   {
      return size() == 0;
   }

   const key_type& empty_key() const
   {
      return m_empty_key;
   }

   const_iterator find(const key_type& key) const
   {
      return const_iterator(*this, m_begin + find(key, m_begin, m_size), m_begin + m_size);
   }

   const mapped_type& operator[] (const key_type& key) const
   {
      size_type index = find(key, m_begin, m_size);

      if (index < m_size)
      {
         return m_begin[index].second;
      }

      throw std::runtime_error("no such key");
   }

private:
   size_type find(const key_type& key, const value_type *begin, size_type size) const
   {
      HashingPolicy hash(key, size);

      for (;;)
      {
         const value_type& target = begin[hash()];

         if (target.first == key)
         {
            break;
         }

         if (target.first == m_empty_key)
         {
            return size;
         }

         ++hash;
      }

      return hash();
   }

   size_type m_size;
   size_type m_population;
   const value_type *m_begin;
   key_type m_empty_key;
};

}}

#endif
