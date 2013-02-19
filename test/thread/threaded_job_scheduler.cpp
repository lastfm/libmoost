/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       threaded_job_scheduler.cpp
 * \brief      Test cases for the threaded job scheduler.
 * \author     Marcus Holland-Moritz (marcus@last.fm)
 * \copyright  Copyright © 2008-2013 Last.fm Limited
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

#include <set>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "../../include/moost/testing/error_matcher.hpp"

#include "../../include/moost/thread/threaded_job_scheduler.hpp"

using namespace moost;

typedef moost::testing::error_matcher matches;

namespace {

void add_to_set(boost::shared_ptr<thread::job_batch> jobs, boost::mutex& mx, std::set<int>& s, int x)
{
   {
      boost::mutex::scoped_lock lock(mx);
      if (!s.insert(x).second)
      {
         std::ostringstream oss;
         oss << "duplicate element " << x;
         throw std::runtime_error(oss.str());
      }
   }

   if (x < 512)
   {
      for (int i = 0; i < 2; ++i)
      {
         jobs->add(boost::bind(&add_to_set, jobs, boost::ref(mx), boost::ref(s), 2*x + i));
      }
   }
}

void run_scheduler_tests(thread::threaded_job_scheduler& scheduler)
{
   boost::shared_ptr<thread::threaded_job_batch> jobs(new thread::threaded_job_batch());
   boost::mutex mx;
   std::set<int> s, ref;

   for (int i = 1; i < 1024; ++i)
   {
      ref.insert(i);
   }

   // we need to check errors for elements that don't trigger
   // new jobs (i.e. >= 512)
   s.insert(631);
   s.insert(632);

   jobs->add(boost::bind(&add_to_set, jobs, boost::ref(mx), boost::ref(s), 1));

   scheduler.dispatch(jobs);

   BOOST_CHECK_EQUAL(jobs->count(), 1023);
   BOOST_CHECK_EQUAL_COLLECTIONS(s.begin(), s.end(), ref.begin(), ref.end());

   BOOST_CHECK_EQUAL(jobs->errors().size(), 2);

   std::set<std::string> err, ref_err;
   ref_err.insert("duplicate element 631");
   ref_err.insert("duplicate element 632");
   std::copy(jobs->errors().begin(), jobs->errors().end(), std::inserter(err, err.begin()));
   BOOST_CHECK_EQUAL_COLLECTIONS(err.begin(), err.end(), ref_err.begin(), ref_err.end());
}

}

BOOST_AUTO_TEST_SUITE(threaded_job_scheduler_test)

BOOST_AUTO_TEST_CASE(threaded_job_scheduler_test)
{
   boost::shared_ptr<thread::threaded_job_scheduler> scheduler;

   BOOST_CHECK_EXCEPTION(scheduler.reset(new thread::threaded_job_scheduler(0)), std::runtime_error, matches("invalid number of worker threads"));

   for (size_t num_threads = 1; num_threads <= 8; ++num_threads)
   {
      scheduler.reset(new thread::threaded_job_scheduler(num_threads));
      run_scheduler_tests(*scheduler);
   }
}

BOOST_AUTO_TEST_SUITE_END()
