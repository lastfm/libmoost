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

#ifndef __NEIGH_MULTI_MAP_CONTAINER_H
#define __NEIGH_MULTI_MAP_CONTAINER_H

#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <fstream>
#include <stdexcept>

#include <boost/cstdint.hpp>

#include "multi_map.hpp"
#include "dense_hash_map.hpp"

namespace moost { namespace container {

// -----------------------------------------------------------------------------

typedef std::pair<int, float> entry_type;
struct IdentityPolicy
{
   inline bool operator()(int entry, int& res) const
   {
      res = entry;
      return true;
   }
};

// -----------------------------------------------------------------------------

/** @brief neigh_multi_map is a container that associates keys to list of pairs of integers and floats
* read from the standard-mir format neigh.
*
* It is similar in concept of having an hashmap:
* hm[key] -> { pair<int, float>, pair<int, float>, pair<int, float>, ..}
* but it's way more efficient.
* \note This is a <b>read only</b> data structure: once it has been built
* no other data can be added, unless clear has been called.
*
* Example usage:
* \code
moost::container::neigh_multi_map test;
test.create_map( "myFile.bin" );

moost::container::multi_map<int, float>::range rt;
rt = test[10];
moost::container::multi_map<int, float>::range_iterator mapIt;
for ( mapIt = rt.begin(); mapIt != rt.end(); ++mapIt )
   cout << *mapIt << endl;
\endcode
*
* Please note that the default policy uses moost::container::dense_hash_map to map the keys.
* If you expect keys with zero value, please specify a policy with a different empty key,
* i.e.
*\code
typedef moost::container::neigh_multi_map<> multi_map_type;
// this defines empty_key=-1
// also note that the bucket size is less important here because neigh_multi_map figures
// out the optimal size given the number of entries
multi_map_type::loc_map_policy_type policy(-1);
multi_map_type data(policy);
\endcode
* Is it also possible to define a different container for the
* locatin map, i.e. if one want to use a sparse hash map the declaration is:
*\code
typedef moost::container::neigh_multi_map< int, float, sparse_hash_map<int, moost::container::multimap_value_type> > multi_map_sparse;
multi_map_sparse data(policy);
\endcode
*
*/
template < typename TLocMap = moost::container::dense_hash_map<int, multimap_value_type> >
class neigh_multi_map :
   public multi_map< int, std::pair<int, float>, TLocMap >
{
public:

   typedef typename multi_map< int, std::pair<int, float>, TLocMap >::loc_map_policy_type loc_map_policy_type;

public:

   //neigh_multi_map( const loc_map_policy_type& locHandlerPolicy = loc_map_policy_type() )
   //   : multi_map<int, std::pair<int, float>, TLocMap>(locHandlerPolicy)
   //{}

   // the default policy uses dense hash map, and the default empty key is 0,
   // so if your data has zeroes for keys you have to make sure
   // you are using a different policy, i.e.
   // neigh_multi_map<> m( neigh_multi_map<>::loc_map_policy_type(-1) );
   neigh_multi_map(const loc_map_policy_type& locHandlerPolicy )
      : multi_map<int, std::pair<int, float>, TLocMap>(locHandlerPolicy), m_externalLocMap(true)
   {}

   neigh_multi_map()
      : m_externalLocMap(false)
   {}


   // WhichKey is either 1 or 2
   // Expect to be sorted by key
   void create_map( const std::string& dataFileName,
                    int maxEntriesPerVec = (std::numeric_limits<int>::max)() )
   {
      IdentityPolicy ip;
      create_map(dataFileName, ip, maxEntriesPerVec);
   }

   // WhichKey is either 1 or 2
   // Expect to be sorted by key

   template <typename TranformIDPolicy>
   void create_map( const std::string& dataFileName,
                    const TranformIDPolicy& getIDPolicy,
                    int maxEntriesPerVec = (std::numeric_limits<int>::max)() );

   inline void create_map_from_vector( std::vector<std::pair<int, std::pair<int, float> > >& i2i )
   {
      multi_map< int, std::pair<int, float>, TLocMap >::template create_map<1>(i2i.begin(), i2i.end());
   }

private:

   using multi_map< int, std::pair<int, float>, TLocMap >::m_data;
   using multi_map< int, std::pair<int, float>, TLocMap >::m_locations;

   using multi_map< int, std::pair<int, float>, TLocMap >::m_locHandlerPolicy;

   bool m_externalLocMap;

   // -----------------------------------------------------------------------------

#ifdef _WIN32
   // In order to seek with large files in windows use
   // this function to "extract" the file pointer from
   // a stream:
   //
   // FILE* fp = getFilePointer(fileStream.rdbuf());
   //
   // then use the 64bits seek function:
   //
   // _fseeki64( fp, pos, SEEK_SET);
   //
   inline FILE* getFilePointer(std::filebuf* pFileBuf)
   {
      return reinterpret_cast<FILE*>(
         *reinterpret_cast<FILE**>(
         reinterpret_cast<char*>(pFileBuf)+76 ) );
   }
#endif

};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename TLocMap>
template <typename TranformIDPolicy>
void neigh_multi_map<TLocMap>::create_map( const std::string& dataFileName,
                                           const TranformIDPolicy& getIDPolicy,
                                           int maxEntriesPerVec /*= (std::numeric_limits<int>::max)() */ )
{
   std::ifstream fileSource(dataFileName.c_str(), std::ios::binary);
   if ( !fileSource.is_open() )
      throw std::runtime_error("Cannot open file <" + dataFileName + ">!");

#ifdef _WIN32
   FILE* fp = getFilePointer(fileSource.rdbuf());
#endif

   int numKeys;
   fileSource.read( reinterpret_cast<char*>( &numKeys ), sizeof(int) );
   if ( fileSource.eof() )
      throw std::runtime_error("Empty source on <" + dataFileName + ">!");

   //////////////////////////////////////////////////////////////////////////
   // scanning
   boost::int64_t currPos = sizeof(int);
   int tmpID, numEntries;
   int numToRead;
   int totEntries = 0;

   for (int i = 0;; ++i)
   {
      fileSource.read( reinterpret_cast<char*>(&tmpID), sizeof(int) );
      if ( fileSource.eof() )
         break;
      fileSource.read( reinterpret_cast<char*>( &numEntries ), sizeof(int) );
      numToRead = (std::min)( numEntries, maxEntriesPerVec );
      totEntries += numToRead;

      currPos += sizeof(int) + // tmpID
                 sizeof(int) + // numEntries
                 numEntries * sizeof(std::pair<int, float>);

#ifdef _WIN32
      _fseeki64( fp, currPos, SEEK_SET);
#else
      fileSource.seekg( currPos, std::ios::beg );
#endif
   }

   // we've got the number!
   if ( totEntries == 0 )
      throw std::runtime_error("Empty source on <" + dataFileName + ">!");

   // allocating
   this->m_data.resize(totEntries);

   if ( !m_externalLocMap )
      m_locHandlerPolicy.resize(this->m_locations, numKeys);
   //TLocHandler::reserve(this->m_locations, numKeys);
   //this->m_locations.resize(numKeys);

   //////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////

   // now loading
   fileSource.clear();
   currPos = sizeof(int);

#ifdef _WIN32
   fileSource.seekg( static_cast<long>(currPos), std::ios::beg );
#else
   fileSource.seekg( currPos, std::ios::beg );
#endif
   int entryPos = 0;

   int transformedID;

   for (int i = 0;; ++i)
   {
      fileSource.read( reinterpret_cast<char*>(&tmpID), sizeof(int) );
      if ( fileSource.eof() )
         break;

      fileSource.read( reinterpret_cast<char*>( &numEntries ), sizeof(int) );
      numToRead = (std::min)( numEntries, maxEntriesPerVec );

      if ( numToRead == 0 )
         continue;

      if ( entryPos + numToRead > totEntries )
         throw std::runtime_error("There were more entries than what was found during scan!");

      fileSource.read( reinterpret_cast<char*>( &(m_data[entryPos]) ), numToRead * sizeof(entry_type) );

      if ( getIDPolicy(tmpID, transformedID) )
         m_locations[transformedID] = std::make_pair(entryPos, numToRead);

      entryPos += numToRead;
      currPos += sizeof(int) + // tmpID
                 sizeof(int) + // numEntries
                 numEntries * sizeof(std::pair<int, float>);

#ifdef _WIN32
      _fseeki64( fp, currPos, SEEK_SET);
#else
      fileSource.seekg( currPos, std::ios::beg );
#endif
   }
}

// -----------------------------------------------------------------------------

}}

#endif // __NEIGH_MULTI_MAP_CONTAINER_H

// -----------------------------------------------------------------------------
