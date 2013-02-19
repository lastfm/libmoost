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

#ifndef MOOST_SERIALIZATION_HASH_SET_SERIALISER_HPP__
#define MOOST_SERIALIZATION_HASH_SET_SERIALISER_HPP__

#include "../container/sparse_hash_set.hpp"
#include "../container/dense_hash_set.hpp"

#include <boost/mpl/if.hpp>
#include <boost/cast.hpp>

/*!
 * Serialization of moost (Google) sparse and dense hashset types
 */

namespace moost { namespace serialization {
   struct hash_set_save__
   {
      template<typename archiveT, typename setT>
         static void serialize(archiveT & ar, setT const & set, const unsigned int /*version*/)
      {
         // force 64 bit for portability
         boost::int64_t size = set.size();
         ar.save_binary(&size , sizeof(size));

         for(typename setT::const_iterator itr = set.begin() ; itr != set.end() ; ++itr)
         {
            ar << *itr;
         }
      }
   };

   struct hash_set_load__
   {
      template<typename archiveT, typename setT>
         static void serialize(archiveT & ar, setT & set, const unsigned int /*version*/)
      {
         // force 64 bit for portability
         boost::int64_t size = 0;
         ar.load_binary(&size , sizeof(size));
         set.resize(boost::numeric_cast<size_t>(size)); // make sure the cast back to size_t is safe

         typename setT::value_type v;

         while(size-- > 0)
         {
            ar >> v;
            set.insert(v);
         }
      }
   };
}}


namespace boost { // this really does have to be in the boost namespace for ADL to work properly :(
   namespace serialization {

      template<typename archiveT, typename Value, typename HashFcn, typename EqualKey, typename Alloc>
      void serialize(archiveT & ar, moost::container::sparse_hash_set<Value, HashFcn, EqualKey, Alloc> & set, const unsigned int version)
      {
         boost::mpl::if_<
            typename archiveT::is_saving,
            moost::serialization::hash_set_save__,
            moost::serialization::hash_set_load__
         >::type::serialize(ar, set, version);
      }

      template<typename archiveT, typename Value, typename HashFcn, typename EqualKey, typename Alloc>
      void serialize(archiveT & ar, moost::container::dense_hash_set<Value, HashFcn, EqualKey, Alloc> & set, const unsigned int version)
      {
         boost::mpl::if_<
            typename archiveT::is_saving,
            moost::serialization::hash_set_save__,
            moost::serialization::hash_set_load__
         >::type::serialize(ar, set, version);
      }
   }
}


#endif /// MOOST_SERIALIZATION_HASH_SET_SERIALISER_HPP__
