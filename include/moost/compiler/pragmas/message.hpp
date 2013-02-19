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
 * @file message.hpp
 * @brief Abstraction of compiler pragma message directive
 * @author Ricky Cormier
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-06-18
 */

#ifndef MOOST_COMPILER_PRAGMAS_MESSAGE_HPP__
#define MOOST_COMPILER_PRAGMAS_MESSAGE_HPP__

#include <boost/preprocessor/stringize.hpp>

/**
 * @brief Compile time message pragma
 *
 * @param msg The message to be displayed
 *
 * @code
 *
 * pragma_message__("some message")
 *
 * @endcode
 */

#if defined( __GNUC__ ) || defined ( _MSC_VER )
#define pragma_message__(msg) \
      pragma__(message(__FILE__ "(" BOOST_PP_STRINGIZE(__LINE__) ") : " msg))
#else
#error "pragma message is unsupported"
#endif

/**
 * @brief Compile time message pragma with message type
 *
 * @param type The message type
 * @param msg The message to be displayed
 *
 * @code
 *
 * pragma_message_type__("warning", "some message")
 *
 * @endcode
 */

#define pragma_msgtype__(type, msg) pragma_message__(type ": " msg)

/**
 * @brief Compile time warning message pragma
 *
 * @param msg The message to be displayed
 *
 * @code
 *
 * pragma_warning_msg__("some message")
 *
 * @endcode
 */

#define pragma_warn__(msg) pragma_msgtype__("warning", msg)

#endif
