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
#include <string>

#include "../../include/moost/thread/token_mutex.hpp"

using namespace moost::thread;

BOOST_AUTO_TEST_SUITE( token_mutex_test )

struct Fixture
{
  token_mutex<int> int_mutex;

  Fixture()
  {
  }

  ~Fixture()
  {
  }
};

BOOST_FIXTURE_TEST_CASE( test_trylock, Fixture )
{
  BOOST_CHECK(int_mutex.trylock(3));
  BOOST_CHECK(int_mutex.trylock(4));
  BOOST_CHECK(!int_mutex.trylock(3));
  int_mutex.unlock(3);
  BOOST_CHECK(int_mutex.trylock(3));
}

BOOST_FIXTURE_TEST_CASE( test_tryscopedlock, Fixture )
{
  token_mutex<int>::scoped_try_lock lock1(int_mutex, 3);
  BOOST_CHECK(lock1);
  token_mutex<int>::scoped_try_lock lock2(int_mutex, 4);
  BOOST_CHECK(lock2);
  token_mutex<int>::scoped_try_lock lock3(int_mutex, 3);
  BOOST_CHECK(!lock3);
}

// TODO: figure out some nifty threaded way of testing rest of token_mutex

BOOST_AUTO_TEST_SUITE_END()
