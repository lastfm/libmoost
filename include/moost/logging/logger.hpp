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

#ifndef MOOST_LOGGER_HPP
#define MOOST_LOGGER_HPP

/*!
 *
 * /file Logging framework
 *
 * The logging is all handled by Apache log4cxx.
 *
 * Please refer to the online documentation for more information.
 *
 * http://logging.apache.org/log4cxx/index.html
 *
 */

#include <string>
#include <stdexcept>
#include <log4cxx/logger.h>

#include <boost/preprocessor/stringize.hpp>

#include "global.hpp"

/*!
 *
 * \brief Log message prefix
 *
 * This macro can be defined at a project level to provide a prefix for all
 * logged messages. It's entirely optional and can be igored.
 *
 */

#ifdef MLOG_PREFIX
#define MLOG_PREFIX__(msg) \
   BOOST_PP_STRINGIZE(MLOG_PREFIX) << " " << msg
#else
#define MLOG_PREFIX__(msg) msg
#endif

/*!
 *
 * \brief Loggin macros
 *
 * These macros directly invoke the underlying logging framework and exist to abstact MLOG from the underlying
 *
 */

#define MLOG_LEVEL_TRACE LOG4CXX_TRACE
#define MLOG_LEVEL_DEBUG LOG4CXX_DEBUG
#define MLOG_LEVEL_INFO  LOG4CXX_INFO
#define MLOG_LEVEL_WARN  LOG4CXX_WARN
#define MLOG_LEVEL_ERROR LOG4CXX_ERROR
#define MLOG_LEVEL_FATAL LOG4CXX_FATAL

/*!
 *
 * \brief Logging macro.
 *
 * All logging is performed using this macro. It provides an abstraction from
 * log4cxx and will include the log prefix if one is defined.
 *
 * \param [in] level: The logging level MLOG_LEVEL_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG(
 *    MLOG_LEVEL_DEBUG,
 *    MLOG_DEFAULT(),
 *    "some value: " << 0xF4 << ", " << "another value: " << 32.02f
 * );
 * \endcode
 *
 */

#define MLOG(level, logger, msg) \
   level(logger, MLOG_PREFIX__(msg));

/*!
 *
 * Alternatives to MLOG, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_TRACE(logger, msg) MLOG(MLOG_LEVEL_TRACE, logger, msg)
#define MLOG_DEBUG(logger, msg) MLOG(MLOG_LEVEL_DEBUG, logger, msg)
#define MLOG_INFO(logger, msg)  MLOG(MLOG_LEVEL_INFO, logger, msg)
#define MLOG_WARN(logger, msg)  MLOG(MLOG_LEVEL_WARN, logger, msg)
#define MLOG_ERROR(logger, msg) MLOG(MLOG_LEVEL_ERROR, logger, msg)
#define MLOG_FATAL(logger, msg) MLOG(MLOG_LEVEL_FATAL, logger, msg)

/*!
 *
 * \brief Gets a named logger
 *
 * Return a logger with a given name
 *
 * \param [in] name: The name to associate with the logger
 *
 * \code
 * MLOG(MLOG_LEVEL_DEBUG,  MLOG_NAMED_LOGGER("some logger"), "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_NAMED_LOGGER(name) \
   log4cxx::Logger::getLogger(name)


/*!
 *
 * \brief Gets an anonymous logger
 *
 * Return an anonymous logger
 *
 * \code
 * MLOG(MLOG_LEVEL_DEBUG,  MLOG_DEFAULT_LOGGER(), "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_DEFAULT_LOGGER() \
   log4cxx::Logger::getRootLogger()

/*!
 *
 * \brief Creates a named logger type
 *
 * The name is arbitrary but can be used in the config file to provide granular
 * control over the formatting and logging levels specific to this logger.
 *
 * \param [out] logger: The name of the logger type being created
 * \param [in] name: The name to associate with the logger
 *
 * \code
 * MLOG_NAMED_DEFINE(my_logger, "some logger");
 * MLOG(MLOG_LEVEL_DEBUG, my_logger, "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_NAMED_DEFINE(logger, name) \
   log4cxx::LoggerPtr logger = MLOG_NAMED_LOGGER(name)

/*!
 *
 * \brief Log to a named logger
 *
 * Allows logging to a named logger without the need to explicitly create it.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED(MLOG_LEVEL_WARN, "some logger", "some value: " << 394)
 * \endcode
 *
 */

