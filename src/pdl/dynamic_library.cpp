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

#include "../../include/moost/pdl/platform.h"
#include "../../include/moost/pdl/dynamic_library.h"
#include "../../include/moost/pdl/dynamic_class.h"
#include "../../include/moost/pdl/exception.h"

#include "dynamic_library_if.hpp"

#if PLATFORM_WIN32
# include "impl/dl_win32.ipp"
#elif PDL_PLATFORM_POSIX
# include "impl/dl_posix.ipp"
#endif

extern "C" void *(*moost_pdl_symbol_to_function_cast_(void *p))();

namespace moost { namespace pdl {

dynamic_library::dynamic_library()
{
}

dynamic_library::dynamic_library(const std::string& library_name, bool resolve_symbols)
{
   open(library_name, resolve_symbols);
}

void dynamic_library::open(const std::string& library_name, bool resolve_symbols)
{
   m_impl.reset(new dynamic_library_impl(library_name, resolve_symbols));
}

void dynamic_library::close()
{
   m_impl.reset();
}

dynamic_class *dynamic_library::create_instance(const std::string& class_name)
{
   typedef dynamic_class *(*factory_t)();

   if (!is_open())
   {
      throw exception("no library loaded");
   }

   const std::string factory_name(std::string("PDL_create_") + std::string(class_name));

   void *symbol = m_impl->get_symbol_by_name(factory_name);
   void *(*function)() = moost_pdl_symbol_to_function_cast_(symbol);
   factory_t factory = reinterpret_cast<factory_t>(function);

   if (!factory)
   {
      throw class_not_found_error("class " + class_name + " not found in " + m_impl->library_path());
   }

   dynamic_class *instance = factory();

   if (!instance)
   {
      throw exception("failed to create instance of class " + class_name);
   }

   instance->m_library = m_impl;

   return instance;
}

void dynamic_library::dynamic_class_deleter(dynamic_class *p)
{
   try
   {
      // Temporarily save the library pointer to avoid unloading the shared
      // object before destruction of the instances is finished.

      boost::shared_ptr<dynamic_library_if> dl = p->m_library;

      delete p;
   }
   catch (...)
   {
   }
}

}}
