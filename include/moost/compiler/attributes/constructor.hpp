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

/**
 * @file constructor.hpp
 * @brief Abstraction of compiler support for constructor attribute
 * @author Ricky Cormier
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-06-18
 */

#ifndef MOOST_COMPILER_ATTRIBUTES_CONSTRUCTOR_HPP__
#define MOOST_COMPILER_ATTRIBUTES_CONSTRUCTOR_HPP__

#include "../pragmas.hpp"

/*!
 * @brief Cross platform constructor attribute
 *
 * Defines a code-block to be called *before* main is entered. Useful for
 * ensuring singletons are fully constructed prior to main being entered.
 *
 * @note the constructor must be defined at (any) namespace level
 *
 * @code
 *
 * // ensure singleton exists before main is executed
 * constructor__(init_singleton)
 * {
 *    some::mayers::singleton::instance();
 * }
 *
 * @endcode
 */

#if defined( __GNUC__ )

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constructor attribute support for gcc
#define constructor__(name) \
   struct name ## __ \
   { \
      static inline void name(void); \
      static void __attribute__ ((constructor)) init(void) \
      { \
         static int once = 1; \
         if(once) { name (); --once; } \
      } \
      private: name ## __(); \
   }; \
   void name ## __::name(void)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#elif defined ( _MSC_VER )

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Constructor attribute support for Visual Studio
#pragma section(".CRT$XCU", read)
#define constructor__(name) \
   struct name ## __ \
   { \
      static inline void name(void); \
      static void init(void) \
      { \
         static int once = 1; \
         if(once) { name (); --once; } \
      } \
      private: name ## __(); \
   }; \
   __declspec(allocate(".CRT$XCU")) \
   void (__cdecl*name##_)(void) = &name ## __::init; \
   void name ## __::name(void)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#else
#error "Constructor attribute is not supported"
#endif

#endif // MOOST_UTILS_COMPILER_HPP__
