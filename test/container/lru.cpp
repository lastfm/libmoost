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

#include "../../include/moost/container/lru.hpp"

using namespace moost::container;

BOOST_AUTO_TEST_SUITE( lru_test )

struct Fixture
{
   typedef lru<int, int> lru_t;
   lru_t lru_;
   int ret_val_;

   Fixture()
   : lru_(3)
   {
     lru_.set_deleted_key(-1);
   }
};

BOOST_FIXTURE_TEST_CASE( test_empty, Fixture )
{
  BOOST_CHECK_EQUAL(lru_.get(3, ret_val_), false);
}

// find nothing!
BOOST_FIXTURE_TEST_CASE( test_nothing, Fixture )
{
  lru_.put(2, 4);
  BOOST_CHECK_EQUAL(lru_.get(3, ret_val_), false);
}

// find something!
BOOST_FIXTURE_TEST_CASE( test_something, Fixture )
{
  lru_.put(3, 4);
  BOOST_REQUIRE_EQUAL(lru_.get(3, ret_val_), true);
  BOOST_CHECK_EQUAL(ret_val_, 4);
}

// evict!
BOOST_FIXTURE_TEST_CASE( test_evict, Fixture )
{
  lru_.put(3, 4);
  lru_.put(4, 5);
  lru_.put(5, 6);
  lru_.put(7, 8);
  BOOST_REQUIRE_EQUAL(lru_.get(7, ret_val_), true);
  BOOST_CHECK_EQUAL(ret_val_, 8);
  BOOST_CHECK_EQUAL(lru_.get(3, ret_val_), false);
}

// don't evict!
BOOST_FIXTURE_TEST_CASE( test_dont_evict, Fixture )
{
  lru_.put(3, 4);
  lru_.put(4, 5);
  lru_.put(5, 6);
  lru_.put(3, 8);
  BOOST_REQUIRE_EQUAL(lru_.get(5, ret_val_), true);
  BOOST_CHECK_EQUAL(ret_val_, 6);
  BOOST_REQUIRE_EQUAL(lru_.get(3, ret_val_), true);
  BOOST_CHECK_EQUAL(ret_val_, 8);
}

// push back twice!
BOOST_FIXTURE_TEST_CASE( test_double_pushback, Fixture )
{
  lru_.put(3, 4);
  lru_.put(3, 5);
  BOOST_REQUIRE_EQUAL(lru_.get(3, ret_val_), true);
  BOOST_CHECK_EQUAL(ret_val_, 5);
}

// test bump
BOOST_FIXTURE_TEST_CASE( test_bump, Fixture )
{
  lru_.put(1, 5);
  lru_.put(2, 6);
  lru_.put(3, 7);

  BOOST_CHECK(lru_.front() == *lru_.find(1));
  BOOST_CHECK(lru_.back() == *lru_.find(3));

  lru_.bump(1);

  BOOST_CHECK(lru_.front() == *lru_.find(2));
  BOOST_CHECK(lru_.back() == *lru_.find(1));

  lru_t::mapped_type get_val = 0;
  lru_.get(3, get_val);

  BOOST_CHECK_EQUAL(get_val, lru_.find(3)->second);
  BOOST_CHECK(lru_.front() == *lru_.find(2));
  BOOST_CHECK(lru_.back() == *lru_.find(3));

  lru_t::mapped_type peek_val = 0;
  lru_.peek(1, peek_val);

  BOOST_CHECK_EQUAL(peek_val, lru_.find(1)->second);
  BOOST_CHECK(lru_.front() == *lru_.find(2));
  BOOST_CHECK(lru_.back() == *lru_.find(3));
}

// test indexer
BOOST_FIXTURE_TEST_CASE( test_indexer, Fixture )
{
  lru_.put(5,2);
  lru_.put(6,3);
  lru_.put(7,4);

  BOOST_CHECK_EQUAL(lru_[5], 2);
  BOOST_CHECK_EQUAL(lru_[6], 3);
  BOOST_CHECK_EQUAL(lru_[7], 4);

  BOOST_CHECK_THROW(lru_[0], std::runtime_error)
  BOOST_CHECK_THROW(lru_[9], std::runtime_error)
}

// test exists
BOOST_FIXTURE_TEST_CASE( test_exists, Fixture )
{
  lru_.put(5,2);
  lru_.put(6,3);
  lru_.put(7,4);

  BOOST_CHECK_EQUAL(lru_.exists(0), false);
  BOOST_CHECK_EQUAL(lru_.exists(1), false);
  BOOST_CHECK_EQUAL(lru_.exists(2), false);
  BOOST_CHECK_EQUAL(lru_.exists(3), false);
  BOOST_CHECK_EQUAL(lru_.exists(4), false);

  BOOST_CHECK_EQUAL(lru_.exists(5), true);
  BOOST_CHECK_EQUAL(lru_.exists(6), true);
  BOOST_CHECK_EQUAL(lru_.exists(7), true);

  BOOST_CHECK_EQUAL(lru_.exists(8), false);
  BOOST_CHECK_EQUAL(lru_.exists(9), false);

  lru_.erase(5);
  lru_.erase(6);
  lru_.erase(7);

  BOOST_CHECK_EQUAL(lru_.exists(5), false);
  BOOST_CHECK_EQUAL(lru_.exists(6), false);
  BOOST_CHECK_EQUAL(lru_.exists(7), false);
}

BOOST_AUTO_TEST_SUITE_END()
