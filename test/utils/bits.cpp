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
#include <boost/cstdint.hpp>

// Include CRT/STL required header(s)
#include <stdexcept>
#include <set>

// Include thrift headers

// Include application required header(s)
#include "../../include/moost/utils/bits.hpp"

// Imported required namespace(s)
using namespace moost::utils;

// Name the test suite
BOOST_AUTO_TEST_SUITE( MoostUtilsBitsTest )

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

BOOST_FIXTURE_TEST_CASE( test_utils_next_power_of_two, Fixture )
{
   // We make use of this utility is KvdsPageStore to let's just check it works!

   // 0 bit
   BOOST_CHECK(0x00U == next_power_of_two(0x00U));

   // 8 bit
   BOOST_CHECK(0x10U == next_power_of_two(0x0FU));
   BOOST_CHECK(0x20U == next_power_of_two(0x1FU));
   BOOST_CHECK(0x40U == next_power_of_two(0x2FU));
   BOOST_CHECK(0x80U == next_power_of_two(0x4FU));

   // 16 bit
   BOOST_CHECK(0x1000U == next_power_of_two(0x0F00U));
   BOOST_CHECK(0x2000U == next_power_of_two(0x1F00U));
   BOOST_CHECK(0x4000U == next_power_of_two(0x2F00U));
   BOOST_CHECK(0x8000U == next_power_of_two(0x4F00U));

   // 32 bit
   BOOST_CHECK(0x10000000U == next_power_of_two(0x0F000000U));
   BOOST_CHECK(0x20000000U == next_power_of_two(0x1F000000U));
   BOOST_CHECK(0x40000000U == next_power_of_two(0x2F000000U));
   BOOST_CHECK(0x80000000U == next_power_of_two(0x4F000000U));

   // 64 bit
   BOOST_CHECK(UINT64_C(0x1000000000000000) == next_power_of_two(UINT64_C(0x0F00000000000000)));
   BOOST_CHECK(UINT64_C(0x2000000000000000) == next_power_of_two(UINT64_C(0x1F00000000000000)));
   BOOST_CHECK(UINT64_C(0x4000000000000000) == next_power_of_two(UINT64_C(0x2F00000000000000)));
   BOOST_CHECK(UINT64_C(0x8000000000000000) == next_power_of_two(UINT64_C(0x4F00000000000000)));
}

BOOST_FIXTURE_TEST_CASE( test_utils_is_power_of_two, Fixture )
{
   // We make use of this utility is KvdsPageStore to let's just check it works!

   // 0 bit
   BOOST_CHECK(is_power_of_two(0x00U));

   // 8 bit
   BOOST_CHECK(!is_power_of_two(0x0FU));
   BOOST_CHECK(!is_power_of_two(0x1FU));
   BOOST_CHECK(!is_power_of_two(0x2FU));
   BOOST_CHECK(!is_power_of_two(0x4FU));

   // 16 bit
   BOOST_CHECK(!is_power_of_two(0x0F00U));
   BOOST_CHECK(!is_power_of_two(0x1F00U));
   BOOST_CHECK(!is_power_of_two(0x2F00U));
   BOOST_CHECK(!is_power_of_two(0x4F00U));

   // 32 bit
   BOOST_CHECK(!is_power_of_two(0x0F000000U));
   BOOST_CHECK(!is_power_of_two(0x1F000000U));
   BOOST_CHECK(!is_power_of_two(0x2F000000U));
   BOOST_CHECK(!is_power_of_two(0x4F000000U));

   // 64 bit
   BOOST_CHECK(!is_power_of_two(UINT64_C(0x0F00000000000000)));
   BOOST_CHECK(!is_power_of_two(UINT64_C(0x1F00000000000000)));
   BOOST_CHECK(!is_power_of_two(UINT64_C(0x2F00000000000000)));
   BOOST_CHECK(!is_power_of_two(UINT64_C(0x4F00000000000000)));


   // 8 bit
   BOOST_CHECK(is_power_of_two(0x10U));
   BOOST_CHECK(is_power_of_two(0x20U));
   BOOST_CHECK(is_power_of_two(0x40U));
   BOOST_CHECK(is_power_of_two(0x80U));

   // 16 bit
   BOOST_CHECK(is_power_of_two(0x1000U));
   BOOST_CHECK(is_power_of_two(0x2000U));
   BOOST_CHECK(is_power_of_two(0x4000U));
   BOOST_CHECK(is_power_of_two(0x8000U));

   // 32 bit
   BOOST_CHECK(is_power_of_two(0x10000000U));
   BOOST_CHECK(is_power_of_two(0x20000000U));
   BOOST_CHECK(is_power_of_two(0x40000000U));
   BOOST_CHECK(is_power_of_two(0x80000000U));

   // 64 bit
   BOOST_CHECK(is_power_of_two(UINT64_C(0x1000000000000000)));
   BOOST_CHECK(is_power_of_two(UINT64_C(0x2000000000000000)));
   BOOST_CHECK(is_power_of_two(UINT64_C(0x4000000000000000)));
   BOOST_CHECK(is_power_of_two(UINT64_C(0x8000000000000000)));
}

