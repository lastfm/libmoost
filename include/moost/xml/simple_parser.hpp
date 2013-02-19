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

#ifndef MOOST_XML_PARSER_H
#define MOOST_XML_PARSER_H

/**
 * \file simple_parser.h
 *
 * A very simple and basic xml parser.
 * Example, to parse:
\verbatim
<base>
   <foo>10</foo>
   <bar>20</bar>
</base>\endverbatim
\code
moost::xml::simple_parser xmlParser;

try
{
  xmlParser.load("somefile.xml", true); // true will transform all the tags into lowercase
}
catch (const std::exception& e)
{
  cerr << moost::terminal_format::getError()
       << ": Can't parse config file. " << e.what() << endl;
  exit(1);
}

typedef moost::xml::simple_parser::tree_branch_t tree_branch_t;
const tree_branch_t& root = xmlParser.get_root();

if (root.empty())
{
  cerr << moost::terminal_format::getError()
       << ": Can't parse config file <" << configFileName << ">" << endl;
  exit(1);
}

tree_branch_t::const_iterator rootIt;
tree_branch_t::const_iterator leafIt;

for ( rootIt = root.begin(); rootIt != root.end(); ++rootIt )
{
   moost::xml::simple_parser::tree_node* pCurrNode = *rootIt; // just for readability
   if ( pCurrNode->header == "base" ) // ignore tags other than "base"
   {
      // the node "base" has two other node "leaves"
      // will print
      // foo: 10
      // bar: 20
      for ( leafIt = pCurrNode->leaves.begin(); leafIt != pCurrNode->leaves.end(); ++leafIt )
         cout << leafIt->header << ": " << leafIt->value << endl;
   }
}
\endcode
*/

#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <cctype> // for tolower
#include <stdexcept>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include  <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>

namespace moost { namespace xml {

/**
 *  A very simple xml parser, generally used to parse config files
 */
class simple_parser
{
public:

   /**
   * The structure that containse the information of an xml node.
   * i.e. for <foo>10</foo>, "foo" is the header and "10" is the value.
   * If it contains subtags, they are parsed in the leaves.
   */
   struct tree_node
   {
      typedef std::vector< boost::shared_ptr<tree_node> > tree_branch_t;

      std::string    header; //< the header tag of the node
      std::string    value;  //< the value between the header tags
      tree_branch_t  leaves; //< a list of subnodes (if any)

      /**
      * Will fill the map with the header/values of the leaves (it will NOT follow the leaves!)
      * \param destMap where to put the header->values data
      */
      inline void leaves2map(std::map<std::string, std::string>& destMap) const;
   };

   typedef boost::shared_ptr<tree_node> shared_node_t;
   typedef tree_node::tree_branch_t     tree_branch_t;

public:

   /**
   * Loads the xml file and parse it.
   * \param fileName the name of the file to load and parse
   * \param makeLowercaseTags if true it will turn the tags (i.e. <foo>) into lowercase
   */
   inline void
      load(const std::string& fileName, bool makeLowercaseTags = false);

   /**
   * Returns the root of the xml document.
   */
   inline const tree_branch_t&
      get_root() const { return m_root; }

private:

   /**
   * Recursively parse a given tag.
   * \param header the header of the tag to parse. Used to make sure that the tog will be closed.
   * \param pCurrNode the node to be parsed
   * makeLowercaseTags if true it will turn the tags (i.e. <foo>) into lowercase
   */
   inline void recursive_parse_tag( const std::string& header,
                                    std::istream& xmlFile,
                                    boost::shared_ptr<tree_node>& pCurrNode,
                                    bool makeLowercaseTags = false);

   /**
   * Recursively parse the xml file.
   * Will return true if the token was IN the token, false if it was OUTSIDE the token,
   * i.e. with opening "<" and closing ">"
   * <test>hello</test>
   * test -> true
   * hello -> false
   * \param xmlFile the file to parse
   * \param token the returned parsed token
   * \param isComment in\out if it's in a comment block
   */
   inline bool parse_token( std::istream& xmlFile,
                            std::string& token, bool& isComment);

private:

