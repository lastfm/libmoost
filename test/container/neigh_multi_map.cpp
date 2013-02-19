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

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/cstdint.hpp>

#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath> // for fabs

#include "../../include/moost/container/neigh_multi_map.hpp"

#include "../../include/moost/container/policies/dense_hash_map.hpp"
#include "../../include/moost/container/policies/sparse_hash_map.hpp"
#include "../../include/moost/container/policies/vector_map.hpp"

using namespace moost::container;
using namespace std;

BOOST_AUTO_TEST_SUITE( neigh_multi_map_test )

template <typename TMultiMap>
struct Fixture_generic
{
   typedef TMultiMap multi_map_type;

   int m_num_keys;
   int m_num_vals;
   const std::string m_fileName;

   Fixture_generic()
      : m_num_keys(10), m_num_vals(5), m_fileName("neigh_multi_map_test_data.bin")
   {
      // create the file
      const std::string fileName = "neigh_multi_map_test_data.bin";

      std::ofstream outFile(m_fileName.c_str(), ios::binary);
      outFile.write( reinterpret_cast<const char*>(&m_num_keys), sizeof(int) );
      float score;

      for ( int k = 0; k < m_num_keys; ++k )
      {
         outFile.write( reinterpret_cast<const char*>(&k), sizeof(int) );
         outFile.write( reinterpret_cast<const char*>(&m_num_vals), sizeof(int) );

         for ( int v = 0; v < m_num_vals; ++v )
         {
            outFile.write( reinterpret_cast<const char*>(&v), sizeof(int) );
            score = static_cast<float>(v);

            outFile.write( reinterpret_cast<const char*>(&score), sizeof(float) );
         }
      }
   }

   ~Fixture_generic()
   {
      remove( m_fileName.c_str() );
   }


   void load_data(multi_map_type& map)
   {
      map.create_map(m_fileName);
   }

   void check_map( multi_map_type& map )
   {
      BOOST_CHECK_EQUAL( map.size(), m_num_keys );
      std::pair<int, float> val;
      float eps = 0.00001f;

      // const
      {
         const multi_map_type& cmap = map;
         typename multi_map_type::const_range rt = cmap[m_num_keys+1];
         BOOST_CHECK( rt.empty() ); // not found!

         typename multi_map_type::const_range_iterator cmapIt;

         for ( typename multi_map_type::first_type k = 0; k < m_num_keys; ++k )
         {
            rt = cmap[k];
            BOOST_CHECK( !rt.empty() );
            BOOST_CHECK_EQUAL( rt.size(), m_num_vals );

            int i = 0;
            for ( cmapIt = rt.begin(); cmapIt != rt.end(); ++cmapIt, ++i )
            {
               val.first = i;
               val.second = static_cast<float>(i);

               BOOST_CHECK_EQUAL( cmapIt->first, val.first );
               BOOST_CHECK( fabs(cmapIt->second - val.second) < eps );
            }
         }

      }

      // non const
      {
         typename multi_map_type::range rt = map[m_num_keys+1];
         BOOST_CHECK( rt.empty() ); // not found!

         typename multi_map_type::range_iterator mapIt;

         for ( typename multi_map_type::first_type k = 0; k < m_num_keys; ++k )
         {
            rt = map[k];
            BOOST_CHECK( !rt.empty() );
            BOOST_CHECK_EQUAL( rt.size(), m_num_vals );

            int i = 0;
            for ( mapIt = rt.begin(); mapIt != rt.end(); ++mapIt, ++i )
            {
               val.first = i;
               val.second = static_cast<float>(i);

               BOOST_CHECK_EQUAL( mapIt->first, val.first );
               BOOST_CHECK( abs(mapIt->second - val.second) < eps );
            }
         }
      }

   }

};

struct Fixture_default : public Fixture_generic< neigh_multi_map<> >
{
   // using dense hash map
   // initialized with bucket size of 1024
   Fixture_default()
      : m_map( multi_map_type::loc_map_policy_type(-1) )  // policy is necessary because I saved key = 0
   {}

   multi_map_type m_map;
};

BOOST_FIXTURE_TEST_CASE( test_default, Fixture_default )
{
   // key on first
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_dense : public Fixture_generic<
                                       neigh_multi_map< moost::container::dense_hash_map< int, multimap_value_type > > >
{
   // using dense hash map
   // empty key is -1
   // no initialization of the bucket as neigh_multi_map will figure it out
   Fixture_dense()
      : m_map( multi_map_type::loc_map_policy_type(-1) )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_dense, Fixture_dense )
{
   // key on first
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_sparse : public Fixture_generic<
                              neigh_multi_map< moost::container::sparse_hash_map< int, multimap_value_type > > >
{
   // using sparse hash map
   // no initialization of the bucket as neigh_multi_map will figure it out
   Fixture_sparse()
      : m_map( multi_map_type::loc_map_policy_type() )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_sparse, Fixture_sparse )
{
   // key on first
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_stl_map : public Fixture_generic<
                     neigh_multi_map< std::map< int, multimap_value_type > > >
{
   // using sparse hash map
   // no initialization of the bucket as neigh_multi_map will figure it out
   Fixture_stl_map()
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_stl_map, Fixture_stl_map )
{
   // key on first
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_vector : public Fixture_generic<
                              neigh_multi_map< vector<multimap_value_type> > >
{
   // using a vector
   // initialized with vector size of m_num_keys
   Fixture_vector()
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_vector, Fixture_vector )
{
   // key on first
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BOOST_AUTO_TEST_SUITE_END()
