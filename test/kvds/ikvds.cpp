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
#include <vector>
#include <set>

#include <boost/cstdint.hpp>

#include "../../include/moost/testing/test_directory_creator.hpp"

// Include thrift headers

// Include application required header(s)
#include "../../include/moost/kvds.hpp"

// Imported required namespace(s)
using boost::uint32_t;
using namespace moost::kvds;
using namespace moost::testing;

// Name the test suite
BOOST_AUTO_TEST_SUITE( ikvdsTest )

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

class IKvdsTester
{
   // NB. We use BOOST_REQUIRE since each test is dependent on the previous one working.
   //     This is to avoid having to re-populate the datastore for each test (save time).

   typedef IKvds kvds_type;

   static uint32_t const val0_mask = 0xF1E2D3C4;
   static uint32_t const val1_mask = 0x12345678;


   void test_get_noexist(kvds_type & kvds)
   {
      // These should ALL fail.
      uint32_t val_get = 0;
      size_t val_get_size = sizeof(val_get);

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         for(uint32_t val_put = 1 ; val_put != 0 ; val_put <<= 1)
         {
            BOOST_REQUIRE(!kvds.get(&key, sizeof(key), &val_get, val_get_size));
         }
      }
   }

   void test_siz(kvds_type & kvds, size_t const cnt, bool const bExists = true)
   {
      size_t const size = sizeof(uint32_t) * cnt;

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         size_t vsize;
         BOOST_REQUIRE(bExists == kvds.siz(&key, sizeof(key), vsize));

         if(bExists)
         {
            BOOST_REQUIRE(size == vsize);
         }
      }
   }

   void test_add_cnt_get(kvds_type & kvds)
   {
      uint32_t const val_add_mask = val1_mask;
      boost::uint64_t ecnt = 0;

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val_add = key ^ val_add_mask;
         uint32_t val_get = 0;
         size_t val_get_size1 = sizeof(val_get);
         size_t val_get_size2 = sizeof(val_get) + 2;
         size_t val_get_size3 = 1;

         boost::uint64_t cnt = 0;
         BOOST_REQUIRE(kvds.cnt(cnt));
         BOOST_REQUIRE(ecnt == cnt);
         BOOST_REQUIRE(kvds.add(&key, sizeof(key), &val_add, sizeof(val_add)));
         BOOST_REQUIRE(kvds.cnt(cnt));
         BOOST_REQUIRE(++ecnt == cnt);

         BOOST_REQUIRE(kvds.get(&key, sizeof(key), &val_get, val_get_size1));
         BOOST_REQUIRE(sizeof(val_get) == val_get_size1);
         BOOST_REQUIRE(val_add == val_get);

         BOOST_REQUIRE(kvds.get(&key, sizeof(key), &val_get, val_get_size2));
         BOOST_REQUIRE(sizeof(val_get) == val_get_size2);
         BOOST_REQUIRE(val_add == val_get);

         BOOST_REQUIRE(kvds.get(&key, sizeof(key), &val_get, val_get_size3));
         BOOST_REQUIRE(1 == val_get_size3);
         BOOST_REQUIRE(val_add == val_get);
      }
   }

   void test_put_cnt_get(kvds_type & kvds, bool bCheckCnt = true)
   {
      uint32_t const val_put_mask = val0_mask;
      boost::uint64_t ecnt = 0;

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val_put = key ^ val_put_mask;
         uint32_t val_get = 0;
         size_t val_get_size1 = sizeof(val_get);
         size_t val_get_size2 = sizeof(val_get) + 2;
         size_t val_get_size3 = 1;

         BOOST_REQUIRE(kvds.put(&key, sizeof(key), &val_put, sizeof(val_put)));

         if(bCheckCnt)
         {
            boost::uint64_t cnt = 0;
            BOOST_REQUIRE(kvds.cnt(cnt));
            BOOST_REQUIRE(++ecnt == cnt);
         }

         BOOST_REQUIRE(kvds.get(&key, sizeof(key), &val_get, val_get_size1));
         BOOST_REQUIRE(sizeof(val_get) == val_get_size1);
         BOOST_REQUIRE(val_put == val_get);

         BOOST_REQUIRE(kvds.get(&key, sizeof(key), &val_get, val_get_size2));
         BOOST_REQUIRE(sizeof(val_get) == val_get_size2);
         BOOST_REQUIRE(val_put == val_get);

         BOOST_REQUIRE(kvds.get(&key, sizeof(key), &val_get, val_get_size3));
         BOOST_REQUIRE(1 == val_get_size3);
         BOOST_REQUIRE(val_put == val_get);
      }
   }

   void test_add_all(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val_add = key ^ val1_mask;
         uint32_t val_get[2] = { 0 };
         size_t val_get_size = sizeof(val_get);

         BOOST_REQUIRE(kvds.add(&key, sizeof(key), &val_add, sizeof(val_add)));
         BOOST_REQUIRE(kvds.all(&key, sizeof(key), &val_get, val_get_size));
         BOOST_REQUIRE(sizeof(val_get) == val_get_size);
         BOOST_REQUIRE((key ^ val0_mask) == val_get[0]);
         BOOST_REQUIRE((key ^ val1_mask) == val_get[1]);
      }
   }

   void test_beg_nxt_end(kvds_type & kvds)
   {
      uint32_t key = 0;
      size_t key_size = sizeof(key);

      // Not called beg() yet so this should signal end!
      BOOST_REQUIRE(kvds.end());
      BOOST_REQUIRE(!kvds.nxt(&key, key_size));
      BOOST_REQUIRE(0 == key_size);

      // Keys come out in no particular order, so we need
      // to build a set of keys we expect to see
      std::set<uint32_t> keySet;

      // Build a set of all they keys we expect to see
      for(key = 1 ; key != 0 ; key <<= 1)
      {
         keySet.insert(key);
      }

      BOOST_REQUIRE(kvds.beg());
      for(uint32_t cnt = 1 ; cnt != 0 ; cnt <<= 1)
      {
         for(int testcase = 0; testcase < 2 ; ++testcase)
         {
            BOOST_REQUIRE(!kvds.end());

            switch(testcase)
            {
            case 0: // Key too small, this should NOT progress the iterator
               {
                  key_size = 0;
                  BOOST_REQUIRE(!kvds.end());
                  BOOST_REQUIRE(!kvds.nxt(&key, key_size));
                  BOOST_REQUIRE(sizeof(key) == key_size);
               }
               break;
            case 1: // This should work and progress the iterator
               {
                  // This should work and progress the iterator
                  key_size = sizeof(key);
                  BOOST_REQUIRE(!kvds.end());
                  BOOST_REQUIRE(kvds.nxt(&key, key_size));
                  BOOST_REQUIRE(sizeof(key) == key_size);
                  BOOST_REQUIRE(keySet.find(key) != keySet.end());

                  // Found it now remove it, we shouldn't see it again
                  keySet.erase(key);
               }
               break;
            default:
               throw std::runtime_error("Unknown test case for test_beg_nxt_end()");
            }
         }
      }

      // We should now be at the end of the iteration
      // Not called beg() yet so this should signal end!
      BOOST_REQUIRE(kvds.end());
      BOOST_REQUIRE(!kvds.nxt(&key, key_size));
      BOOST_REQUIRE(0 == key_size);

      // We populated out keySet with all the keys we expected
      // to see if there are any left something when wrong
      BOOST_REQUIRE(keySet.empty());
   }

   void test_get_all(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         for(int testcase = 0 ; testcase < 4 ; ++testcase)
         {
            uint32_t val_all[2] = { 0 };
            size_t val_all_size = sizeof(val_all);

            switch(testcase)
            {
            case 0 : // key not found
               {
                  uint32_t badkey = ~key;
                  BOOST_REQUIRE(!kvds.all(&badkey, sizeof(badkey), &val_all, val_all_size));
                  BOOST_REQUIRE(0 == val_all_size);
               }
               break;
            case 1 : // key found but value too small
               {
                  uint32_t bad_val_all[1] = { 0 };
                  val_all_size = sizeof(bad_val_all);
                  BOOST_REQUIRE(!kvds.all(&key, sizeof(key), &bad_val_all, val_all_size));
                  BOOST_REQUIRE(sizeof(val_all) == val_all_size);
               }
               break;
            case 2 : // key found and value correct size
               {
                  BOOST_REQUIRE(kvds.all(&key, sizeof(key), &val_all, val_all_size));
                  BOOST_REQUIRE(sizeof(val_all) == val_all_size);
                  BOOST_REQUIRE((key ^ val0_mask) == val_all[0]);
                  BOOST_REQUIRE((key ^ val1_mask) == val_all[1]);
               }
               break;
            case 3 : // key found but value larger than required
               {
                  uint32_t good_val_all[3] = { 0 };
                  val_all_size = sizeof(good_val_all);
                  BOOST_REQUIRE(kvds.all(&key, sizeof(key), &good_val_all, val_all_size));
                  BOOST_REQUIRE(sizeof(val_all) == val_all_size);
                  BOOST_REQUIRE((key ^ val0_mask) == good_val_all[0]);
                  BOOST_REQUIRE((key ^ val1_mask) == good_val_all[1]);
               }
               break;
            default:
               throw std::runtime_error("Unknown test case for test_get_all()");
            }
         }
      }
   }

   void test_xst_del(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         BOOST_REQUIRE(kvds.xst(&key, sizeof(key)));
         BOOST_REQUIRE(kvds.del(&key, sizeof(key)));
         BOOST_REQUIRE(!kvds.xst(&key, sizeof(key)));
      }
   }

   void test_nil(kvds_type & kvds, bool const nil = true)
   {
      bool isnil = false;
      BOOST_REQUIRE(kvds.nil(isnil));
      BOOST_REQUIRE(isnil == nil);
   }

   void test_clr_nil(kvds_type & kvds)
   {
      BOOST_REQUIRE(kvds.clr());

      bool isnil = false;
      BOOST_REQUIRE(kvds.nil(isnil));
      BOOST_REQUIRE(isnil);
   }

   void test_put_add_get(kvds_type & kvds)
   {
      uint32_t key = {0xF0F0F0F0};
      uint32_t put_vals[] = {0x29292929, 0xABABABAB};

      BOOST_REQUIRE(kvds.put(&key, sizeof(key), &put_vals[0], sizeof(put_vals[0])));

      uint32_t get_val = 0;
      size_t get_val_size = sizeof(get_val);

      BOOST_REQUIRE(kvds.get(&key, sizeof(key), &get_val, get_val_size));
      BOOST_REQUIRE(sizeof(get_val) == get_val_size);
      BOOST_REQUIRE(get_val == put_vals[0]);

      BOOST_REQUIRE(kvds.add(&key, sizeof(key), &put_vals[1], sizeof(put_vals[2])));

      uint32_t all_vals[2] = { 0 };
      size_t all_val_size = sizeof(all_vals);

      BOOST_REQUIRE(kvds.all(&key, sizeof(key), all_vals, all_val_size));
      BOOST_REQUIRE(sizeof(all_vals) == all_val_size);
      BOOST_REQUIRE(all_vals[0] == put_vals[0]);
      BOOST_REQUIRE(all_vals[1] == put_vals[1]);
   }

