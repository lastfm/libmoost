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

#ifndef MOOST_FUNCTION_LOGGER_HPP
#define MOOST_FUNCTION_LOGGER_HPP

#include <string>
#include <boost/current_function.hpp>
#include "logger.hpp"
#include "../utils/demangler.hpp"

/*!
 *
 * \brief Get the full function name of the current function
 *
 * Gets the full function name of the current function, which will include it's return type,
 * fully qualified name and argument types.
 *
 */

#define MLOG_FUNC_FULL_NAME() \
   BOOST_CURRENT_FUNCTION

/*!
 *
 * \brief Get the short function name of the current function
 *
 * Gets the function name of the current function with all the other parafinalia removed.
 *
 */

#define MLOG_FUNC_SHORT_NAME() \
   moost::utils::short_function_name(MLOG_FUNC_FULL_NAME())

/*!
 *
 * \brief Get the short function name of the current function
 *
 * Gets the function name of the current function.
 *
 */

#if defined(NDEBUG) && !defined(MLOG_SHORT_FUNC_NAME_LOGGING)
#define MLOG_FUNC_NAME() \
   MLOG_FUNC_FULL_NAME()
#else
#define MLOG_FUNC_NAME() \
   MLOG_FUNC_SHORT_NAME()
#endif


/*!
 *
 * \brief Get a function level logging
 *
 * Gets a logger named after the current function.
 *
 * You do not have to use this to log in a function but in doing so you will
 * have control of logging at function level granularity.
 *
 * This can be used in both free-standing functions and class member functions.
 *
 * /note In production builds the full function name will be used unless MLOG_SHORT_FUNC_NAME_LOGGING is defined.
 *       This is to avoid the runtime overhead of converting the long name to a short name unless you specificall
 *       want the shorterned version and your code isn't performance critical.
 *
 * \param [out] logger: The name of the logger type being created
 *
 * \code
 * MLOG_FUNC_DEFINE(my_logger)
 * MLOG(MLOG_LEVEL_TRACE, MLOG_FUNC_LOGGER(), "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_FUNC_LOGGER() \
   log4cxx::Logger::getLogger(MLOG_FUNC_NAME())

/*!
 *
 * \brief Create a function level logging
 *
 * Creates a logger named after the current function.
 *
 * You do not have to use this to log in a function but in doing so you will
 * have control of logging at function level granularity.
 *
 * This can be used in both free-standing functions and class member functions.
 *
 * \param [out] logger: The name of the logger type being created
 *
 * \code
 * MLOG_FUNC_DEFINE(my_logger)
 * MLOG(MLOG_LEVEL_TRACE, my_logger, "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_FUNC_DEFINE(logger) \
   log4cxx::LoggerPtr logger = MLOG_FUNC_LOGGER()

/*!
 *
 * \brief Log to a function logger
 *
 * Allows logging to a function logger without the need to explicitly create it.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * void foo()
 * {
 *    MLOG_FUNC(MLOG_LEVEL_DEBUG, "some value: " << 394)
 * }
 * \endcode
 *
 */

#define MLOG_FUNC(level, msg) \
   MLOG(level, MLOG_FUNC_LOGGER(), msg) \

/*!
 *
 * Alternatives to MLOG_FUNC, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_FUNC_TRACE(msg) MLOG_FUNC(MLOG_LEVEL_TRACE, msg)
#define MLOG_FUNC_DEBUG(msg) MLOG_FUNC(MLOG_LEVEL_DEBUG, msg)
#define MLOG_FUNC_INFO(msg)  MLOG_FUNC(MLOG_LEVEL_INFO, msg)
#define MLOG_FUNC_WARN(msg)  MLOG_FUNC(MLOG_LEVEL_WARN, msg)
#define MLOG_FUNC_ERROR(msg) MLOG_FUNC(MLOG_LEVEL_ERROR, msg)
#define MLOG_FUNC_FATAL(msg) MLOG_FUNC(MLOG_LEVEL_FATAL, msg)

/*!
 *
 * \brief Assert to a function logger
 *
 * Allows you to assert an expression and if it's false it will log an error.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * void foo()
 * {
 *    MLOG_FUNC_ASSERT(x == y, "x and y are not equal");
 * }
 * \endcode
 *
 */

#define MLOG_FUNC_ASSERT(cond, msg) \
   MLOG_ASSERT(cond, MLOG_FUNC_LOGGER(), msg) \

