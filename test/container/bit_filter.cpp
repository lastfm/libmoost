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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "../../include/moost/container/bit_filter.hpp"
#include <vector>
#include <set>

using namespace moost::container;

BOOST_AUTO_TEST_SUITE( bit_filter_tests )

struct Fixture
{
   Fixture()
   {
   }
};

namespace {

   // Arbitrary and unique values
   size_t const raw_entropy[] = {
      0x7f, 0x00, 0x5d, 0x0c, 0x9c, 0x65, 0xa2, 0xc9,
      0x3a, 0x78, 0x59, 0xa7, 0xf0, 0x98, 0x0f, 0xcf,
      0xd4, 0xfa, 0x2c, 0xd1, 0x90, 0xb5, 0x6f, 0x4d,
      0xc0, 0x73, 0xeb, 0xa6, 0x53, 0xa3, 0x92, 0xa9,
      0x69, 0xa0, 0x4a, 0x29, 0xa5, 0xa8, 0x44, 0x61,
      0x0e, 0x57, 0x75, 0xc8, 0x8a, 0x7b, 0x14, 0x2f,
      0xf3, 0x04, 0xb8, 0xed, 0xd7, 0xe6, 0xb7, 0x70,
      0xff, 0xdc, 0x0d, 0x51, 0xc1, 0x41, 0x9a, 0xac,
      0x1f, 0xf6, 0xc4, 0xc6, 0x23, 0xaf, 0xbb, 0x5a,
      0x4b, 0x87, 0x24, 0x95, 0xa1, 0x27, 0x30, 0xec,
      0x2b, 0xe2, 0x88, 0xde, 0x05, 0xdf, 0xef, 0x3d,
      0x16, 0x07, 0xf9, 0xd3, 0xd5, 0x06, 0xca, 0x35,
      0xcd, 0x6c, 0xfe, 0x5b, 0x5c, 0x0b, 0x4e, 0x25
   };

   size_t const raw_entropy_size = sizeof(raw_entropy)/sizeof(raw_entropy[0]);


typedef std::vector<size_t> entropy_t;

entropy_t
   create_entroy_vector()
{
   return std::vector<size_t> (raw_entropy, raw_entropy + raw_entropy_size);
}

entropy_t
   create_alt_entroy_vector()
{
   std::set<size_t> s(raw_entropy, raw_entropy + raw_entropy_size);
   std::vector<size_t> v;

   for(size_t i = 0 ; i <= 0xFF ; ++i)
   {
      if(s.find(i) == s.end())
      {
         v.push_back(i);
      }
   }

   return v;
}

entropy_t
   create_all_entroy_vector()
{
   std::vector<size_t> v;

   for(size_t i = 0 ; i <= 0xFF ; ++i)
   {
      v.push_back(i);
   }

   return v;
}

bit_filter<size_t>
   create_bit_filter(size_t const size = 0xFFFF, size_t const entry_cnt = raw_entropy_size)
{
   entropy_t const & entropy = create_entroy_vector();
   return bit_filter<size_t> (size, entropy.begin(), entropy.begin() + std::min(entry_cnt, entropy.size()), true);
}

bit_filter<size_t>
   create_alt_bit_filter(size_t const size = 0xFFFF, size_t const entry_cnt = raw_entropy_size)
{
   entropy_t const & entropy = create_alt_entroy_vector();
   return bit_filter<size_t> (size, entropy.begin(), entropy.begin() + std::min(entry_cnt, entropy.size()), true);
}

bit_filter<size_t>
   create_all_bit_filter(size_t const size = 0xFFFF, size_t const entry_cnt = raw_entropy_size)
{
   entropy_t const & entropy = create_all_entroy_vector();
   return bit_filter<size_t> (size, entropy.begin(), entropy.begin() + std::min(entry_cnt, entropy.size()), true);
}
}

