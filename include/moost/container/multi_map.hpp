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

#ifndef MULTI_MAP_CONTAINER_H__
#define MULTI_MAP_CONTAINER_H__

#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "../which.hpp"
#include "policies/dense_hash_map.hpp"

#include "dense_hash_map.hpp"

namespace moost { namespace container {

typedef std::pair<int, int> multimap_value_type;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/** @brief multi_map is a container that associates keys to list of values.
*
* It is similar in concept of having an hashmap:
* hm[key] -> {val, val, val, ..}
* but it's way more efficient.
* \note This is a <b>read only</b> data structure: once it has been built
* no other data can be added, unless clear has been called.
*
* Example usage:
* \code
moost::container::multi_map<int, float> test;
vector< pair<int, float> > data;

data.push_back( make_pair(10, 2.0f) );
data.push_back( make_pair(10, 3.0f) );
data.push_back( make_pair(10, 1.0f) );
data.push_back( make_pair(12, 1.0f) );

test.create_map<1>(data); // using the first of the pair as key, it will sort itself

moost::container::multi_map<int, float>::range rt;
rt = test[10];
moost::container::multi_map<int, float>::range_iterator mapIt;
for ( mapIt = rt.begin(); mapIt != rt.end(); ++mapIt )
   cout << *mapIt << endl;
\endcode
* Is it also possible to define a different container for the
* locatin map, i.e. when the keys are within a well know range
* a vector might be a better solution:
*\code
typedef multi_map< int, float, vector<multimap_value_t> > multi_map_vec;
multi_map_vec::loc_map_policy_type policy(10) // max key index is 9
multi_map_vec data(policy);
\endcode
* In the case of the default location map (dense_hash_map) the empty key can
* be easily set by using the location map policy, i.e. to set it to -1 we just do
*\code
typedef moost::container::multi_map<int, float> map_type;
map_type test(map_type::loc_map_policy_type(-1));
\endcode
*
*/
template < typename TKey,
           typename TVal,
           typename TLocMap = moost::container::dense_hash_map<TKey, multimap_value_type >
         >
class multi_map
{
private:

   typedef multi_map<TKey, TVal, TLocMap> self_type;

public:

   typedef TKey first_type;
   typedef TVal second_type;

   typedef typename
      policies::map_policy_selector
            < TKey, multimap_value_type, TLocMap>::policy_type loc_map_policy_type;

   typedef typename std::vector<TVal>::iterator        range_iterator;
   typedef typename std::vector<TVal>::const_iterator  const_range_iterator;

   class range
   {
   public:
      range() {}
      range( range_iterator f, range_iterator l ) : first(f), last(l) {}

      range_iterator begin() const { return first; }
      range_iterator end()   const { return last; }
      size_t size() const { return last - first; }
      bool empty() const { return (last - first) == 0; }

   private:
      range_iterator first;
      range_iterator last;
   };

   class const_range
   {
   public:

      const_range() {}
      const_range( const_range_iterator f, const_range_iterator l ) : first(f), last(l) {}
      const_range( const range& r ) : first(r.begin()), last(r.end()) {}

      void operator=( const range& r )
      { first = r.begin(); last = r.end(); }

      const_range_iterator begin() const { return first; }
      const_range_iterator end() const { return last; }

      size_t size() const { return last - first; }
      bool empty() const { return (last - first) == 0; }

   private:
      const_range_iterator first;
      const_range_iterator last;
   };


public:

   class const_iterator;

   class iterator /*: public const_iterator*/
   {
      friend class const_iterator;

   public:

      iterator() : m_pSelf(NULL) {}
      iterator( typename TLocMap::iterator it,
                typename TLocMap::iterator lastIt,
                self_type* pSelf )
         : m_loc_map_it(it), m_pSelf(pSelf)
      {
         if ( it == lastIt )
            std::make_pair( -1, range(m_pSelf->m_data.end(), m_pSelf->m_data.end()) );
         else
            update_iterator();
      }

      std::pair<int, range>& operator*()
      {
         update_iterator();
         return m_it;
      }
      std::pair<int, range>* operator->()
      {
         update_iterator();
         return &m_it;
      }

      iterator& operator++()
      {  // preincrement
         ++m_loc_map_it;
         return (*this);
      }

