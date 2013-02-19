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

// TODO: compression has not been tested yet!

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/cstdint.hpp>

#include <vector>
#include <map>
#include <set>

#include "../../include/moost/container/multi_map.hpp"

#include "../../include/moost/container/policies/dense_hash_map.hpp"
#include "../../include/moost/container/policies/sparse_hash_map.hpp"
#include "../../include/moost/container/policies/vector_map.hpp"

using namespace moost::container;
using namespace std;

BOOST_AUTO_TEST_SUITE( multi_map_test )

template <typename TMultiMap>
struct Fixture_generic
{
   typedef TMultiMap multi_map_type;

   template< int WhichKey, typename TKey, typename TVal >
   void create_data(multi_map_type& map, TKey num_keys, TVal num_val)
   {

      vector< pair<TKey, TVal> > vec;

      if ( WhichKey == 1 )
      {
         for ( TKey k = 0; k < num_keys; ++k )
         {
            for ( TVal v = 0; v < num_val; ++v )
               vec.push_back( make_pair(k, v) );
         }
      }
      else
      {
         for ( TVal v = 0; v < num_val; ++v )
         {
            for ( TKey k = 0; k < num_keys; ++k )
               vec.push_back( make_pair(k, v) );
         }
      }

      map.template create_map<WhichKey>(vec);
   }

   void check_map( multi_map_type& map,
                   typename multi_map_type::first_type num_keys,
                   typename multi_map_type::second_type num_val)
   {
      // how many keys?
      BOOST_CHECK_EQUAL( map.size(), num_keys );

      std::set<typename multi_map_type::first_type> retrieved_keys;
      typename std::set<typename multi_map_type::first_type>::const_iterator rkIt;

      typename multi_map_type::first_type key;
      typename multi_map_type::second_type val;

      // const
      {
         const multi_map_type& cmap = map; // take a const to simulate passing by const

         // try to get a key that does NOT exist
         typename multi_map_type::const_range rt = cmap[num_keys+1];
         BOOST_CHECK( rt.empty() ); // not found!

         typename multi_map_type::const_range_iterator cmRangeIt;
         typename multi_map_type::second_type val;

         // checking getting a range from key
         for ( typename multi_map_type::first_type k = 0; k < num_keys; ++k )
         {
            // get the range for key 'k'
            rt = cmap[k];
            BOOST_CHECK( !rt.empty() );
            BOOST_CHECK_EQUAL( rt.size(), num_val );

            val = 0;
            // check each value associated to key 'k'
            for ( cmRangeIt = rt.begin(); cmRangeIt != rt.end(); ++cmRangeIt, ++val )
               BOOST_CHECK_EQUAL( *cmRangeIt, val );
         }

         // checking the available keys
         typename multi_map_type::const_iterator cmIt;
         for ( cmIt = cmap.begin(); cmIt != cmap.end(); ++cmIt )
         {
            retrieved_keys.insert(cmIt->first);
            BOOST_CHECK_EQUAL( cmIt->second.size(), num_val );
         }

         BOOST_CHECK_EQUAL(retrieved_keys.size(), num_keys);
         key = 0;
         for ( rkIt = retrieved_keys.begin(); rkIt != retrieved_keys.end(); ++rkIt, ++key )
            BOOST_CHECK_EQUAL( *rkIt, key );
      }

      // non const
      {
         // try to get a key that does NOT exist
         typename multi_map_type::range rt = map[num_keys+1];
         BOOST_CHECK( rt.empty() ); // not found!

         typename multi_map_type::range_iterator mRangeIt;

         for ( typename multi_map_type::first_type k = 0; k < num_keys; ++k )
         {
            // get the range for key 'k'
            rt = map[k];
            BOOST_CHECK( !rt.empty() );
            BOOST_CHECK_EQUAL( rt.size(), num_val );

            val = 0;
            // check each value associated to key 'k'
            for ( mRangeIt = rt.begin(); mRangeIt != rt.end(); ++mRangeIt, ++val )
               BOOST_CHECK_EQUAL( *mRangeIt, val );
         }

         // checking the available keys
         typename multi_map_type::iterator mIt;
         retrieved_keys.clear();
         for ( mIt = map.begin(); mIt != map.end(); ++mIt )
         {
            retrieved_keys.insert(mIt->first);
            BOOST_CHECK_EQUAL( mIt->second.size(), num_val );
         }

         BOOST_CHECK_EQUAL(retrieved_keys.size(), num_keys);
         key = 0;
         for ( rkIt = retrieved_keys.begin(); rkIt != retrieved_keys.end(); ++rkIt, ++key )
            BOOST_CHECK_EQUAL( *rkIt, key );
      }
   }

};

