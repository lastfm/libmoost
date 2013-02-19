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

#ifndef MOOST_CONFIGURABLE_BINDING_HPP__
#define MOOST_CONFIGURABLE_BINDING_HPP__

#include <stdexcept>
#include <string>

#include "persistable.h"

namespace moost { namespace configurable {

/// @brief binding wraps the reference to a value with persistable qualities
template<typename T>
class binding : public persistable
{
private:
  T &         m_value;
  const bool  m_has_default_value;
  const T     m_default_value;

public:
  binding(T & value)
  : m_value(value),
    m_has_default_value(false),
    m_default_value(T()) {}

  binding(T & value, const T & default_value)
  : m_value(value),
    m_has_default_value(true),
    m_default_value(default_value) {}

  void read(std::istream & source)
  {
    source >> m_value;
    if (source.fail())
      throw std::runtime_error("cannot interpret value");
  }
  void write(std::ostream & dest, int /* indent */ = 0) const { dest << m_value; }
  void set_default()
  {
    if (!m_has_default_value)
      throw std::runtime_error("must set binding (no default value)");
    m_value = m_default_value;
  }
};

/// string-specific template specialization: we need to read the whole line for strings
template<>
class binding<std::string> : public persistable
{
private:
  std::string &     m_value;
  const bool        m_has_default_value;
  const std::string m_default_value;

public:
  binding(std::string & value)
  : m_value(value),
    m_has_default_value(false),
    m_default_value("") {}

  binding(std::string & value, const std::string & default_value)
  : m_value(value),
    m_has_default_value(true),
    m_default_value(default_value) {}

  // TODO: escape newlines
  void read(std::istream & source) { std::getline(source, m_value); }
  void write(std::ostream & dest, int /* indent */ = 0) const { dest << m_value; }
  void set_default()
  {
    if (!m_has_default_value)
      throw std::runtime_error("must set binding (no default value)");
    m_value = m_default_value;
  }
};

/// bool-specific template specialization: we want to interpret true and false
template<>
class binding<bool> : public persistable
{
private:
  bool &     m_value;
  const bool        m_has_default_value;
  const bool m_default_value;

public:
  binding(bool & value)
  : m_value(value),
    m_has_default_value(false),
    m_default_value("") {}

  binding(bool & value, const bool & default_value)
  : m_value(value),
    m_has_default_value(true),
    m_default_value(default_value) {}

  void read(std::istream & source)
  {
    std::string token;
    source >> token;
    if (token == "true" || token == "True" || token == "TRUE" || token == "1")
      m_value = true;
    else if (token == "false" || token == "False" || token == "FALSE" || token == "0")
      m_value = false;
    else
      throw std::runtime_error("cannot interpret value");
  }
  void write(std::ostream & dest, int /* indent */ = 0) const { dest << (m_value ? "true" : "false"); }
  void set_default()
  {
    if (!m_has_default_value)
      throw std::runtime_error("must set binding (no default value)");
    m_value = m_default_value;
  }
};

}} // moost::configurable

#endif // MOOST_CONFIGURABLE_BINDING_HPP__
