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

#include <map>
#include <fstream>
#include <cstdio>
#include <stdexcept>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "../../include/moost/io/file_backed_data_source.hpp"
#include "../../include/moost/testing/test_directory_creator.hpp"

using namespace moost::io;
using namespace std;

BOOST_AUTO_TEST_SUITE( file_backed_data_source_test )

namespace
{
   const int MIN_SECS = 100;
   const bool THROW_ON_FIRST = false;
   const double MIN_PROPORTION = 0.8;

   char const VALID_OPTIONS_XML[] = {
      "<FileBackedDataSource>\n"
      "  <Filepath>testpath</Filepath>\n"
      "  <MinSecsSinceLastLoad>100</MinSecsSinceLastLoad>\n"
      "  <ThrowOnFirstLoadFail>false</ThrowOnFirstLoadFail>\n"
      "  <MinProportionOfLastLoad>0.8</MinProportionOfLastLoad>\n"
      "</FileBackedDataSource>"
   };

   char const SOME_OPTIONS_XML[] = {
      "<FileBackedDataSource>\n"
      "  <Filepath>testpath</Filepath>\n"
      "  <MinSecsSinceLastLoad>100</MinSecsSinceLastLoad>\n"
      "  <ThrowOnFirstLoadFail>false</ThrowOnFirstLoadFail>\n"
      "</FileBackedDataSource>"
   };

   char const INVALID_OPTIONS_XML[] = {
      "<FileBackedDataSource>\n"
      "  <Filepath>testpath</Filepath>\n"
      "  <MinSecsSinceLastLoad>100</MinSecsSinceLastLoad>\n"
      "  <ThrowOnFirstLoadFail>zog</ThrowOnFirstLoadFail>\n"
      "  <MinProportionOfLastLoad>0.8</MinProportionOfLastLoad>\n"
      "</FileBackedDataSource>"
   };

   char const WRONG_OPTIONS_XML[] = {
      "<FileBackedDataSource>\n"
      "  <Filepath>testpath</Filepath>\n"
      "  <MinSecsSinceLastLoad>100</MinSecsSinceLastLoad>\n"
      "  <ThrowOnFirstLoadFail>false</ThrowOnFirstLoadFail>\n"
      "  <MinProportionOfLastLoad>0.8</MinProportionOfLastLoad>\n"
      "  <Zog>0.8</Zog>\n"
      "</FileBackedDataSource>"
   };
}

class IncrementingIntDataPolicy : public data_policy_base<int>
{
public:
   IncrementingIntDataPolicy(int iThrow = -1) : m_i(0), m_iThrow(iThrow) { }

   std::string getName() const
   {
      return "IncrementingIntData";
   }

   boost::shared_ptr<int> loadFromFile(const std::string& /*filepath*/) const
   {
      // doesn't actually read from file!
      if (m_i == m_iThrow)
      {
         ++m_i;
         throw runtime_error("this policy just threw");
      }
      boost::shared_ptr<int> pData(new int);
      *pData = m_i;
      ++m_i;
      return pData;
   }

   size_t size(boost::shared_ptr<int> /*pData*/) const
   {
      return m_i;
   }

private:
   mutable int m_i, m_iThrow;
};

struct Fixture
{
   typedef file_backed_data_source<IncrementingIntDataPolicy> int_source_t;

   Fixture() : m_tdc("MoostFileBackedDataSourceTestDirectory")
   {

   }

   virtual ~Fixture()
   {
   }

   void waitASecond() const
   {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
   }

   void writeToFile(const std::string& filepath, const std::string& toWrite, bool wait) const
   {
      if (wait)  // wait briefly so we know that the write time of the file will change
         waitASecond();

      ofstream ofs(filepath.c_str());
      ofs << toWrite;
      ofs.close();

      // useful for debug to see the time that the file_watcher is seeing
      MLOG_DEFAULT_INFO(filepath << " updated at " << boost::filesystem::last_write_time(filepath) << endl);

      if (wait)  // give the source a chance to update
         waitASecond();
   }

   void testReloadFails(boost::shared_ptr<int_source_t> pSource, const std::string& filepath) const
   {
      writeToFile(filepath, "1", false);

      pSource->load();

      size_t currentSize = pSource->size();
      boost::shared_ptr<int> pData = pSource->get_shared_ptr();
      int currentData = *pData;

      writeToFile(filepath, "2", true);  // force a reload by updating the file

      BOOST_CHECK(pSource->size() == currentSize);

      boost::shared_ptr<int> pNewData = pSource->get_shared_ptr();
      BOOST_CHECK(*pNewData == currentData);
   }

   moost::testing::test_directory_creator m_tdc;
   file_backed_data_source_factory m_sourceFactory;
};

BOOST_FIXTURE_TEST_CASE( test_initial_load, Fixture )
{
   file_backed_data_source_config conf;
   IncrementingIntDataPolicy dataPolicy;
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   BOOST_CHECK(pSource->size() == 0);

   pSource->load();

   BOOST_CHECK(pSource->size() == 1);

   boost::shared_ptr<int> pData = pSource->get_shared_ptr();
   BOOST_CHECK(*pData == 0);
}

