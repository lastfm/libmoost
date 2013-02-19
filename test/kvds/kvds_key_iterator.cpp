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

// Include boost test framework required headers
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

// Include CRT/STL required header(s)
#include <stdexcept>
#include <set>

#include <boost/cstdint.hpp>

#include "../../include/moost/testing/test_directory_creator.hpp"

// Include thrift headers

// Include application required header(s)
#include "../../include/moost/kvds/kvds_tch.hpp"
#include "../../include/moost/kvds/kvds_mem.hpp"
#include "../../include/moost/kvds/kvds.hpp"
#include "../../include/moost/kvds/kvds_key_iterator.hpp"

// Imported required namespace(s)
using boost::uint32_t;
using namespace moost::kvds;
using namespace moost::testing;

// Name the test suite
BOOST_AUTO_TEST_SUITE( kvdsIteratorTest )

// Define the test fixture
struct Fixture
{
   test_directory_creator tdc;

   // C_tor
   Fixture()
   {
   }

   // D_tor
   ~Fixture()
   {
   }
};

template <typename kvdsT>
class KvdsKeyIteratorTester
{
   // NB. We use BOOST_REQUIRE since each test is dependent on the previous one working.
   //     This is to avoid having to re-populate the datastore for each test (save time).

   typedef kvdsT kvds_type;

   static uint32_t const val0_mask = 0xF1E2D3C4;
   static uint32_t const val1_mask = 0x12345678;

   void test_create(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         typename kvds_type::kvds_values_t put_vals;
         put_vals.push_back(key ^ val0_mask);
         put_vals.push_back(key ^ val1_mask);

         typename kvds_type::kvds_values_t get_vals;

         BOOST_REQUIRE(!kvds.get(key, get_vals));
         BOOST_REQUIRE(get_vals.empty());

         BOOST_REQUIRE(kvds.put(key, put_vals));

         BOOST_REQUIRE(kvds.get(key, get_vals));
         BOOST_REQUIRE(!get_vals.empty());
         BOOST_REQUIRE(2 == get_vals.size());
         BOOST_REQUIRE(get_vals[0] == put_vals[0]);
         BOOST_REQUIRE(get_vals[1] == put_vals[1]);
      }
   }

   void test_iteration_prefix(kvds_type & kvds)
   {
      // Keys come out in no particular order, so we need
      // to build a set of keys we expect to see
      std::set<uint32_t> keySet;

      // Build a set of all they keys we expect to see
      for(uint32_t key = 1;key != 0 ; key <<= 1)
      {
         keySet.insert(key);
      }

      KvdsKeyIterator<kvds_type> const itrKvdsEnd;
      KvdsKeyIterator<kvds_type> itrAssignKvds;

      for(KvdsKeyIterator<kvds_type> itrKvds(kvds) ; itrKvds != itrKvdsEnd ; ++itrKvds)
      {
         BOOST_REQUIRE(itrKvds != itrKvdsEnd);

         BOOST_REQUIRE(keySet.find(*itrKvds) != keySet.end());

         KvdsKeyIterator<kvds_type> itrCopyKvds = itrKvds;
         BOOST_REQUIRE(itrKvds == itrCopyKvds);

         itrAssignKvds = itrKvds;
         BOOST_REQUIRE(itrKvds == itrAssignKvds);

         // Found it now remove it, we shouldn't see it again
         keySet.erase(*itrKvds);
      }

      // We populated out keySet with all the keys we expected
      // to see if there are any left something when wrong
      BOOST_REQUIRE(keySet.empty());
   }


   void test_iteration_postfix(kvds_type & kvds)
   {
      // Keys come out in no particular order, so we need
      // to build a set of keys we expect to see
      std::set<uint32_t> keySet;

      // Build a set of all they keys we expect to see
      for(uint32_t key = 1;key != 0 ; key <<= 1)
      {
         keySet.insert(key);
      }

      KvdsKeyIterator<kvds_type> const itrKvdsEnd;
      KvdsKeyIterator<kvds_type> itrKvds(kvds);
      KvdsKeyIterator<kvds_type> itrAssignKvds;

      while(itrKvds != itrKvdsEnd)
      {
         BOOST_REQUIRE(itrKvds != itrKvdsEnd);

         uint32_t key = *itrKvds++;

         BOOST_REQUIRE(keySet.find(key) != keySet.end());

         KvdsKeyIterator<kvds_type> itrCopyKvds = itrKvds;
         BOOST_REQUIRE(itrKvds == itrCopyKvds);

         itrAssignKvds = itrKvds;
         BOOST_REQUIRE(itrKvds == itrAssignKvds);

         // Found it now remove it, we shouldn't see it again
         keySet.erase(key);
      }

      // We populated out keySet with all the keys we expected
      // to see if there are any left something when wrong
      BOOST_REQUIRE(keySet.empty());
   }

public:

   void operator()(kvds_type & kvds)
   {
       test_create(kvds);
       test_iteration_prefix(kvds);
       test_iteration_postfix(kvds);
   }
};

namespace {
   typedef Kvds<uint32_t, uint32_t> kvds_test_t;
}

BOOST_FIXTURE_TEST_CASE( test_kvds_tch, Fixture )
{
   boost::shared_ptr<KvdsTch> spKvdsTch(new KvdsTch);
   spKvdsTch->open(tdc.GetFilePath("KvsaTch").c_str());
   kvds_test_t kvds(spKvdsTch);
   KvdsKeyIteratorTester<kvds_test_t>()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_mem_map, Fixture )
{
   kvds_test_t kvds(ikvds_ptr_t(new KvdsMemMap));
   KvdsKeyIteratorTester<kvds_test_t>()(kvds);
}
// Define end of test suite
BOOST_AUTO_TEST_SUITE_END()
