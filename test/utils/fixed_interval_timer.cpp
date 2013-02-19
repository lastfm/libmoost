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

// Include thrift headers

// Include application required header(s)
#include "../../include/moost/utils/fixed_interval_timer.hpp"

// Imported required namespace(s)
using namespace moost::utils;

// Name the test suite
BOOST_AUTO_TEST_SUITE( MoostUtilsFixedIntervalTimerTest )

// Define the test fixture
struct Fixture
{
   // C_tor
   Fixture()
   {
   }

   // D_tor
   ~Fixture()
   {
   }
};

BOOST_FIXTURE_TEST_CASE( test_utils_callback_timer_future, Fixture )
{
   fixed_interval_timer fit(boost::posix_time::milliseconds(500));

   fixed_interval_timer::future_t f1;
   fixed_interval_timer::future_t f2;

   BOOST_CHECK(!f1.is_ready());
   BOOST_CHECK(!f2.is_ready());

   fit.notify(f1);

   BOOST_CHECK(!f1.is_ready());
   BOOST_CHECK(!f2.is_ready());

   boost::thread::sleep(
      boost::posix_time::microsec_clock::universal_time() +
      boost::posix_time::seconds(1));

   BOOST_CHECK(f1.is_ready());
   BOOST_CHECK(!f2.is_ready());

   fit.notify(f2);

   BOOST_CHECK(f1.is_ready());
   BOOST_CHECK(!f2.is_ready());

   boost::thread::sleep(
      boost::posix_time::microsec_clock::universal_time() +
      boost::posix_time::seconds(1));

   BOOST_CHECK(f1.is_ready());
   BOOST_CHECK(f2.is_ready());
}

BOOST_FIXTURE_TEST_CASE( test_utils_callback_timer_signal, Fixture )
{
   fixed_interval_timer fit(boost::posix_time::milliseconds(500));

   fixed_interval_timer::signal_t s1;
   fixed_interval_timer::signal_t s2;

   BOOST_CHECK(!s1.is_ready());
   BOOST_CHECK(!s2.is_ready());

   fit.notify(s1);

   BOOST_CHECK(!s1.is_ready());
   BOOST_CHECK(!s2.is_ready());

   boost::thread::sleep(
      boost::posix_time::microsec_clock::universal_time() +
      boost::posix_time::seconds(1));

   BOOST_CHECK(s1.is_ready());
   BOOST_CHECK(!s2.is_ready());

   fit.notify(s2);

   BOOST_CHECK(s1.is_ready());
   BOOST_CHECK(!s2.is_ready());

   boost::thread::sleep(
      boost::posix_time::microsec_clock::universal_time() +
      boost::posix_time::seconds(1));

   BOOST_CHECK(s1.is_ready());
   BOOST_CHECK(s2.is_ready());
}


// Define end of test suite
BOOST_AUTO_TEST_SUITE_END()
