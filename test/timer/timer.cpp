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

#include <limits>

#include <boost/thread.hpp>

#include "../../include/moost/thread/xtime_util.hpp"
#include "../../include/moost/timer.h"

using namespace moost;
using namespace moost::thread;

BOOST_AUTO_TEST_SUITE( timer_test )

struct Fixture
{
   Fixture()
   : timer_(48)
   {
   }

   ~Fixture()
   {
   }

   timer timer_;
};

struct FixtureThreshold
{
   FixtureThreshold()
      : timer_(48, 10, 5)
   {
   }

   ~FixtureThreshold()
   {
   }

   timer timer_;
};

BOOST_FIXTURE_TEST_CASE( test_timer_empty, Fixture )
{
  // test preconditions of timer
  BOOST_CHECK_EQUAL(timer_.avg_time(), -1);
  BOOST_CHECK_EQUAL(timer_.min_time(), -1);
  BOOST_CHECK_EQUAL(timer_.max_time(), -1);
  BOOST_CHECK_EQUAL(timer_.count_per_second(), 0);
}

BOOST_FIXTURE_TEST_CASE( test_timer_time, Fixture )
{
  {
    timer::scoped_time lock(timer_);
    boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), 100));
  }
  double time = timer_.avg_time();
  double eps = 0.0000001;
  BOOST_CHECK( fabs(timer_.avg_time() - time) < eps );
  BOOST_CHECK( fabs(timer_.min_time() - time) < eps );
  BOOST_CHECK( fabs(timer_.max_time() - time) < eps );
  //BOOST_CHECK(timer_.count_per_second() > 1);
}


BOOST_FIXTURE_TEST_CASE( test_past_threshold, FixtureThreshold )
{
   std::vector< std::pair<int, boost::posix_time::ptime> > past_thresholds = timer_.past_threshold_times(2);
   BOOST_REQUIRE( past_thresholds.empty() );

   {
      timer::scoped_time lock(timer_);
      boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), 1));
   }

   int longTimeMs = 25;
   {
      timer::scoped_time lock(timer_);
      boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), longTimeMs));
   }

   past_thresholds = timer_.past_threshold_times(2);
   BOOST_REQUIRE( past_thresholds.size() == 1 );
   int tolerance = 1; //ms
   BOOST_CHECK( abs( past_thresholds[0].first - longTimeMs ) < tolerance );
}

BOOST_FIXTURE_TEST_CASE( test_past_threshold_rollover, FixtureThreshold )
{
   {
      timer::scoped_time lock(timer_);
      boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), 1));
   }

   int longTimeMs = 25;
   for ( int i = 0; i < 6; ++i ) // 6 times, but the size of the threshold vector is 5!
   {
      timer::scoped_time lock(timer_);
      boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), longTimeMs+i));
   }

   std::vector< std::pair<int, boost::posix_time::ptime> > past_thresholds = timer_.past_threshold_times(10);
   BOOST_REQUIRE( past_thresholds.size() == 5 ); // the size of the threshold vector is 5!

   boost::posix_time::time_duration dur;
   int totMs;
   int tolerance = 1; //ms

   for ( int i = 0; i < 4; ++i )
   {
      dur = past_thresholds[i].second - past_thresholds[i+1].second;
      totMs = static_cast<int>(dur.total_milliseconds());
      BOOST_CHECK( abs(totMs - (longTimeMs + 5 - i)) < tolerance );
   }

}

/*BOOST_FIXTURE_TEST_CASE( test_timer_rollover, Fixture )
{
  for (int i = 0; i != 49; ++i)
  {
    timer::scoped_time lock(timer_);
    boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), 10));
  }
  int cps = timer_.count_per_second();
  int time = timer_.avg_time();
  BOOST_CHECK_PREDICATE( std::greater_equal<int>(), (time) (8) );
  BOOST_CHECK_PREDICATE( std::less_equal<int>(), (time) (12) );
  BOOST_CHECK_PREDICATE( std::greater<int>(), (timer_.min_time()) (5) );
  BOOST_CHECK_PREDICATE( std::less<int>(), (timer_.max_time()) (15) );
  BOOST_CHECK_PREDICATE( std::greater<int>(), (cps) (95) );
  BOOST_CHECK_PREDICATE( std::less<int>(), (cps) (105) );
}*/

BOOST_AUTO_TEST_SUITE_END()
