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

#ifndef MOOST_CONTAINER_INDEX_TRANSLATOR_HPP
#define MOOST_CONTAINER_INDEX_TRANSLATOR_HPP

#include "policies/dense_hash_map.hpp"
#include <vector>

#include <google/dense_hash_map>

namespace moost { namespace container {

/**
*
* with std::map
* typedef moost::container::index_translator< int, int, std::map<int, int> > idx_trans_t;
* with google::sparse_hash
* typedef moost::container::index_translator< int, int, google::sparse_hash_map<int,int> > idx_trans_t;
*/
template < typename TFrom = int,
           typename TTo = int,
           typename TMap = google::dense_hash_map<TFrom, TTo> >
class index_translator
{
public:

   typedef typename policies::map_policy_selector<TFrom, TTo, TMap>::policy_type map_policy_t;

public:

   index_translator()
   {
      map_policy_t defPolicy;
      defPolicy.init(m_item_index);
   }

   index_translator(const map_policy_t& mapInitPolicy )
   {
      mapInitPolicy.init(m_item_index);
   }

   int add(const TFrom& id)
   {
      typename TMap::const_iterator it = m_item_index.find(id);
      if ( it != m_item_index.end() )
         return it->second;
      else
      {
         TTo index = static_cast<TTo>(m_ids.size());
         m_item_index[id] = index;
         m_ids.push_back(id);
         return index;
      }
   }

   TTo find_index(const TFrom& id) const
   {
      typename TMap::const_iterator it = m_item_index.find(id);
      if (it == m_item_index.end())
         return -1;

      return it->second;
   }

   bool find_index( const TFrom& id,
                    TTo& result_index ) const
   {
      result_index = find_index(id);
      if ( result_index < 0 )
         return false;
      else
         return true;
   }

   TFrom get_id(TTo index) const
   {
      assert( index >= 0 );
      return m_ids[index];
   }

   TFrom safe_get_id(TTo index) const
   {
      if ( index >= static_cast<int>(this->size()) )
         throw std::runtime_error("index_translator: index out of range");
      return get_id(index);
   }

   size_t size() const
   {
      return m_ids.size();
   }

   bool empty() const
   {
      return m_ids.empty();
   }

public:

   bool operator()(const TFrom& id, TTo& result_index) const
   { return find_index(id, result_index); }

private:

   TMap                 m_item_index; // id -> index
   std::vector<TFrom>   m_ids;        // index -> id

};

}} // moost::container

#endif // MOOST_CONTAINER_INDEX_TRANSLATOR_HPP