      iterator& operator--()
      {  // predecrement
         --m_loc_map_it;
         return (*this);
      }

      bool operator!=(const iterator& other) const
      {  // test for iterator inequality
         return other.m_loc_map_it != m_loc_map_it;
      }

   private:

      void update_iterator()
      {
         m_it.first = m_pSelf->m_locHandlerPolicy.template get_key<int>(m_pSelf->m_locations, m_loc_map_it);
         //m_it.first = TLocHandler::get_index(m_pSelf->m_locations, m_loc_map_it); // m_loc_map_it->first
         m_it.second = create_range();
      }

      range create_range()
      {
         multimap_value_type loc = m_pSelf->m_locHandlerPolicy.template get_value<multimap_value_type>(m_pSelf->m_locations, m_loc_map_it);

         //multimap_value_t loc = TLocHandler::get_location(m_loc_map_it);
         return range( m_pSelf->m_data.begin() +  loc.first, // start
                       m_pSelf->m_data.begin() + loc.first + loc.second ); //end
      }

      std::pair< int, range >    m_it;
      typename TLocMap::iterator m_loc_map_it;
      self_type*                 m_pSelf;
   };

   class const_iterator
   {
   public:

      const_iterator() : m_pSelf(NULL) {}
      const_iterator(const iterator& i)
         : m_it(i.m_it),
           m_loc_map_it(i.m_loc_map_it),
           m_pSelf(i.m_pSelf) {}

      const_iterator( typename TLocMap::const_iterator it,
                      typename TLocMap::const_iterator lastIt,
                      const self_type* pSelf )
         : m_loc_map_it(it), m_pSelf(pSelf)
      {
         if ( it == lastIt )
            std::make_pair( -1, const_range(m_pSelf->m_data.end(), m_pSelf->m_data.end()) );
         else
            update_iterator();
      }

      void operator=(const iterator& i)
      {  m_pSelf = i.m_pSelf; m_it = i.m_it; m_loc_map_it = i.m_loc_map_it; }

      std::pair<int, const_range>& operator*()
      {
         update_iterator();
         return m_it;
      }
      std::pair<int, const_range>* operator->()
      {
         update_iterator();
         return &m_it;
      }

      const_iterator& operator++()
      {  // preincrement
         ++m_loc_map_it;
         return (*this);
      }

      const_iterator& operator--()
      {  // predecrement
         --m_loc_map_it;
         return (*this);
      }

      bool operator!=(const const_iterator& other) const
      {  // test for iterator inequality
         return other.m_loc_map_it != m_loc_map_it;
      }

   protected:

      void update_iterator()
      {
         m_it.first = m_pSelf->m_locHandlerPolicy.template get_key<int>(m_pSelf->m_locations, m_loc_map_it); // m_loc_map_it->first
         //m_it.first = TLocHandler::get_index(m_pSelf->m_locations, m_loc_map_it); // m_loc_map_it->first
         m_it.second = create_range();
      }

      const_range create_range()
      {
         multimap_value_type loc = m_pSelf->m_locHandlerPolicy.template get_value<multimap_value_type>(m_pSelf->m_locations, m_loc_map_it);
         //multimap_value_t loc = TLocHandler::get_location(m_loc_map_it);
         return const_range( m_pSelf->m_data.begin() + loc.first, // start
                             m_pSelf->m_data.begin() + loc.first + loc.second ); //end
      }

      std::pair< int, const_range >    m_it;
      typename TLocMap::const_iterator m_loc_map_it;
      const self_type*                 m_pSelf;
      //std::vector<TVal>*         m_pData;
   };


   //////////////////////////////////////////////////////////////////////////

   iterator       begin()       { return iterator(m_locations.begin(), m_locations.end(), this); }
   const_iterator begin() const { return const_iterator(m_locations.begin(), m_locations.end(), this); }

   iterator       end()       { return iterator(m_locations.end(), m_locations.end(), this); }
   const_iterator end() const { return const_iterator(m_locations.end(), m_locations.end(), this); }

public:

   multi_map(const loc_map_policy_type& locHandlerPolicy = loc_map_policy_type() )
   : m_locHandlerPolicy(locHandlerPolicy)
   { m_locHandlerPolicy.init(m_locations); }

