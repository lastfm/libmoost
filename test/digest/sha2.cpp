/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file sha2.cpp
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

#include <string>
#include <sstream>

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

#include "../../include/moost/digest/sha2.h"

BOOST_AUTO_TEST_SUITE(moost_digest_sha2)

namespace
{

std::string hex2bin(const std::string& hex)
{
   BOOST_REQUIRE(hex.size() % 2 == 0);
   std::vector<boost::uint8_t> bin(hex.size()/2);
   for (size_t i = 0; i < bin.size(); ++i)
   {
      std::istringstream iss(hex.substr(2*i, 2));
      int byteval;
      iss >> std::hex >> byteval;
      BOOST_REQUIRE(!iss.fail());
      bin[i] = boost::numeric_cast<boost::uint8_t>(byteval);
   }
   return std::string(reinterpret_cast<const char *>(&bin[0]), bin.size());
}

struct hexdigests
{
   std::string empty;
   std::string abc;
   std::string abc_long;
   std::string zero32;
};

template <class Sha2Impl>
void sha2_test(const hexdigests& hd)
{
   Sha2Impl d;
   d.add(std::string(""));
   BOOST_CHECK_EQUAL(d.hexdigest(), hd.empty);
   BOOST_CHECK_EQUAL(d.digest(), hex2bin(hd.empty));

   d.reset();
   d.add(std::string("abc"));
   BOOST_CHECK_EQUAL(d.hexdigest(), hd.abc);

   d.reset();
   d.add(std::string("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"));
   BOOST_CHECK_EQUAL(d.hexdigest(), hd.abc_long);

   d.reset();
   d.add(std::string("abc"));
   d.add(std::string("dbcdec"));
   d.add(std::string("defdefgefghfghighijhijkijkljklmklmnlmnomnopnop"));
   d.add(std::string("q"));
   BOOST_CHECK_EQUAL(d.hexdigest(), hd.abc_long);

   boost::scoped_ptr<moost::digest::base> b(new Sha2Impl);
   b->add(std::string("abc"));
   BOOST_CHECK_EQUAL(b->hexdigest(), hd.abc);
   BOOST_CHECK_EQUAL(b->digest(), hex2bin(hd.abc));

   b->reset();
   boost::uint32_t zero = 0;
   b->add(zero);
   BOOST_CHECK_EQUAL(b->hexdigest(), hd.zero32);
   BOOST_CHECK_EQUAL(b->digest(), hex2bin(hd.zero32));
}

}

BOOST_AUTO_TEST_CASE(digest_sha224)
{
   hexdigests hd;
   hd.empty = "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f";
   hd.abc = "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7";
   hd.abc_long = "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525";
   hd.zero32 = "ac2d118dd210c8401caff1e8b29fa85d29286831505da1b86a91ed63";
   sha2_test<moost::digest::sha224>(hd);
}

BOOST_AUTO_TEST_CASE(digest_sha256)
{
   hexdigests hd;
   hd.empty = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
   hd.abc = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
   hd.abc_long = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
   hd.zero32 = "df3f619804a92fdb4057192dc43dd748ea778adc52bc498ce80524c014b81119";
   sha2_test<moost::digest::sha256>(hd);
}

BOOST_AUTO_TEST_CASE(digest_sha384)
{
   hexdigests hd;
   hd.empty = "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b";
   hd.abc = "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7";
   hd.abc_long = "3391fdddfc8dc7393707a65b1b4709397cf8b1d162af05abfe8f450de5f36bc6b0455a8520bc4e6f5fe95b1fe3c8452b";
   hd.zero32 = "394341b7182cd227c5c6b07ef8000cdfd86136c4292b8e576573ad7ed9ae41019f5818b4b971c9effc60e1ad9f1289f0";
   sha2_test<moost::digest::sha384>(hd);
}

BOOST_AUTO_TEST_CASE(digest_sha512)
{
   hexdigests hd;
   hd.empty = "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e";
   hd.abc = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
   hd.abc_long = "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445";
   hd.zero32 = "ec2d57691d9b2d40182ac565032054b7d784ba96b18bcb5be0bb4e70e3fb041eff582c8af66ee50256539f2181d7f9e53627c0189da7e75a4d5ef10ea93b20b3";
   sha2_test<moost::digest::sha512>(hd);
}

BOOST_AUTO_TEST_SUITE_END()
