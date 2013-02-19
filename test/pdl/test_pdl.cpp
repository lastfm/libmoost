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

#include <vector>
#include <deque>
#include <string>

#include <boost/test/unit_test.hpp>

#include "../../include/moost/pdl/dynamic_library.h"
#include "../../include/moost/utils/foreach.hpp"
#include "../../include/moost/testing/error_matcher.hpp"

#include "test_interface.h"

BOOST_AUTO_TEST_SUITE(moost_pdl)

namespace
{

typedef moost::testing::error_matcher matches;

std::deque<std::string> events;

std::string next_event()
{
   std::string event;

   if (!events.empty())
   {
      event = events.front();
      events.pop_front();
   }

   return event;
}

class other_test_interface : public moost::pdl::dynamic_class
{
public:
   virtual int bla() = 0;
};

typedef std::pair<std::string, int> class_t;

void test_loader(const std::string& dir, const std::string& libname, const std::vector<class_t>& classvec)
{
   std::string library = dir + '/' + libname;

   BOOST_REQUIRE(classvec.size() > 0);
   BOOST_REQUIRE_EQUAL(events.size(), 0);

   {
      moost::pdl::dynamic_library lib(library);
      BOOST_CHECK(lib.is_open());
   }

   BOOST_CHECK_EQUAL(events.size(), 2);
   BOOST_CHECK_EQUAL(next_event(), "load " + libname);
   BOOST_CHECK_EQUAL(next_event(), "unload " + libname);

   {
      moost::pdl::dynamic_library lib(library);

      BOOST_CHECK_EQUAL(events.size(), 1);
      BOOST_CHECK_EQUAL(next_event(), "load " + libname);

      foreach (const class_t& cls, classvec)
      {
         {
            boost::shared_ptr<my_test_interface> inst(lib.create<my_test_interface>(cls.first));
            BOOST_REQUIRE(inst != 0);
            BOOST_CHECK_EQUAL(events.size(), 1);
            BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 1");

            BOOST_CHECK_EQUAL(inst->do_it(), cls.second);

            boost::shared_ptr<other_test_interface> inst2;
            BOOST_CHECK_THROW(inst2 = lib.create<other_test_interface>(cls.first), moost::pdl::exception);
            BOOST_CHECK(inst2 == 0);
            BOOST_CHECK_EQUAL(events.size(), 2);
            BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 2");
            BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 2");
         }

         BOOST_CHECK_EQUAL(events.size(), 1);
         BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 1");
      }
   }

   BOOST_CHECK_EQUAL(events.size(), 1);
   BOOST_CHECK_EQUAL(next_event(), "unload " + libname);

   moost::pdl::dynamic_library lib;
   BOOST_CHECK(!lib.is_open());

   BOOST_CHECK_EQUAL(events.size(), 0);

   lib.open(library);

   BOOST_CHECK_EQUAL(events.size(), 1);
   BOOST_CHECK_EQUAL(next_event(), "load " + libname);

   lib.open(library);

   BOOST_CHECK_EQUAL(events.size(), 0);
   BOOST_CHECK(lib.is_open());

   foreach (const class_t& cls, classvec)
   {
      {
         boost::shared_ptr<my_test_interface> inst(lib.create<my_test_interface>(cls.first));
         BOOST_REQUIRE(inst != 0);
         BOOST_CHECK_EQUAL(events.size(), 1);
         BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 1");

         BOOST_CHECK_EQUAL(inst->do_it(), cls.second);

         boost::shared_ptr<other_test_interface> inst2;
         BOOST_CHECK_THROW(inst2 = lib.create<other_test_interface>(cls.first), moost::pdl::exception);
         BOOST_CHECK(inst2 == 0);
         BOOST_CHECK_EQUAL(events.size(), 2);
         BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 2");
         BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 2");
      }

      BOOST_CHECK_EQUAL(events.size(), 1);
      BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 1");
   }

   lib.close();

   BOOST_CHECK_EQUAL(events.size(), 1);
   BOOST_CHECK_EQUAL(next_event(), "unload " + libname);
   BOOST_CHECK(!lib.is_open());

   foreach (const class_t& cls, classvec)
   {
      {
         moost::pdl::dynamic_library(library).create<my_test_interface>(cls.first);
         BOOST_CHECK_EQUAL(events.size(), 4);
         BOOST_CHECK_EQUAL(next_event(), "load " + libname);
         BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 1");
         BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 1");
         BOOST_CHECK_EQUAL(next_event(), "unload " + libname);

         boost::shared_ptr<my_test_interface> inst = moost::pdl::dynamic_library(library).create<my_test_interface>(cls.first);
         BOOST_REQUIRE(inst != 0);
         BOOST_CHECK_EQUAL(events.size(), 2);
         BOOST_CHECK_EQUAL(next_event(), "load " + libname);
         BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 1");

         BOOST_CHECK_EQUAL(inst->do_it(), cls.second);

         boost::shared_ptr<other_test_interface> inst2;
         BOOST_CHECK_THROW(inst2 = moost::pdl::dynamic_library(library).create<other_test_interface>(cls.first), moost::pdl::exception);
         BOOST_CHECK(inst2 == 0);
         BOOST_CHECK_EQUAL(events.size(), 2);
         BOOST_CHECK_EQUAL(next_event(), "ctor " + libname + ":" + cls.first + " 2");
         BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 2");
      }

      BOOST_CHECK_EQUAL(events.size(), 2);
      BOOST_CHECK_EQUAL(next_event(), "dtor " + libname + ":" + cls.first + " 1");
      BOOST_CHECK_EQUAL(next_event(), "unload " + libname);
   }
}

}

