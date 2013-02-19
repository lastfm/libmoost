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
#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

#include "../../include/moost/testing/error_matcher.hpp"
#include "../../include/moost/container/memory_mapped_dataset.hpp"

using namespace moost::container;

typedef moost::testing::error_matcher matches;

BOOST_AUTO_TEST_SUITE(memory_mapped_dataset_test)

struct scoped_tempfile
{
   scoped_tempfile(const std::string& name, bool keep = false)
      : m_path(name)
      , m_keep(keep)
   {
   }

   ~scoped_tempfile()
   {
      if (!keep())
      {
         boost::filesystem::remove(m_path);
      }
   }

   bool keep() const
   {
      const char *env = std::getenv("KEEP_TEMP");

      if (env)
      {
         try
         {
            return boost::lexical_cast<bool>(env);
         }
         catch (const boost::bad_lexical_cast& e)
         {
            BOOST_TEST_MESSAGE("invalid value [" << env << "] for KEEP_TEMP (" << e.what() << ")");
         }
      }

      return m_keep;
   }

   std::string path() const
   {
      return m_path.string();
   }

   bool exists() const
   {
      return boost::filesystem::exists(m_path);
   }

private:
   const boost::filesystem::path m_path;
   const bool m_keep;
};

struct test_val
{
   char str[16];
};

struct const_hash
{
   template <typename T>
   size_t operator() (const T&) const
   {
      return 4711U;
   }
};

struct test_dataset : memory_mapped_dataset
{
   struct writer : public memory_mapped_dataset::writer
   {
      writer(const std::string& file)
         : memory_mapped_dataset::writer(file, "test_dataset", 4711)
      {
      }
   };

   test_dataset(const std::string& file)
      : memory_mapped_dataset(file, "test_dataset", 4711)
   {
   }
};

template <class ArchiveType>
void mmd_archive_test(const scoped_tempfile& dsfile)
{
   {
      test_dataset::writer wr(dsfile.path());
      typename ArchiveType::writer arch_wr(wr, "archive");

      std::map<std::string, boost::int32_t> testmap;
      boost::uint64_t testval = 4711;

      testmap["answer"] = 42;
      testmap["pi"] = 4;

      arch_wr << testmap << testval;

      arch_wr.commit();
      wr.close();
   }
   BOOST_REQUIRE(dsfile.exists());

   test_dataset ds(dsfile.path());
   ArchiveType arch(ds, "archive");

   std::map<std::string, boost::int32_t> map;
   boost::uint64_t val;

   arch >> map >> val;

   BOOST_CHECK_EQUAL(map.size(), 2);
   BOOST_CHECK_EQUAL(map["answer"], 42);
   BOOST_CHECK_EQUAL(map["pi"], 4);
   BOOST_CHECK_EQUAL(val, 4711);
}

BOOST_AUTO_TEST_CASE(test_mmd_archive)
{
   scoped_tempfile dsfile("archive.mmd");

   mmd_archive_test<mmd_archive>(dsfile);

   test_dataset ds(dsfile.path());
   boost::shared_ptr<mmd_binary_archive> bin;
   BOOST_CHECK_EXCEPTION(bin.reset(new mmd_binary_archive(ds, "archive")), std::runtime_error, matches("archive\\.mmd: invalid section type mmd_archive:text \\(expected mmd_archive:binary\\)"));
}

BOOST_AUTO_TEST_CASE(test_mmd_archive_binary)
{
   scoped_tempfile dsfile("archive_binary.mmd");

   mmd_archive_test<mmd_binary_archive>(dsfile);

   test_dataset ds(dsfile.path());
   boost::shared_ptr<mmd_text_archive> bin;
   BOOST_CHECK_EXCEPTION(bin.reset(new mmd_text_archive(ds, "archive")), std::runtime_error, matches("archive_binary\\.mmd: invalid section type mmd_archive:binary \\(expected mmd_archive:text\\)"));
}

BOOST_AUTO_TEST_CASE(test_mmd_archive_generic)
{
   scoped_tempfile dsfile("archive_generic.mmd");
   mmd_archive_test< mmd_generic_archive<TextArchivePolicy> >(dsfile);
}