BOOST_FIXTURE_TEST_CASE( test_utils_msb_set, Fixture )
{
   // We make use of this utility is KvdsPageStore to let's just check it works!

   // 0 bit
   BOOST_CHECK(-1 == msb_set(0x00U));

   // 8 bit
   BOOST_CHECK(3 == msb_set(0x0FU));
   BOOST_CHECK(4 == msb_set(0x1FU));
   BOOST_CHECK(5 == msb_set(0x2FU));
   BOOST_CHECK(6 == msb_set(0x4FU));

   // 16 bit
   BOOST_CHECK(11 == msb_set(0x0F00U));
   BOOST_CHECK(12 == msb_set(0x1F00U));
   BOOST_CHECK(13 == msb_set(0x2F00U));
   BOOST_CHECK(14 == msb_set(0x4F00U));

   // 32 bit
   BOOST_CHECK(27 == msb_set(0x0F000000U));
   BOOST_CHECK(28 == msb_set(0x1F000000U));
   BOOST_CHECK(29 == msb_set(0x2F000000U));
   BOOST_CHECK(30 == msb_set(0x4F000000U));

   // 64 bit
   BOOST_CHECK(59 == msb_set(UINT64_C(0x0F00000000000000)));
   BOOST_CHECK(60 == msb_set(UINT64_C(0x1F00000000000000)));
   BOOST_CHECK(61 == msb_set(UINT64_C(0x2F00000000000000)));
   BOOST_CHECK(62 == msb_set(UINT64_C(0x4F00000000000000)));


   // 8 bit
   BOOST_CHECK(4 == msb_set(0x10U));
   BOOST_CHECK(5 == msb_set(0x20U));
   BOOST_CHECK(6 == msb_set(0x40U));
   BOOST_CHECK(7 == msb_set(0x80U));

   // 16 bit
   BOOST_CHECK(12 == msb_set(0x1000U));
   BOOST_CHECK(13 == msb_set(0x2000U));
   BOOST_CHECK(14 == msb_set(0x4000U));
   BOOST_CHECK(15 == msb_set(0x8000U));

   // 32 bit
   BOOST_CHECK(28 == msb_set(0x10000000U));
   BOOST_CHECK(29 == msb_set(0x20000000U));
   BOOST_CHECK(30 == msb_set(0x40000000U));
   BOOST_CHECK(31 == msb_set(0x80000000U));

   // 64 bit
   BOOST_CHECK(60 == msb_set(UINT64_C(0x1000000000000000)));
   BOOST_CHECK(61 == msb_set(UINT64_C(0x2000000000000000)));
   BOOST_CHECK(62 == msb_set(UINT64_C(0x4000000000000000)));
   BOOST_CHECK(63 == msb_set(UINT64_C(0x8000000000000000)));
}

// Define end of test suite
BOOST_AUTO_TEST_SUITE_END()