extern "C" void pdl_test_event(const std::string& event)
{
   events.push_back(event);
}

BOOST_AUTO_TEST_CASE(load_library)
{
   // Check loading, unloading, creating & destroying instances

   std::vector<class_t> classvec(2);

   classvec[0].first = "my_test_class1";
   classvec[1].first = "my_test_class2";

   classvec[0].second = 1;
   classvec[1].second = 2;

   test_loader("../lib", "test_module1", classvec);

   classvec[0].second = 3;
   classvec[1].second = 4;

   test_loader("../lib", "test_module2", classvec);
}

BOOST_AUTO_TEST_CASE(load_library_multi)
{
   // Check that multiple library instances only load the shared object once

   {
      const std::string library("../lib/test_module1");

      moost::pdl::dynamic_library lib1(library);
      BOOST_CHECK_EQUAL(events.size(), 1);
      BOOST_CHECK_EQUAL(next_event(), "load test_module1");

      {
         moost::pdl::dynamic_library lib2(library);
         BOOST_CHECK_EQUAL(events.size(), 0);
      }

      BOOST_CHECK_EQUAL(events.size(), 0);
   }

   BOOST_CHECK_EQUAL(events.size(), 1);
   BOOST_CHECK_EQUAL(next_event(), "unload test_module1");
}

BOOST_AUTO_TEST_CASE(multi_instances)
{
   // Check that every instance we create is distinct

   {
      boost::shared_ptr<my_test_interface> inst2;

      {
         moost::pdl::dynamic_library lib("../lib/test_module1");
         BOOST_CHECK_EQUAL(events.size(), 1);
         BOOST_CHECK_EQUAL(next_event(), "load test_module1");

         {
            boost::shared_ptr<my_test_interface> inst1;

            inst1 = lib.create<my_test_interface>("my_test_class1");
            BOOST_CHECK_EQUAL(events.size(), 1);
            BOOST_CHECK_EQUAL(next_event(), "ctor test_module1:my_test_class1 1");

            inst2 = lib.create<my_test_interface>("my_test_class1");
            BOOST_CHECK_EQUAL(events.size(), 1);
            BOOST_CHECK_EQUAL(next_event(), "ctor test_module1:my_test_class1 2");

            BOOST_CHECK_EQUAL(inst1->do_it(), 1);
            BOOST_CHECK_EQUAL(inst1->do_it(), 11);
            BOOST_CHECK_EQUAL(inst2->do_it(), 1);
            BOOST_CHECK_EQUAL(inst2->do_it(), 11);
            BOOST_CHECK_EQUAL(inst1->do_it(), 21);
            BOOST_CHECK_EQUAL(inst2->do_it(), 21);
         }

         BOOST_CHECK_EQUAL(events.size(), 1);
         BOOST_CHECK_EQUAL(next_event(), "dtor test_module1:my_test_class1 1");

         BOOST_CHECK_EQUAL(inst2->do_it(), 31);
      }

      BOOST_CHECK_EQUAL(events.size(), 0);
   }

   BOOST_CHECK_EQUAL(events.size(), 2);
   BOOST_CHECK_EQUAL(next_event(), "dtor test_module1:my_test_class1 2");
   BOOST_CHECK_EQUAL(next_event(), "unload test_module1");
}

BOOST_AUTO_TEST_CASE(pdl_error)
{
   moost::pdl::dynamic_library dl;

   BOOST_CHECK_EXCEPTION(dl.create<my_test_interface>("foo"), moost::pdl::exception,
                         matches("no library loaded"));

   // XXX: This check won't necessarily work on Windows, as the error message is probably different
   BOOST_CHECK_EXCEPTION(dl.open("Does_Not_Exist"), moost::pdl::library_not_found_error, matches(".*Does_Not_Exist.*"));

   BOOST_CHECK(!dl.is_open());

   dl.open("../lib/test_module1");

   BOOST_CHECK(dl.is_open());

   BOOST_CHECK_EXCEPTION(dl.create<my_test_interface>("foo"), moost::pdl::class_not_found_error,
                         matches("class foo not found in ../lib/.*test_module1.*"));

   BOOST_CHECK_EXCEPTION(dl.create<my_test_interface>("my_fail_class"), moost::pdl::exception,
                         matches("failed to create instance of class my_fail_class"));

   BOOST_CHECK_EXCEPTION(dl.create<other_test_interface>("my_test_class1"), moost::pdl::exception,
                         matches("invalid type for class my_test_class1"));

   dl.close();

   BOOST_CHECK(!dl.is_open());

   BOOST_CHECK_EQUAL(events.size(), 4);
   BOOST_CHECK_EQUAL(next_event(), "load test_module1");
   BOOST_CHECK_EQUAL(next_event(), "ctor test_module1:my_test_class1 1");
   BOOST_CHECK_EQUAL(next_event(), "dtor test_module1:my_test_class1 1");
   BOOST_CHECK_EQUAL(next_event(), "unload test_module1");
}

BOOST_AUTO_TEST_SUITE_END()