BOOST_FIXTURE_TEST_CASE( test_bit_filter_scalar_find, Fixture )
{
   bit_filter<size_t> bf = create_bit_filter();
   entropy_t const & entropy = create_entroy_vector();
   entropy_t const & alt_entropy = create_alt_entroy_vector();

   foreach(size_t item, entropy)
   {
      BOOST_CHECK(bf.find(item));
   }

   foreach(size_t item, alt_entropy)
   {
      BOOST_CHECK(!bf.find(item));
   }

}

BOOST_FIXTURE_TEST_CASE( test_bit_filter_vector_find_only, Fixture )
{
   bit_filter<size_t> bf = create_bit_filter();
   entropy_t const & entropy = create_entroy_vector();
   entropy_t const & alt_entropy = create_alt_entroy_vector();

   BOOST_CHECK(entropy.size() == bf.find(entropy.begin(), entropy.end()));
   BOOST_CHECK(0 == bf.find(alt_entropy.begin(), alt_entropy.end()));
}

BOOST_FIXTURE_TEST_CASE( test_bit_filter_vector_find_what, Fixture )
{
   bit_filter<size_t> bf = create_bit_filter();
   entropy_t const & entropy = create_entroy_vector();
   entropy_t const & alt_entropy = create_alt_entroy_vector();

   entropy_t result1;
   BOOST_CHECK(entropy.size() == bf.find(entropy.begin(), entropy.end(), std::back_inserter(result1)));
   BOOST_CHECK(entropy == result1);

   entropy_t result2;
   BOOST_CHECK(0 == bf.find(alt_entropy.begin(), alt_entropy.end(), std::back_inserter(result2)));
   BOOST_CHECK(alt_entropy != result2);
}

BOOST_FIXTURE_TEST_CASE( test_bit_filter_vector_find_any, Fixture )
{
   bit_filter<size_t> bf = create_bit_filter();
   bit_filter<size_t> alt_bf = create_alt_bit_filter();
   bit_filter<size_t> all_bf = create_all_bit_filter();

   BOOST_CHECK(all_bf.find(all_bf));
   BOOST_CHECK(all_bf.find(alt_bf));
   BOOST_CHECK(all_bf.find(bf));

   BOOST_CHECK(alt_bf.find(all_bf));
   BOOST_CHECK(alt_bf.find(alt_bf));
   BOOST_CHECK(!alt_bf.find(bf));

   BOOST_CHECK(bf.find(all_bf));
   BOOST_CHECK(!bf.find(alt_bf));
   BOOST_CHECK(bf.find(bf));
}

BOOST_FIXTURE_TEST_CASE( test_bit_filter_count_and_size, Fixture )
{
   bit_filter<size_t> bf = create_bit_filter();
   bit_filter<size_t> alt_bf = create_alt_bit_filter();
   bit_filter<size_t> all_bf = create_all_bit_filter();

   BOOST_CHECK(bf.size() == 0xFFFF);
   BOOST_CHECK(alt_bf.size() == 0xFFFF);
   BOOST_CHECK(all_bf.size() == 0xFFFF);
}

BOOST_FIXTURE_TEST_CASE( test_bit_filter_count_serialize, Fixture )
{
   bit_filter<size_t> bf = create_bit_filter();
   bit_filter<size_t> alt_bf = create_alt_bit_filter();
   bit_filter<size_t> all_bf = create_all_bit_filter();

   bit_filter_types::serial_buffer_t serial_buffer;
   bit_filter_types::serial_buffer_t alt_serial_buffer;
   bit_filter_types::serial_buffer_t all_serial_buffer;

   bf >> serial_buffer;
   alt_bf >> alt_serial_buffer;
   all_bf >> all_serial_buffer;

   bit_filter<size_t> bf_result = create_bit_filter();
   bit_filter<size_t> alt_bf_result = create_alt_bit_filter();
   bit_filter<size_t> all_bf_result = create_all_bit_filter();

   bf_result << serial_buffer;
   alt_bf_result << alt_serial_buffer;
   all_bf_result << all_serial_buffer;

   BOOST_CHECK(bf == bf_result);
   BOOST_CHECK(alt_bf == alt_bf_result);
   BOOST_CHECK(all_bf == all_bf_result);
}

BOOST_AUTO_TEST_SUITE_END()