#define MLOG_NAMED(level, name, msg) \
   MLOG(level, MLOG_NAMED_LOGGER(name), msg)

/*!
 *
 * Alternatives to MLOG_NAMED, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_NAMED_TRACE(name, msg) MLOG_NAMED(MLOG_LEVEL_TRACE, name, msg)
#define MLOG_NAMED_DEBUG(name, msg) MLOG_NAMED(MLOG_LEVEL_DEBUG, name, msg)
#define MLOG_NAMED_INFO(name, msg)  MLOG_NAMED(MLOG_LEVEL_INFO, name, msg)
#define MLOG_NAMED_WARN(name, msg)  MLOG_NAMED(MLOG_LEVEL_WARN, name, msg)
#define MLOG_NAMED_ERROR(name, msg) MLOG_NAMED(MLOG_LEVEL_ERROR, name, msg)
#define MLOG_NAMED_FATAL(name, msg) MLOG_NAMED(MLOG_LEVEL_FATAL, name, msg)

/*!
 *
 * \brief Creates a default logger type
 *
 * Note the you'll have no specific granular control over this logger.
 *
 * \param [out] logger: The name of the logger type being created
 *
 * \code
 * MLOG_DEFAULT_DEFINE(my_logger)
 * MLOG(MLOG_LEVEL_DEBUG, my_logger, "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_DEFAULT_DEFINE(logger) \
   logger = MLOG_DEFAULT_LOGGER()

/*!
 *
 * \brief Default logger
 *
 * Allows logging to an anonymous logger without the need to explicitly
 * create it.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT(MLOG_LEVEL_DEBUG, "some value: " << 394)
 * \endcode
 *
 */

#define MLOG_DEFAULT(level, msg) \
   MLOG(level, MLOG_DEFAULT_LOGGER(), msg)

