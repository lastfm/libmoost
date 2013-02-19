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
#include <boost/lexical_cast.hpp>

#include <map>
#include <string>

#include "../../include/moost/testing/test_directory_creator.hpp"

// to be changed to <moost/xml/simple_parser.hpp>!!!
#include "../../include/moost/xml/simple_parser.hpp"

using namespace moost;
using namespace moost::xml;

// -----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( simple_parser_test )

#define TESTPATH "SimpleParserTest_Directory"

struct Fixture
{

   Fixture()
   : m_tdc(TESTPATH)
   {
   }
   ~Fixture()
   {
   }

   moost::testing::test_directory_creator m_tdc;
   simple_parser m_parser;
};

// -----------------------------------------------------------------------------

namespace
{

   char const NO_CHILDREN_XML[] = {
      "<base>\n"
      "  <foo>1</foo>\n"
      "  <bar>2</bar>\n"
      "</base>\n"
   };

   char const ONE_LEVEL_CHILD_XML[] = {
      "<base>\n"
      "  <node>\n"
      "    <foo>1</foo>\n"
      "    <bar>2</bar>\n"
      "  </node>\n"
      "  <node>\n"
      "    <foo>3</foo>\n"
      "    <bar>4</bar>\n"
      "  </node>\n"
      "</base>\n"
   };

   char const NOT_CLOSED_TAG_XML[] = {
      "<base>\n"
      "  <node>\n"
      "    <foo>1\n"  // here!
      "    <bar>2</bar>\n"
      "  </node>\n"
      "</base>\n"
   };

   char const CLOSING_SLASH_MISSING_XML[] = {
      "<base>\n"
      "  <node>\n"
      "    <foo>1<foo>\n" // here
      "    <bar>2</bar>\n"
      "  </node>\n"
      "</base>\n"
   };

   char const WRONG_CLOSING_TAG_XML[] = {
      "<base>\n"
      "  <node>\n"
      "    <foo>1</somethingelse>\n" // here
      "    <bar>2</bar>\n"
      "  </node>\n"
      "</base>\n"
   };

   char const COMMENT_XML[] = {
      "<base>\n"
      "  <node>\n"
      "    <foo>1</foo>\n" // here
      "   <!-- <bar>2</bar> \n"
      "      hello hello\n"
      "      <commented_out>10</commented_out>\n"
      "  end of commend here -->\n"
      "  </node>\n"
      "</base>\n"
   };

