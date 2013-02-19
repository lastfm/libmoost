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

#ifndef MOOST_SPARSE_HASH_MAP_POLICY_HPP
#define MOOST_SPARSE_HASH_MAP_POLICY_HPP

#include "generic_map.hpp"
#include "../sparse_hash_map.hpp"

namespace moost { namespace container { namespace policies {

/**
* dense_hash_map: works with moost::container::dense_hash_map
*/
template <typename TKey, typename TVal>
class sparse_hs_map : public generic_map< moost::container::sparse_hash_map<TKey, TVal> >
{
public:

   typedef typename moost::container::sparse_hash_map<TKey, TVal> map_type;

public:

   sparse_hs_map(size_t hint_size = 1048576)
      : m_hintSize(hint_size)
   {}

   virtual ~sparse_hs_map(){}

   /// override
   virtual void init(map_type& map) const
   {
      if ( m_hintSize > 0 )
         map.resize(m_hintSize);
   }

   /// override
   virtual void resize(map_type& map, size_t numKeys) const
   { map.resize(numKeys); }

private:
   size_t m_hintSize;
};

// -----------------------------------------------------------------------------

template <typename TKey, typename TVal>
struct map_policy_selector< TKey, TVal, moost::container::sparse_hash_map<TKey, TVal> >
{
   // will return generic_map
   typedef sparse_hs_map<TKey, TVal>  policy_type;
};

}}}

#endif // MOOST_SPARSE_HASH_MAP_POLICY_HPP
