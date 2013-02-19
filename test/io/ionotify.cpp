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

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include "../../include/moost/io/ionotify.hpp"
#include "../../include/moost/testing/test_directory_creator.hpp"

#include <vector>
#include <fstream>

using namespace moost::io;

BOOST_AUTO_TEST_SUITE( ionotify_test )

struct Fixture
{
   moost::testing::test_directory_creator tdc;


   Fixture()
   {
   }

   ~Fixture()
   {
   }

};

namespace
{
   typedef std::vector<ionotify::file_action> file_actions_t;

   struct functor
   {
      functor(file_actions_t & file_actions) : file_actions(file_actions)
      {
      }

      file_actions_t & file_actions;

      void operator()(ionotify::file_action fa, const std::string & /*path*/)
      {
         if(file_actions.empty() || file_actions.back() != fa)
            file_actions.push_back(fa);
      }
   };
}

BOOST_FIXTURE_TEST_CASE( test_ionotify1, Fixture )
{
   std::string const & fname = tdc.GetFilePath("testfile");

   std::ofstream out(fname.c_str());

   file_actions_t file_actions;
   functor f(file_actions);

   file_actions_t expected;
   expected.push_back(ionotify::CHANGED);
   expected.push_back(ionotify::DELETED);

   ionotify ion;
   ion.insert(fname, f);
   ion.start();

   out << "hello" << std::endl;
   boost::this_thread::sleep(boost::posix_time::seconds(1));
   out << "world" << std::endl;
   boost::this_thread::sleep(boost::posix_time::seconds(1));
   out << "goodnight" << std::endl;
   boost::this_thread::sleep(boost::posix_time::seconds(1));
   out << "vienna" << std::endl;
   boost::this_thread::sleep(boost::posix_time::seconds(1));

   out.close();

   boost::filesystem::remove(fname);

   // Give the IO time to react and trigger the events
   for(size_t x = 0 ; x < 5 && file_actions.size() < expected.size() ;  ++x)
   {
      boost::this_thread::sleep(boost::posix_time::time_duration(0,0,1));
   }

   ion.stop();

// The windows version uses file_watcher and the results are not
// as accurate as inotify so there tests fail in windows :(
#ifndef WIN32
   BOOST_ASSERT(expected == file_actions);
#endif
}


BOOST_FIXTURE_TEST_CASE( test_ionotify2, Fixture )
{
   std::string const & fname = tdc.GetFilePath("testfile");

   std::ofstream out(fname.c_str());

   file_actions_t file_actions;
   functor f(file_actions);

   file_actions_t expected;
   expected.push_back(ionotify::CHANGED);
   expected.push_back(ionotify::DELETED);

   ionotify ion;
   ion.insert(fname, f);
   ion.start();

   out << "hello" << std::endl;
   out << "world" << std::endl;
   out << "goodnight" << std::endl;
   out << "vienna" << std::endl;

   out.close();

   boost::filesystem::remove(fname);

   // Give the IO time to react and trigger the events
   for(int x = 0 ; x < 5 && file_actions.size() < expected.size() ;  ++x)
   {
      boost::this_thread::sleep(boost::posix_time::time_duration(0,0,1));
   }

   ion.stop();

// The windows version uses file_watcher and the results are not
// as accurate as inotify so there tests fail in windows :(
#ifndef WIN32
   BOOST_ASSERT(expected == file_actions);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
