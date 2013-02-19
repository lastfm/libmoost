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

#include <functional>
#include <vector>

#include "../../include/moost/algorithm/inplace_set_intersection.hpp"

using namespace moost::algorithm;

BOOST_AUTO_TEST_SUITE( inplace_set_intersection_test )

BOOST_AUTO_TEST_CASE( test_set_intersection )
{
  std::vector<int> foo;
  foo.push_back(2); foo.push_back(3); foo.push_back(5); foo.push_back(6);
  foo.push_back(1); foo.push_back(3); foo.push_back(4); foo.push_back(5);

  std::vector<int>::iterator it_end = inplace_set_intersection(foo.begin(), foo.begin() + 4, foo.begin() + 4, foo.end());

  BOOST_REQUIRE(it_end == foo.begin() + 2);
  BOOST_REQUIRE_EQUAL(foo[0], 3);
  BOOST_REQUIRE_EQUAL(foo[1], 5);
}

BOOST_AUTO_TEST_CASE( test_set_intersection_comp )
{
  std::vector<int> foo;
  foo.push_back(6); foo.push_back(5); foo.push_back(3); foo.push_back(2);
  foo.push_back(5); foo.push_back(4); foo.push_back(3); foo.push_back(1);

  std::vector<int>::iterator it_end = inplace_set_intersection(foo.begin(), foo.begin() + 4,
                                                               foo.begin() + 4, foo.end(),
                                                               std::greater<int>());

  BOOST_REQUIRE(it_end == foo.begin() + 2);
  BOOST_REQUIRE_EQUAL(foo[0], 5);
  BOOST_REQUIRE_EQUAL(foo[1], 3);
}

BOOST_AUTO_TEST_SUITE_END()