struct Fixture_default : public Fixture_generic< multi_map<int, boost::int8_t>  >
{
   // using dense hash map
   // empty key is -1
   // initialized with bucket size of 1024
   Fixture_default()
      : m_map( multi_map_type::loc_map_policy_type(-1, 1024) )
   {}

   multi_map_type m_map;
};

BOOST_FIXTURE_TEST_CASE( test_default, Fixture_default )
{
   multi_map_type::first_type num_keys = 5;
   multi_map_type::second_type num_vals = 10;

   // key on first
   create_data<1>( m_map, num_keys, num_vals );
   check_map(m_map, num_keys, num_vals);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   create_data<2>( m_map, num_keys, num_vals );

   // key on second
   check_map( m_map, num_vals, num_keys );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_dense : public Fixture_generic<
                              multi_map< int,
                                         boost::int8_t,
                                         moost::container::dense_hash_map< int, multimap_value_type >
                                       > >
{
   // using dense hash map
   // empty key is -1
   // initialized with bucket size of 1024
   Fixture_dense()
      : m_map( multi_map_type::loc_map_policy_type(-1, 1024) )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_dense, Fixture_dense )
{
   multi_map_type::first_type num_keys = 5;
   multi_map_type::second_type num_vals = 10;

   // key on first
   create_data<1>( m_map, num_keys, num_vals );
   check_map(m_map, num_keys, num_vals);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   create_data<2>( m_map, num_keys, num_vals );

   // key on second
   check_map( m_map, num_vals, num_keys );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_sparse : public Fixture_generic<
                              multi_map< int,
                                         boost::int8_t,
                                         moost::container::sparse_hash_map< int, multimap_value_type >
                                       > >
{
   // using sparse hash map
   // initialized with bucket size of 1024
   Fixture_sparse()
      : m_map( multi_map_type::loc_map_policy_type(1024) )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_sparse, Fixture_sparse )
{
   multi_map_type::first_type num_keys = 5;
   multi_map_type::second_type num_vals = 10;

   // key on first
   create_data<1>( m_map, num_keys, num_vals );
   check_map(m_map, num_keys, num_vals);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   create_data<2>( m_map, num_keys, num_vals );

   // key on second
   check_map( m_map, num_vals, num_keys );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_stl_map : public Fixture_generic<
                              multi_map< int,
                                         boost::int8_t,
                                         std::map< int, multimap_value_type >
                                       > >
{
   // using sparse hash map
   // initialized with bucket size of 1024
   Fixture_stl_map()
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_stl_map, Fixture_stl_map )
{
   multi_map_type::first_type num_keys = 5;
   multi_map_type::second_type num_vals = 10;

   // key on first
   create_data<1>( m_map, num_keys, num_vals );
   check_map(m_map, num_keys, num_vals);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   create_data<2>( m_map, num_keys, num_vals );

   // key on second
   check_map( m_map, num_vals, num_keys );
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_map : public Fixture_generic<
                              multi_map< int,
                                         boost::int8_t,
                                         map<int, multimap_value_type>
                                       > >
{
   // using a stl::map
   Fixture_map()
      : m_map( )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_map, Fixture_map )
{
   multi_map_type::first_type num_keys = 5;
   multi_map_type::second_type num_vals = 10;

   // key on first
   create_data<1>( m_map, num_keys, num_vals );
   check_map(m_map, num_keys, num_vals);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   create_data<2>( m_map, num_keys, num_vals );

   // key on second
   check_map( m_map, num_vals, num_keys );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_vector : public Fixture_generic<
                              multi_map< int,
                                         boost::int8_t,
                                         vector<multimap_value_type>
                                       > >
{
   // using a vector
   // initialized with vector size of m_num_keys
   Fixture_vector()
      : m_num_keys(5), m_map( multi_map_type::loc_map_policy_type(m_num_keys) )
   {}

   multi_map_type::first_type m_num_keys;
   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_vector1, Fixture_vector )
{
   multi_map_type::second_type num_vals = 10;

   // key on first
   create_data<1>( m_map, m_num_keys, num_vals );
   check_map(m_map, m_num_keys, num_vals);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_vector2, Fixture_vector )
{
   multi_map_type::second_type num_vals = 5; // must be the same size of the initialized one!

   create_data<2>( m_map, m_num_keys, num_vals );
   // key on second
   check_map( m_map, num_vals, m_num_keys );

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BOOST_AUTO_TEST_SUITE_END()
