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

#ifndef __SIMPLE_MULTI_MAP_CONTAINER_H
#define __SIMPLE_MULTI_MAP_CONTAINER_H

#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "multi_map.hpp"

#include "dense_hash_map.hpp"

namespace moost { namespace container {

template <typename TVal>
struct IdentityTransform
{
   inline void operator()(TVal& /*val*/) const { }
};

/*
*  simple_multi_map is an alternative to neigh_multi_map that
*  allows you to load data from arbitrary file formats
*  by specifying a Reader policy class to create_map()
*
*  some Readers for various simple text formats are defined in
*  policies/readers.hpp, or you can write your own satisfying this interface:
*     Reader::Reader(std::istream&)
*     bool Reader::read(int id, std::vector<TVal>& vec, bool sort_by_value)
*     1. read() should return false when there is no more data to read
*     2. it should sort vec in accordance with sort_by_value
*     3. sort_by_value is meaningful for sparse vectors i.e. when TVal is
*        an <idx,value> pair, otherwise it can be ignored by the Reader
*
*  simple usage to load vectors of ints:
*     typedef simple_multi_map<int, int> mmap_t;
*     mmap_t myMap;
*     int maxEntriesPerVec = 5;
*     // use a ready-made reader for tsv data
*     myMap.create_map<tsv_vec_reader<int, int> >(myFile, maxEntriesPerVec, false);
*     // now access the data like this:
*     mmap_t::range r = myMap[3];
*     mmap_t::range_iterator mapIt;
*     for ( mapIt = r.begin(); mapIt != r.end(); ++mapIt )
*        cout << *mapIt << endl;
*
*  note that files of sparse vectors of floats (i.e. TVal = std::pair<int, float>)
*  can be loaded *much* more quickly by converting to "neigh binary" format and
*  using neigh_multi_map
*
*/
template < typename TKey = int,
           typename TVal = std::pair<int, float>,
           typename TLocMap = moost::container::dense_hash_map<TKey, multimap_value_type>
         >
class simple_multi_map : public multi_map< TKey, TVal, TLocMap >
{
public:

   typedef typename multi_map<TKey, TVal, TLocMap>::loc_map_policy_type loc_map_policy_type;

   // the default policy uses dense hash map, and the default empty key is 0,
   // so if your data has zeroes for keys you have to make sure
   // you are using a different policy, i.e.
   // simple_multi_map<> m( simple_multi_map<>::loc_map_policy_type(-1) );
   simple_multi_map( const loc_map_policy_type& locHandlerPolicy = loc_map_policy_type() )
      : multi_map<TKey, TVal, TLocMap>(locHandlerPolicy)
   {}

   // entries can be sorted by value if requested
   template <typename Reader>
   void create_map( const std::string& dataFileName,
                    int maxEntriesPerVec = std::numeric_limits<int>::max(),
                    bool sortByValue = false )
   {
      Reader reader(dataFileName);
      IdentityTransform<TVal> identityTransform;
      create_map( reader, identityTransform,
                  maxEntriesPerVec, sortByValue);
   }

   // entries can be sorted by value if requested
   template <typename Reader, typename ValueTransform>
   void create_map( const std::string& dataFileName,
                    const ValueTransform& valueTransformPolicy,
                    int maxEntriesPerVec = std::numeric_limits<int>::max(),
                    bool sortByValue = false )
   {
      Reader reader(dataFileName);
      create_map( reader,
                  valueTransformPolicy,
                  maxEntriesPerVec,
                  sortByValue );
   }

   // entries can be sorted by value if requested
   template <typename Reader, typename ValueTransform>
   void create_map( Reader& reader,
                    const ValueTransform& valueTransformPolicy,
                    int maxEntriesPerVec = std::numeric_limits<int>::max(),
                    bool sortByValue = false );

   inline void create_map_from_vector( std::vector<std::pair<TKey, TVal> >& i2i )
   {
      multi_map<TKey, TVal, TLocMap>::template create_map<1>(i2i.begin(), i2i.end());
   }

private:

   using multi_map< TKey, TVal, TLocMap >::m_data;
   using multi_map< TKey, TVal, TLocMap >::m_locations;
   using multi_map< TKey, TVal, TLocMap >::m_locHandlerPolicy;
};


template <typename TKey, typename TVal, typename TLocMap>
template <typename Reader, typename ValueTransform>
void simple_multi_map<TKey, TVal, TLocMap>::create_map( Reader& reader,
                                                        const ValueTransform& valueTransformPolicy,
                                                        int maxEntriesPerVec /*= (std::numeric_limits<int>::max)() */,
                                                        bool sortByValue /* = false */ )
{
   TKey tmpID;
   std::vector<TVal> vec;

   int numKeys = 0;
   size_t totEntries = 0;

   std::cerr << "scanning...";

   while (reader.read(tmpID, vec, sortByValue))
   {
      ++numKeys;
      totEntries += std::min(vec.size(), static_cast<size_t>(maxEntriesPerVec));

      if ((numKeys & (numKeys - 1)) == 0)
         std::cerr << numKeys << "...";
   }

   if (totEntries == 0)
      throw std::runtime_error("Empty source!");

   // reserve/allocate memory
   this->m_data.reserve(totEntries);
   m_locHandlerPolicy.resize(this->m_locations, numKeys);

   reader.clear();

   // now load the data
   size_t entryPos = 0;
   size_t numToRead;
   int i = 0;

   std::cerr << "reading...";

   while (reader.read(tmpID, vec, sortByValue))
   {
      if (vec.size() > static_cast<size_t>(maxEntriesPerVec))
         vec.resize(maxEntriesPerVec);

      // transform values with the supplied policy
      for (typename std::vector<TVal>::iterator it = vec.begin(); it != vec.end(); ++it)
         valueTransformPolicy(*it);

      numToRead = vec.size();

      if (entryPos + numToRead > totEntries )
         throw std::runtime_error("There were more entries than what was found during scan!");

      for (size_t j = 0; j < numToRead; ++j)
         m_data.push_back(vec[j]);
      m_locations[tmpID] = std::make_pair(entryPos, numToRead);

      entryPos += numToRead;

      if ((i & (i - 1)) == 0)
         std::cerr << i << "...";
      ++i;
   }

   std::cerr << "done" << std::endl;
}

// -----------------------------------------------------------------------------

}}

#endif // __SIMPLE_MULTI_MAP_CONTAINER_H

// -----------------------------------------------------------------------------
