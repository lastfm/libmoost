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
#include <istream>
#include <ostream>
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "../../include/moost/thread/xtime_util.hpp"
#include "../../include/moost/io/async_writer.hpp"

#ifdef _WIN32
   #define gmtime_r( _clock, _result ) \
      ( *(_result) = *gmtime( (_clock) ), \
      (_result) )
#endif

using namespace boost::filesystem;
using namespace moost::thread;
using namespace moost::io;

BOOST_AUTO_TEST_SUITE( async_writer_test )

struct Item
{
  int x;

  void read(std::istream & in)
  {
    in.read(reinterpret_cast<char *>(&x), sizeof(x));
  }

  void write(std::ostream & out)
  {
    out.write(reinterpret_cast<const char *>(&x), sizeof(x));
  }
};

struct Fixture
{
  struct TempDirectory
  {
    path Path;
    TempDirectory(const std::string & path_name) : Path(path_name) { create_directory(Path); }
    ~TempDirectory() { remove_all(Path); }
  };
  TempDirectory temp_dir;
  async_writer<Item> aw_simple;
  async_writer<Item> aw_roll;
  Item item;

  Fixture()
  : temp_dir("async_writer_test"),
    aw_simple((temp_dir.Path / "simple").string()),
    aw_roll((temp_dir.Path / "roll").string(), count_rollover(3))
  {
    item.x = 22;
  }
  ~Fixture()
  {
  }

  void sleep(int seconds)
  {
    boost::thread::sleep(xtime_util::add_sec(xtime_util::now(), seconds));
  }
};

// what happens when we do nothing?
BOOST_FIXTURE_TEST_CASE( test_nothing, Fixture )
{
   aw_simple.stop();
   aw_roll.stop();

   directory_iterator end_it; // default construction yields past-the-end

   for (directory_iterator it( temp_dir.Path ); it != end_it; ++it )
   {
     BOOST_CHECK(false);
   }
}

// what happens when we write something
BOOST_FIXTURE_TEST_CASE( test_something, Fixture )
{
  aw_simple.enqueue(item);
  aw_simple.stop();

  directory_iterator it( temp_dir.Path );
  BOOST_REQUIRE(it != directory_iterator());

  std::ifstream in(it->string().c_str(), std::ios::binary);
  Item item2;
  item2.read(in);
  BOOST_CHECK_EQUAL(item2.x, item.x);
  BOOST_CHECK(++it == directory_iterator());
}

// what happens when we stop and then start?
BOOST_FIXTURE_TEST_CASE( test_stopstart, Fixture )
{
  aw_simple.enqueue(item);
  aw_simple.stop();

  aw_simple.start();
  aw_simple.enqueue(item);
  aw_simple.stop();

  directory_iterator it( temp_dir.Path );
  BOOST_REQUIRE(it != directory_iterator());

  std::ifstream in(it->string().c_str(), std::ios::binary);

  Item item2;
  item2.read(in);
  BOOST_CHECK_EQUAL(item2.x, item.x);

  BOOST_REQUIRE(++it != directory_iterator());

  in.close();
  in.clear();
  in.open(it->string().c_str(), std::ios::binary);
  item2.x = 0;
  item2.read(in);
  BOOST_CHECK_EQUAL(item2.x, item.x);
  BOOST_REQUIRE(++it == directory_iterator());
}

// can we force a rollover
BOOST_FIXTURE_TEST_CASE( test_rollover, Fixture )
{
  for (int i = 0; i != 4; ++i)
    aw_roll.enqueue(item);

  aw_roll.stop();

  // we should have two files, one should have 1000, the other 1
  int first_file_count = 0;
  int second_file_count = 0;

  directory_iterator it( temp_dir.Path );
  BOOST_REQUIRE(it != directory_iterator());

  std::ifstream in(it->string().c_str(), std::ios::binary);
  Item item2;
  for ( ;in.is_open(); )
  {
    item2.read(in);
    if (in.eof())
      break;
    ++first_file_count;
  }
  in.close(); in.clear();

  BOOST_REQUIRE(++it != directory_iterator());
  in.open(it->string().c_str(), std::ios::binary);
  for ( ;in.is_open(); )
  {
    item2.read(in);
    if (in.eof())
      break;
    ++second_file_count;
  }

  BOOST_CHECK(++it == directory_iterator());
  BOOST_CHECK((first_file_count == 3 && second_file_count == 1) || (first_file_count == 1 && second_file_count == 3));
}

// how about timeofday rollover
BOOST_FIXTURE_TEST_CASE( test_timeofday_rollover, Fixture )
{
  struct tm time_info;
  time_t now;
  time(&now);
  gmtime_r(&now, &time_info);
  async_writer<Item, timeofday_rollover> aw_timeofday_roll((temp_dir.Path / "roll").string(),
      timeofday_rollover(time_info.tm_hour, time_info.tm_min, time_info.tm_sec + 2));
  aw_timeofday_roll.enqueue(item);
  sleep(3);
  aw_timeofday_roll.enqueue(item);
  aw_timeofday_roll.stop();

  directory_iterator it( temp_dir.Path );
  BOOST_REQUIRE(it != directory_iterator());

  std::ifstream in(it->string().c_str(), std::ios::binary);

  Item item2;
  item2.read(in);
  BOOST_CHECK_EQUAL(item2.x, item.x);

  BOOST_REQUIRE(++it != directory_iterator());

  in.close();
  in.clear();
  in.open(it->string().c_str(), std::ios::binary);
  item2.x = 0;
  item2.read(in);
  BOOST_CHECK_EQUAL(item2.x, item.x);
  BOOST_REQUIRE(++it == directory_iterator());
}

BOOST_AUTO_TEST_SUITE_END()
