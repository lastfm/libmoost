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

#ifndef MOOST_CONTAINER_POLICIES_READERS_HPP
#define MOOST_CONTAINER_POLICIES_READERS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>

#include "../../which.hpp"

namespace moost { namespace container { namespace policies {

/*
*  some simple readers to parse vector data
*  from text files in various formats
*  for use with simple_multi_map
*
*  K is the key type and must support:
*  K k; istream >> k;
*/

/*
* first some little traits classes
* so we can use sscanf to speed up
* reading from file
*/
template<typename T>
struct reader_value_traits;

template<>
struct reader_value_traits<float>
{
   typedef float value_type;
   static inline std::string format_string() { return "%f"; }
};

template<>
struct reader_value_traits<double>
{
   typedef double value_type;
   static inline std::string format_string() { return "%lg"; }
};

template<>
struct reader_value_traits<int>
{
   typedef int value_type;
   static inline std::string format_string() { return "%d"; }
};

template<>
struct reader_value_traits<long>
{
   typedef long value_type;
   static inline std::string format_string() { return "%l"; }
};

#if defined(__GNUC__) && ULONG_MAX == 0xFFFFFFFF

template<>
struct reader_value_traits<long long>
{
   typedef long long value_type;
   static inline std::string format_string() { return "%ll"; }
};

template<>
struct reader_value_traits<unsigned long long>
{
   typedef unsigned long long value_type;
   static inline std::string format_string() { return "%llu"; }
};

#endif

/*
* now the readers themselves
*/

// this expects
// id idx val idx val idx val...
template<typename K, typename T>
class tsv_sparsevec_reader
{
public:
   typedef std::vector<std::pair<int, T> > sparsevec_t;

   tsv_sparsevec_reader(std::istream& is)
      : m_is(is) { }

   tsv_sparsevec_reader(const std::string& fileName)
      : m_is(m_ifs)
   {
      m_ifs.open(fileName.c_str());
      if ( !m_ifs.is_open() )
         throw std::runtime_error("Cannot open file <" + fileName + ">!");
   }

   bool read(K& key, sparsevec_t& vec, bool sort_by_value)
   {
      if (m_is.eof())
         return false;
      std::string line;
      getline(m_is, line);
      if (line.empty())
         return false;

      vec.clear();

      std::istringstream iss(line);
      iss >> key;
      // now read the remaining values into a vector
      int idx;
      T val;
      while (iss >> idx >> val)
      {
         vec.push_back(std::make_pair(idx, val));
      }

      if (sort_by_value)   // sort by value desc
         std::stable_sort(vec.begin(), vec.end(), moost::which<2>::comparer<std::greater>());
      else // sort by idx
         std::stable_sort(vec.begin(), vec.end(), moost::which<1>::comparer<std::less>());

      return true;
   }

   void clear()
   {
      m_is.clear();
      m_is.seekg(0, std::ios::beg);
   }

private:
   std::ifstream m_ifs;
   std::istream& m_is;

};

// this expects
// id (idx, val) (idx, val) (idx, val)...
// we sometimes get this format back from dumbo
template<typename K, typename T>
class python_sparsevec_reader
{
public:
   typedef std::vector<std::pair<int, T> > sparsevec_t;

   python_sparsevec_reader(std::istream& is)
      : m_is(is), m_format_string("%*2c%d%*2c" + reader_value_traits<T>::format_string()) { }

   python_sparsevec_reader(const std::string fileName)
      : m_is(m_ifs),  m_format_string("%*2c%d%*2c" + reader_value_traits<T>::format_string())
   {
      m_ifs.open(fileName.c_str());
      if ( !m_ifs.is_open() )
         throw std::runtime_error("Cannot open file <" + fileName + ">!");
   }

   bool read(K& key, sparsevec_t& vec, bool sort_by_value)
   {
      if (m_is.eof())
         return false;
      std::string line;
      getline(m_is, line);
      if (line.empty())
         return false;

      vec.clear();

      std::istringstream iss(line);
      iss >> key;
      // now read the remaining values into a vector
      std::string s;
      int idx;
      T val;
      while (getline(iss, s, ')')){
         sscanf(s.c_str(), m_format_string.data(), &idx, &val);
         vec.push_back(std::make_pair(idx, val));
      }

      if (sort_by_value)   // sort by value desc
         std::stable_sort(vec.begin(), vec.end(), moost::which<2>::comparer<std::greater>());
      else // sort by idx
         std::stable_sort(vec.begin(), vec.end(), moost::which<1>::comparer<std::less>());

      return true;
   }