   /**
   * Create the map from a vector of pairs, where the key is either the first or the second element of the
   * pair, and it is specified by the template argument (either 1 or 2 are valid).
   * \param i2i the item2item vector of pairs
   * \param doSort if true it will sort by the keys. This is a required step and takes a bit of time because
   * it is using a stable sort. If your data is already sorted by the key, you can avoid this.
   */
   template <int WhichKey, typename Iterator>
   void create_map(Iterator first, Iterator last, size_t suggestedSize = 128);

   /**
   * Create the map from a couple of iterators. It allows to specify getKey and getValue policies
   * to access special type of structures.
   */
   template < typename Iterator,
              typename GetKeyPolicy, // int operator()( typename const Entry& e )
              typename GetValuesPolicy > // void operator()( typename const Entry& e, vector<TVal>& vals)
   void create_map( Iterator first, Iterator last,
                    const GetKeyPolicy& getKey,
                    const GetValuesPolicy& getValues,
                    size_t suggestedSize = 128 );

   /**
   * Create the map from a vector of pairs, where the key is either the first or the second element of the
   * pair, and it is specified by the template argument (either 1 or 2 are valid).
   * \param i2i the item2item vector of pairs
   * \param doSort if true it will sort by the keys. This is a required step and takes a bit of time because
   * it is using a stable sort. If your data is already sorted by the key, you can avoid this.
   */
   template <int WhichKey, typename TFirst, typename TSecond>
   void create_map( std::vector<TFirst, TSecond>& i2i, bool doSort = true);

   template <int WhichKey, typename TFirst, typename TSecond>
   void create_map_compressed(std::vector<TFirst, TSecond>& i2i, bool doSort = true);

   /**
   * Equivalent to create_map, but will map the keys with a _single_ value to the same memory location
   * thus saving memory.
   * Use it only if you have a lot of entries with a single value and (if it's a data structure)
   * the operator < is implemented.
   * \see create_map
   */
   template <int WhichKey, typename Iterator>
   void create_map_compressed(Iterator first, Iterator last, size_t suggestedSize = 128);


   range operator[](const TKey& key);
   const_range operator[](const TKey& key) const;

   /// Will clear the whole table
   void clear();

   /// Returns the number of keys
   size_t size() const
   { return m_locations.size(); }

   bool empty() const
   { return m_locations.empty(); }

protected:

