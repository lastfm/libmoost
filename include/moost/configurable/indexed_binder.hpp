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

#ifndef MOOST_CONFIGURABLE_INDEXED_BINDER_HPP__
#define MOOST_CONFIGURABLE_INDEXED_BINDER_HPP__

#include "configurable.h"

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <set>
#include <stdexcept>
#include <istream>
#include <ostream>

#include <boost/lexical_cast.hpp>

namespace moost { namespace configurable {

/** @brief indexed_binder allows a variable number of configurables accessed by index.
 *
 * Note that template parameter T must be of type configurable.
 */
template<typename T>
class indexed_binder : public configurable, boost::noncopyable
{
private:

  std::vector< boost::shared_ptr< configurable > > m_pconfigurables;

public:

  indexed_binder() {}

  virtual ~indexed_binder() {}

  const T& operator[](const size_t index) const
  {
    return *static_cast<const T *>(m_pconfigurables[index].get());
  }

  T& operator[](const size_t index)
  {
    return *static_cast<T *>(m_pconfigurables[index].get());
  }

  size_t size()
  {
    return m_pconfigurables.size();
  }

  void resize(size_t size)
  {
    size_t old_size = m_pconfigurables.size();
    m_pconfigurables.resize(size);
    for (; old_size < size; ++old_size)
      m_pconfigurables[old_size].reset(new T());
  }

  void read(std::istream & source);

  void write(std::ostream & dest, int indent = 0) const;

  void set(const std::string & key, const std::string & value);

  void get(const std::string & key, std::string & value) const;

  void list(std::vector< std::pair< std::string, std::string > > & items);

  // set_default for indexed_binder should just resize to zero
  void set_default();
};

// implementation:

template<typename T>
void indexed_binder<T>::read(std::istream & source)
{
  // first, read {
  std::string token;

  source >> token;

  if (token != "{")
    throw std::runtime_error("bad token: '" + token + "', expecting '}'");

  // any index that doesn't read, we must set the default
  std::set< size_t > found_indices;

  // first, set ourselves to zero
  resize(0);

  for (;;)
  {
    source >> token;

    if (token == "}")
      break;

    if (source.eof())
      throw std::runtime_error("unexpected eof, expecting '}'");

    try
    {
      size_t index = boost::lexical_cast<size_t>(token);

      if (index >= size())
        resize(index + 1);

      source.get(); // skip past the ' '
      try
      {
        m_pconfigurables[index]->read(source);
        found_indices.insert(index);
      }
      catch (const std::runtime_error & e)
      {
        throw std::runtime_error(boost::lexical_cast<std::string>(index) + ":" + e.what());
      }
    }
    catch (boost::bad_lexical_cast)
    {
      throw std::runtime_error("bad token: '" + token + "', expecting index");
    }
  }

  // set defaults for any remaining indices
  for (size_t i = 0; i != size(); ++i)
  {
    if (found_indices.find(i) == found_indices.end())
    {
      try
      {
        m_pconfigurables[i]->set_default();
      }
      catch (const std::runtime_error & e)
      {
        throw std::runtime_error(boost::lexical_cast<std::string>(i) + ":" + e.what());
      }
    }
  }
}

template<typename T>
void indexed_binder<T>::write(std::ostream & dest, int indent /* = 0 */) const
{
  // new context always means new indent
  indent += configurable::DEFAULT_INDENT;

  // first, write {
  dest << '{' << std::endl;
  for (int i = 0; i != indent; ++i)
    dest << ' ';

  for (size_t i = 0; i != m_pconfigurables.size(); ++i)
  {
    dest << i << ' ';
    m_pconfigurables[i]->write(dest, indent);
    dest << std::endl;
    for (int i = 0; i != indent; ++i)
      dest << ' ';
  }

  // backup!
  dest.seekp(-configurable::DEFAULT_INDENT, std::ios::cur);
  dest << '}';
}

template<typename T>
void indexed_binder<T>::set(const std::string & key, const std::string & value)
{
  if (key == "size")
  {
    size_t size = boost::lexical_cast<size_t>(value);
    resize(size);
  }
  else
  {
    size_t sep_pos = key.find('.');
    if (sep_pos == std::string::npos)
      throw std::runtime_error("bad key: '" + key + "', expecting separator '.'");
    size_t index = boost::lexical_cast<size_t>(key.substr(0, sep_pos));
    if (index >= m_pconfigurables.size())
       throw std::runtime_error("index out of range");
    m_pconfigurables[index]->set(key.substr(sep_pos + 1), value);
  }
}

template<typename T>
void indexed_binder<T>::get(const std::string & key, std::string & value) const
{
  if (key == "size")
  {
    value = boost::lexical_cast<std::string>(m_pconfigurables.size());
  }
  else
  {
    size_t sep_pos = key.find('.');
    if (sep_pos == std::string::npos)
      throw std::runtime_error("bad key: '" + key + "', expecting separator '.'");
    size_t index = boost::lexical_cast<size_t>(key.substr(0, sep_pos));
    if (index >= m_pconfigurables.size())
      throw std::runtime_error("index out of range");
    m_pconfigurables[index]->get(key.substr(sep_pos + 1), value);
  }
}

template<typename T>
void indexed_binder<T>::list(std::vector< std::pair< std::string, std::string > > & items)
{
  for (size_t i = 0; i != m_pconfigurables.size(); ++i)
  {
    std::vector< std::pair< std::string, std::string > > sub_items;
    m_pconfigurables[i]->list(sub_items);
    for (std::vector< std::pair< std::string, std::string > >::const_iterator it_s = sub_items.begin(); it_s != sub_items.end(); ++it_s)
      items.push_back(std::make_pair(boost::lexical_cast<std::string>(i) + '.' + it_s->first, it_s->second));
  }
  items.push_back(std::make_pair("size", boost::lexical_cast<std::string>(size())));
}

// set_default for indexed_binder should just resize to zero
template<typename T>
void indexed_binder<T>::set_default()
{
  resize(0);
}

}}

#endif // MOOST_CONFIGURABLE_INDEXED_BINDER_HPP__