public:

   void operator()(kvds_type & kvds)
   {
      // Since tests are dependent on each other to build up the
      // datastore if any test fails we fail them all.
      // All tests MUST be run in the order presented!

      test_nil(kvds);
      test_siz(kvds, 0, false);
      test_put_add_get(kvds);
      test_clr_nil(kvds);
      test_get_noexist(kvds);
      test_add_cnt_get(kvds);
      test_nil(kvds, false);
      test_put_cnt_get(kvds, false);
      test_nil(kvds, false);
      test_clr_nil(kvds);
      test_put_cnt_get(kvds);
      test_nil(kvds, false);
      test_siz(kvds, 1);
      test_add_all(kvds);
      test_nil(kvds, false);
      test_siz(kvds, 2);
      test_get_all(kvds);
      test_beg_nxt_end(kvds);
      test_xst_del(kvds);
   }
};


BOOST_FIXTURE_TEST_CASE( test_kvds_bbt, Fixture )
{
   KvdsBbt kvds;
   kvds.open(tdc.GetFilePath("KvdsBbt").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_bht, Fixture )
{
   KvdsBht kvds;
   kvds.open(tdc.GetFilePath("KvdsBht").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_intrinsic_pagemap, Fixture )
{
   KvdsPageStore<KvdsPageMapIntrinsicKey<uint32_t> > kvds;
   kvds.open(tdc.GetFilePath("KvdsPageStore").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_nonintrinsic_pagemap, Fixture )
{
   KvdsPageStore<KvdsPageMapNonIntrinsicKey<> > kvds;
   kvds.open(tdc.GetFilePath("KvdsPageStore").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_intrinsic_shared_pagemap, Fixture )
{
   KvdsPageStore<KvdsPageMapShared<KvdsPageMapIntrinsicKey<uint32_t> > >kvds;
   kvds.open(tdc.GetFilePath("KvdsPageStore").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_nonintrinsic_shared_pagemap, Fixture )
{
   KvdsPageStore<KvdsPageMapShared <KvdsPageMapNonIntrinsicKey<> > >kvds;
   kvds.open(tdc.GetFilePath("KvdsPageStore").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_tch, Fixture )
{
   KvdsTch kvds;
   kvds.open(tdc.GetFilePath("KvdsTch").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_kch, Fixture )
{
   KvdsKch kvds;
   kvds.open(tdc.GetFilePath("KvdsKch").c_str());
   IKvdsTester()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_mem_map, Fixture )
{
   KvdsMemMap kvds;
   // This test doesn't need opening :)
   IKvdsTester()(kvds);
}

// Define end of test suite
BOOST_AUTO_TEST_SUITE_END()
