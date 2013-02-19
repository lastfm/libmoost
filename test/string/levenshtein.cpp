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
#include "../../include/moost/string/levenshtein.hpp"

using namespace moost::string;

BOOST_AUTO_TEST_SUITE( levenshtein_test )

BOOST_AUTO_TEST_CASE( test_same )
{
  BOOST_CHECK_EQUAL(levenshtein("foo", "foo"), 0);
}

BOOST_AUTO_TEST_CASE( test_diff )
{
  BOOST_CHECK_EQUAL(levenshtein("foo", "fob"), 1);
}

BOOST_AUTO_TEST_CASE( test_order )
{
  BOOST_CHECK_EQUAL(levenshtein("foo", "oof"), 2);
}

BOOST_AUTO_TEST_CASE( test_max )
{
  BOOST_CHECK_EQUAL(levenshtein("foo", "abc"), 3);
}

BOOST_AUTO_TEST_CASE( test_transposition )
{
  BOOST_CHECK_EQUAL(levenshtein("abc", "acb"), 1);
}

BOOST_AUTO_TEST_CASE( test_fast_same )
{
  BOOST_CHECK_EQUAL(fast_levenshtein("foo", "foo"), 0);
}

BOOST_AUTO_TEST_CASE( test_fast_diff )
{
  BOOST_CHECK_EQUAL(fast_levenshtein("foo", "fob"), 1);
}

BOOST_AUTO_TEST_CASE( test_fast_order )
{
  BOOST_CHECK_EQUAL(fast_levenshtein("foo", "oof"), 2);
}

BOOST_AUTO_TEST_CASE( test_fast_max )
{
  BOOST_CHECK_EQUAL(fast_levenshtein("foo", "abc"), 3);
}

BOOST_AUTO_TEST_CASE( test_fast_transposition )
{
  BOOST_CHECK_EQUAL(fast_levenshtein("abc", "acb"), 2);  // NB e=different from levenshtein!
}

BOOST_AUTO_TEST_SUITE_END()
