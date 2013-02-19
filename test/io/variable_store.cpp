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

#include "../../include/moost/io/variable_store.hpp"

using namespace boost::filesystem;
using namespace boost::archive;
using namespace moost::io;

BOOST_AUTO_TEST_SUITE( variable_store_test )

struct Fixture
{
  struct TempDirectory
  {
    path Path;
    TempDirectory(const std::string & path_name) : Path(path_name) { create_directory(Path); }
    ~TempDirectory() { remove_all(Path); }
  };
  TempDirectory temp_dir;
  variable_store vs;

  Fixture()
  : temp_dir("variable_store_test"),
    vs("variable_store_test", 128, 1024)
  {
  }
  ~Fixture()
  {
  }
};

// can we alloc?
BOOST_FIXTURE_TEST_CASE( test_alloc, Fixture )
{
  variable_store::scoped_block block(vs, 64);
  BOOST_REQUIRE_EQUAL(block.index(), 0);
  variable_store::scoped_block block2(vs, 100);
  BOOST_REQUIRE_EQUAL(block2.index(), 1);
}

// can we write/read?
BOOST_FIXTURE_TEST_CASE( test_readwrite, Fixture )
{
  std::string in = "hello how are you";
  std::string out;
  {
    variable_store::scoped_block block(vs, 64);
    binary_oarchive(*block) << in;
  }
  {
    variable_store::scoped_block block(vs, 64, 0);
    binary_iarchive(*block) >> out;
  }
  BOOST_CHECK_EQUAL(in, out);
}

// what happens if we try to get too big a block?
BOOST_FIXTURE_TEST_CASE( test_block_too_big, Fixture )
{
  variable_store::scoped_block block(vs, 1024); // this is okay
  // but this is too big
  BOOST_CHECK_THROW(
    variable_store::scoped_block(vs, 1025),
    std::invalid_argument
  );
}

// can we write to two different blocks stores?
BOOST_FIXTURE_TEST_CASE( test_different_block_stores, Fixture )
{
  std::string in1 = "hello how are you";
  std::string in2 = "very well thanks";
  std::string out1, out2;
  {
    variable_store::scoped_block block1(vs, 64);
    BOOST_CHECK_EQUAL(block1.index(), 0);
    binary_oarchive(*block1) << in1;
    variable_store::scoped_block block2(vs, 129);
    BOOST_CHECK_EQUAL(block2.index(), 0);
    binary_oarchive(*block2) << in2;
  }
  {
    variable_store::scoped_block block1(vs, 64, 0);
    binary_iarchive(*block1) >> out1;
    variable_store::scoped_block block2(vs, 129, 0);
    binary_iarchive(*block2) >> out2;
  }
  BOOST_CHECK_EQUAL(in1, out1);
  BOOST_CHECK_EQUAL(in2, out2);
}

BOOST_AUTO_TEST_SUITE_END()