/*!
 *
 * \brief Logging macro for logging false predicate results.
 *
 * All test logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to test an expression and if it's false it
 * will log it at the desired level.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_FUNC_TEST(
 *    MLOG_LEVEL_DEBUG
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_FUNC_TEST(level, cond, msg) \
   MLOG_TEST(level, cond, MLOG_FUNC_LOGGER(), msg);

/*!
 *
 * Alternatives to MLOG_FUNC_TEST, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_FUNC_TEST_TRACE(cond, msg) MLOG_FUNC_TEST(MLOG_LEVEL_TRACE, cond, msg)
#define MLOG_FUNC_TEST_DEBUG(cond, msg) MLOG_FUNC_TEST(MLOG_LEVEL_DEBUG, cond, msg)
#define MLOG_FUNC_TEST_INFO(cond, msg)  MLOG_FUNC_TEST(MLOG_LEVEL_INFO, cond, msg)
#define MLOG_FUNC_TEST_WARN(cond, msg)  MLOG_FUNC_TEST(MLOG_LEVEL_WARN, cond, msg)
#define MLOG_FUNC_TEST_ERROR(cond, msg) MLOG_FUNC_TEST(MLOG_LEVEL_ERROR, cond, msg)
#define MLOG_FUNC_TEST_FATAL(cond, msg) MLOG_FUNC_TEST(MLOG_LEVEL_FATAL, cond, msg)

/*!
 *
 * \brief Logging macro for checking predicate results.
 *
 * All check logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to check an expression and if it's false it
 * will log an error.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_FUNC_CHECK(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_FUNC_CHECK(cond,  msg) \
   MLOG_FUNC_TEST_ERROR(cond, msg)

/*!
 *
 * \brief Logging macro for checking predicate results.
 *
 * All throw logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to check an expression and if it's false it
 * will log an error and then throw a runtime_error exception.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_FUNC_THROW(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_FUNC_THROW(cond,  msg)\
   MLOG_THROW(cond, MLOG_FUNC_LOGGER(), msg)

/*!
 * \brief Function macro to either throw or log an error
 *
 * \param [in] cond: If true, throw an exception, otherwise log an error
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_FUNC_THROW_IF(
 *    do_throw,
 *    "something's clearly wrong here"
 * );
 * \endcode
 */

#define MLOG_FUNC_THROW_IF(cond,  msg)\
   MLOG_THROW_IF(cond, MLOG_FUNC_LOGGER(), msg)

/*!
 *
 * \brief Logging macro for checking predicate results.
 *
 * All verify logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to check an expression and if it's false it
 * will log an error. If NDEBUG is define a runtime_error exception is thrown
 * otherwise a runtime assert is generate.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_CLASS_VERIFY(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_FUNC_VERIFY(cond,  msg)\
   MLOG_VERIFY(cond, MLOG_FUNC_LOGGER(), msg)


/*!
 *
 * \brief Programatically set the level
 *
 * This macro allows you to change the logging level for the function. Use with
 * caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_FUNC_SET_LEVEL(level) \
   MLOG_SET_LEVEL(level, MLOG_FUNC_LOGGER());

/*!
 *
 * Alternatives to MLOG_FUNC_SET_LEVEL, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_FUNC_SET_LEVEL_ALL() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_ALL)
#define MLOG_FUNC_SET_LEVEL_OFF() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_OFF)
#define MLOG_FUNC_SET_LEVEL_TRACE() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_TRACE)
#define MLOG_FUNC_SET_LEVEL_DEBUG() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_DEBUG)
#define MLOG_FUNC_SET_LEVEL_INFO() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_INFO)
#define MLOG_FUNC_SET_LEVEL_WARN() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_WARN)
#define MLOG_FUNC_SET_LEVEL_ERROR() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_ERROR)
#define MLOG_FUNC_SET_LEVEL_FATAL() MLOG_FUNC_SET_LEVEL(MLOG_GET_LEVEL_FATAL)

/*!
 *
 * \brief Programatically set the level
 *
 * This macro allows you to change the logging level of the function using a string representation
 * of its name. Use with caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_FUNC_SET_LEVELSTR(levelstr) \
   MLOG_SET_LEVEL(str2level(levelstr), MLOG_FUNC_LOGGER());

#endif