/*!
 *
 * Alternatives to MLOG_DEFAULT, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_DEFAULT_TRACE(msg) MLOG_DEFAULT(MLOG_LEVEL_TRACE, msg)
#define MLOG_DEFAULT_DEBUG(msg) MLOG_DEFAULT(MLOG_LEVEL_DEBUG, msg)
#define MLOG_DEFAULT_INFO(msg)  MLOG_DEFAULT(MLOG_LEVEL_INFO, msg)
#define MLOG_DEFAULT_WARN(msg)  MLOG_DEFAULT(MLOG_LEVEL_WARN, msg)
#define MLOG_DEFAULT_ERROR(msg) MLOG_DEFAULT(MLOG_LEVEL_ERROR, msg)
#define MLOG_DEFAULT_FATAL(msg) MLOG_DEFAULT(MLOG_LEVEL_FATAL, msg)

/*!
 *
 * \brief Logging macro for asserts.
 *
 * All assert logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to assert an expression and if it's false it
 * will log an error  and then generate a runtime assert if NDEBUG is not defined.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_ASSERT(
 *    (x == y),
 *    MLOG_DEFAULT_LOGGER(),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_ASSERT(cond, logger, msg) \
   if(!(cond)) \
   { \
      LOG4CXX_ASSERT(logger, false, MLOG_PREFIX__(msg)); \
      assert(false && "MLOG_ASSERT assertion failure"); \
   }

/*!
 *
 * \brief Named logging macro for asserts.
 *
 * Allows you to assert an expression and if it's false it will log an error
 * to a named logger and then generate a runtime assert if NDEBUG is not defined.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED_ASSERT(
 *    (x == y),
 *    "some logger",
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_NAMED_ASSERT(cond, name, msg) \
   MLOG_ASSERT(cond, MLOG_NAMED_LOGGER(name), msg)

/*!
 *
 * \brief Default logging macro for asserts.
 *
 * Allows you to assert an expression and if it's false it will log an error to
 * an anonymous logger and then generate a runtime assert if NDEBUG is not defined.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT_ASSERT(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_DEFAULT_ASSERT(cond, msg) \
   MLOG_ASSERT(cond, MLOG_DEFAULT_LOGGER(), msg)

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
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_TEST(
 *    MLOG_LEVEL_DEBUG
 *    (x == y),
 *    MLOG_DEFAULT_LOGGER(),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_TEST(level, cond, logger, msg) \
   if(!(cond)) MLOG(level, logger, msg);

/*!
 *
 * \brief Named logging macro for logging false predicate results.
 *
 * Allows you to test an expression and if it's false it will log it
 * to a named logger at the desired level.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED_TEST(
 *    MLOG_LEVEL_DEBUG
 *    (x == y),
 *    "some logger",
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_NAMED_TEST(level, cond, name, msg) \
   MLOG(level, cond, MLOG_NAMED_LOGGER(name), msg)

/*!
 *
 * \brief Default logging macro for logging false predicate results.
 *
 * Allows you to test an expression and if it's false it will log it
 * to an anonymous logger at the desired level.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT_TEST(
 *    MLOG_LEVEL_DEBUG
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_DEFAULT_TEST(level, cond, msg) \
   MLOG(level, cond, MLOG_DEFAULT_LOGGER(), msg)

/*!
 *
 * Alternatives to MLOG_TEST, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_TEST_TRACE(cond, logger, msg) MLOG_TEST(MLOG_LEVEL_TRACE, cond, logger, msg)
#define MLOG_TEST_DEBUG(cond, logger, msg) MLOG_TEST(MLOG_LEVEL_DEBUG, cond, logger, msg)
#define MLOG_TEST_INFO(cond, logger, msg)  MLOG_TEST(MLOG_LEVEL_INFO, cond, logger, msg)
#define MLOG_TEST_WARN(cond, logger, msg)  MLOG_TEST(MLOG_LEVEL_WARN, cond, logger, msg)
#define MLOG_TEST_ERROR(cond, logger, msg) MLOG_TEST(MLOG_LEVEL_ERROR, cond, logger, msg)
#define MLOG_TEST_FATAL(cond, logger, msg) MLOG_TEST(MLOG_LEVEL_FATAL, cond, logger, msg)

/*!
 *
 * Alternatives to MLOG_NAMED_TEST, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_NAMED_TEST_TRACE(cond, name, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_TRACE, cond, name, msg)
#define MLOG_NAMED_TEST_DEBUG(cond, name, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_DEBUG, cond, name, msg)
#define MLOG_NAMED_TEST_INFO(cond, name, msg)  MLOG_DEFAULT_TEST(MLOG_LEVEL_INFO, cond, name, msg)
#define MLOG_NAMED_TEST_WARN(cond, name, msg)  MLOG_DEFAULT_TEST(MLOG_LEVEL_WARN, cond, name, msg)
#define MLOG_NAMED_TEST_ERROR(cond, name, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_ERROR, cond, name, msg)
#define MLOG_NAMED_TEST_FATAL(cond, name, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_FATAL, cond, name, msg)

/*!
 *
 * Alternatives to MLOG_DEFAULT_TEST, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_DEFAULT_TEST_TRACE(cond, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_TRACE, cond, msg)
#define MLOG_DEFAULT_TEST_DEBUG(cond, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_DEBUG, cond, msg)
#define MLOG_DEFAULT_TEST_INFO(cond, msg)  MLOG_DEFAULT_TEST(MLOG_LEVEL_INFO, cond, msg)
#define MLOG_DEFAULT_TEST_WARN(cond, msg)  MLOG_DEFAULT_TEST(MLOG_LEVEL_WARN, cond, msg)
#define MLOG_DEFAULT_TEST_ERROR(cond, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_ERROR, cond, msg)
#define MLOG_DEFAULT_TEST_FATAL(cond, msg) MLOG_DEFAULT_TEST(MLOG_LEVEL_FATAL, cond, msg)

/*!
 *
 * \brief Logging macro for checking predicate results.
 *
 * All check logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to check an expression and if it's false it
 * will log an error.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_CHECK(
 *    (x == y),
 *    MLOG_DEFAULT_LOGGER(),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_CHECK(cond, logger, msg) \
   MLOG_TEST_ERROR(cond, logger, msg)

/*!
 *
 * \brief Named logging macro for checking predicate predicate results.
 *
 * Allows you to check an expression and if it's false it will log it
 * to a named logger as an error.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED_CHECK(
 *    (x == y),
 *    "some logger",
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_NAMED_CHECK(cond, name, msg) \
   MLOG_CHECK(cond, MLOG_NAMED_LOGGER(name), msg)

/*!
 *
 * \brief Default logging macro for checking false predicate results.
 *
 * Allows you to check an expression and if it's false it will log it
 * to an anonymous logger as an error.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT_CHECK(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_DEFAULT_CHECK(cond, msg) \
   MLOG_CHECK(cond, MLOG_DEFAULT_LOGGER(), msg)

/*!
 *
 * \brief Logging macro for checking predicate results.
 *
 * All throw logging is performed using this macro. It provides an abstraction
 * from log4cxx and will allow you to check an expression and if it's false it
 * will log an error and then throw a runtime_error exception.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_THROW(
 *    (x == y),
 *    MLOG_DEFAULT_LOGGER(),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_THROW(cond, logger, msg)\
   if(!(cond)) \
   { \
      MLOG_CHECK(false, logger, msg); \
      std::stringstream oss; \
      oss << msg; \
      throw std::runtime_error(oss.str()); \
   }

/*!
 * \brief Macro to either throw or log an error
 *
 * \param [in] cond: If true, throw an exception, otherwise log an error
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_THROW_IF(
 *    do_throw,
 *    MLOG_DEFAULT_LOGGER(),
 *    "something's clearly wrong here"
 * );
 * \endcode
 */