BOOST_AUTO_TEST_CASE(test_mmd_vector)
{
   scoped_tempfile dsfile("vector.mmd");
   {
      test_dataset::writer wr(dsfile.path());
      mmd_vector<boost::uint64_t>::writer vec_wr(wr, "vec");

      for (boost::uint64_t i = 0; i < 10; ++i)
      {
         vec_wr << 3*i;
      }

      vec_wr.commit();
      wr.close();
   }
   BOOST_REQUIRE(dsfile.exists());

   test_dataset ds(dsfile.path());
   mmd_vector<boost::uint64_t> vec(ds, "vec");

   for (boost::uint64_t i = 0; i < 10; ++i)
   {
      BOOST_CHECK_EQUAL(vec[i], 3*i);
   }

   boost::shared_ptr< mmd_dense_hash_map<int, int> > dhm;
   BOOST_CHECK_EXCEPTION(dhm.reset(new mmd_dense_hash_map<int, int>(ds, "vec")), std::runtime_error, matches("vector.mmd: invalid section type mmd_vector \\(expected mmd_dense_hash_map\\)"));
}

template <class HashFcn = MMD_DEFAULT_HASH_FCN<boost::uint32_t> >
class test_dense_hash_map
{
   typedef mmd_dense_hash_map<boost::uint32_t, boost::uint32_t, HashFcn> map_type;

public:
   test_dense_hash_map(const scoped_tempfile& dsfile, size_t elements, float max_pop_ratio)
      : m_dsfile(dsfile)
      , m_elements(elements)
   {
      test_dataset::writer wr(m_dsfile.path());
      typename map_type::writer map_wr(wr, "dense", std::numeric_limits<boost::uint32_t>::max(), max_pop_ratio);

      for (size_t i = 0; i < m_elements; ++i)
      {
         std::pair<boost::uint32_t, boost::uint32_t> p;
         p.first = 7*i;
         p.second = 42 + i;
         map_wr << p;
      }

      BOOST_CHECK_EQUAL(map_wr.size(), m_elements);

      map_wr.commit();
      wr.close();
   }

   void operator() ()
   {
      BOOST_REQUIRE(m_dsfile.exists());

      test_dataset ds(m_dsfile.path());
      m_map.reset(new map_type(ds, "dense"));

      BOOST_CHECK_EQUAL(map().size(), m_elements);
      BOOST_CHECK_EQUAL(map().empty(), m_elements == 0);

      for (size_t i = 0; i < map().size(); ++i)
      {
         BOOST_CHECK_EQUAL(map()[7*i], 42 + i);
      }

      for (size_t i = 0; i < 8*map().size(); ++i)
      {
         typename map_type::const_iterator it = map().find(static_cast<typename map_type::key_type>(i));
         if (i % 7 == 0 && i < 7*map().size())
         {
            BOOST_REQUIRE(it != map().end());
            BOOST_CHECK_EQUAL(it->first, i);
            BOOST_CHECK_EQUAL(it->second, 42 + i/7);
         }
         else
         {
            BOOST_CHECK(it == map().end());
         }
      }

      std::vector<bool> existence(m_elements);
      size_t count = 0;

      for (typename map_type::const_iterator it = map().begin(); it != map().end(); ++it)
      {
         BOOST_CHECK_EQUAL(it->first % 7, 0U);
         const typename map_type::value_type& v = *it;
         BOOST_CHECK_EQUAL(it->second, 42 + v.first/7);
         BOOST_CHECK(!existence[v.first/7]);
         BOOST_CHECK(it->first/7 < m_elements);
         existence[it->first/7] = true;
         ++count;
      }

      BOOST_CHECK_EQUAL(count, m_elements);
   }

   const map_type& map() const
   {
      return *m_map;
   }

private:
   const scoped_tempfile& m_dsfile;
   const size_t m_elements;
   boost::scoped_ptr<map_type> m_map;
};

BOOST_AUTO_TEST_CASE(test_mmd_dense_hash_map)
{
   scoped_tempfile dsfile("dense_hash_map.mmd");
   test_dense_hash_map<> test(dsfile, 10, 0.8);
   test();
}

BOOST_AUTO_TEST_CASE(test_mmd_dense_hash_map_collision)
{
   scoped_tempfile dsfile("dense_hash_map_collision.mmd");
   test_dense_hash_map<const_hash> test(dsfile, 10, 0.8);
   test();
}

