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
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "../../include/moost/io/map_store.hpp"

using namespace boost::filesystem;
using namespace boost::archive;
using namespace moost::io;

BOOST_AUTO_TEST_SUITE( map_store_test )

struct Fixture
{
  struct TempDirectory
  {
    path Path;
    TempDirectory(const std::string & path_name) : Path(path_name) { create_directory(Path); }
    ~TempDirectory() { remove_all(Path); }
  };
  TempDirectory temp_dir;
  TempDirectory temp_dir2;
  map_store<std::string> ms;

  Fixture()
  : temp_dir("map_store_test"),
    temp_dir2("map_store_test2"),
    ms("map_store_test", 128, 1024)
  {
    ms.set_deleted_key("");
  }
  ~Fixture()
  {
  }
};

// can we alloc?
BOOST_FIXTURE_TEST_CASE( test_alloc, Fixture )
{
  map_store<std::string>::scoped_block block(ms, "hello", 64);
  BOOST_REQUIRE_EQUAL(block.index(), 0);
}

// can we write/read?
BOOST_FIXTURE_TEST_CASE( test_readwrite, Fixture )
{
  std::string in = "hello how are you";
  std::string out;
  {
    map_store<std::string>::scoped_block block(ms, "the key", 64);
    binary_oarchive(*block) << in;
  }
  {
    map_store<std::string>::scoped_block block(ms, "the key");
    binary_iarchive(*block) >> out;
  }
  BOOST_CHECK_EQUAL(in, out);
}

// what happens when we try to realloc?
BOOST_FIXTURE_TEST_CASE( test_realloc, Fixture )
{
  map_store<std::string>::scoped_block block(ms, "the key", 64);
  BOOST_CHECK_THROW(
    map_store<std::string>::scoped_block(ms, "the key", 64);,
    std::invalid_argument
  );
}

// okay, are we safe if we delete?
BOOST_FIXTURE_TEST_CASE( test_free, Fixture )
{
  {
    map_store<std::string>::scoped_block block(ms, "the key", 64);
    block.free();
  }
  {
    map_store<std::string>::scoped_block block(ms, "the key", 64);
    BOOST_CHECK_EQUAL(block.index(), 0);
  }
}

// can we find stuff if we open and close the map_store ?
BOOST_FIXTURE_TEST_CASE( test_open_close, Fixture )
{
  std::string in1 = "hello how are you";
  std::string in2 = "very well thanks";
  std::string out1, out2;
  {
    map_store<std::string> ms1("map_store_test2");
    map_store<std::string>::scoped_block block1(ms1, "the first key", 128);
    binary_oarchive(*block1) << in1;
    map_store<std::string>::scoped_block block2(ms1, "the second key", 256);
    binary_oarchive(*block2) << in2;
  }
  {
    map_store<std::string> ms2("map_store_test2");
    map_store<std::string>::scoped_block block1(ms2, "the first key");
    binary_iarchive(*block1) >> out1;
    map_store<std::string>::scoped_block block2(ms2, "the second key");
    binary_iarchive(*block2) >> out2;
  }
  BOOST_CHECK_EQUAL(in1, out1);
  BOOST_CHECK_EQUAL(in2, out2);
}

BOOST_AUTO_TEST_SUITE_END()
