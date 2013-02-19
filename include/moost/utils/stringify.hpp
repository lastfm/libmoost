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

#ifndef MOOST_UTILS_STRINGIFY_HPP__
#define MOOST_UTILS_STRINGIFY_HPP__

#include <map>
#include <set>
#include <vector>
#include <ostream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "foreach.hpp"

// TODO: overload for google:: thingies, std::list

namespace moost { namespace utils {

template <class T>
std::string stringify(const T& val, size_t truncate = 0);

template <class T>
void streamstringify(std::ostream& os, const T& val, size_t = 0);

template <class T1, class T2>
void streamstringify(std::ostream& os, const std::pair<T1, T2>& val, size_t truncate = 0);

template <class T>
void streamstringify(std::ostream& os, const std::set<T>& val, size_t truncate = 0);

template <class T>
void streamstringify(std::ostream& os, const std::vector<T>& val, size_t truncate = 0);

template <class T1, class T2>
void streamstringify(std::ostream& os, const std::map<T1, T2>& val, size_t truncate = 0);

/**
 * Turn complex data structures into plain strings.
 *
 * Usage is pretty simple: just call stringify with an arbitrarily nested data
 * structure argument and it will turn its contents into a string representation.
 * It's a bit like Perl's Data::Dumper for C++.
 *
 * You can add support for your own types by overloading the template.
 */
template <class T>
std::string stringify(const T& val, size_t truncate)
{
   std::ostringstream oss;
   streamstringify(oss, val, truncate);
   return oss.str();
}

template <class T>
void streamstringify(std::ostream& os, const T& val, size_t)
{
   os << val;
}

template <class T1, class T2>
void streamstringify(std::ostream& os, const std::pair<T1, T2>& val, size_t truncate)
{
   os << "(";
   streamstringify(os, val.first, truncate);
   os << ", ";
   streamstringify(os, val.second, truncate);
   os << ")";
}

template <class T>
void streamstringify(std::ostream& os, const std::set<T>& val, size_t truncate)
{
   size_t count = 0;
   os << "(";
   foreach (const T& v, val)
   {
      if (count)
         os << ", ";
      streamstringify(os, v, truncate);
      if (++count == truncate && count < val.size())
      {
         os << ", <+" << val.size() - count << ">";
         break;
      }
   }
   os << ")";
}

template <class T>
void streamstringify(std::ostream& os, const std::vector<T>& val, size_t truncate)
{
   size_t count = 0;
   os << "[";
   foreach (const T& v, val)
   {
      if (count)
         os << ", ";
      streamstringify(os, v, truncate);
      if (++count == truncate && count < val.size())
      {
         os << ", <+" << val.size() - count << ">";
         break;
      }
   }
   os << "]";
}

template <class T1, class T2>
void streamstringify(std::ostream& os, const std::map<T1, T2>& val, size_t truncate)
{
   typedef typename std::map<T1, T2>::value_type type_pair;
   size_t count = 0;
   os << "{";
   foreach (const type_pair& p, val)
   {
      if (count)
         os << ", ";
      streamstringify(os, p, truncate);
      if (++count == truncate && count < val.size())
      {
         os << ", <+" << val.size() - count << ">";
         break;
      }
   }
   os << "}";
}

}}

#endif