   void clear()
   {
      m_is.clear();
      m_is.seekg(0, std::ios::beg);
   }

private:
   std::ifstream m_ifs;
   std::istream& m_is;
   const std::string m_format_string;
};

// this expects CF format i.e.
// 10 200 3.0
// 10 300 2.0
// 20 100 1.0
// 20 300 4.0
// means
// 10 -> (200,3.0), (300,2.0)
// 20 -> (100,1.0), (300,4.0)
template<typename K, typename T>
class cf_sparsevec_reader
{
public:
   typedef std::vector<std::pair<int, T> > sparsevec_t;

   cf_sparsevec_reader(std::istream& is)
      : m_is(is), m_eof(false)
   {
      cache_first_line();
   }

   cf_sparsevec_reader(const std::string fileName)
      : m_is(m_ifs), m_eof(false)
   {
      m_ifs.open(fileName.c_str());
      if ( !m_ifs.is_open() )
         throw std::runtime_error("Cannot open file <" + fileName + ">!");

      cache_first_line();
   }
   void cache_first_line()
   {
      getline(m_is, m_line);
      std::istringstream iss(m_line);
      iss >> m_currentid;
   }

   bool parseline(sparsevec_t& vec)
   {
      if (m_line.empty())
      {
         m_eof = true;
         return false;
      }

      // returns false if id doesn't match
      std::istringstream iss(m_line);
      K id;
      int idx;
      T value;
      iss >> id >> idx >> value;
      if (id != m_currentid)
      {
         m_currentid = id;
         return false;
      }
      vec.push_back(std::make_pair(idx, value));
      return true;
   }

   bool read(K& key, sparsevec_t& vec, bool sort_by_value)
   {
      if (m_eof)
         return false;

      vec.clear();
      key = m_currentid;

      // build up the vector
      while (parseline(vec) && !m_is.eof())
         getline(m_is, m_line);

      if (sort_by_value)   // sort by value desc
         std::stable_sort(vec.begin(), vec.end(), moost::which<2>::comparer<std::greater>());
      else // sort by idx
         std::stable_sort(vec.begin(), vec.end(), moost::which<1>::comparer<std::less>());

      return true;
   }

   void clear()
   {
      m_is.clear();
      m_is.seekg(0, std::ios::beg);
   }

private:
   std::ifstream m_ifs;
   std::istream& m_is;
   bool m_eof;
   std::string m_line;
   K m_currentid;
};

// this expects
// id idx idx idx...
template<typename K, typename T>
class tsv_vec_reader
{
public:
   typedef std::vector<T> vec_t;

   tsv_vec_reader(std::istream& is)
      : m_is(is) { }

   tsv_vec_reader(const std::string fileName)
      : m_is(m_ifs)
   {
      m_ifs.open(fileName.c_str());
      if ( !m_ifs.is_open() )
         throw std::runtime_error("Cannot open file <" + fileName + ">!");
   }

   // sort_by_value is ignored
   bool read(K& key, vec_t& vec, bool /*sort_by_value*/)
   {
      if (m_is.eof())
         return false;
      std::string line;
      getline(m_is, line);
      if (line.empty())
         return false;

      vec.clear();

      std::istringstream iss(line);
      iss >> key;
      T val;
      while (iss >> val)
      {
         vec.push_back(val);
      }

      return true;
   }

   void clear()
   {
      m_is.clear();
      m_is.seekg(0, std::ios::beg);
   }

private:
   std::ifstream m_ifs;
   std::istream& m_is;
};

// this expects
// id [idx, idx, idx,... ]
// sometimes we can get dumbo output back in this format
template<typename K, typename T>
class python_vec_reader
{
public:
   typedef std::vector<T> vec_t;

   python_vec_reader(std::istream& is)
      : m_is(is) { }

   python_vec_reader(const std::string fileName)
      : m_is(m_ifs)
   {
      m_ifs.open(fileName.c_str());
      if ( !m_ifs.is_open() )
         throw std::runtime_error("Cannot open file <" + fileName + ">!");
   }

   // sort_by_value is ignored
   bool read(K& key, vec_t& vec, bool /*sort_by_value*/)
   {
      if (m_is.eof())
         return false;
      m_is >> key;
      std::string line;
      getline(m_is, line);
      if (line.length() < 3)
         return false;

      vec.clear();

      std::istringstream iss(line);
      // now read the python list into a vector
      std::string val;
      iss >> val;
      // trim leading [ and trailing , or ] from the first val
      vec.push_back(boost::lexical_cast<T>(val.substr(1, val.length() - 2)));
      // trim trailing , or ] from the rest
      while (iss >> val)
      {
         vec.push_back(boost::lexical_cast<T>(val.substr(0, val.length() - 1)));
      }

      return true;
   }

   void clear()
   {
      m_is.clear();
      m_is.seekg(0, std::ios::beg);
   }

private:
   std::ifstream m_ifs;
   std::istream& m_is;
};

}}}

#endif
