/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       murmur3.cpp
 * \brief      Test cases for murmur3 hash class.
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

#include <vector>

#include <boost/cstdint.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/tr1/unordered_map.hpp>

#include "../../include/moost/hash/murmur3.hpp"

using namespace moost::hash;

BOOST_AUTO_TEST_SUITE(murmur3_hash_test)

BOOST_AUTO_TEST_CASE(murmur3_hash_test)
{
   uint8_t u8 = 0;
   BOOST_CHECK_EQUAL(murmur3::compute32(u8, 0), 0x514e28b7);
   BOOST_CHECK_EQUAL(murmur3::compute32(u8, 1), 0x00000000);
   u8 = 255;
   BOOST_CHECK_EQUAL(murmur3::compute32(u8, 0), 0xfd6cf10d);
   BOOST_CHECK_EQUAL(murmur3::compute32(u8, 123456789), 0x8a6ddd1e);

   uint16_t u16 = 0;
   BOOST_CHECK_EQUAL(murmur3::compute32(u16, 0), 0x30f4c306);
   BOOST_CHECK_EQUAL(murmur3::compute32(u16, 1), 0x85f0b427);
   u16 = 65535;
   BOOST_CHECK_EQUAL(murmur3::compute32(u16, 0), 0x8619621f);
   BOOST_CHECK_EQUAL(murmur3::compute32(u16, 123456789), 0x1bf78566);

   uint32_t u32 = 0;
   BOOST_CHECK_EQUAL(murmur3::compute32(u32, 0), 0x2362f9de);
   BOOST_CHECK_EQUAL(murmur3::compute32(u32, 1), 0x78ed212d);
   u32 = 4294967295ul;
   BOOST_CHECK_EQUAL(murmur3::compute32(u32, 0), 0x76293b50);
   BOOST_CHECK_EQUAL(murmur3::compute32(u32, 123456789), 0xb3f4a79d);

   std::string str("");
   BOOST_CHECK_EQUAL(murmur3::compute32(str, 0), 0x00000000);
   BOOST_CHECK_EQUAL(murmur3::compute32(str, 1), 0x514e28b7);
   str = "marcus";
   BOOST_CHECK_EQUAL(murmur3::compute32(str, 0), 0xa6091d51);
   BOOST_CHECK_EQUAL(murmur3::compute32(str, 123456789), 0xe09e097b);

   std::vector<uint16_t> vec;
   BOOST_CHECK_EQUAL(murmur3::compute32(vec, 0), 0x00000000);
   BOOST_CHECK_EQUAL(murmur3::compute32(vec, 1), 0x514e28b7);
   vec.push_back(0x616d);
   vec.push_back(0x6372);
   vec.push_back(0x7375);
   BOOST_CHECK_EQUAL(murmur3::compute32(vec, 0), 0xa6091d51);
   BOOST_CHECK_EQUAL(murmur3::compute32(vec, 123456789), 0xe09e097b);

   const char *buf = "mur\0mur3!";
   BOOST_CHECK_EQUAL(murmur3::compute32(buf + 1, 7, 0), 0x2ff4e066);
   BOOST_CHECK_EQUAL(murmur3::compute32(buf + 1, 7, 1), 0x93806485);
}

BOOST_AUTO_TEST_CASE(murmur3_functor_test)
{
   std::tr1::unordered_map< std::string, int, murmur3::hash32<std::string> > umap;

   umap["42"] = 42;
   umap["marcus"] = 13;

   BOOST_CHECK_EQUAL(umap.count("42"), 1);
   BOOST_CHECK_EQUAL(umap.count("marcus"), 1);
   BOOST_CHECK_EQUAL(umap.count("foo"), 0);

   murmur3::hash32<uint32_t, 123456789> hasher;
   BOOST_CHECK_EQUAL(hasher(4294967295ul), 0xb3f4a79d);
}

BOOST_AUTO_TEST_SUITE_END()
