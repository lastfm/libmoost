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

#ifndef MOOST_PDL_DYNAMIC_LIBRARY_HPP__
#define MOOST_PDL_DYNAMIC_LIBRARY_HPP__

#include <string>

#include <boost/shared_ptr.hpp>

#include "exception.h"
#include "dynamic_class.h"

namespace moost { namespace pdl {

class dynamic_library_if;

/**
 * Portable shared object loader and class factory
 *
 * This class manages shared library loading and unloading as well
 * as instantiation of classes provided by the shared libraries.
 *
 * It is only useful if the classes in these shared libraries have
 * been properly exported and inherit from moost::pdl::dynamic_class.
 *
 * It is crucial to understand how the lifetime of a shared library
 * is managed. A shared library is not unloaded while it is being
 * referenced from either a dynamic_library instance or any instance
 * of any class exported by that shared library. It will be immediately
 * unloaded once the last reference is gone. As a consequence, you have
 * to ensure that no instances (other than global instances) implemented
 * by the shared library have a lifetime that exceeds the lifetime of
 * the classes instantiated through dynamic_library.
 *
 * Instantiating a class within a shared library can be extremely easy:

\code
boost::shared_ptr<interface> i;
i = moost::pdl::dynamic_library(library).create<interface>(class_name);
\endcode

 * This will load the library \c library and create an instance of class
 * \c class_name implementing \c interface. Once \c i goes out of scope,
 * the instance will automatically be destroyed and the shared library
 * will be unloaded.
 */
class dynamic_library
{
public:
   /**
    * Construct a dynamic_library object
    */
   dynamic_library();

   /**
    * Construct a dynamic_library object and open library
    *
    * This will immediately attempt to open the library. It will throw
    * and exception of the library cannot be opened.
    *
    * \param library_name      Name of the shared library, potentially
    *                          including a path.
    *
    * \param resolve_symbols   Whether or not symbols shall be resolved
    *                          immediately.
    */
   dynamic_library(const std::string& library_name, bool resolve_symbols = true);

   /**
    * Open a shared library
    *
    * This will attempt to open a shared library. It will throw and
    * exception of the library cannot be opened. If another library
    * was opened before, that library will implicitly be closed. Note
    * that this doesn't imply that the shared object will immediately
    * be unloaded (it will only be unloaded once the last reference
    * to it is gone). In other words, it is perfectly safe to open
    * a new shared library while there are still instances referencing
    * the old one.
    *
    * \param library_name      Name of the shared library, potentially
    *                          including a path.
    *
    * \param resolve_symbols   Whether or not symbols shall be resolved
    *                          immediately.
    */
   void open(const std::string& library_name, bool resolve_symbols = true);

   /**
    * Close a shared library
    *
    * This will merely remove the internal reference to the shared
    * library. It will not necessarily unload the shared object
    * immediately.
    */
   void close();

   /**
    * Check if a shared library has been opened
    *
    * This will check if the dynamic_library instance currenly references
    * a valid shared library that can be used to create instances.
    *
    * \returns  Boolean indicating if it's open.
    */
   bool is_open() const
   {
      return m_impl;
   }

   /**
    * Create a new instance of a class exported by the shared library
    *
    * This will throw an exception if a new instance cannot be created.
    *
    * \tparam T                Interface type implemented by the class.
    *
    * \param class_name        Name of the class that should be instantiated.
    *
    * \returns  A shared pointer to the newly created instance.
    */
   template <typename T>
   boost::shared_ptr<T> create(const std::string& class_name)
   {
      dynamic_class *instance = create_instance(class_name);

      T *dci = dynamic_cast<T *>(instance);

      if (!dci)
      {
         delete instance;
         throw exception("invalid type for class " + class_name);
      }

      return boost::shared_ptr<T>(dci, dynamic_class_deleter);
   }

   /**
    * Associate an instance of a class with the shared library
    *
    * \tparam T                Class type.
    *
    * \param instance          The instance to be associated.
    *
    * \returns  A shared pointer to the associated instance.
    */
   template <typename T>
   boost::shared_ptr<T> associate(T *instance)
   {
      instance->m_library = m_impl;
      return boost::shared_ptr<T>(instance, dynamic_class_deleter);
   }

private:
   static void dynamic_class_deleter(dynamic_class *p);

   dynamic_class *create_instance(const std::string& class_name);

   boost::shared_ptr<dynamic_library_if> m_impl;
};

}}

#endif
