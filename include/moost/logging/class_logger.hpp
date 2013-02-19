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

#ifndef MOOST_CLASS_LOGGER_HPP
#define MOOST_CLASS_LOGGER_HPP

#include <log4cxx/logger.h>
#include "logger.hpp"
#include "../utils/demangler.hpp"

#include <typeinfo>

/*!
 * \brief Get a mangled class name
 *
 * If possible the mangled name is returned for *this
 */

#define MLOG_CLASS_MANGLED_NAME(thisptr) \
   typeid(*thisptr).name()

/*!
 * \brief Get a demangled class name
 *
 * If possible the demangled name is returned for *this
 */

#define MLOG_CLASS_DEMANGLED_NAME(thisptr) \
   DEMANGLE_NAME(MLOG_CLASS_MANGLED_NAME(thisptr))

/*!
 * \brief Get a class name
 *
 * If possible the demangled name is returned, else it'll be the mangled version.
 */

#define MLOG_CLASS_NAME() \
   MLOG_CLASS_DEMANGLED_NAME(this)

/*!
 * \brief The class member logger
 *
 * When using the MLOG_DECLARE_CLASS_LOGGER() and MLOG_INIT_CLASS_LOGGER() macros to declare and initialise a class
 * member logger, this macro will give you access to that logger. Note that the name is defined to be as unique
 * as possible to avoid any name collisions with other members. If, for some odd reason, this is a problem you will
 * need to use the NAMED version of the declare and init macros to create a logger with your own name.
 */

#define MLOG_CLASS_MEMBER_LOGGER_NAME() \
   CLASS_LOGGER_60EB35B012964beaA884558022BC99E2___

#define MLOG_CLASS_MEMBER_LOGGER() \
   MLOG_CLASS_MEMBER_LOGGER_NAME()()


/*!
 * \brief Declare a named class member logger
 *
 * Use this in the body of your class to declare a class member logger and then use MLOG_INIT_NAMED_CLASS_LOGGER(logger)
 * to initialise it in your constructor. The macro actually creates a virtual function based Meyer's singleton, which
 * will create the logger on the initial call (this isn't thread safe but by calling it in the classes constructor we
 * can avoid that issue) and subsequent calls just return a member to the function static logger. Because the function
 * is virtual, you can override it in sub-classes to ensure that each class in the hierachy has it's own logger. Those
 * of you who are observant will spot a potential issue here -- it is generally considered unsafe to call a virtual
 * function from a constructor because at that time the v-table will not exist so the call will not be directed to the
 * more derived class. In this special case that is actually just fine because we don't want it to do that we do want it
 * to create the logger object for this instance and not any derived instances. Put simply, every class that implements
 * a class member logger needs to use the MLOG_INIT_NAMED_CLASS_LOGGER(logger) macro to ensure it's logger is created!
 */

#define MLOG_DECLARE_NAMED_CLASS_MEMBER_LOGGER(name) \
   virtual log4cxx::LoggerPtr const & name () const \
   { \
      static log4cxx::LoggerPtr ptr = log4cxx::Logger::getLogger(MLOG_CLASS_DEMANGLED_NAME(this)); \
      return ptr; \
   } \

/*!
 * \brief Declare a default class member logger
 *
 * Use this in the body of your class to declare a class member logger and then use MLOG_INIT_CLASS_LOGGER(logger) to
 * initialise it in your constructor. The logger itself with be declared with a default name, which should be unique
 * enough for 99% of circumstances; however, you can use MLOG_DECLARE_NAMED_CLASS_LOGGER(logger) if you happen across
 * that 1% where it's not. The default logger's name is a bit of a monster so to make accessing it easier you can make
 * use of the MLOG_CLASS_MEMBER_LOGGER() convenience macro.
 *
 */

#define MLOG_DECLARE_CLASS_MEMBER_LOGGER() \
   MLOG_DECLARE_NAMED_CLASS_MEMBER_LOGGER(MLOG_CLASS_MEMBER_LOGGER_NAME());

