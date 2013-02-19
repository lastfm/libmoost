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
#include "../../include/moost/container/resource_stack.hpp"

using namespace moost::container;

BOOST_AUTO_TEST_SUITE( resource_stack_test )

struct Fixture
{
   resource_stack<int> resource_stack_;

   Fixture()
   {
     resource_stack_.add_resource(boost::shared_ptr<int>(new int(3)));
     resource_stack_.add_resource(boost::shared_ptr<int>(new int(5)));
   }
};

BOOST_FIXTURE_TEST_CASE( test_initial, Fixture )
{
  BOOST_CHECK_EQUAL(resource_stack_.size(), 2);
  BOOST_CHECK_EQUAL(resource_stack_.total_size(), 2);
}

// find nothing!
BOOST_FIXTURE_TEST_CASE( test_get_resource, Fixture )
{
  resource_stack<int>::scoped_resource sr(resource_stack_);

  BOOST_CHECK_EQUAL(resource_stack_.size(), 1);
  BOOST_CHECK_EQUAL(*sr, 5);
  BOOST_CHECK_EQUAL(resource_stack_.total_size(), 2);

  resource_stack<int>::scoped_resource sr2(resource_stack_, false);
  BOOST_CHECK_EQUAL(resource_stack_.size(), 0);
  BOOST_CHECK_EQUAL(*sr2, 3);
  BOOST_CHECK_EQUAL(resource_stack_.total_size(), 2);

  BOOST_CHECK_THROW(
    resource_stack<int>::scoped_resource sr3(resource_stack_, false),
    no_resource_available
  );
}

// find something!
BOOST_FIXTURE_TEST_CASE( test_get_multiple, Fixture )
{
  resource_stack<int>::scoped_resource sr(resource_stack_);

  BOOST_CHECK_EQUAL(resource_stack_.size(), 1);
  BOOST_CHECK_EQUAL(*sr, 5);

  resource_stack<int>::scoped_resource sr2(resource_stack_);

  BOOST_CHECK_EQUAL(resource_stack_.size(), 0);
  BOOST_CHECK_EQUAL(*sr2, 3);
  BOOST_CHECK_EQUAL(resource_stack_.total_size(), 2);
}

// find nothing! (bounds search)
BOOST_FIXTURE_TEST_CASE( test_release, Fixture )
{
  resource_stack<int>::scoped_resource sr(resource_stack_);

  {
    resource_stack<int>::scoped_resource sr2(resource_stack_);
  }
  BOOST_CHECK_EQUAL(resource_stack_.size(), 1);
  BOOST_CHECK_EQUAL(*sr, 5);

  resource_stack<int>::scoped_resource sr3(resource_stack_);
  BOOST_CHECK_EQUAL(resource_stack_.size(), 0);
  BOOST_CHECK_EQUAL(*sr3, 3);
  BOOST_CHECK_EQUAL(resource_stack_.total_size(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