BOOST_AUTO_TEST_CASE(test_mmd_dense_hash_map_full)
{
   scoped_tempfile dsfile("dense_hash_map_full.mmd");
   test_dense_hash_map<> test(dsfile, 16, 0.99);
   test();
   BOOST_CHECK_EQUAL(test.map().capacity(), 16U);
   BOOST_CHECK_EQUAL(test.map().size(), 16U);
}

BOOST_AUTO_TEST_CASE(test_mmd_dense_hash_map_variety)
{
   size_t elements[] = { 0, 1, 2, 3, 4, 128, 5000 };
   float max_pop_ratio[] = {0.01, 0.1, 0.5, 0.9, 0.99};

   for (size_t ei = 0; ei < sizeof(elements)/sizeof(elements[0]); ++ei)
   {
      for (size_t ri = 0; ri < sizeof(max_pop_ratio)/sizeof(max_pop_ratio[0]); ++ri)
      {
         std::ostringstream name;
         name << "dense_hash_map_" << elements[ei] << "_" << max_pop_ratio[ri] << ".mmd";
         scoped_tempfile dsfile(name.str());
         test_dense_hash_map<> test(dsfile, elements[ei], max_pop_ratio[ri]);
         test();
      }
   }
}

BOOST_AUTO_TEST_CASE(test_mmd_dense_hash_map_error)
{
   typedef mmd_dense_hash_map<boost::int32_t, test_val, const_hash> map_type;

   test_val x[3] = {
      { "Hello World!" },
      { "012345678901234" },
      { "" },
   };

   scoped_tempfile dsfile("dense_hash_map_error.mmd");
   {
      test_dataset::writer wr(dsfile.path());
      boost::scoped_ptr<map_type::writer> map_wr;

      BOOST_CHECK_EXCEPTION(map_wr.reset(new map_type::writer(wr, "dense", -13, 1.0)), std::runtime_error, matches("invalid max_population_ratio.*"));
      BOOST_CHECK_EXCEPTION(map_wr.reset(new map_type::writer(wr, "dense", -13, 1e-4)), std::runtime_error, matches("invalid max_population_ratio.*"));
      map_wr.reset(new map_type::writer(wr, "dense", -13));
      BOOST_CHECK_EXCEPTION(*map_wr << std::make_pair(-13, x[0]), std::runtime_error, matches("attempt to insert empty key"));
      *map_wr << std::make_pair(-12, x[0]);
      BOOST_CHECK_EXCEPTION(*map_wr << std::make_pair(-13, x[1]), std::runtime_error, matches("attempt to insert empty key"));
      *map_wr << std::make_pair(-12, x[1]);
      BOOST_CHECK_EXCEPTION(map_wr->commit(), std::runtime_error, matches("duplicate key detected"));
   }
   {
      test_dataset::writer wr(dsfile.path());
      map_type::writer map_wr(wr, "dense", -13);
      map_wr << std::make_pair(-12, x[0]);
      map_wr << std::make_pair(12, x[1]);
      map_wr << std::make_pair(0, x[2]);
      map_wr.commit();
      wr.close();
   }
   BOOST_REQUIRE(dsfile.exists());

   test_dataset ds(dsfile.path());

   mmd_dense_hash_map<boost::int32_t, boost::int32_t, const_hash> map1;
   BOOST_CHECK_EXCEPTION(map1.set(ds, "dense"), std::runtime_error, matches("wrong mapped size.*"));

   mmd_dense_hash_map<boost::int64_t, test_val, const_hash> map2;
   BOOST_CHECK_EXCEPTION(map2.set(ds, "dense"), std::runtime_error, matches("wrong key size.*"));

   mmd_dense_hash_map<boost::int32_t, test_val, const_hash> map;
   map.set(ds, "dense");
   test_val target;
   BOOST_CHECK_EXCEPTION(target = map[33], std::runtime_error, matches("no such key"));

   boost::shared_ptr<test_dataset> doesnotexist;
   BOOST_CHECK_EXCEPTION(doesnotexist.reset(new test_dataset("/does/not/live/here.mmd")), BOOST_IOSTREAMS_FAILURE, matches("/does/not/live/here\\.mmd:.*"));
}

BOOST_AUTO_TEST_SUITE_END()
