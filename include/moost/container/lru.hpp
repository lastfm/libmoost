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

#ifndef MOOST_CONTAINER_LRU_HPP__
#define MOOST_CONTAINER_LRU_HPP__

#include <list>
#include <limits>

#include <boost/functional/hash.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/function.hpp>

#include "sparse_hash_map.hpp"

namespace moost { namespace container {

   template <typename Key, bool>
   struct get_deleted_key
   {
      Key operator()() const
      { return Key(); } // generic version
   };

   template <typename Key>
   struct get_deleted_key<Key, true>
   {
      Key operator()() const
      { return (std::numeric_limits<Key>::max)(); } // specialized for integrals
   };

   /** @brief an lru is a collection of keys and values, with
   * a max size.  Once the max size is reached, further inserted elements push
   * out least recently used elements.
   * \note IMPORTANT: don't forget to set the deleted key if you expect anything
   * with key = 0!!!!
   * \note IMPORTANT: it's NOT thread safe!
   */
   template<class Key,
      typename Data,
      typename HashFcn = boost::hash<Key> >
   class lru
   {
   private:
      typedef std::list< std::pair< Key, Data > > lru_t;

   public:
      typedef typename lru_t::value_type value_type;
      typedef typename lru_t::value_type::first_type key_type;
      typedef typename lru_t::value_type::second_type mapped_type;

      typedef typename lru_t::iterator iterator;
      typedef typename lru_t::const_iterator const_iterator;
      typedef typename lru_t::reference reference;
      typedef typename lru_t::const_reference const_reference;

   private:
      typedef google::sparse_hash_map< key_type, iterator, HashFcn > hm_key_data;
      typedef boost::function<bool(key_type, const mapped_type & value)> evict_func_t;

      lru_t m_lru; // head = oldest, tail = newest
      hm_key_data       m_key_data;
      size_t            m_max_size;

   public:

      /// Constructs an lru
      lru(size_t max_size = std::numeric_limits<size_t>::max()) : m_max_size(max_size)
      {
         m_key_data.set_deleted_key( get_deleted_key<key_type, boost::is_integral<key_type>::value >()() );
      }

      void set_deleted_key(const key_type & deleted_key)
      {
         m_key_data.set_deleted_key( deleted_key );
      }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // stuff with keys

      /// Gets a value given a key.  Returns false if not in lru.
      bool get(const key_type & key, mapped_type & value, bool bbump = true)
      {
         iterator itr = find(key);

         if(itr == m_lru.end())
            return false;

         // first, move the iterator to the tail of the lru
         if(bbump)
            bump(itr);

         // now get the value
         value = itr->second;
         return true;
      }

      /// Puts a value into the lru, and evicts an old value if necessary.
      /// Any candidate for eviction is passed to the evict function, and only
      /// evicted if the function evaluates true
      void put(const key_type & key, const mapped_type & value, evict_func_t evict_func = evict_func_t())
      {
         insert(key, value, evict_func);
      }

      /// erase an element from the lru
      void erase(const key_type & key)
      {
         iterator itr = find(key);

         if(itr != m_lru.end())
            erase(itr);
      }

      /// gets a value from the lru but doesn't bump it
      bool peek(const key_type & key, mapped_type & value)
      {
         return get(key, value, false);
      }

      // bump something to the top of the lru
      bool bump(const key_type & key)
      {
         iterator itr = find(key);

         if(itr == m_lru.end())
            return false;

         return bump(itr);
      }

      bool exists(const key_type & key) const
      {
         return find(key) != m_lru.end();
      }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // stuff with iterators

      const_iterator find(const key_type & key) const
      {
         typename hm_key_data::const_iterator it_key_data = m_key_data.find(key);
         return it_key_data == m_key_data.end() ? m_lru.end() : it_key_data->second;
      }

      iterator find(const key_type & key)
      {
         typename hm_key_data::iterator it_key_data = m_key_data.find(key);
         return it_key_data == m_key_data.end() ? m_lru.end() : it_key_data->second;
      }

      reference front()
      {
         return m_lru.front();
      }

      const_reference front() const
      {
         return m_lru.front();
      }

      reference back()
      {
         return m_lru.back();
      }

      const_reference back() const
      {
         return m_lru.back();
      }

      std::pair<iterator, bool> insert(const key_type & key, const mapped_type & value, evict_func_t evict_func = evict_func_t())
      {
         if (m_max_size == 0)
            m_lru.end();

         typename hm_key_data::iterator it_key_data = m_key_data.find(key);

         bool const existed = (it_key_data != m_key_data.end());

         if (existed)
         {
            erase(it_key_data->second);
         }

         if (m_key_data.size() == m_max_size)
         {
            if(evict_func)
            {
               typename std::list< std::pair< key_type, mapped_type > >::iterator it;
               for (it = m_lru.begin(); it != m_lru.end(); ++it)
               {
                  if (evict_func(it->first, it->second))
                     break;
               }

               if (it == m_lru.end()) // we couldn't evict anything?  then we can't insert this element
               {
                  evict_func(key, value);
                  return std::make_pair(it, existed);
               }

               erase(it);
            }
            else
            {
               erase(m_lru.begin());
            }
         }

         // insert
         return std::make_pair(
            m_key_data[key] = m_lru.insert( m_lru.end(), std::make_pair(key, value) ),
            existed
         );
      }

      bool bump(iterator itr)
      {
         m_lru.splice(m_lru.end(), m_lru, itr);
         return true;
      }

      void erase(iterator it)
      {
         m_key_data.erase(it->first);
         m_lru.erase(it);
      }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // operators

      mapped_type const & operator[](key_type const & key) const
      {
         iterator itr = find(key);
         if(itr == m_lru.end()) { throw std::runtime_error("key not found"); }
         return itr->second;
      }

      mapped_type & operator[](key_type const & key)
      {
         iterator itr = find(key);
         if(itr == m_lru.end()) { throw std::runtime_error("key not found"); }
         return itr->second;
      }

      // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      // everything else

      /// Clear the lru
      void clear()
      {
          m_key_data.clear();
          m_lru.clear();
      }

      /// Purge the lru
      void purge()
      {
         clear();

         // We use Scott Meyers "swap trick" to purge memory. This is necessary
         // because the underlying structure of a hash map is a vector, which
         // doesn't give up its memory without a fight
         hm_key_data(m_key_data).swap(m_key_data);
      }

      /// Gets the number of elements in the lru.
      size_t size() const
      {
         return m_key_data.size();
      }

      /// Returns true if empty.
      bool empty() const
      {
         return m_lru.empty();
      }

      /// Gets the max number of elements in the lru.
      size_t max_size() const
      {
         return m_max_size;
      }

      /// Gets the beginning of the lru
      iterator begin()
      {
         return m_lru.begin();
      }

      /// Gets the beginning of the lru
      const_iterator begin() const
      {
         return m_lru.begin();
      }

      /// Gets the end of the lru
      iterator end()
      {
         return m_lru.end();
      }

      /// Gets the end of the lru
      const_iterator end() const
      {
         return m_lru.end();
      }

   };

}} // moost::container

#endif // MOOST_CONTAINER_LRU_HPP__
