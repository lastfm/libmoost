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

#ifndef MOOST_CONFIGURABLE_BINDER_H__
#define MOOST_CONFIGURABLE_BINDER_H__

#include <vector>
#include <string>
#include <istream>
#include <ostream>
#include <map>
#include <stdexcept>

#include <boost/noncopyable.hpp>

#include "configurable.h"
#include "binding.hpp"

namespace moost { namespace configurable {

/** @brief binder is a helper class that aids synchronized binding/setting/getting of nested configurations.
 *
 * Inherit from binder to obtain property getting/setting superpowers.  Define your bindings in your ctor using
 * the helper functions child and bind (see the tests for examples).
 */
// TODO: is there a clever way to do copy ctor?
class binder : public configurable, boost::noncopyable
{
private:

  typedef std::map<std::string, persistable * > route_map;

  route_map m_routes;

  std::vector< persistable * > m_bindings;

protected:

  /// add persistable as subcomponent with name key
  void child(const std::string & key, persistable & value);

  /// add reference to field with name key
  template<typename T>
  void bind(const std::string & key, T & value);

  /// add reference to field with name key with a default value
  template<typename T>
  void bind(const std::string & key, T & value, const T & default_value);

public:

  /// Constructs a binder.
  binder() {}

  /// Destroys binder.
  virtual ~binder();

  /// read properties from source, and set defaults where properties omitted
  void read(std::istream & source);

  /// write properties to dest
  void write(std::ostream & dest, int indent = 0) const;

  /// set a particular binding to value
  void set(const std::string & key, const std::string & value);

  /// get the value of a particular binding and store it in value
  void get(const std::string & key, std::string & value) const;

  /// list all bindings and their values
  void list(std::vector< std::pair< std::string, std::string > > & items);

  /// set default values to all bindings
  void set_default();
};

template<typename T>
void binder::bind(const std::string & key, T & value)
{
  m_bindings.push_back(new binding<T>(value));
  m_routes[key] = m_bindings.back();
}

template<typename T>
void binder::bind(const std::string & key, T & value, const T & default_value)
{
  m_bindings.push_back(new binding<T>(value, default_value));
  m_routes[key] = m_bindings.back();
}

}}

#endif // MOOST_CONFIGURABLE_BINDER_H__
