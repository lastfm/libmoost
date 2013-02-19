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

#include <vector>
#include "../../include/moost/container/geo_map.hpp"

using namespace moost::container;

BOOST_AUTO_TEST_SUITE( geo_map_test )

struct Fixture
{
   geo_map<int> table;
   geo_map<int>::location query1;
   geo_map<int>::location query2;
   std::vector< geo_map<int>::value_type > results;

   Fixture()
   : query1(-20.3F, 40.9F),
     query2(-19.9F, 48.4F)
   {
      // add some test data
      table.insert(geo_map<int>::value_type(geo_map<int>::location(-20.1F, 45.3F), 1));
      table.insert(geo_map<int>::value_type(geo_map<int>::location(-20.1F, 48.3F), 2));
      table.insert(geo_map<int>::value_type(geo_map<int>::location(13.3F, -12.0F), 3));

      geo_map<int>::const_iterator it = table.begin();
      BOOST_CHECK(it != table.end());
   }
};

// what happens when we search something empty?
BOOST_FIXTURE_TEST_CASE( test_empty, Fixture )
{
   table.clear();
   table.find(query1, 100.0F, std::back_inserter(results));

   BOOST_CHECK(results.empty());
}

// find nothing!
BOOST_FIXTURE_TEST_CASE( test_not_found, Fixture )
{
   table.find(query1, 100.0F, std::back_inserter(results));

   BOOST_CHECK(results.empty());
}

// find something!
BOOST_FIXTURE_TEST_CASE( test_found, Fixture )
{
   table.find(query1, 460.0F, std::back_inserter(results));

   BOOST_REQUIRE_EQUAL(results.size(), 1);
   BOOST_CHECK(results[0].second == 1);
}

// find nothing! (bounds search)
BOOST_FIXTURE_TEST_CASE( test_not_found_bounding, Fixture )
{
   table.find(query1, query1, std::back_inserter(results));

   BOOST_CHECK(results.empty());
}

// find something! (bounds search)
BOOST_FIXTURE_TEST_CASE( test_found_bounding, Fixture )
{
   table.find(query1, query2, std::back_inserter(results));

   BOOST_REQUIRE_EQUAL(results.size(), 2);
   BOOST_CHECK(results[0].second == 1);
   BOOST_CHECK(results[1].second == 2);
}

BOOST_AUTO_TEST_SUITE_END()