/*!
 * \brief Initialises a named class member logger
 *
 * Since the name of the class logger is derived using RTTI it can be very expensive to use the standard class logging
 * macros as they have to derive the class name each time they are called. Alternatively, you can create a class member
 * logger type, which only needs initialising once. First of all declare, in the class body, the logger as a member of
 * the class you wish to log for using the MLOG_DECLARE_LOGGER(logger) macro. Then, in the constructor of the class just
 * invoke the MLOG_INIT_CLASSMLOG_INIT_NAMED_CLASS_MEMBER_LOGGER_LOGGER(my_logger_) macro with the same name you used
 * when declaring it. Once your logger has been initialised you can use the standard (rather than the class specific)
 * MLOG functions, passing it your logger
 *
 * Defining your logger as a class member rather than using the class logging convenience macros below is a matter of
 * choice but if your code is performance critical this will allow you to avoid any unnecessary logging overhead.
 *
 * /note This MUST be called in the constructor body and not the initialisation list because it needs access to "this".

 * \code
 * class foo
 * {
 * public:
 *    foo()
 *    {
 *       MLOG_INIT_NAMED_CLASS_MEMBER_LOGGER(my_logger_);
 *    }
 *
 *    void some_func() const
 *    {
 *       MLOG_DECLARE_NAMED_CLASS_MEMBER_LOGGER(my_logger_, "logging some debug info);
 *    }
 *
 * private:
 *    MLOG_DECLARE_LOGGER(my_logger_)
 * };
 * \endcode
 */

#define MLOG_INIT_NAMED_CLASS_MEMBER_LOGGER(name) \
   name ()\

/*!
 * \brief Declare a default class member logger
 *
 * The same as MLOG_INIT_NAMED_CLASS_MEMBER_LOGGER(logger) but you don't need to specify a name as it will use the
 * default. See the documemtation for MLOG_DECLARE_NAMED_CLASS_MEMBER_LOGGER() for more information on the default
 * class member logger.
 *
 */

#define MLOG_INIT_CLASS_MEMBER_LOGGER() \
   MLOG_INIT_NAMED_CLASS_MEMBER_LOGGER(MLOG_CLASS_MEMBER_LOGGER_NAME())

/*!
 *
 * Alternatives to using MLOG with the named class member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_TRACE(msg) MLOG_TRACE(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_DEBUG(msg) MLOG_DEBUG(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_INFO(msg)  MLOG_INFO(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_WARN(msg)  MLOG_WARN(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_ERROR(msg) MLOG_ERROR(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_FATAL(msg) MLOG_FATAL(MLOG_CLASS_MEMBER_LOGGER(), msg)

/*!
 *
 * Alternatives to using MLOG with the default class member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_TRACE(msg) MLOG_TRACE(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_DEBUG(msg) MLOG_DEBUG(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_INFO(msg)  MLOG_INFO(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_WARN(msg)  MLOG_WARN(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_ERROR(msg) MLOG_ERROR(MLOG_CLASS_MEMBER_LOGGER(), msg)
#define MLOG_CLASS_MEMBER_LOGGER_FATAL(msg) MLOG_FATAL(MLOG_CLASS_MEMBER_LOGGER(), msg)

/*!
 *
 * \brief Get a class level logging
 *
 * Gets a logger named after the current class.
 *
 * You do not have to use this to log in a class but in doing so you will
 * have control of logging at class level granularity.
 *
 * \code
 * MLOG(MLOG_LEVEL_INFO, MLOG_CLASS_LOGGER(), "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_CLASS_LOGGER() \
   log4cxx::Logger::getLogger( MLOG_CLASS_NAME() )

/*!
 *
 * \brief Define a class level logging
 *
 * Creates a logger named after the current class.
 *
 * You do not have to use this to log in a class but in doing so you will
 * have control of logging at class level granularity.
 *
 * \param [out] logger: The name of the logger type being created
 *
 * \code
 * MLOG_CLASS_DEFINE(my_logger)
 * MLOG(MLOG_LEVEL_INFO, my_logger, "log this: " << 944534634578UL);
 * \endcode
 *
 */

#define MLOG_CLASS_DEFINE(logger) \
   log4cxx::LoggerPtr logger = MLOG_CLASS_LOGGER()

/*!
 *
 * \brief Log to a class logger
 *
 * Allows logging to a class logger without the need to explicitly
 * create it.
 *
 * \param [in] level: The logging level MLOG_[TRACE, DEBUG, INFO, WARN, ERROR and FATAL]
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * void foo()
 * {
 *    MLOG_CLASS(MLOG_LEVEL_DEBUG, "some value: " << 394)
 * }
 * \endcode
 *
 */

#define MLOG_CLASS(level, msg) \
   MLOG(level, MLOG_CLASS_LOGGER(), msg)

