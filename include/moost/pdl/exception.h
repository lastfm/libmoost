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

#ifndef MOOST_PDL_EXCEPTION_H__
#define MOOST_PDL_EXCEPTION_H__

#include <stdexcept>

namespace moost { namespace pdl {

/**
 * PDL exception base class
 */
class exception: public std::runtime_error
{
public:
   exception(const std::string& err)
      : std::runtime_error(err)
   {
   }
};

/**
 * Library could not be loaded
 */
class library_load_error: public exception
{
public:
   library_load_error(const std::string& err)
      : exception(err)
   {
   }
};

/**
 * Library file was not found
 */
class library_not_found_error: public exception
{
public:
   library_not_found_error(const std::string& err)
      : exception(err)
   {
   }
};

/**
 * Class was not found in library
 */
class class_not_found_error: public exception
{
public:
   class_not_found_error(const std::string& err)
      : exception(err)
   {
   }
};

}}

#endif
