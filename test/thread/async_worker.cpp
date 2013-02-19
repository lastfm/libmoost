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

#include "../../include/moost/thread/async_worker.hpp"
#include "../../include/moost/thread/xtime_util.hpp"

#include <boost/thread/thread.hpp>

using namespace moost::thread;

BOOST_AUTO_TEST_SUITE( async_worker_test )

class SimpleAsyncWorker : public async_worker<int>
{
private:

  std::vector<char> & m_set_bytes;

protected:

  void do_work(int & work)
  {
    boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), 100));
    m_set_bytes[work] = 1;
  }

public:

  SimpleAsyncWorker(std::vector<char> & set_bytes)
  : async_worker<int>(4, 4),
    m_set_bytes(set_bytes)
  {
  }

  ~SimpleAsyncWorker()
  {
    stop();
  }
};

class TimeoutAsyncWorker : public async_worker<int>
{
private:

  std::vector<char> & m_set_bytes;

protected:

  void do_work(int & work)
  {
    boost::thread::sleep(xtime_util::add_ms(xtime_util::now(), 100));
    m_set_bytes[work] = 1;
  }

public:

  TimeoutAsyncWorker(std::vector<char> & set_bytes)
  : async_worker<int>(4, 4, 40),
    m_set_bytes(set_bytes)
  {
  }

  ~TimeoutAsyncWorker()
  {
    stop();
  }
};

struct Fixture
{
  std::vector<char> set_bytes;
  SimpleAsyncWorker aw;
  TimeoutAsyncWorker taw;

  Fixture()
  : set_bytes(16, 0),
    aw(set_bytes),
    taw(set_bytes)
  {
  }

  ~Fixture()
  {
    // force a stop before we destroy set_bytes
    aw.stop();
    taw.stop();
  }
};

BOOST_FIXTURE_TEST_CASE( test_do_nothing, Fixture )
{
  for (size_t i = 0; i != set_bytes.size(); ++i)
    BOOST_CHECK_EQUAL(set_bytes[i], 0);
}

BOOST_FIXTURE_TEST_CASE( test_do_something, Fixture )
{
  aw.enqueue(3);

  aw.stop();

  for (size_t i = 0; i != set_bytes.size(); ++i)
    BOOST_CHECK_EQUAL(set_bytes[i], (i == 3 ? 1 : 0));
}

BOOST_FIXTURE_TEST_CASE( test_do_multiple, Fixture )
{
  aw.enqueue(1);

  aw.enqueue(3);

  aw.enqueue(5);

  aw.enqueue(7);

  aw.stop();

  for (size_t i = 0; i != 8; ++i)
    BOOST_CHECK_EQUAL(set_bytes[i], (i % 2 == 1 ? 1 : 0));
}

// simple async should wait indefinitely for a spot to open up on the queue
BOOST_FIXTURE_TEST_CASE( test_wait, Fixture )
{
  // first 4 get taken instantly
  aw.enqueue(1);
  aw.enqueue(2);
  aw.enqueue(3);
  aw.enqueue(4);

  // next 4 get on the queue
  aw.enqueue(5);
  aw.enqueue(6);
  aw.enqueue(7);
  aw.enqueue(8);

  // final enqueue should go through
  aw.enqueue(9);

  aw.stop();

  for (size_t i = 1; i != 10; ++i)
    BOOST_CHECK_EQUAL(set_bytes[i], 1);
}

BOOST_FIXTURE_TEST_CASE( test_timeout, Fixture )
{
  // first 4 get taken instantly
  taw.enqueue(1);
  taw.enqueue(2);
  taw.enqueue(3);
  taw.enqueue(4);

  // next 4 get on the queue
  taw.enqueue(5);
  taw.enqueue(6);
  taw.enqueue(7);
  taw.enqueue(8);

  // final enqueue should time out
  BOOST_CHECK_THROW(taw.enqueue(1), enqueue_timeout);
}

BOOST_AUTO_TEST_SUITE_END()
