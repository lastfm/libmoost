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

#include "../../include/moost/algorithm/ketama_partitioner.hpp"

using namespace moost::algorithm;

BOOST_AUTO_TEST_SUITE( ketama_partitioner_test )

BOOST_AUTO_TEST_CASE( test_ketama )
{
  int from = 9;
  int to = 10;

  ketama_partitioner<int> old_kp(from, 4096);
  ketama_partitioner<int> new_kp(to, 4096);

  std::vector<int> bucket_counts(new_kp.num_buckets());

  for (int i = 0; i != 5000000; ++i)
  {
     int old_bucket = old_kp.partition(i);
     int new_bucket = new_kp.partition(i);
     ++bucket_counts[new_bucket];
     BOOST_REQUIRE(old_bucket == new_bucket || new_bucket == to - 1);
  }

  // make sure none deviate more than 5% from the average
  int average_min = (5000000 / to) * 0.95F;
  int average_max = (5000000 / to) * 1.05F;
  for (size_t i = 0; i != bucket_counts.size(); ++i)
  {
    BOOST_CHECK_PREDICATE( std::greater<int>(), (bucket_counts[i]) (average_min) );
    BOOST_CHECK_PREDICATE( std::less<int>(), (bucket_counts[i]) (average_max) );
  }
}

BOOST_AUTO_TEST_CASE( test_bucket_Y_ketama )
{
   std::vector<int> from;
   from.push_back(1);
   from.push_back(2);
   from.push_back(3);

   std::vector<int> to;
   to.push_back(1);
   to.push_back(2);
   to.push_back(3);
   to.push_back(4);

   ketama_partitioner<int> old_kp(from, 4096);
   ketama_partitioner<int> new_kp(to, 4096);

  std::vector<int> bucket_counts(new_kp.num_buckets());

  for (int i = 0; i != 5000000; ++i)
  {
     int old_bucket = old_kp.partition(i);
     int new_bucket = new_kp.partition(i);
     ++bucket_counts[new_bucket];
     BOOST_REQUIRE(old_bucket == new_bucket || new_bucket == 3); // 3 is the index of the new server
  }

  // make sure none deviate more than 5% from the average
  int average_min = (5000000 / to.size() ) * 0.95F;
  int average_max = (5000000 / to.size() ) * 1.05F;
  for (size_t i = 0; i != bucket_counts.size(); ++i)
  {
    BOOST_CHECK_PREDICATE( std::greater<int>(), (bucket_counts[i]) (average_min) );
    BOOST_CHECK_PREDICATE( std::less<int>(), (bucket_counts[i]) (average_max) );
  }
}

BOOST_AUTO_TEST_CASE( test_bucket_string_ketama )
{
   std::vector<std::string> from;
   from.push_back("10.0.1.101:11211");
   from.push_back("10.0.1.102:11211");
   from.push_back("10.0.1.103:11211");

   std::vector<std::string> to;
   to.push_back("10.0.1.101:11211");
   to.push_back("10.0.1.102:11211");
   to.push_back("10.0.1.103:11211");
   to.push_back("10.0.1.104:11211");

   ketama_partitioner<int> old_kp(from, 4096);
   ketama_partitioner<int> new_kp(to, 4096);

  std::vector<int> bucket_counts(new_kp.num_buckets());

  const std::string new_guy = "10.0.1.104:11211";
  for (int i = 0; i != 5000000; ++i)
  {
     int old_bucket = old_kp.partition(i);
     int new_bucket = new_kp.partition(i);
     ++bucket_counts[new_bucket];
     BOOST_REQUIRE(old_bucket == new_bucket || new_bucket == 3); // 3 is the index of the new server
  }

  // make sure none deviate more than 5% from the average
  int average_min = (5000000 / to.size()) * 0.95F;
  int average_max = (5000000 / to.size()) * 1.05F;
  for (size_t i = 0; i != bucket_counts.size(); ++i)
  {
    BOOST_CHECK_PREDICATE( std::greater<int>(), (bucket_counts[i]) (average_min) );
    BOOST_CHECK_PREDICATE( std::less<int>(), (bucket_counts[i]) (average_max) );
  }
}

BOOST_AUTO_TEST_SUITE_END()