#define MLOG_THROW_IF(cond, logger, msg) \
   do { \
      std::ostringstream oss; \
      oss << msg; \
      const std::string& err = oss.str(); \
      if (cond) \
         throw std::runtime_error(err); \
      else \
         MLOG_ERROR(logger, err); \
   } while (0)

/*!
 *
 * \brief Named logging macro for checking predicate predicate results.
 *
 * Allows you to check an expression and if it's false it will log it
 * to a named logger as an error and then throw a runtime_error exception.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED_THROW(
 *    (x == y),
 *    "some logger",
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_NAMED_THROW(cond, name, msg) \
   MLOG_THROW(cond, MLOG_NAMED_LOGGER(name), msg)

/*!
 * \brief Named macro to either throw or log an error
 *
 * \param [in] cond: If true, throw an exception, otherwise log an error
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED_THROW_IF(
 *    do_throw,
 *    "some logger",
 *    "something's clearly wrong here"
 * );
 * \endcode
 */

#define MLOG_NAMED_THROW_IF(cond, name, msg) \
   MLOG_THROW_IF(cond, MLOG_NAMED_LOGGER(name), msg)

/*!
 *
 * \brief Default logging macro for checking false predicate results.
 *
 * Allows you to check an expression and if it's false it will log it
 * to an anonymous logger as an error and then throw a runtime_error exception.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT_THROW(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_DEFAULT_THROW(cond, msg) \
   MLOG_THROW(cond, MLOG_DEFAULT_LOGGER(), msg)

/*!
 * \brief Default macro to either throw or log an error
 *
 * \param [in] cond: If true, throw an exception, otherwise log an error
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT_THROW_IF(
 *    do_throw,
 *    "something's clearly wrong here"
 * );
 * \endcode
 */

