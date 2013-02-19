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
#include <iostream>
#include <cmath>
#include <climits>

#include "../../include/moost/container/simple_multi_map.hpp"

#include "../../include/moost/container/policies/dense_hash_map.hpp"
#include "../../include/moost/container/policies/sparse_hash_map.hpp"
#include "../../include/moost/container/policies/vector_map.hpp"

#include "../../include/moost/container/policies/readers.hpp"

using namespace moost::container;
using namespace moost::container::policies;
using namespace std;

BOOST_AUTO_TEST_SUITE( simple_multi_map_test )

struct FloatValueDoubler
{
   void operator()(pair<int, float>& val) const
   {
      val.second = val.second * 2.0f;
   }
};

template <typename TMultiMap>
struct Fixture_generic
{
   typedef TMultiMap multi_map_type;

   int m_num_keys;
   int m_num_vals;
   const std::string m_fileName;

   Fixture_generic()
      : m_num_keys(10), m_num_vals(5), m_fileName("simple_multi_map_test_data.txt")
   {
      // create a test input file
      ofstream outFile(m_fileName.c_str());

      for ( int k = 0; k < m_num_keys; ++k )
      {
         outFile << k;
         for ( int v = 0; v < m_num_vals; ++v )
         {
            outFile << "\t" << v << "\t" << static_cast<float>(v);
         }
         outFile << endl;
      }
   }

   ~Fixture_generic()
   {
      remove( m_fileName.c_str() );
   }

   void load_data(multi_map_type& map, int maxEntriesPerVec = std::numeric_limits<int>::max(),
                    bool sortByValue = false, bool doubleValues = false)
   {
      if (doubleValues)
      {
         FloatValueDoubler floatValueDoubler;
         map.template create_map<tsv_sparsevec_reader<typename multi_map_type::first_type, float> >(
            m_fileName, floatValueDoubler, maxEntriesPerVec, sortByValue);
      }
      else
      {
         map.template create_map<tsv_sparsevec_reader<typename multi_map_type::first_type, float> >(
            m_fileName, maxEntriesPerVec, sortByValue);
      }
   }

   void check_map( multi_map_type& map, int maxEntriesPerVec = (std::numeric_limits<int>::max)(),
                    bool sortByValue = false, bool doubleValues = false)
   {
      BOOST_CHECK_EQUAL( static_cast<int>(map.size()), m_num_keys );
      pair<int, float> val;
      float eps = 0.00001f;

      int num_vals = min(m_num_vals, maxEntriesPerVec);

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
            BOOST_CHECK_EQUAL( static_cast<int>(rt.size()), num_vals );

            int i = 0;
            for ( cmapIt = rt.begin(); cmapIt != rt.end(); ++cmapIt, ++i )
            {
               if (sortByValue)  // order will be reversed
                  val.first = m_num_vals - 1 - i;
               else
                  val.first = i;
               val.second = static_cast<float>(val.first);

               BOOST_CHECK_EQUAL( cmapIt->first, val.first );
               if (doubleValues)
                  BOOST_CHECK( abs(cmapIt->second - val.second * 2.0f) < eps );
               else
                  BOOST_CHECK( abs(cmapIt->second - val.second) < eps );
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
            BOOST_CHECK_EQUAL( static_cast<int>(rt.size()), num_vals );

            int i = 0;
            for ( mapIt = rt.begin(); mapIt != rt.end(); ++mapIt, ++i )
            {
               if (sortByValue)  // order will be reversed
                  val.first = m_num_vals - 1 - i;
               else
                  val.first = i;
               val.second = static_cast<float>(val.first);

               BOOST_CHECK_EQUAL( mapIt->first, val.first );
               if (doubleValues)
                  BOOST_CHECK( fabs(mapIt->second - val.second * 2.0f) < eps );
               else
                  BOOST_CHECK( fabs(mapIt->second - val.second) < eps );
            }
         }
      }
   }

};

struct Fixture_default : public Fixture_generic< simple_multi_map<> >
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
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest with truncated data, sorted by value
   load_data(m_map, 3, true);
   check_map(m_map, 3, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest transforming the values
   load_data(m_map, 3, true, true);
   check_map(m_map, 3, true, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_dense : public Fixture_generic<
                                       simple_multi_map< int, std::pair<int, float>,
                                       moost::container::dense_hash_map< int, multimap_value_type > > >
{
   // using dense hash map
   // empty key is -1
   // no initialization of the bucket as simple_multi_map will figure it out
   Fixture_dense()
      : m_map( multi_map_type::loc_map_policy_type(-1) )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_dense, Fixture_dense )
{
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest with truncated data, sorted by value
   load_data(m_map, 3, true);
   check_map(m_map, 3, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest transforming the values
   load_data(m_map, 3, true, true);
   check_map(m_map, 3, true, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_sparse : public Fixture_generic<
                              simple_multi_map< int, std::pair<int, float>,
                              moost::container::sparse_hash_map< int, multimap_value_type > > >
{
   // using sparse hash map
   // no initialization of the bucket as simple_multi_map will figure it out
   Fixture_sparse()
      : m_map( multi_map_type::loc_map_policy_type() )
   {}

   multi_map_type m_map;
};

// just check if it compiles
BOOST_FIXTURE_TEST_CASE( test_sparse, Fixture_sparse )
{
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest with truncated data, sorted by value
   load_data(m_map, 3, true);
   check_map(m_map, 3, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest transforming the values
   load_data(m_map, 3, true, true);
   check_map(m_map, 3, true, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

struct Fixture_vector : public Fixture_generic<
                              simple_multi_map< int, std::pair<int, float>,
                              vector<multimap_value_type> > >
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
   load_data(m_map);
   check_map(m_map);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest with truncated data, sorted by value
   load_data(m_map, 3, true);
   check_map(m_map, 3, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );

   // retest transforming the values
   load_data(m_map, 3, true, true);
   check_map(m_map, 3, true, true);

   m_map.clear();
   BOOST_CHECK( m_map.empty() );
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

BOOST_AUTO_TEST_SUITE_END()
