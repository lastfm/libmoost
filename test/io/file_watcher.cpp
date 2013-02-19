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

#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include "../../include/moost/io/file_watcher.hpp"
#include "../../include/moost/thread/xtime_util.hpp"

using namespace moost::io;
using namespace moost::thread;

BOOST_AUTO_TEST_SUITE( file_watcher_test )

struct Fixture
{
   file_watcher fw;
   std::string test_path;
   std::ofstream out;

   int Creates;
   int Changes;
   int Deletes;

   void file_changed(file_watcher::file_action action, const std::string & /*path*/)
   {
     switch (action)
     {
     case file_watcher::CREATED: ++Creates; break;
     case file_watcher::CHANGED: ++Changes; break;
     case file_watcher::DELETED: ++Deletes; break;
     }
   }

   Fixture()
   : test_path("file_watcher_Test_File"),
     Creates(0),
     Changes(0),
     Deletes(0)
   {
      fw.insert(test_path, boost::bind(&Fixture::file_changed, this, _1, _2));

      // start up the file_watcher
      fw.start();
   }
   ~Fixture()
   {
      // stop the file_watcher
      fw.stop();

      if (out.is_open())
         out.close();
      boost::filesystem::remove(boost::filesystem::path(test_path));
   }

   void sleep(int seconds)
   {
      boost::thread::sleep(xtime_util::add_sec(xtime_util::now(), seconds));
   }
};

// what happens when we do nothing?
BOOST_FIXTURE_TEST_CASE( test_nothing, Fixture )
{
   sleep(1);
   BOOST_REQUIRE_EQUAL(Creates, 0);
   BOOST_REQUIRE_EQUAL(Changes, 0);
   BOOST_REQUIRE_EQUAL(Deletes, 0);
}

// what happens when we create a file?
BOOST_FIXTURE_TEST_CASE( test_create, Fixture )
{
   out.open(test_path.c_str());
   sleep(1);

   BOOST_REQUIRE_EQUAL(Creates, 1);
   BOOST_REQUIRE_EQUAL(Changes, 0);
   BOOST_REQUIRE_EQUAL(Deletes, 0);
}

BOOST_FIXTURE_TEST_CASE( test_change, Fixture )
{
   out.open(test_path.c_str());
   sleep(2);

   out << "wasssaaaaapp!!!" << std::flush;

   sleep(2);

   BOOST_REQUIRE_EQUAL(Creates, 1);
   BOOST_REQUIRE_EQUAL(Changes, 1);
   BOOST_REQUIRE_EQUAL(Deletes, 0);
}

BOOST_FIXTURE_TEST_CASE( test_delete, Fixture )
{
   out.open(test_path.c_str());
   sleep(1);

   out.close();
   boost::filesystem::remove(boost::filesystem::path(test_path));

   sleep(1);

   BOOST_REQUIRE_EQUAL(Creates, 1);
   BOOST_REQUIRE_EQUAL(Changes, 0);
   BOOST_REQUIRE_EQUAL(Deletes, 1);
}

BOOST_AUTO_TEST_SUITE_END()
