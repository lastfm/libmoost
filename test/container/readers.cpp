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
#include <boost/cstdint.hpp>

#include <vector>
#include <fstream>
#include <iostream>

#include "../../include/moost/container/policies/readers.hpp"

using namespace moost::container::policies;
using namespace std;

BOOST_AUTO_TEST_SUITE( readers_test )

struct Fixture_sparsevec
{
   typedef vector<pair<int, float> > sparsevec_t;

   Fixture_sparsevec() : m_testbase("reader_test.txt"),
      m_tsv_testfile("tsv_" + m_testbase),
      m_python_testfile("python_" + m_testbase),
      m_cf_testfile("cf_" + m_testbase)
   {
      ofstream out;
      out.open(m_tsv_testfile.c_str());
      out << "1\t10\t10.1\t100\t100.1\t1000\t1000.1\n";
      out << "2\t20\t20.2\t200\t200.2\t2000\t2000.2\t20000\t20000.2\n";
      out.close();
      out.open(m_python_testfile.c_str());
      out << "1\t(10, 10.1)\t(100, 100.1)\t(1000, 1000.1)\n";
      out << "2\t(20, 20.2)\t(200, 200.2)\t(2000, 2000.2)\t(20000, 20000.2)\n";
      out.close();
      out.open(m_cf_testfile.c_str());
      out << "1\t10\t10.1\n";
      out << "1\t100\t100.1\n";
      out << "1\t1000\t1000.1\n";
      out << "2\t20\t20.2\n";
      out << "2\t200\t200.2\n";
      out << "2\t2000\t2000.2\n";
      out << "2\t20000\t20000.2\n";
      out.close();

      sparsevec_t v;

      m_keys.push_back(1);
      v.push_back(std::make_pair(10, 10.1f));
      v.push_back(std::make_pair(100, 100.1f));
      v.push_back(std::make_pair(1000, 1000.1f));
      m_vecs.push_back(v);

      m_keys.push_back(2);
      v.clear();
      v.push_back(std::make_pair(20, 20.2f));
      v.push_back(std::make_pair(200, 200.2f));
      v.push_back(std::make_pair(2000, 2000.2f));
      v.push_back(std::make_pair(20000, 20000.2f));
      m_vecs.push_back(v);
   }

   virtual ~Fixture_sparsevec()
   {
      remove(m_tsv_testfile.c_str());
      remove(m_python_testfile.c_str());
      remove(m_cf_testfile.c_str());
   }

   void check_sparsevec(const sparsevec_t& vec, const sparsevec_t& expected)
   {
      BOOST_CHECK_MESSAGE(vec.size() == expected.size(), "wrong vec size " << vec.size());
      for (size_t i = 0; i < vec.size(); ++i)
      {
         BOOST_CHECK(vec[i] == expected[i]);
      }
   }

   template<typename Reader>
   void check_reader(const string& datafile)
   {
      ifstream ifs(datafile.c_str());
      BOOST_CHECK_MESSAGE(ifs.is_open(), "couldn't open test data file " << datafile);

      Reader reader(ifs);

      int key;
      sparsevec_t val;

      bool sort_by_value = false;

      size_t i = 0;
      while (reader.read(key, val, sort_by_value))
      {
         BOOST_CHECK(key == m_keys[i]);
         check_sparsevec(val, m_vecs[i]);
         ++i;
      }

      BOOST_CHECK_MESSAGE(i == m_vecs.size(), "read too many records from " << datafile);
   }

   string m_testbase, m_tsv_testfile, m_python_testfile, m_cf_testfile;
   vector<int> m_keys;
   vector<sparsevec_t> m_vecs;
};

struct Fixture_vec
{
   typedef vector<int> vec_t;

   Fixture_vec() : m_testbase("reader_test.txt"),
      m_tsv_testfile("tsv_" + m_testbase),
      m_python_testfile("python_" + m_testbase),
      m_cf_testfile("cf_" + m_testbase)
   {
      ofstream out;
      out.open(m_tsv_testfile.c_str());
      out << "95069315566190\t10\t100\t1000\n";
      out << "95069315566191\t20\t200\t2000\t20000\n";
      out.close();
      out.open(m_python_testfile.c_str());
      out << "95069315566190\t[10, 100, 1000]\n";
      out << "95069315566191\t[20, 200, 2000, 20000]\n";
      out.close();

      vec_t v;

      m_keys.push_back(95069315566190);
      v.push_back(10);
      v.push_back(100);
      v.push_back(1000);
      m_vecs.push_back(v);

      m_keys.push_back(95069315566191);
      v.clear();
      v.push_back(20);
      v.push_back(200);
      v.push_back(2000);
      v.push_back(20000);
      m_vecs.push_back(v);
   }

   virtual ~Fixture_vec()
   {
      remove(m_tsv_testfile.c_str());
      remove(m_python_testfile.c_str());
      remove(m_cf_testfile.c_str());
   }

   void check_vec(const vec_t& vec, const vec_t& expected)
   {
      BOOST_CHECK_MESSAGE(vec.size() == expected.size(), "wrong vec size " << vec.size());
      for (size_t i = 0; i < vec.size(); ++i)
      {
         BOOST_CHECK(vec[i] == expected[i]);
      }
   }

   template<typename Reader>
   void check_reader(const string& datafile)
   {
      ifstream ifs(datafile.c_str());
      BOOST_CHECK_MESSAGE(ifs.is_open(), "couldn't open test data file " << datafile);

      Reader reader(ifs);

      boost::intmax_t key;
      vec_t val;

      bool sort_by_value = false;

      size_t i = 0;
      while (reader.read(key, val, sort_by_value))
      {
         BOOST_CHECK(key == m_keys[i]);
         check_vec(val, m_vecs[i]);
         ++i;
      }

      BOOST_CHECK_MESSAGE(i == m_vecs.size(), "read too many records from " << datafile);
   }

   string m_testbase, m_tsv_testfile, m_python_testfile, m_cf_testfile;
   vector<boost::intmax_t> m_keys;
   vector<vec_t> m_vecs;
};

BOOST_FIXTURE_TEST_CASE( test_sparsevec_readers, Fixture_sparsevec )
{
   check_reader<tsv_sparsevec_reader<int, float> >(m_tsv_testfile);
   check_reader<cf_sparsevec_reader<int, float> >(m_cf_testfile);
   check_reader<python_sparsevec_reader<int, float> >(m_python_testfile);
}

BOOST_FIXTURE_TEST_CASE( test_vec_readers, Fixture_vec )
{
   check_reader<tsv_vec_reader<boost::intmax_t, int> >(m_tsv_testfile);
   check_reader<python_vec_reader<boost::intmax_t, int> >(m_python_testfile);
}

BOOST_AUTO_TEST_SUITE_END()
