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

#ifndef MOOST_PDL_DYNAMIC_CLASS_H__
#define MOOST_PDL_DYNAMIC_CLASS_H__

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "platform.h"

namespace moost { namespace pdl {

class dynamic_library;
class dynamic_library_if;

/**
 * Common interface for all dynamically loaded classes
 *
 * All classes that are supposed to be exported from a shared library
 * need to derive from dynamic_class.
 */
class dynamic_class : public boost::noncopyable
{
   // We have to be friends with dynamic_library so that m_library can
   // be accessed.
   friend class dynamic_library;

public:
   virtual ~dynamic_class();

private:
   // This reference is used to manage the lifetime of a loaded shared
   // library. Once the last reference to a shared library is gone, the
   // shared library will be unloaded.
   boost::shared_ptr<dynamic_library_if> m_library;
};

}}

/**
 * Export constructor for dynamically loaded class
 *
 * Call this export macro once per plugin for every class that is supposed
 * to be exported.
 */
#define PDL_EXPORT_DYNAMIC_CLASS(class_name) \
   extern "C" PDL_DECL_EXPORT ::moost::pdl::dynamic_class *PDL_create_ ## class_name() \
   {                                \
      try                           \
      {                             \
         return new class_name();   \
      }                             \
      catch (...)                   \
      {                             \
      }                             \
      return 0;                     \
   }

#endif