   char const TO_LOWERCASE_XML[] = {
      "<BASE>\n"
      "  <FOO>one</FOO>\n"
      "  <BAR>two</BAR>\n"
      "</BASE>\n"
   };
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_file_not_found, Fixture )
{
   BOOST_CHECK_THROW( m_parser.load("unexisting_file.xml"), std::exception );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_no_children, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_no_children.xml");
   std::string sCfg(NO_CHILDREN_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   m_parser.load(sProxyFilename);
   const simple_parser::tree_branch_t& root = m_parser.get_root();

   BOOST_REQUIRE( root.size() == 1 ); // the head

   BOOST_CHECK_EQUAL( root.front()->header, "base" );

   simple_parser::tree_branch_t& leaves = root.front()->leaves;

   //std::vector<simple_parser::tree_node*>& leaves = root.front()->leaves;
   BOOST_REQUIRE( leaves.size() == 2 ); // must contain foo and bar

   BOOST_CHECK_EQUAL( leaves[0]->header, "foo" );
   BOOST_CHECK_EQUAL( leaves[0]->value, "1" );
   BOOST_CHECK( leaves[0]->leaves.empty() );

   BOOST_CHECK_EQUAL( leaves[1]->header, "bar" );
   BOOST_CHECK_EQUAL( leaves[1]->value, "2" );
   BOOST_CHECK( leaves[1]->leaves.empty() );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_one_level_child, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_one_level.xml");
   std::string sCfg(ONE_LEVEL_CHILD_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   m_parser.load(sProxyFilename);
   const simple_parser::tree_branch_t& root = m_parser.get_root();

   BOOST_REQUIRE( root.size() == 1 ); // the head
   BOOST_CHECK_EQUAL( root.front()->header, "base" );

   simple_parser::tree_branch_t& nodes = root.front()->leaves;
   BOOST_REQUIRE( nodes.size() == 2 ); // must contain two nodes

   BOOST_CHECK_EQUAL( nodes[0]->header, "node" );
   BOOST_CHECK( nodes[0]->value.empty() );
   BOOST_CHECK_EQUAL( nodes[1]->header, "node" );
   BOOST_CHECK( nodes[1]->value.empty() );

   for ( int i = 0; i < 2; ++i )
   {
      simple_parser::tree_branch_t& node_leaves = nodes[i]->leaves;
      BOOST_CHECK_EQUAL( node_leaves[0]->header, "foo" );
      BOOST_CHECK_EQUAL( node_leaves[0]->value, boost::lexical_cast<std::string>(2*i + 1) );
      BOOST_CHECK( node_leaves[0]->leaves.empty() );

      BOOST_CHECK_EQUAL( node_leaves[1]->header, "bar" );
      BOOST_CHECK_EQUAL( node_leaves[1]->value, boost::lexical_cast<std::string>(2*i + 2) );
      BOOST_CHECK( node_leaves[1]->leaves.empty() );
   }
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_no_closed_tags, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_no_closed_tags.xml");
   std::string sCfg(NOT_CLOSED_TAG_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   BOOST_CHECK_THROW( m_parser.load(sProxyFilename), std::exception );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_slash_missing, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_slash_missing.xml");
   std::string sCfg(CLOSING_SLASH_MISSING_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   BOOST_CHECK_THROW( m_parser.load(sProxyFilename), std::exception );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_wrong_closing, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_wrong_closing.xml");
   std::string sCfg(WRONG_CLOSING_TAG_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   BOOST_CHECK_THROW( m_parser.load(sProxyFilename), std::exception );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_comment, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("test_comment.xml");
   std::string sCfg(COMMENT_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   m_parser.load(sProxyFilename);
   const simple_parser::tree_branch_t& root = m_parser.get_root();

   BOOST_REQUIRE( root.size() == 1 ); // the head
   BOOST_CHECK_EQUAL( root.front()->header, "base" );

   simple_parser::tree_branch_t& nodes = root.front()->leaves;
   BOOST_REQUIRE( nodes.size() == 1 ); // must contain one node

   BOOST_CHECK_EQUAL( nodes[0]->header, "node" );
   BOOST_CHECK( nodes[0]->value.empty() );

   simple_parser::tree_branch_t& node_leaves = nodes[0]->leaves;
   BOOST_CHECK_EQUAL( node_leaves[0]->header, "foo" );
   BOOST_CHECK_EQUAL( node_leaves[0]->value, "1" );
   BOOST_CHECK( node_leaves[0]->leaves.empty() );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_to_lowercase_1, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_to_lowercase_1.xml");
   std::string sCfg(TO_LOWERCASE_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   m_parser.load(sProxyFilename);

   const simple_parser::tree_branch_t& root = m_parser.get_root();

   BOOST_REQUIRE( root.size() == 1 ); // the head
   BOOST_CHECK_EQUAL( root.front()->header, "BASE" );

   simple_parser::tree_branch_t& leaves = root.front()->leaves;
   BOOST_REQUIRE( leaves.size() == 2 ); // must contain foo and bar

   BOOST_CHECK_EQUAL( leaves[0]->header, "FOO" );
   BOOST_CHECK_EQUAL( leaves[0]->value, "one" );
   BOOST_CHECK_EQUAL( leaves[1]->header, "BAR" );
   BOOST_CHECK_EQUAL( leaves[1]->value, "two" );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_to_lowercase_2, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_to_lowercase_2.xml");
   std::string sCfg(TO_LOWERCASE_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   m_parser.load(sProxyFilename, true);

   const simple_parser::tree_branch_t& root = m_parser.get_root();

   BOOST_REQUIRE( root.size() == 1 ); // the head
   BOOST_CHECK_EQUAL( root.front()->header, "base" );

   simple_parser::tree_branch_t& leaves = root.front()->leaves;
   BOOST_REQUIRE( leaves.size() == 2 ); // must contain foo and bar

   BOOST_CHECK_EQUAL( leaves[0]->header, "foo" );
   BOOST_CHECK_EQUAL( leaves[0]->value, "one" );
   BOOST_CHECK_EQUAL( leaves[1]->header, "bar" );
   BOOST_CHECK_EQUAL( leaves[1]->value, "two" );
}

// -----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( test_fill_from_branch, Fixture )
{
   std::string const& sProxyFilename = m_tdc.GetFilePath("parser_test_fill_from_branch.xml");
   std::string sCfg(NO_CHILDREN_XML);
   {
      std::ofstream ofs(sProxyFilename.c_str());
      ofs << sCfg;
   }

   m_parser.load(sProxyFilename);
   const simple_parser::tree_branch_t& root = m_parser.get_root();

   std::map<std::string, std::string> localMap;
   root.front()->leaves2map(localMap);

   BOOST_CHECK_EQUAL( localMap.size(), 2 );
   BOOST_CHECK_EQUAL( localMap["foo"], "1" );
   BOOST_CHECK_EQUAL( localMap["bar"], "2" );
}

// -----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
