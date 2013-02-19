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
#include <boost/cstdint.hpp>

#include "../../include/moost/io/file_operations.hpp"

using namespace boost::filesystem;
using namespace moost::io;

BOOST_AUTO_TEST_SUITE( file_operations_test )

struct Fixture
{
  struct TempDirectory
  {
    path Path;
    TempDirectory(const std::string & path_name) : Path(path_name) { create_directory(Path); }
    ~TempDirectory() { remove_all(Path); }
  };
  TempDirectory temp_dir;

  Fixture()
  : temp_dir("file_operations_test")
  {
  }
  ~Fixture()
  {
  }
};

// can we grow?
BOOST_FIXTURE_TEST_CASE( test_grow_size, Fixture )
{
  std::ofstream out("file_operations_test/blah.txt");
  out << "blah" << std::flush;
  file_operations::change_size("file_operations_test/blah.txt", 128);
  BOOST_CHECK_EQUAL(file_size(path("file_operations_test/blah.txt")), 128);
}

// can we grow past the 4 gb limit?
BOOST_FIXTURE_TEST_CASE( test_grow_4gb, Fixture )
{
  std::ofstream out("file_operations_test/blah.txt");
  out << "blah" << std::flush;
  file_operations::change_size("file_operations_test/blah.txt", UINT64_C(4294967396));
  BOOST_CHECK_EQUAL(file_size(path("file_operations_test/blah.txt")), UINT64_C(4294967396));
}

// can we shrink?
BOOST_FIXTURE_TEST_CASE( test_shrink_size, Fixture )
{
  std::ofstream out("file_operations_test/blah.txt");
  out << "blah blahblahblah blah" << std::flush;
  file_operations::change_size("file_operations_test/blah.txt", 4);
  BOOST_CHECK_EQUAL(file_size(path("file_operations_test/blah.txt")), 4);
}

// can we shrink to zero?
BOOST_FIXTURE_TEST_CASE( test_shrink_zero, Fixture )
{
  std::ofstream out("file_operations_test/blah.txt");
  out << "blah blahblahblah blah" << std::flush;
  file_operations::change_size("file_operations_test/blah.txt", 0);
  BOOST_CHECK_EQUAL(file_size(path("file_operations_test/blah.txt")), 0);
}

// what happens if we change something that doesn't exist?
BOOST_FIXTURE_TEST_CASE( test_filenotfound, Fixture )
{
  BOOST_CHECK_THROW(file_operations::change_size("file_operations_test/blah.txt", 128), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
