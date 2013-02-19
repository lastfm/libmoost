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

#ifndef MOOST_SERIALIZATION_HASH_MAP_SERIALISER_HPP__
#define MOOST_SERIALIZATION_HASH_MAP_SERIALISER_HPP__

/*!
 * Serialization of moost (Google) sparse and dense hashmap types
 */

#include "../container/sparse_hash_map.hpp"
#include "../container/dense_hash_map.hpp"

#include <boost/mpl/if.hpp>

namespace moost { namespace serialization {
   struct hash_map_save__
   {
      template<typename archiveT, typename mapT>
         static void serialize(archiveT & ar, mapT const & map, const unsigned int /*version*/)
      {
         // force 64 bit for portability
         boost::int64_t size = map.size();
         ar.save_binary(&size , sizeof(size));

         for(typename mapT::const_iterator itr = map.begin() ; itr != map.end() ; ++itr)
         {
            ar << itr->first;
            ar << itr->second;
         }
      }
   };

   struct hash_map_load__
   {
      template<typename archiveT, typename mapT>
         static void serialize(archiveT & ar, mapT & map, const unsigned int /*version*/)
      {
         // force 64 bit for portability
         boost::int64_t size = 0;
         ar.load_binary(&size , sizeof(size));
         map.resize(boost::numeric_cast<size_t>(size)); // make sure the cast back to size_t is safe

         typename mapT::key_type k;

         while(size-- > 0)
         {
            ar >> k;
            ar >> map[k];
         }
      }
   };
}}

namespace boost { // this really does have to be in the boost namespace for ADL to work properly :(
   namespace serialization {

      template<typename archiveT, typename Key, typename Data, typename HashFcn, typename EqualKey, typename Alloc>
      void serialize(archiveT & ar, moost::container::sparse_hash_map<Key, Data, HashFcn, EqualKey, Alloc> & map, const unsigned int version)
      {
         boost::mpl::if_<
            typename archiveT::is_saving,
            moost::serialization::hash_map_save__,
            moost::serialization::hash_map_load__
         >::type::serialize(ar, map, version);
      }

      template<typename archiveT, typename Key, typename Data, typename HashFcn, typename EqualKey, typename Alloc>
      void serialize(archiveT & ar, moost::container::dense_hash_map<Key, Data, HashFcn, EqualKey, Alloc> & map, const unsigned int version)
      {
         boost::mpl::if_<
            typename archiveT::is_saving,
            moost::serialization::hash_map_save__,
            moost::serialization::hash_map_load__
         >::type::serialize(ar, map, version);
      }
   }
}

#endif /// MOOST_SERIALIZATION_HASH_MAP_SERIALISER_HPP__
