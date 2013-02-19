/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       async_batch_processor.cpp
 * \brief      Test cases for the async batch processor.
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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/test/unit_test.hpp>

#include "../../include/moost/utils/foreach.hpp"
#include "../../include/moost/thread/async_batch_processor.hpp"

using namespace moost;

namespace {

void a_job(boost::mutex& mx, volatile size_t& value, const boost::posix_time::time_duration& time)
{
   boost::this_thread::sleep(time);
   boost::mutex::scoped_lock lock(mx);
   ++value;
}

void run_batch_processor_tests(size_t num_threads, const std::vector<boost::posix_time::time_duration>& sleep_times)
{
   thread::async_batch_processor processor(num_threads);

   foreach (const boost::posix_time::time_duration& t, sleep_times)
   {
      for (size_t num_jobs = 0; num_jobs <= 16; ++num_jobs)
      {
         thread::async_batch_processor::jobs_t jobs;
         boost::mutex mx;
         volatile size_t val = 0;

         for (size_t j = 0; j < num_jobs; ++j)
         {
            jobs.push_back(boost::bind(&a_job, boost::ref(mx), boost::ref(val), boost::cref(t)));
         }

         processor.dispatch(jobs);

         BOOST_CHECK_EQUAL(val, num_jobs);
      }
   }
}

}

BOOST_AUTO_TEST_SUITE(async_batch_processor_test)

BOOST_AUTO_TEST_CASE(async_batch_processor_test)
{
   std::vector<boost::posix_time::time_duration> sleep_times;

   sleep_times.push_back(boost::posix_time::milliseconds(0));
   sleep_times.push_back(boost::posix_time::milliseconds(1));
   sleep_times.push_back(boost::posix_time::milliseconds(10));

   for (size_t num_threads = 1; num_threads <= 8; ++num_threads)
   {
      run_batch_processor_tests(num_threads, sleep_times);
   }
}

BOOST_AUTO_TEST_SUITE_END()
