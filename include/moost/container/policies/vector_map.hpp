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

#ifndef MOOST_VECTOR_POLICY_HPP
#define MOOST_VECTOR_POLICY_HPP

#include "generic_map.hpp"
#include <vector>

namespace moost { namespace container { namespace policies {

/**
* dense_hash_map: works with moost::container::dense_hash_map
*/
template <typename TVal>
class vector_map : public generic_map< std::vector<TVal> >
{
public:

   typedef typename std::vector<TVal> map_type;

public:

   vector_map(size_t vector_size = 0)
      : m_initial_vector_size(vector_size)
   {}

   virtual ~vector_map(){}

   /// override
   virtual void init(map_type& map) const
   {
      if ( m_initial_vector_size > 0 )
         map.resize(m_initial_vector_size);
   }

   /// overrides
   virtual void resize(map_type& map, size_t numKeys) const
   { map.resize(numKeys); }

   template <typename TKey>
   TVal operator()( const std::vector<TVal>& vec, const TKey& key ) const
   {
      if ( static_cast<size_t>(key) >= vec.size() )
         throw std::runtime_error("vector_map_policy::operator(): key not found");
      else
         return vec[key];
   }

   template <typename TKey>
   bool operator()( const std::vector<TVal>& vec, const TKey& key, TVal& val ) const
   {
      if ( static_cast<size_t>(key) >= vec.size() )
         return false;
      else
         val = vec[key];
      return true;
   }


   template <typename TKey, typename TMapIterator>
   TKey get_key( const std::vector<TVal>& vec, const TMapIterator& it ) const
   { return static_cast<int>(it - vec.begin()); }

   template <typename TIgnoreThis, typename TMapIterator>
   TVal get_value( const std::vector<TVal>& /*vec*/, const TMapIterator& it ) const
   { return *it; }

   //template <typename TKey>
   //bool find(const std::vector<TVal>& vec, const TKey& key) const
   //{
   //   if ( static_cast<size_t>(key) >= vec.size() )
   //      return vec.end();
   //   else
   //      return vec.begin() + key;
   //}

private:
   const size_t m_initial_vector_size;
};

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal>
struct map_policy_selector< TKey, TVal, std::vector<TVal> >
{
   // will return generic_map
   typedef vector_map<TVal> policy_type;
};

}}}

#endif // MOOST_VECTOR_POLICY_HPP
