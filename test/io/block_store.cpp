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

#include "../../include/moost/io/block_store.hpp"

using namespace boost::filesystem;
using namespace boost::archive;
using namespace moost::io;

BOOST_AUTO_TEST_SUITE( block_store_test )

struct Fixture
{
  struct TempDirectory
  {
    path Path;
    TempDirectory(const std::string & path_name) : Path(path_name) { create_directory(Path); }
    ~TempDirectory() { remove_all(Path); }
  };
  TempDirectory temp_dir;
  block_store bs;

  Fixture()
  : temp_dir("block_store_test"),
    bs("block_store_test/128", 128)
  {
  }
  ~Fixture()
  {
  }
};

// can we alloc?
BOOST_FIXTURE_TEST_CASE( test_alloc, Fixture )
{
  block_store::scoped_block block(bs);
  BOOST_REQUIRE_EQUAL(block.index(), 0);
  block_store::scoped_block block2(bs);
  BOOST_REQUIRE_EQUAL(block2.index(), 1);
}

// can we write/read?
BOOST_FIXTURE_TEST_CASE( test_readwrite, Fixture )
{
  std::string in = "hello how are you";
  std::string out;
  {
    block_store::scoped_block block(bs);
    binary_oarchive(*block) << in;
  }
  {
    block_store::scoped_block block(bs, 0);
    binary_iarchive(*block) >> out;
  }
  BOOST_CHECK_EQUAL(in, out);
}

// can we write/read past the beginning of file?
BOOST_FIXTURE_TEST_CASE( test_readwrite_pbof, Fixture )
{
  std::string in = "hello how are you";
  std::string out;
  {
    block_store::scoped_block block(bs, 1);
    binary_oarchive(*block) << in;
  }
  {
    block_store::scoped_block block(bs, 1);
    binary_iarchive(*block) >> out;
  }
  BOOST_CHECK_EQUAL(in, out);
}

// can we free, and does it happen at the right point?
BOOST_FIXTURE_TEST_CASE( test_free, Fixture )
{
  {
    block_store::scoped_block block(bs);
    block.free();
    block_store::scoped_block block2(bs);
    BOOST_CHECK_EQUAL(block2.index(), 1);
  }
  block_store::scoped_block block3(bs);
  BOOST_CHECK_EQUAL(block3.index(), 0);
}

// can we close a block store, then open it again and see that the end has been truncated
BOOST_FIXTURE_TEST_CASE( test_save_load_end, Fixture )
{
  {
    block_store bs2("block_store_test/128", 128);
    block_store::scoped_block block(bs2);
    block_store::scoped_block block2(bs2);
    block2.free();
  }
  block_store bs3("block_store_test/128", 128);
  block_store::scoped_block block3(bs3);
  BOOST_CHECK_EQUAL(block3.index(), 1);
}

// can we close a block store, then open it again and see that the free list has been maintained
BOOST_FIXTURE_TEST_CASE( test_save_load_freelist, Fixture )
{
  {
    block_store bs2("block_store_test/128", 128);
    block_store::scoped_block block(bs2);
    block_store::scoped_block block2(bs2);
    block2.free();
    block_store::scoped_block block3(bs2);
  }
  block_store bs3("block_store_test/128", 128);
  block_store::scoped_block block4(bs3);
  BOOST_CHECK_EQUAL(block4.index(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
