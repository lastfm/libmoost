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

#ifndef MOOST_GENERIC_MAP_POLICY_HPP
#define MOOST_GENERIC_MAP_POLICY_HPP

#include <vector>
#include <stdexcept>

namespace moost { namespace container { namespace policies {

/**
* generic_map: works with std::map
*/
template <typename TMap>
class generic_map
{
public:

   typedef TMap map_type;

   virtual ~generic_map() {}

   virtual void init(TMap& /*map*/) const
   {}

   /// Tells how to resize a map to make enough space for a given number of keys
   virtual void resize(TMap& /*map*/, size_t /*numKeys*/) const
   {}

   virtual void swap(TMap& first, TMap& second) const
   { first.swap(second); }

   /// Tells how to retrieve the size of the map
   virtual size_t size(const TMap& map) const
   { return map.size(); }

   virtual void clear(TMap& map) const
   { map.clear(); }

   template <typename TKey>
   bool remove(TMap& map, const TKey& key) const
   {
      typename TMap::iterator it = map.find(key);
      if ( it == map.end() )
         return false;
      map.erase(it);
      return true;
   }

   template <typename TKey>
   bool find(const TMap& map, const TKey& key) const
   { return map.find(key) != map.end(); }

   template <typename TKey, typename TVal>
   TVal operator()( const TMap& map, const TKey& key ) const
   {
      typename TMap::const_iterator it = map.find(key);
      if ( it == map.end() )
         throw std::runtime_error("generic_policy::operator(): key not found");
      return get_value<TVal>(map, it);
   }

   template <typename TKey, typename TVal>
   bool operator()( const TMap& map, const TKey& key, TVal& val ) const
   {
      typename TMap::const_iterator it = map.find(key);
      if ( it == map.end() )
         return false;
      get_value(val, map, it);
      return true;
   }

   template <typename Key, typename T>
   bool put( TMap& map, const Key& key, const T& val ) const
   {
      map[key] = val;
      return true;
   }

   template <typename TKey>
   void get_keys( const TMap& map, std::vector<TKey>& keys ) const
   {
      typename TMap::const_iterator it;
      keys.resize( size(map) );
      int i = 0;
      for ( it = map.begin(); it != map.end(); ++it, ++i)
         keys[i] = get_key<TKey>(map, it);
   }

   //////////////////////////////////////////////////////////////////////////
   // if not using maps with first and second, those need to be
   // overridden

   template <typename TKey, typename TMapIterator>
   TKey get_key( const TMap& /*map*/, const TMapIterator& it ) const
   { return it->first; }

   template <typename TVal, typename TMapIterator>
   TVal get_value( const TMap& /*map*/, const TMapIterator& it ) const
   { return it->second; }

   template <typename TKey, typename TMapIterator>
   void get_key( TKey& key, const TMap& map, const TMapIterator& it ) const
   { key = get_key<TKey>(map, it); }

   template <typename TVal, typename TMapIterator>
   void get_value( TVal& value, const TMap& map, const TMapIterator& it ) const
   { value = get_value<TVal>(map, it); }
};

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal, typename TMap>
struct map_policy_selector
{
   // will return generic_map
   typedef generic_map<TMap> policy_type;
};


}}}

#endif // MOOST_GENERIC_MAP_POLICY_HPP
