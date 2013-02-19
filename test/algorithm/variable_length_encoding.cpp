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
#include <vector>

#include "../../include/moost/algorithm/variable_length_encoding.hpp"

using namespace moost::algorithm;

BOOST_AUTO_TEST_SUITE( variable_length_encoding_test )

BOOST_AUTO_TEST_CASE( test_in_out )
{
  std::vector<char> data;

  std::back_insert_iterator< std::vector<char> > data_out(data);

  variable_length_encoding::write(123, data_out);

  std::vector<char>::iterator it = data.begin();
  int i;
  variable_length_encoding::read(i, it);
  BOOST_CHECK_EQUAL(i, 123);
  BOOST_CHECK(it == data.end());
}

BOOST_AUTO_TEST_CASE( test_zero )
{
  std::vector<char> data;

  std::back_insert_iterator< std::vector<char> > data_out(data);

  variable_length_encoding::write(0, data_out);

  std::vector<char>::iterator it = data.begin();
  int i;
  variable_length_encoding::read(i, it);
  BOOST_CHECK_EQUAL(i, 0);
  BOOST_CHECK(it == data.end());
  BOOST_CHECK_EQUAL(data.size(), 1);
}

BOOST_AUTO_TEST_CASE( test_limits_min )
{
  std::vector<char> data;

  std::back_insert_iterator< std::vector<char> > data_out(data);

  variable_length_encoding::write(std::numeric_limits<int>::min(), data_out);

  std::vector<char>::iterator it = data.begin();
  int i;
  variable_length_encoding::read(i, it);
  BOOST_CHECK_EQUAL(i, std::numeric_limits<int>::min());
  BOOST_CHECK(it == data.end());
}

BOOST_AUTO_TEST_CASE( test_limits_max )
{
  std::vector<char> data;

  std::back_insert_iterator< std::vector<char> > data_out(data);

  variable_length_encoding::write(std::numeric_limits<int>::max(), data_out);

  std::vector<char>::iterator it = data.begin();
  int i;
  variable_length_encoding::read(i, it);
  BOOST_CHECK_EQUAL(i, std::numeric_limits<int>::max());
  BOOST_CHECK(it == data.end());
}

BOOST_AUTO_TEST_SUITE_END()
