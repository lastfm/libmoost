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
 * @file deprecated.hpp
 * @brief Abstraction of compiler support for deprecated attribute
 * @author Ricky Cormier
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-06-18
 */

#ifndef MOOST_COMPILER_ATTRIBUTES_DEPRECATED_HPP__
#define MOOST_COMPILER_ATTRIBUTES_DEPRECATED_HPP__

/*!
 * @brief Cross platform deprecation
 *
 * Provides a cross platform mechanism for deprecating functions and types. In
 * general the behaviour is the same on both Windows and Linux but note that on
 * Linux if a struct/class is deprecated but its members are not you will not
 * receive a warning when the members are used, only when the struct/class name
 * itself is referenced.
 *
 * @note No action is performed on unsupported compilers
 *
 * @code
 *
 * // deprecating a free function
 *
 * deprecated__ void foo()
 * {
 * }
 *
 * // deprecating variables
 *
 * deprecated__ int i;
 *
 * // deprecating a struct/class/union
 *
 * struct deprecated__ bar
 * {
 * };
 *
 * // deprecating a struct/class/union's members
 *
 * struct foobar
 * {
 *    deprecated__ void somefunc() {}
 *    deprecated__ static void somestaticfunc() {}
 *    deprecated__ static int si;
 *    deprecated__ int i;
 * };
 *
 * // deprecating a templated struct/class/union's members
 *
 * template <typename T>
 * struct templated
 * {
 *    deprecated__ void somefunc();
 * };
 *
 * template <typename T>
 * void templated<T>::somefunc(){}
 *
 * @endcode
 */

#if defined( __GNUC__ )

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Deprecated attribute support for gcc
#define deprecated__ __attribute__ ((deprecated))
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#elif defined ( _MSC_VER )

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Deprecated attribute support for Visual Studio
#define deprecated__ __declspec(deprecated("is deprecated"))
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#else

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Unknown compiler - do the best we can
pragma_warning__("item is deprecated")
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#endif

#endif
