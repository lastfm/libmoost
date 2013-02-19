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

#include <stdexcept>
#include <set>
#include <iostream>
#include <string>
#include <vector>

#include "../../include/moost/kvstore/kyoto_tycoon_client.hpp"
#include "../../include/moost/kvstore/mock_connection.hpp"

using namespace moost::kvstore;

BOOST_AUTO_TEST_SUITE( kvstore_client_test )

struct Fixture
{
   Fixture()
   {
   }

   ~Fixture()
   {
   }
};

struct StringLiteralToPodTypePolicy
{
   template <class T>
   static T& get(T& str)
   {
      return str;
   }
};

struct StringLiteralToStringPolicy
{
   template <class T>
   static std::string get(T& str)
   {
      return std::string(str);
   }
};

template <typename TClient, typename TConnection, typename TKeyTransformPolicy>
class ClientTester
{
   struct pod
   {
      int i;
      double d;
      bool b;
   };

   void test_equal(const pod& lhs, const pod& rhs) const
   {
      BOOST_REQUIRE_EQUAL(lhs.i, rhs.i);
      BOOST_REQUIRE_CLOSE(lhs.d, rhs.d, 0.000000001);
      BOOST_REQUIRE_EQUAL(lhs.b, rhs.b);
   }

   void test_set_get_intrinsic() const
   {
      TConnection conn;
      conn.open("", 0, 0);

      TClient client(conn);

      client.set(TKeyTransformPolicy::get("one"), 1);
      int i = 0;
      client.get(TKeyTransformPolicy::get("one"), i);
      BOOST_REQUIRE_EQUAL(1, i);

      client.set(TKeyTransformPolicy::get("two"), 2.0);
      double d = 0;
      client.get(TKeyTransformPolicy::get("two"), d);
      BOOST_REQUIRE_CLOSE(2.0, d, 0.0001);
   }

   void test_set_get_vector_intrinsic() const
   {
      TConnection conn;
      conn.open("", 0, 0);

      TClient client(conn);

      std::vector<int> three;
      three.push_back(1);
      three.push_back(2);
      three.push_back(3);
      client.set(TKeyTransformPolicy::get("three"), three);
      std::vector<int> v;
      client.get(TKeyTransformPolicy::get("three"), v);
      BOOST_REQUIRE_EQUAL(three.size(), v.size());
      for (size_t i = 0; i < three.size(); ++i)
         BOOST_REQUIRE_EQUAL(three[i], v[i]);
   }

   void test_set_get_pod() const
   {
      TConnection conn;
      conn.open("", 0, 0);

      TClient client(conn);

      pod p = { 1, 2.0, true }, q = { 3, 4.0, false };
      client.set(TKeyTransformPolicy::get("p"), p);
      client.set(TKeyTransformPolicy::get("q"), q);

      pod p2, q2;
      client.get(TKeyTransformPolicy::get("p"), p2);
      client.get(TKeyTransformPolicy::get("q"), q2);
      test_equal(p, p2);
      test_equal(q, q2);
   }

public:

   void run() const
   {
      test_set_get_intrinsic();
      test_set_get_pod();
      test_set_get_vector_intrinsic();
   }
};

class UnitTestAccessPolicy
{
public:
   template <class StoreT>
   static boost::shared_array<char> get(StoreT& store, const char* pkey, size_t ksize, size_t& vsize)
   {
      std::string key(pkey, ksize);
      return store.get(key, vsize);
   }

   // throws on failure, supply key "fail" to force failure
   template <class StoreT>
   static void set(StoreT& store, const char* pkey, size_t ksize, const char* pval, size_t vsize)
   {
      std::string key(pkey, ksize);

      if ("fail" == key) { throw std::runtime_error("set failed"); }

      store.set(key, pval, vsize);
   }

   // throws on failure, supply key "fail" to force failure
   template <class StoreT>
   static void cache(StoreT& store, const char* pkey, size_t ksize, const char* pval, size_t vsize, boost::int64_t expiryTime)
   {
      std::string key(pkey, ksize);

      if ("fail" == key) { throw std::runtime_error("cache failed"); }

      store.cache(key, pval, vsize, expiryTime);
   }
};

BOOST_FIXTURE_TEST_CASE( test_kyoto_tycoon_client_literal_keys, Fixture )
{
   ClientTester< KyotoTycoonClient,
                 MockKyotoTycoonConnection<UnitTestAccessPolicy>,
                 StringLiteralToPodTypePolicy > tester;
   tester.run();
}

BOOST_FIXTURE_TEST_CASE( test_kyoto_tycoon_client_string_keys, Fixture )
{
   ClientTester< KyotoTycoonClient,
                 MockKyotoTycoonConnection<UnitTestAccessPolicy>,
                 StringLiteralToStringPolicy > tester;
   tester.run();
}

BOOST_AUTO_TEST_SUITE_END()