   TLocMap m_locations;
   loc_map_policy_type m_locHandlerPolicy;
   //TLocHandler m_locHandler;
   std::vector<TVal> m_data;

};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
template <int WhichKey, typename TFirst, typename TSecond>
void multi_map<TKey, TVal, TLocMap>::create_map( std::vector<TFirst, TSecond>& i2i,
                                                 bool doSort )
{
   if ( i2i.empty() )
      return;
   if ( doSort )
   {
      using namespace moost;
      typename which<WhichKey>::template comparer<std::less> comparer;
      std::stable_sort(i2i.begin(), i2i.end(), comparer );
   }

   create_map<WhichKey>( i2i.begin(), i2i.end(), i2i.size() );
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
template <int WhichKey, typename Iterator>
void multi_map<TKey, TVal, TLocMap>::create_map( Iterator first, Iterator last,
                                                 size_t suggestedSize )
{
   if ( first == last )
      return;

   int currPos = 0;
   int currSize = 0;

   typename moost::which<WhichKey> getKey;
   typename moost::which<WhichKey>::other_type getValue;

   TKey currKey = getKey(*first);

   if ( suggestedSize > 0 )
      m_data.reserve(suggestedSize);

   Iterator it;
   for ( it = first; it != last; ++it )
   {
      if ( currKey != getKey(*it) )
      {
         m_locHandlerPolicy.put(m_locations, currKey, std::make_pair(currPos, currSize));
         currKey = getKey(*it);
         currPos = static_cast<int>(m_data.size());
         currSize = 0;
      }

      m_data.push_back( getValue(*it) );
      ++currSize;
   }

   // last one
   m_locHandlerPolicy.put(m_locations, currKey, std::make_pair(currPos, currSize));
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
template < typename Iterator, typename GetKeyPolicy, typename GetValuesPolicy >
void multi_map<TKey, TVal, TLocMap>::create_map( Iterator first,
                                                 Iterator last,
                                                 const GetKeyPolicy& getKey,
                                                 const GetValuesPolicy& getValues,
                                                 size_t suggestedSize /*= 128 */)
{
   if ( suggestedSize > 0 )
      m_data.reserve(suggestedSize);

   int currKey;
   std::vector<TVal> values;

   int entryPos = 0;
   int valuesSize;

   for ( Iterator it = first; it != last; ++it )
   {
      currKey = getKey(*it);
      getValues(*it, values);
      if ( values.empty() )
         continue;

      valuesSize = static_cast<int>(values.size());

      std::copy( values.begin(), values.end(), back_inserter(m_data) );
      m_locHandlerPolicy.put(m_locations, currKey, std::make_pair(entryPos, valuesSize));
      entryPos += valuesSize;
   }
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
template <int WhichKey, typename TFirst, typename TSecond>
void multi_map<TKey, TVal, TLocMap>::create_map_compressed( std::vector<TFirst, TSecond>& i2i,
                                                          bool doSort )
{
   if ( i2i.empty() )
      return;
   if ( doSort )
   {
      using namespace moost;
      typename which<WhichKey>::template comparer<std::less> comparer;
      std::stable_sort(i2i.begin(), i2i.end(), comparer );
   }

   create_map_compressed<WhichKey>( i2i.begin(), i2i.end(), i2i.size() );
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
template <int WhichKey, typename Iterator>
void multi_map<TKey, TVal, TLocMap>::create_map_compressed( Iterator first, Iterator last,
                                                          size_t suggestedSize )
{
   if ( first == last )
      return;

   int currPos = 0;
   int currSize = 0;

   typename moost::which<WhichKey> getKey;
   typename moost::which<WhichKey>::other_type getValue;

   TKey currKey = getKey(*first);

   if ( suggestedSize > 0 )
      m_data.reserve(suggestedSize);

   std::map<TVal, int> compressedLocsMap;
   typename std::map<TVal, int>::iterator clIt;

   Iterator it;
   for ( it = first; it != last; ++it )
   {
      if ( currKey != getKey(*it) )
      {
         if ( currSize == 1 )
         {
            // does it have the given value?
            clIt = compressedLocsMap.find( m_data.back() );
            if ( clIt == compressedLocsMap.end() )
            {
               // not found! Add it!
               compressedLocsMap[m_data.back()] = currPos;
            }
            else
            {
               m_data.pop_back();
               currPos = clIt->second;
            }
         }

         m_locHandlerPolicy.put(m_locations, currKey, std::make_pair(currPos, currSize));
         currKey = getKey(*it);
         currPos = static_cast<int>(m_data.size());
         currSize = 0;
      }

      m_data.push_back( getValue(*it) );
      ++currSize;
   }

   // last one
   m_locHandlerPolicy.put(m_locations, currKey, std::make_pair(currPos, currSize));
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
typename multi_map<TKey, TVal, TLocMap>::range
multi_map<TKey, TVal, TLocMap>::operator[]( const TKey& key )
{
   multimap_value_type loc;
   if ( m_locHandlerPolicy(m_locations, key, loc) )
   {
      return range( m_data.begin() + loc.first, // start
                    m_data.begin() + loc.first + loc.second  // end
                  );
   }
   else
   {
      return range(m_data.end(), m_data.end());
   }
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
typename multi_map<TKey, TVal, TLocMap>::const_range
multi_map<TKey, TVal, TLocMap>::operator[]( const TKey& key ) const
{
   multimap_value_type loc;
   if ( m_locHandlerPolicy(m_locations, key, loc) )
   {
      return const_range( m_data.begin() + loc.first, // start
                          m_data.begin() + loc.first + loc.second  // end
                         );
   }
   else
   {
      return const_range(m_data.end(), m_data.end());
   }
}

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TLocMap>
void multi_map<TKey, TVal, TLocMap>::clear()
{
   m_data.clear();
   m_locations.clear();
}

// -----------------------------------------------------------------------------

}}

#endif // MULTI_MAP_CONTAINER_H__

// -----------------------------------------------------------------------------