/*!
 *
 * Alternatives to MLOG_CLASS, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_CLASS_TRACE(msg) MLOG_CLASS(MLOG_LEVEL_TRACE, msg)
#define MLOG_CLASS_DEBUG(msg) MLOG_CLASS(MLOG_LEVEL_DEBUG, msg)
#define MLOG_CLASS_INFO(msg)  MLOG_CLASS(MLOG_LEVEL_INFO, msg)
#define MLOG_CLASS_WARN(msg)  MLOG_CLASS(MLOG_LEVEL_WARN, msg)
#define MLOG_CLASS_ERROR(msg) MLOG_CLASS(MLOG_LEVEL_ERROR, msg)
#define MLOG_CLASS_FATAL(msg) MLOG_CLASS(MLOG_LEVEL_FATAL, msg)

/*!
 *
 * \brief Assert to a class logger
 *
 * Allows you to assert an expression and if it's false it will log an error.
 *
 * \param [in] cond: A predicate condition that will be tested
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * void foo()
 * {
 *    MLOG_CLASS_ASSERT(x == y, "x and y are not equal");
 * }
 * \endcode
 *
 */

#define MLOG_CLASS_ASSERT(cond, msg) \
   MLOG_ASSERT(cond, MLOG_CLASS_LOGGER(), msg)

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_ASSERT but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_ASSERT(cond, msg) \
   MLOG_ASSERT(cond, MLOG_CLASS_MEMBER_LOGGER(), msg)

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
 * MLOG_CLASS_TEST(
 *    MLOG_LEVEL_DEBUG
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_CLASS_TEST(level, cond, msg) \
   MLOG_TEST(level, cond, MLOG_CLASS_LOGGER(), msg);

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_TEST but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_TEST(level, cond, msg) \
   MLOG_TEST(level, cond, MLOG_CLASS_MEMBER_LOGGER(), msg);

/*!
 *
 * Alternatives to MLOG_CLASS_TEST, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_CLASS_TEST_TRACE(cond, msg) MLOG_CLASS_TEST(MLOG_LEVEL_TRACE, cond, msg)
#define MLOG_CLASS_TEST_DEBUG(cond, msg) MLOG_CLASS_TEST(MLOG_LEVEL_DEBUG, cond, msg)
#define MLOG_CLASS_TEST_INFO(cond, msg)  MLOG_CLASS_TEST(MLOG_LEVEL_INFO, cond, msg)
#define MLOG_CLASS_TEST_WARN(cond, msg)  MLOG_CLASS_TEST(MLOG_LEVEL_WARN, cond, msg)
#define MLOG_CLASS_TEST_ERROR(cond, msg) MLOG_CLASS_TEST(MLOG_LEVEL_ERROR, cond, msg)
#define MLOG_CLASS_TEST_FATAL(cond, msg) MLOG_CLASS_TEST(MLOG_LEVEL_FATAL, cond, msg)

/*!
 *
 * Alternatives to MLOG_CLASS_MEMBER_LOGGER_TEST, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_TEST_TRACE(cond, msg) MLOG_CLASS_MEMBER_LOGGER_TEST(MLOG_LEVEL_TRACE, cond, msg)
#define MLOG_CLASS_MEMBER_LOGGER_TEST_DEBUG(cond, msg) MLOG_CLASS_MEMBER_LOGGER_TEST(MLOG_LEVEL_DEBUG, cond, msg)
#define MLOG_CLASS_MEMBER_LOGGER_TEST_INFO(cond, msg)  MLOG_CLASS_MEMBER_LOGGER_TEST(MLOG_LEVEL_INFO, cond, msg)
#define MLOG_CLASS_MEMBER_LOGGER_TEST_WARN(cond, msg)  MLOG_CLASS_MEMBER_LOGGER_TEST(MLOG_LEVEL_WARN, cond, msg)
#define MLOG_CLASS_MEMBER_LOGGER_TEST_ERROR(cond, msg) MLOG_CLASS_MEMBER_LOGGER_TEST(MLOG_LEVEL_ERROR, cond, msg)
#define MLOG_CLASS_MEMBER_LOGGER_TEST_FATAL(cond, msg) MLOG_CLASS_MEMBER_LOGGER_TEST(MLOG_LEVEL_FATAL, cond, msg)

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
 * MLOG_CLASS_CHECK(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_CLASS_CHECK(cond,  msg) \
   MLOG_CLASS_TEST_ERROR(cond, msg)

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_CHECK but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_CHECK(cond,  msg) \
   MLOG_CLASS_MEMBER_LOGGER_TEST_ERROR(cond, msg)

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
 * MLOG_CLASS_THROW(
 *    (x == y),
 *    "Expected x and y to be the same!"
 * );
 * \endcode
 */

#define MLOG_CLASS_THROW(cond,  msg)\
   MLOG_THROW(cond, MLOG_CLASS_LOGGER(), msg)

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_THROW but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_THROW(cond,  msg)\
   MLOG_THROW(cond, MLOG_CLASS_MEMBER_LOGGER(), msg)

/*!
 * \brief Class macro to either throw or log an error
 *
 * \param [in] cond: If true, throw an exception, otherwise log an error
 * \param [in] msg: Stream expression that will be rendered as a message
 *
 * \code
 * MLOG_CLASS_THROW_IF(
 *    do_throw,
 *    "something's clearly wrong here"
 * );
 * \endcode
 */

#define MLOG_CLASS_THROW_IF(cond,  msg)\
   MLOG_THROW_IF(cond, MLOG_CLASS_LOGGER(), msg)

/*!
 * \brief Class macro to either throw or log an error
 *
 * Does the same as MLOG_CLASS_THROW but uses the default member logger
 */

#define MLOG_CLASS_MEMBER_LOGGER_THROW_IF(cond,  msg)\
   MLOG_THROW_IF(cond, MLOG_CLASS_MEMBER_LOGGER(), msg)

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

#define MLOG_CLASS_VERIFY(cond,  msg)\
   MLOG_VERIFY(cond, MLOG_CLASS_LOGGER(), msg)

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_VERIFY but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_VERIFY(cond,  msg)\
   MLOG_VERIFY(cond, MLOG_CLASS_MEMBER_LOGGER(), msg)

/*!
 *
 * \brief Programatically set the level
 *
 * This macro allows you to change the logging level for the class. Use with
 * caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_CLASS_SET_LEVEL(level) \
    MLOG_SET_LEVEL(level, MLOG_CLASS_LOGGER());

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_SET_LEVEL but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(level) \
   MLOG_SET_LEVEL(level, MLOG_CLASS_MEMBER_LOGGER());

/*!
 *
 * Alternatives to MLOG_CLASS_SET_LEVEL, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_CLASS_SET_LEVEL_ALL() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_ALL)
#define MLOG_CLASS_SET_LEVEL_OFF() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_OFF)
#define MLOG_CLASS_SET_LEVEL_TRACE() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_TRACE)
#define MLOG_CLASS_SET_LEVEL_DEBUG() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_DEBUG)
#define MLOG_CLASS_SET_LEVEL_INFO() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_INFO)
#define MLOG_CLASS_SET_LEVEL_WARN() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_WARN)
#define MLOG_CLASS_SET_LEVEL_ERROR() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_ERROR)
#define MLOG_CLASS_SET_LEVEL_FATAL() MLOG_CLASS_SET_LEVEL(MLOG_GET_LEVEL_FATAL)

/*!
 *
 * Alternatives to MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL, where the logging level is part of the macro name
 * rather than passed as a parameter. Which you use is entirely up to you.
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_ALL() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_ALL)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_OFF() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_OFF)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_TRACE() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_TRACE)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_DEBUG() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_DEBUG)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_INFO() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_INFO)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_WARN() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_WARN)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_ERROR() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_ERROR)
#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL_FATAL() MLOG_CLASS_MEMBER_LOGGER_SET_LEVEL(MLOG_GET_LEVEL_FATAL)

/*!
 *
 * \brief Programatically set the level
 *
 * This macro allows you to change the logging level of the class using a string representation
 * of its name. Use with caution because you might be overriding settings defined in a configuration file.
 *
 */

#define MLOG_CLASS_SET_LEVELSTR(levelstr) \
   MLOG_SET_LEVEL(str2level(levelstr), MLOG_CLASS_LOGGER());

/*!
 *
 * \brief Assert to a class logger
 *
 * Does the same as MLOG_CLASS_SET_LEVELSTR but uses the default member logger
 *
 */

#define MLOG_CLASS_MEMBER_LOGGER_SET_LEVELSTR(levelstr) \
   MLOG_SET_LEVEL(str2level(levelstr), MLOG_CLASS_MEMBER_LOGGER_LOGGER());

#endif
