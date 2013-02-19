/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       log2.cpp
 * \brief      Test cases for base-2 integer log algorithm.
 * \author     Marcus Holland-Moritz (marcus@last.fm)
 * \copyright  Copyright © 2008-2013 Last.fm Limited
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
#include <boost/cstdint.hpp>

#include "../../include/moost/math/integer/log2.hpp"

using namespace moost;

BOOST_AUTO_TEST_SUITE(int_log2_test)

BOOST_AUTO_TEST_CASE(int_log2_test)
{
   BOOST_CHECK_EQUAL(math::integer::log2(1U), 0);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(1U), 0);

   BOOST_CHECK_EQUAL(math::integer::log2(2U), 1);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(2U), 1);

   BOOST_CHECK_EQUAL(math::integer::log2(3U), 1);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(3U), 1);

   BOOST_CHECK_EQUAL(math::integer::log2(4U), 2);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(4U), 2);

   BOOST_CHECK_EQUAL(math::integer::log2(65535U), 15);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(65535U), 15);

   BOOST_CHECK_EQUAL(math::integer::log2(65535UL), 15);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(65535UL), 15);

   BOOST_CHECK_EQUAL(math::integer::log2(65536UL), 16);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(65536UL), 16);

   BOOST_CHECK_EQUAL(math::integer::log2(65537UL), 16);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(65537UL), 16);

   BOOST_CHECK_EQUAL(math::integer::log2(0xFFFFFFFFUL), 31);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(0xFFFFFFFFUL), 31);

   BOOST_CHECK_EQUAL(math::integer::log2(UINT64_C(0xFFFFFFFF)), 31);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(UINT64_C(0xFFFFFFFF)), 31);

   BOOST_CHECK_EQUAL(math::integer::log2(UINT64_C(0x100000000)), 32);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(UINT64_C(0x100000000)), 32);

   BOOST_CHECK_EQUAL(math::integer::log2(UINT64_C(0x100000001)), 32);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(UINT64_C(0x100000001)), 32);

   BOOST_CHECK_EQUAL(math::integer::log2(UINT64_C(0xFFFFFFFFFFFFFFFF)), 63);
   BOOST_CHECK_EQUAL(math::integer::log2_compat(UINT64_C(0xFFFFFFFFFFFFFFFF)), 63);
}

BOOST_AUTO_TEST_SUITE_END()
