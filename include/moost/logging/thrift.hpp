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
 * @file thrift.hpp
 * @brief Thrift bindings to the logging API
 * @author Marcus Holland-Moritz
 * @date 2012-12-01
 *
 * This header should ONLY be included if you're using thrift
 * in your project. Then a simple call to
 *
 *    moost::logging::enable_thrift_logger();
 *
 * is sufficient to enable thrift logging through the moost
 * logging framework.
 */

#ifndef MOOST_LOGGING_THRIFT_HPP__
#define MOOST_LOGGING_THRIFT_HPP__

#include <thrift/Thrift.h>

#include "detail/thrift.hpp"

namespace moost { namespace logging {

inline void enable_thrift_logger(void (*function)(const char *) = &detail::warn_logger)
{
   ::apache::thrift::GlobalOutput.setOutputFunction(function);
}

}}

#endif