#define MLOG_DEFAULT_THROW_IF(cond, msg) \
   MLOG_THROW_IF(cond, MLOG_DEFAULT_LOGGER(), msg)

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
 * \param [in] logger: Logger object
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_VERIFY(
 *    (x == y),
 *    MLOG_DEFAULT_LOGGER(),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_VERIFY(cond, logger, msg) \
   if(!(cond)) \
   { \
      MLOG_ASSERT(false, logger, msg); \
      throw std::runtime_error(msg); \
   }

/*!
 *
 * \brief Named logging macro for checking predicate predicate results.
 *
 * Allows you to verify an expression and if it's false and if it's false it
 * will log an error to a named logger. If NDEBUG is define a runtime_error
 * exception is thrown otherwise a runtime assert is generate.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] name: The name to associate with the logger
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_NAMED_VERIFY(
 *    (x == y),
 *    "some logger",
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_NAMED_VERIFY(cond, name, msg) \
   MLOG_VERIFY(cond, MLOG_NAMED_LOGGER(name), msg)

/*!
 *
 * \brief Default logging macro for checking false predicate results.
 *
 * Allows you to verify an expression and if it's false and if it's false it
 * will log an error to an anonymous logger. If NDEBUG is define a runtime_error
 * exception is thrown otherwise a runtime assert is generate.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_DEFAULT_VERIFY(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_DEFAULT_VERIFY(cond, msg) \
   MLOG_VERIFY(cond, MLOG_DEFAULT_LOGGER(), msg)

/*!
 *
 * \brief Get logging level objects
 *
 * These macros can be used to obtain the appropriate logging framework object for a given level
 *
 */

#define MLOG_GET_LEVEL_ALL log4cxx::Level::getAll()
#define MLOG_GET_LEVEL_OFF log4cxx::Level::getOff()
#define MLOG_GET_LEVEL_TRACE log4cxx::Level::getTrace()
#define MLOG_GET_LEVEL_DEBUG log4cxx::Level::getDebug()
#define MLOG_GET_LEVEL_INFO log4cxx::Level::getInfo()
#define MLOG_GET_LEVEL_WARN log4cxx::Level::getWarn()
#define MLOG_GET_LEVEL_ERROR log4cxx::Level::getError()
#define MLOG_GET_LEVEL_FATAL log4cxx::Level::getFatal()

/*!
 *
 * \brief Programatically set the level of a logger
 *
 * This macro allows you to change the logging level of a specified logger. Use with
 * caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_SET_LEVEL(level, logger) \
   logger->setLevel(level);

/*!
 *
 * \brief Programatically set the level of a named logger
 *
 * This macro allows you to change the logging level of a named logger. Use with
 * caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_SET_NAMED_LEVEL(level, name) \
   MLOG_SET_LEVEL(level, MLOG_NAMED_LOGGER(name))

/*!
 *
 * \brief Programatically set the level of the default (root) logger
 *
 * This macro allows you to change the logging level of the default logger. Use with
 * caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_SET_DEFAULT_LEVEL(level) \
   MLOG_SET_LEVEL(level, MLOG_DEFAULT_LOGGER())

/*!
 *
 * Alternatives to MLOG_SET_LEVEL, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_SET_LEVEL_ALL(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_ALL, logger)
#define MLOG_SET_LEVEL_OFF(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_OFF, logger)
#define MLOG_SET_LEVEL_TRACE(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_TRACE, logger)
#define MLOG_SET_LEVEL_DEBUG(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_DEBUG, logger)
#define MLOG_SET_LEVEL_INFO(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_INFO, logger)
#define MLOG_SET_LEVEL_WARN(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_WARN, logger)
#define MLOG_SET_LEVEL_ERROR(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_ERROR, logger)
#define MLOG_SET_LEVEL_FATAL(logger) MLOG_SET_LEVEL(MLOG_GET_LEVEL_FATAL, logger)

/*!
 *
 * Alternatives to MLOG_SET_LEVEL, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_SET_NAMED_LEVEL_ALL(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_ALL, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_OFF(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_OFF, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_TRACE(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_TRACE, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_DEBUG(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_DEBUG, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_INFO(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_INFO, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_WARN(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_WARN, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_ERROR(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_ERROR, MLOG_NAMED_LOGGER(name))
#define MLOG_SET_NAMED_LEVEL_FATAL(name) MLOG_SET_LEVEL(MLOG_GET_LEVEL_FATAL, MLOG_NAMED_LOGGER(name))

/*!
 *
 * Alternatives to MLOG_SET_LEVEL, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_SET_DEFAULT_LEVEL_ALL() MLOG_SET_LEVEL(MLOG_GET_LEVEL_ALL, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_OFF() MLOG_SET_LEVEL(MLOG_GET_LEVEL_OFF, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_TRACE() MLOG_SET_LEVEL(MLOG_GET_LEVEL_TRACE, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_DEBUG() MLOG_SET_LEVEL(MLOG_GET_LEVEL_DEBUG, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_INFO() MLOG_SET_LEVEL(MLOG_GET_LEVEL_INFO, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_WARN() MLOG_SET_LEVEL(MLOG_GET_LEVEL_WARN, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_ERROR() MLOG_SET_LEVEL(MLOG_GET_LEVEL_ERROR, MLOG_DEFAULT_LOGGER())
#define MLOG_SET_DEFAULT_LEVEL_FATAL() MLOG_SET_LEVEL(MLOG_GET_LEVEL_FATAL, MLOG_DEFAULT_LOGGER())

/*!
 *
 * \brief Names of the standard logging levels
 *
 * These macros are the string representaion of the level objects
 *
 */

#define MLOG_GET_LEVELSTR_ALL "all"
#define MLOG_GET_LEVELSTR_OFF "off"
#define MLOG_GET_LEVELSTR_TRACE "trace"
#define MLOG_GET_LEVELSTR_DEBUG "debug"
#define MLOG_GET_LEVELSTR_INFO "info"
#define MLOG_GET_LEVELSTR_WARN "warn"
#define MLOG_GET_LEVELSTR_ERROR "error"
#define MLOG_GET_LEVELSTR_FATAL "fatal"

/*!
 *
 * \brief Convert stiing to level object
 *
 * Takes a string and converts it to a level object, throwing an exception if the convertion fails
 *
 */

namespace moost { namespace logging {

   inline
   log4cxx::LevelPtr str2level(std::string const level)
   {
      log4cxx::LevelPtr invalid(new log4cxx::Level(-1, LOG4CXX_STR("INVALID"), 7 ));
      log4cxx::LevelPtr new_level = log4cxx::Level::toLevel(level, invalid);

      if (new_level == invalid)
      {
         // [29/3/2011 ricky] we really don't want this to be ignored so after much soul
         // searching I decided this was the best way to handle an invalid level request
         throw std::runtime_error("invalid level string");
      }

      return new_level;
   }

}}

/*!
 *
 * \brief Programatically set the level of a logger
 *
 * This macro allows you to change the logging level of a specified logger using a string representation
 * of its name. Use with caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_SET_LEVELSTR(levelstr, logger) \
   MLOG_SET_LEVEL(moost::logging::str2level(levelstr), logger);

/*!
 *
 * \brief Programatically set the level of the default (root) logger
 *
 * This macro allows you to change the logging level of a named logger using a string representation
 * of its name. Use with caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_SET_NAMED_LEVELSTR(levelstr, name) \
   MLOG_SET_LEVELSTR(levelstr, MLOG_NAMED_LOGGER(name))

/*!
 *
 * This macro allows you to change the logging level of the default logger using a string representation
 * of its name. Use with caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_SET_DEFAULT_LEVELSTR(levelstr) \
   MLOG_SET_LEVELSTR(levelstr, MLOG_DEFAULT_LOGGER())

#endif
