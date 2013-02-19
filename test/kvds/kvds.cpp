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

#include <boost/lexical_cast.hpp>

// Include CRT/STL required header(s)
#include <stdexcept>
#include <set>

#include <boost/cstdint.hpp>

#include "../../include/moost/testing/test_directory_creator.hpp"
#include "../../include/moost/utils/foreach.hpp"

// Include thrift headers

// Include application required header(s)
#include "../../include/moost/kvds.hpp"

// This header contains some test data
#include "testdata.h"

// Imported required namespace(s)
using namespace boost;
using namespace moost::kvds;
using namespace moost::testing;

// Name the test suite
BOOST_AUTO_TEST_SUITE( kvdsTest )

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
class KvdsTester
{
   // NB. We use BOOST_REQUIRE since each test is dependent on the previous one working.
   //     This is to avoid having to re-populate the datastore for each test (save time).

   typedef kvdsT kvds_type;

   static uint32_t const val0_mask = 0xF1E2D3C4;
   static uint32_t const val1_mask = 0x12345678;

   void test_get_noexist(kvds_type & kvds)
   {
      // These should ALL fail.

      uint32_t val = 0;

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         for(uint32_t val_put = 1 ; val_put != 0 ; val_put <<= 1)
         {
            BOOST_REQUIRE(!kvds.get(key, val));
         }
      }
   }

   void test_size(kvds_type & kvds, size_t const size, bool const bExists = true)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         size_t vsize;
         BOOST_REQUIRE(bExists == kvds.size(key, vsize));

         BOOST_REQUIRE(!bExists == kvds.empty());

         if(bExists)
         {
            BOOST_REQUIRE(size == vsize);
         }
      }
   }

   void test_add_cnt_get(kvds_type & kvds)
   {
      uint32_t const val_add_mask = 0x12345678;
      uint64_t ecnt = 0;

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val_add = key ^ val_add_mask;
         uint32_t val_get = 0;

         uint64_t cnt = 0;
         BOOST_REQUIRE(kvds.count(cnt));
         BOOST_REQUIRE(ecnt == cnt);
         BOOST_REQUIRE(kvds.add(key, val_add));
         BOOST_REQUIRE(kvds.count(cnt));
         BOOST_REQUIRE(++ecnt == cnt);

         BOOST_REQUIRE(kvds.get(key, val_get));
         BOOST_REQUIRE(val_add == val_get);
      }
   }

   void test_put_cnt_get(kvds_type & kvds, bool bCheckCnt = true)
   {
      uint32_t const val_put_mask = val0_mask;
      uint32_t ecnt = 0;

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val_put = key ^ val_put_mask;
         uint32_t val_get = 0;

         if(bCheckCnt)
         {
            uint64_t cnt = 0;
            BOOST_REQUIRE(kvds.count(cnt));
            BOOST_REQUIRE(ecnt == cnt);
         }

         BOOST_REQUIRE(kvds.put(key, val_put));

         if(bCheckCnt)
         {
            uint64_t cnt = 0;
            BOOST_REQUIRE(kvds.count(cnt));
            BOOST_REQUIRE(++ecnt == cnt);
         }

         BOOST_REQUIRE(kvds.get(key, val_get));
         BOOST_REQUIRE(val_put == val_get);
      }
   }

   void test_add_get(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val_add = key ^ val1_mask;
         typename kvds_type::kvds_values_t vals_all;

         BOOST_REQUIRE(kvds.add(key, val_add));
         BOOST_REQUIRE(kvds.get(key, vals_all));
         BOOST_REQUIRE(vals_all.size() == 2);
         BOOST_REQUIRE((key ^ val0_mask) == vals_all[0]);
         BOOST_REQUIRE((key ^ val1_mask) == vals_all[1]);
      }
   }

   void test_get_all(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         for(int testcase = 0 ; testcase < 2 ; ++testcase)
         {
            typename kvds_type::kvds_values_t vals_all;

            switch(testcase)
            {
            case 0 : // key not found
               {
                  uint32_t badkey = ~key;
                  BOOST_REQUIRE(!kvds.get(badkey, vals_all));
                  BOOST_REQUIRE(vals_all.empty());
               }
               break;
            case 1 : // key found and value correct size
               {
                  BOOST_REQUIRE(kvds.get(key, vals_all));
                  BOOST_REQUIRE(vals_all.size() == 2);
                  BOOST_REQUIRE((key ^ val0_mask) == vals_all[0]);
                  BOOST_REQUIRE((key ^ val1_mask) == vals_all[1]);
               }
               break;
            default:
               throw std::runtime_error("Unknown test case for test_get_all()");
            }
         }
      }
   }

   void test_get_few(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         for(size_t cnt = 0 ; cnt < 3 ; ++cnt)
         {
            for(int testcase = 0 ; testcase < 2 ; ++testcase)
            {
               typename kvds_type::kvds_values_t vals(cnt);

               switch(testcase)
               {
               case 0 : // key not found
                  {
                     uint32_t badkey = ~key;
                     BOOST_REQUIRE(!kvds.get(badkey, vals, cnt));
                     BOOST_REQUIRE(vals.size() == cnt);
                  }
                  break;
               case 1 : // key found and value correct size
                  {
                     BOOST_REQUIRE(kvds.get(key, vals, cnt) == ((0 == cnt) ? false : true));
                     BOOST_REQUIRE(vals.size() == cnt);
                     BOOST_REQUIRE(cnt > 0 ? (key ^ val0_mask) == vals[0] : true);
                     BOOST_REQUIRE(cnt > 1 ? (key ^ val1_mask) == vals[1] : true);
                  }
                  break;
               default:
                  throw std::runtime_error("Unknown test case for test_get_few()");
               }
            }
         }
      }
   }

   void test_put_get(kvds_type & kvds)
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

   void test_exist_erase(kvds_type & kvds)
   {
      BOOST_REQUIRE(!kvds.empty());

      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         BOOST_REQUIRE(kvds.exists(key));
         BOOST_REQUIRE(kvds.erase(key));
         BOOST_REQUIRE(!kvds.exists(key));
      }

      BOOST_REQUIRE(kvds.empty());
   }

   void test_empty(kvds_type & kvds, bool const empty = true)
   {
      BOOST_REQUIRE(empty == kvds.empty());
   }

   void test_clear_empty(kvds_type & kvds)
   {
      BOOST_REQUIRE(kvds.clear());
      BOOST_REQUIRE(kvds.empty());
   }

   void test_indexing(kvds_type & kvds)
   {
      for(uint32_t key = 1 ; key != 0 ; key <<= 1)
      {
         uint32_t const val = key ^ val0_mask ^ val1_mask;

         BOOST_REQUIRE(kvds[key] == uint32_t());

         kvds[key] = val;

         BOOST_REQUIRE(kvds[key] == val);
      }
   }

