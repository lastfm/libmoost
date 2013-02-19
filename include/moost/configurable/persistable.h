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

#ifndef MOOST_CONFIGURABLE_PERSISTABLE_H__
#define MOOST_CONFIGURABLE_PERSISTABLE_H__

namespace moost { namespace configurable {

/** @brief Anything that inherits from this type guarantees that it can be persisted to/loaded from a stream
 */
class persistable
{
public:
  /// load state from source
  virtual void read(std::istream & source) = 0;
  /// persist state to dest
  virtual void write(std::ostream & dest, int indent = 0) const = 0;
  /// if this object can be used without deserializing (otherwise throw)
  virtual void set_default() = 0;
  /// dtor
  virtual ~persistable() {}
};

}} // moost::configurable

#endif // MOOST_CONFIGURABLE_PERSISTABLE_H__