   tree_branch_t m_root; //< the stored xml tree
};

// -----------------------------------------------------------------------------

void simple_parser::load(const std::string& fileName, bool makeLowercaseTags)
{
   std::ifstream xmlFile(fileName.c_str());

   if ( !xmlFile.is_open() )
      throw std::runtime_error("Cannot open file <" + fileName + ">!" );

   xmlFile.exceptions(std::ios::badbit);

   std::string tokenName;
   bool isComment;
   bool isInTag;
   for (;;)
   {
      isInTag = parse_token(xmlFile, tokenName, isComment);
      if ( xmlFile.eof() )
         break;

      if ( isInTag )
      {
         if ( isComment )
            continue;

         boost::shared_ptr<tree_node> pTmpMap(new tree_node());
         m_root.push_back(pTmpMap);

         recursive_parse_tag(tokenName, xmlFile, pTmpMap, makeLowercaseTags);
      }
   }
}

// -----------------------------------------------------------------------------

bool simple_parser::parse_token( std::istream& xmlFile,
                                 std::string& token, bool& isComment)
{
   token.clear();
   bool isInTag = false;
   isComment = false;
   char c = 0;

   for(;;)
   {
      c = xmlFile.get();
      if ( xmlFile.eof() )
         break;

      if ( isspace(c) && token.empty() )
         continue;

      if ( c == '<' && !isComment )
      {
         if ( !token.empty() ) // end of a in-token
         {
            xmlFile.putback(c);
            break;
         }

         isInTag = true;
         continue;
      }
      else if ( c == '>' )
      {
         if ( isComment )
         {
            if ( token.substr(token.size()-2,2) == "--" )
               break;
         }
         else
            break;
      }

      token.append(1, c);
      if ( token.size() == 3 && token == "!--" )
         isComment = true;
   }

   return isInTag;
}

// -----------------------------------------------------------------------------

void simple_parser::recursive_parse_tag( const std::string& header,
                                         std::istream& xmlFile,
                                         boost::shared_ptr<tree_node>& pCurrNode,
                                         bool makeLowercaseTags)
{
   pCurrNode->header = header;

   if ( makeLowercaseTags )
   {
      std::transform( pCurrNode->header.begin(), pCurrNode->header.end(),
                      pCurrNode->header.begin(), (int(*)(int)) std::tolower);
   }

   bool isInTag = false;
   bool isComment = false;

   std::string tokenName;
   std::string closeToken = "/" + header;

   for (;;)
   {
      isInTag = parse_token(xmlFile, tokenName, isComment);
      if ( xmlFile.eof() )
         throw std::runtime_error("EOF Before finding the right token!" );

      if ( isInTag )
      {
         if ( tokenName.empty() )
            continue;

         if ( isComment )
            continue;

         if ( tokenName == closeToken )
            return;

         if ( tokenName[0] == '/' )
            throw std::runtime_error("Cannot find closing token for <" + header + ">! Found <" + tokenName + "> instead!" );

         boost::shared_ptr<tree_node> pTmpMap(new tree_node());
         pCurrNode->leaves.push_back( pTmpMap );
         recursive_parse_tag(tokenName, xmlFile, pTmpMap, makeLowercaseTags);
      }
      else
         pCurrNode->value = tokenName;
   }

   throw std::runtime_error("simple_parser::recursive_parse_tag. Should never end here!");
}

// -----------------------------------------------------------------------------

void simple_parser::tree_node::leaves2map( std::map<std::string, std::string>& currentLevel ) const
{
   tree_branch_t::const_iterator leafIt;
   for ( leafIt = leaves.begin(); leafIt != leaves.end(); ++leafIt )
      currentLevel[(*leafIt)->header] = (*leafIt)->value;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename T>
struct get_opt_detail
{
   static T lexical_cast(const std::string& val)
   {
      return boost::lexical_cast<T>(val);
   }
};

template <>
struct get_opt_detail<bool>
{
   static bool lexical_cast(const std::string& val)
   {
      if (val == "true")
         return true;
      if (val == "false")
         return false;
      return boost::lexical_cast<bool>(val);
   }
};

/**
* Free function to get the value off a map (generated with leaves2map for instance).
* \param value The returned value.
* \param key The key of the map.
* \param optMap The map.
* \param throwIfNotFound if true it will throw a runtime error if the key was not found in
* the map.
*/
template <typename T>
static void get_opt( T& value,
                     const std::string& key,
                     const std::map<std::string, std::string>& optMap,
                     bool throwIfNotFound = true )
{
   std::string localKey = boost::to_lower_copy(key);
   std::map<std::string, std::string>::const_iterator f = optMap.find(localKey);
   if ( f == optMap.end() )
   {
      if ( throwIfNotFound )
         throw std::runtime_error("mandatory key <" + key + "> not found!");
   }
   else
   {
      try
      {
         value = get_opt_detail<T>::lexical_cast(f->second);
      }
      catch (const boost::bad_lexical_cast&)
      {
         throw std::runtime_error("bad cast for key <" + key + ">");
      }
   }
}

}}

// -----------------------------------------------------------------------------

#endif // MOOST_XML_PARSER_H