public:

   void operator()(kvds_type & kvds)
   {
      // Each test has a dependency on the previous so don't change the ordering.
      test_empty(kvds);
      test_size(kvds, 0, false);
      test_get_noexist(kvds);
      test_add_cnt_get(kvds);
      test_empty(kvds, false);
      test_put_cnt_get(kvds, false);
      test_empty(kvds, false);
      test_clear_empty(kvds);
      test_put_cnt_get(kvds);
      test_empty(kvds, false);
      test_size(kvds, 1);
      test_add_get(kvds);
      test_empty(kvds, false);
      test_size(kvds, 2);
      test_get_all(kvds);
      test_get_few(kvds);
      test_exist_erase(kvds);
      test_put_get(kvds);
      test_exist_erase(kvds);
      test_indexing(kvds);
   }
};

namespace {
   typedef Kvds<uint32_t, uint32_t> kvds_test_t;
}

BOOST_FIXTURE_TEST_CASE( test_kvds_bbt, Fixture )
{
   shared_ptr<KvdsBbt> spKvdsBbt(new KvdsBbt);
   spKvdsBbt->open(tdc.GetFilePath("KvdsBbt").c_str());
   kvds_test_t kvds(spKvdsBbt);
   KvdsTester<kvds_test_t>()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_bht, Fixture )
{
   shared_ptr<KvdsBht> spKvdsBht(new KvdsBht);
   spKvdsBht->open(tdc.GetFilePath("KvdsBht").c_str());
   kvds_test_t kvds(spKvdsBht);
   KvdsTester<kvds_test_t>()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_nonintrinsic_key, Fixture )
{
   shared_ptr<KvdsPageStore<KvdsPageMapNonIntrinsicKey<> > >
      spKvdsPageStore(new KvdsPageStore<KvdsPageMapNonIntrinsicKey<> >);
   spKvdsPageStore->open(tdc.GetFilePath("KvdsPageStore").c_str());
   kvds_test_t kvds(spKvdsPageStore);
   KvdsTester<kvds_test_t>()(kvds);
}


BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_intrinsic_key, Fixture )
{
   shared_ptr<KvdsPageStore<KvdsPageMapIntrinsicKey<uint32_t> > >
      spKvdsPageStore(new KvdsPageStore<KvdsPageMapIntrinsicKey<uint32_t> >);
   spKvdsPageStore->open(tdc.GetFilePath("KvdsPageStore").c_str());
   kvds_test_t kvds(spKvdsPageStore);
   KvdsTester<kvds_test_t>()(kvds);
}


BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_shared_nonintrinsic_key, Fixture )
{
   shared_ptr<KvdsPageStore<KvdsPageMapShared<KvdsPageMapNonIntrinsicKey<> > > >
      spKvdsPageStore(new KvdsPageStore<KvdsPageMapShared<KvdsPageMapNonIntrinsicKey<> > >);
   spKvdsPageStore->open(tdc.GetFilePath("KvdsPageStore").c_str());
   kvds_test_t kvds(spKvdsPageStore);
   KvdsTester<kvds_test_t>()(kvds);
}


BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_shared_intrinsic_key, Fixture )
{
   shared_ptr<KvdsPageStore<KvdsPageMapShared<KvdsPageMapIntrinsicKey<uint32_t> > > >
      spKvdsPageStore(new KvdsPageStore<KvdsPageMapShared<KvdsPageMapIntrinsicKey<uint32_t> > >);
   spKvdsPageStore->open(tdc.GetFilePath("KvdsPageStore").c_str());
   kvds_test_t kvds(spKvdsPageStore);
   KvdsTester<kvds_test_t>()(kvds);
}


BOOST_FIXTURE_TEST_CASE( test_kvds_tch, Fixture )
{
   shared_ptr<KvdsTch> spKvdsTch(new KvdsTch);
   spKvdsTch->open(tdc.GetFilePath("KvdsTch").c_str());
   kvds_test_t kvds(spKvdsTch);
   KvdsTester<kvds_test_t>()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_kch, Fixture )
{
   shared_ptr<KvdsKch> spKvdsKch(new KvdsKch);
   spKvdsKch->open(tdc.GetFilePath("KvdsKch").c_str());
   kvds_test_t kvds(spKvdsKch);
   KvdsTester<kvds_test_t>()(kvds);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_mem_map, Fixture )
{
   kvds_test_t kvds(ikvds_ptr_t(new KvdsMemMap));
   KvdsTester<kvds_test_t>()(kvds);
}


