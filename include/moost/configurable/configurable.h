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

#ifndef MOOST_CONFIGURABLE_CONFIGURABLE_H__
#define MOOST_CONFIGURABLE_CONFIGURABLE_H__

#include <vector>
#include <string>

#include "persistable.h"

namespace moost { namespace configurable {

/** @brief Anything that inherits from this type guarantees it can have configurations stored/retrieved/listed
 */
class configurable : virtual public persistable
{
public:
  /// set one key/value pair
  virtual void set(const std::string & key, const std::string & value) = 0;
  /// get one key/value pair
  virtual void get(const std::string & key, std::string & value) const = 0;
  /// get all key/value pairs
  virtual void list(std::vector< std::pair< std::string, std::string > > & items) = 0;
  /// dtor
  virtual ~configurable() {}
  /// default indentation for pretty-printing configurations
  static const int DEFAULT_INDENT = 2;
};

}} // moost::configurable

#endif // MOOST_CONFIGURABLE_CONFIGURABLE_H__
