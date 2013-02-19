/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file timestamp.cpp
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

#include "../../include/moost/posix_time/timestamp.hpp"

BOOST_AUTO_TEST_SUITE(moost_posix_time_timestamp)

namespace
{

const time_t NOW = 1335378426;
const boost::posix_time::ptime PNOW = boost::posix_time::from_time_t(NOW);

class test_timebase
{
public:
   static boost::posix_time::ptime now()
   {
      return boost::posix_time::from_time_t(NOW);
   }

   static boost::posix_time::ptime base()
   {
      return moost::posix_time::universal_timebase::base();
   }
};

typedef moost::posix_time::basic_timestamp<test_timebase> test_timestamp;

}

BOOST_AUTO_TEST_CASE(timestamp)
{
   BOOST_CHECK_EQUAL(test_timestamp(100).as_time_t(), 100);
   BOOST_CHECK_EQUAL(test_timestamp("100").as_time_t(), 100);
   BOOST_CHECK_EQUAL(test_timestamp("0h").as_time_t(), NOW);
   BOOST_CHECK_EQUAL(test_timestamp("+2h").as_time_t(), NOW + 7200);

   BOOST_CHECK_EQUAL(test_timestamp("20120425T182706").as_time_t(), NOW);
   BOOST_CHECK_EQUAL(test_timestamp("2012-04-25 18:27:06").as_time_t(), NOW);
   BOOST_CHECK_EQUAL(test_timestamp("2012-Apr-25 18:27:06").as_time_t(), NOW);

   // TODO more tests
}

BOOST_AUTO_TEST_CASE(timestamp_operators)
{
   BOOST_CHECK(test_timestamp("+24h") == test_timestamp("1d"));
   BOOST_CHECK(test_timestamp("+23h") != test_timestamp("1d"));
   BOOST_CHECK(test_timestamp("+23h") < test_timestamp("1d"));
   BOOST_CHECK(test_timestamp("+25h") > test_timestamp("1d"));
   BOOST_CHECK(test_timestamp("+24h") <= test_timestamp("1d"));
   BOOST_CHECK(test_timestamp("+24h") >= test_timestamp("1d"));

   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:06") == NOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:07") != NOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:06") <= NOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:06") >= NOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:05") < NOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:07") > NOW);
   BOOST_CHECK(NOW == test_timestamp("2012-Apr-25 18:27:06"));
   BOOST_CHECK(NOW != test_timestamp("2012-Apr-25 18:27:07"));
   BOOST_CHECK(NOW >= test_timestamp("2012-Apr-25 18:27:06"));
   BOOST_CHECK(NOW <= test_timestamp("2012-Apr-25 18:27:06"));
   BOOST_CHECK(NOW > test_timestamp("2012-Apr-25 18:27:05"));
   BOOST_CHECK(NOW < test_timestamp("2012-Apr-25 18:27:07"));

   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:06") == PNOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:07") != PNOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:06") <= PNOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:06") >= PNOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:05") < PNOW);
   BOOST_CHECK(test_timestamp("2012-Apr-25 18:27:07") > PNOW);
   BOOST_CHECK(PNOW == test_timestamp("2012-Apr-25 18:27:06"));
   BOOST_CHECK(PNOW != test_timestamp("2012-Apr-25 18:27:07"));
   BOOST_CHECK(PNOW >= test_timestamp("2012-Apr-25 18:27:06"));
   BOOST_CHECK(PNOW <= test_timestamp("2012-Apr-25 18:27:06"));
   BOOST_CHECK(PNOW > test_timestamp("2012-Apr-25 18:27:05"));
   BOOST_CHECK(PNOW < test_timestamp("2012-Apr-25 18:27:07"));
}

BOOST_AUTO_TEST_SUITE_END()
