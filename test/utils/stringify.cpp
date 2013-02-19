/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file stringify.cpp
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

#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <boost/test/unit_test.hpp>

#include "../../include/moost/utils/stringify.hpp"

using moost::utils::stringify;

BOOST_AUTO_TEST_SUITE(moost_utils_stringify)

BOOST_AUTO_TEST_CASE(stringify_scalar)
{
   BOOST_CHECK_EQUAL(stringify(42), "42");
   BOOST_CHECK_EQUAL(stringify("foo"), "foo");
}

BOOST_AUTO_TEST_CASE(stringify_pair)
{
   std::pair<std::string, std::string> p("foo", "bar");
   BOOST_CHECK_EQUAL(stringify(p), "(foo, bar)");
}

BOOST_AUTO_TEST_CASE(stringify_vector)
{
   std::vector<int> v;
   BOOST_CHECK_EQUAL(stringify(v), "[]");
   v.push_back(42);
   BOOST_CHECK_EQUAL(stringify(v), "[42]");
   v.push_back(7);
   BOOST_CHECK_EQUAL(stringify(v), "[42, 7]");
   v.push_back(2);
   BOOST_CHECK_EQUAL(stringify(v, 0), "[42, 7, 2]");
   BOOST_CHECK_EQUAL(stringify(v, 1), "[42, <+2>]");
   BOOST_CHECK_EQUAL(stringify(v, 2), "[42, 7, <+1>]");
   BOOST_CHECK_EQUAL(stringify(v, 3), "[42, 7, 2]");
   BOOST_CHECK_EQUAL(stringify(v, 4), "[42, 7, 2]");
}

BOOST_AUTO_TEST_CASE(stringify_map)
{
   std::map<std::string, int> m;
   BOOST_CHECK_EQUAL(stringify(m), "{}");
   m["foo"] = 13;
   BOOST_CHECK_EQUAL(stringify(m), "{(foo, 13)}");
   m["bar"] = 42;
   BOOST_CHECK_EQUAL(stringify(m), "{(bar, 42), (foo, 13)}");
   BOOST_CHECK_EQUAL(stringify(m, 0), "{(bar, 42), (foo, 13)}");
   BOOST_CHECK_EQUAL(stringify(m, 1), "{(bar, 42), <+1>}");
   BOOST_CHECK_EQUAL(stringify(m, 2), "{(bar, 42), (foo, 13)}");
   BOOST_CHECK_EQUAL(stringify(m, 3), "{(bar, 42), (foo, 13)}");
}

BOOST_AUTO_TEST_SUITE_END()
