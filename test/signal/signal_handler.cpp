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
#include <string>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "../../include/moost/signal/signal_handler.h"
#include "../../include/moost/thread/xtime_util.hpp"

using namespace moost::signal;
using namespace moost::thread;

BOOST_AUTO_TEST_SUITE( signal_handler_test )

struct Fixture
{
  int m_signal;
  signal_handler m_handler;

  void handle_signal(int signal)
  {
    m_signal = signal;
  }

  Fixture()
  : m_signal(0),
    m_handler(boost::bind(&Fixture::handle_signal, this, _1))
  {
  }

  ~Fixture()
  {
  }
};

BOOST_FIXTURE_TEST_CASE(test_do_nothing, Fixture )
{
  BOOST_CHECK_EQUAL(m_signal, 0);
}

// TODO: damn, I think boost.test's signal handling (see execution_monitor.ipp)
// is interfering with my own. this may make this class difficult to test

/*BOOST_FIXTURE_TEST_CASE(test_do_something, Fixture )
{
  raise(SIGHUP);

  BOOST_CHECK_EQUAL(m_signal, SIGHUP);
}*/

BOOST_AUTO_TEST_SUITE_END()