BOOST_FIXTURE_TEST_CASE( test_auto_reload, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_auto_reload"));
   writeToFile(filepath, "1", false);

   file_backed_data_source_config conf;
   conf.minSecsSinceLastLoad = 0;  // so we can force a reload straight away
   conf.filepath = filepath;
   IncrementingIntDataPolicy dataPolicy;
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   pSource->load();

   BOOST_CHECK(pSource->size() == 1);

   boost::shared_ptr<int> pData = pSource->get_shared_ptr();
   BOOST_CHECK(*pData == 0);

   writeToFile(filepath, "2", true);  // force a reload by updating the file

   BOOST_CHECK(pSource->size() == 2);

   boost::shared_ptr<int> pNewData = pSource->get_shared_ptr();
   BOOST_CHECK(*pNewData == 1);
   BOOST_CHECK(*pData == 0);  // old pointer to data remains unaffected
}

BOOST_FIXTURE_TEST_CASE( test_first_load_fail_throw, Fixture )
{
   file_backed_data_source_config conf;

   // configure policy to throw on first load
   IncrementingIntDataPolicy dataPolicy(0);
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   BOOST_CHECK_THROW(pSource->load(), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE( test_first_load_fail_log, Fixture )
{
   file_backed_data_source_config conf;
   conf.throwOnFirstLoadFail = false;

   // configure policy to throw on first load
   IncrementingIntDataPolicy dataPolicy(0);
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   BOOST_CHECK_NO_THROW(pSource->load());
}

BOOST_FIXTURE_TEST_CASE( test_reload_too_soon, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_reload_too_soon"));

   file_backed_data_source_config conf;
   conf.minSecsSinceLastLoad = 60;
   conf.filepath = filepath;
   IncrementingIntDataPolicy dataPolicy;
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   testReloadFails(pSource, filepath);
}

BOOST_FIXTURE_TEST_CASE( test_new_data_too_small, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_new_data_too_small"));

   file_backed_data_source_config conf;
   conf.minProportionOfLastLoad = 2;  // usually this would be less than 1 of course
   conf.filepath = filepath;
   IncrementingIntDataPolicy dataPolicy;
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   testReloadFails(pSource, filepath);
}

BOOST_FIXTURE_TEST_CASE( test_load_throws, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_load_throws"));

   file_backed_data_source_config conf;

   // configure policy to throw on second load
   IncrementingIntDataPolicy dataPolicy(1);
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);

   testReloadFails(pSource, filepath);
}

BOOST_FIXTURE_TEST_CASE( test_register, Fixture )
{
   file_backed_data_source_config conf;
   IncrementingIntDataPolicy dataPolicy;
   boost::shared_ptr<int_source_t> pSource = m_sourceFactory.createFromConfig(dataPolicy, conf);
   boost::shared_ptr<int_source_t> pOther = m_sourceFactory.createFromConfig(dataPolicy, conf);

   pSource->registerLoadable(pOther);
   pSource->load();

   BOOST_CHECK(pOther->size() == 1);

   boost::shared_ptr<int> pData = pOther->get_shared_ptr();
   BOOST_CHECK(*pData == 0);
}

BOOST_FIXTURE_TEST_CASE( test_valid_options_from_xml, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_options_from_xml"));

   std::string xml(VALID_OPTIONS_XML);
   std::ofstream ofs(filepath.c_str());
   ofs << xml;
   ofs.close();

   file_backed_data_source_config_factory factory;
   file_backed_data_source_config conf = factory.createFromXml(filepath);

   BOOST_CHECK(conf.minSecsSinceLastLoad == MIN_SECS);
   BOOST_CHECK(conf.throwOnFirstLoadFail == THROW_ON_FIRST);
   BOOST_CHECK(conf.minProportionOfLastLoad == MIN_PROPORTION);
}

BOOST_FIXTURE_TEST_CASE( test_some_valid_options_from_xml, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_some_valid_options_from_xml"));

   std::string xml(SOME_OPTIONS_XML);
   std::ofstream ofs(filepath.c_str());
   ofs << xml;
   ofs.close();

   file_backed_data_source_config_factory factory;
   file_backed_data_source_config conf = factory.createFromXml(filepath);

   BOOST_CHECK(conf.minSecsSinceLastLoad == MIN_SECS);
   BOOST_CHECK(conf.throwOnFirstLoadFail == THROW_ON_FIRST);
}

BOOST_FIXTURE_TEST_CASE( test_invalid_options_from_xml, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_invalid_options_from_xml"));

   std::string xml(INVALID_OPTIONS_XML);
   std::ofstream ofs(filepath.c_str());
   ofs << xml;
   ofs.close();

   file_backed_data_source_config_factory factory;
   BOOST_CHECK_THROW(factory.createFromXml(filepath), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE( test_wrong_options_from_xml, Fixture )
{
   string filepath(m_tdc.GetFilePath("test_wrong_options_from_xml"));

   std::string xml(WRONG_OPTIONS_XML);
   std::ofstream ofs(filepath.c_str());
   ofs << xml;
   ofs.close();

   file_backed_data_source_config_factory factory;
   BOOST_CHECK_THROW(factory.createFromXml(filepath), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