template<typename kvdsT>
void TestSaveLoad(kvdsT & kvds, std::string const & sPath, bool saveTest, unsigned int const maxcnt)
{
   // Now try and open a new datastore -- this should NOT fail
   kvds.open(sPath.c_str(), true);

   for(unsigned int key = 0 ; key < maxcnt ; ++key)
   {
      for(unsigned int val = 0 ; val < maxcnt ; ++val)
      {
         kvds.add(&key, sizeof(key), &val, sizeof(val));
      }
   }

   if(saveTest)
   {
      kvds.save();
   }
   else
   {
      // Now close the store
      kvds.close();

      // Finally, reopen and check keys and values are still all present and correct
      kvds.open(sPath.c_str());
   }

   uint64_t cnt = 0;
   BOOST_REQUIRE(kvds.cnt(cnt));
   BOOST_REQUIRE(maxcnt == cnt);

   for(unsigned int key = 0 ; key < maxcnt ; ++key)
   {
      size_t vals_size = 0;
      BOOST_REQUIRE(kvds.siz(&key, sizeof(key), vals_size));
      BOOST_REQUIRE(sizeof(unsigned int) * maxcnt == vals_size);

      std::vector<unsigned int> vals(maxcnt);
      kvds.all(&key, sizeof(key), &vals[0], vals_size);
      BOOST_REQUIRE(sizeof(unsigned int) * maxcnt == vals_size);

      for(unsigned int val = 0 ; val < maxcnt ; ++val)
      {
         BOOST_REQUIRE(vals[val] == val);
      }
   }

   kvds.close();
}
template<typename kvdsT>
void TestSaveLoad(std::string const & sPath, bool saveTest)
{
   for(unsigned int cnt = 0 ; cnt < 10 ; cnt+=3)
   {
      kvdsT kvds;
      TestSaveLoad(kvds, sPath, saveTest, cnt);
   }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// BDB - B-Tree
BOOST_FIXTURE_TEST_CASE( test_kvds_bbt_save, Fixture )
{
   TestSaveLoad<KvdsBbt>(tdc.GetFilePath("KvdsBbt"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_bbt_close_load, Fixture )
{
   TestSaveLoad<KvdsBbt>(tdc.GetFilePath("KvdsBbt"), false);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// BDB - hash table
BOOST_FIXTURE_TEST_CASE( test_kvds_bht_save, Fixture )
{
   TestSaveLoad<KvdsBht>(tdc.GetFilePath("KvdsBht"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_bht_close_load, Fixture )
{
   TestSaveLoad<KvdsBht>(tdc.GetFilePath("KvdsBht"), false);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Page store -- intrinsic pagemap
BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_save_intrinsic_pagemap, Fixture )
{
   TestSaveLoad<KvdsPageStore<KvdsPageMapIntrinsicKey<uint32_t> > >(tdc.GetFilePath("KvdsPageStore"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_close_load_intrinsic_pagemap, Fixture )
{
   TestSaveLoad<KvdsPageStore<KvdsPageMapIntrinsicKey<uint32_t> > >(tdc.GetFilePath("KvdsPageStore"), false);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Page store -- non-intrinsic pagemap
BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_save_nonintrinsic_pagemap, Fixture )
{
   TestSaveLoad< KvdsPageStore<KvdsPageMapNonIntrinsicKey<> > >(tdc.GetFilePath("KvdsPageStore"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_close_load_nonintrinsic_pagemap, Fixture )
{
   TestSaveLoad< KvdsPageStore<KvdsPageMapNonIntrinsicKey<> > >(tdc.GetFilePath("KvdsPageStore"), false);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Page store -- intrinsic shared pagemap
BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_save_intrinsic_shared_pagemap, Fixture )
{
   TestSaveLoad<KvdsPageStore<KvdsPageMapShared<KvdsPageMapIntrinsicKey<uint32_t> > > >(tdc.GetFilePath("KvdsPageStore"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_close_load_intrinsic_shared_pagemap, Fixture )
{
   TestSaveLoad<KvdsPageStore<KvdsPageMapShared<KvdsPageMapIntrinsicKey<uint32_t> > > >(tdc.GetFilePath("KvdsPageStore"), false);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Page store -- non-intrinsic shared pagemap
BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_save_nonintrinsic_shared_pagemap, Fixture )
{
   TestSaveLoad<KvdsPageStore<KvdsPageMapShared<KvdsPageMapNonIntrinsicKey<> > > >(tdc.GetFilePath("KvdsPageStore"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_page_store_close_load_nonintrinsic_shared_pagemap, Fixture )
{
   TestSaveLoad<KvdsPageStore<KvdsPageMapShared<KvdsPageMapNonIntrinsicKey<> > > >(tdc.GetFilePath("KvdsPageStore"), false);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// TCH
BOOST_FIXTURE_TEST_CASE( test_kvds_tch_save, Fixture )
{
   TestSaveLoad<KvdsTch>(tdc.GetFilePath("KvdsTch"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_tch_close_load, Fixture )
{
   TestSaveLoad<KvdsTch>(tdc.GetFilePath("KvdsTch"), false);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// KCH
BOOST_FIXTURE_TEST_CASE( test_kvds_kch_save, Fixture )
{
   TestSaveLoad<KvdsKch>(tdc.GetFilePath("KvdsKch"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_kch_close_load, Fixture )
{
   TestSaveLoad<KvdsKch>(tdc.GetFilePath("KvdsKch"), false);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Mem map
BOOST_FIXTURE_TEST_CASE( test_kvds_mem_map_save, Fixture )
{
   TestSaveLoad<KvdsMemMap>(tdc.GetFilePath("KvdsMemMap"), true);
}

BOOST_FIXTURE_TEST_CASE( test_kvds_mem_map_close_load, Fixture )
{
   TestSaveLoad<KvdsMemMap>(tdc.GetFilePath("KvdsMemMap"), false);
}

// Define end of test suite
BOOST_AUTO_TEST_SUITE_END()
