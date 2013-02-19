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

#ifndef MOOST_LOGGING_GLOBAL_SCOPED_LOGGER_H
#define MOOST_LOGGING_GLOBAL_SCOPED_LOGGER_H

#include <boost/thread/tss.hpp>
#include "scoped_logger.hpp"

namespace moost { namespace logging {

typedef boost::thread_specific_ptr<scoped_logger> global_scoped_logger_t;
typedef boost::shared_ptr<scoped_logger>          LoggerPtr;

/**
* The thread-specific logging object.
*/
extern global_scoped_logger_t pgScopedLogging;

/**
* \brief handles scoped logging gracefully across the whole project.
* global_scoped_logger takes a scoped_logger and makes it available across the whole project
* without having to pass the object around. global_scoped_logger is thread safe because
* each object is thread specific.
* In order to be able to use global_scoped_logger you need to define the scope where the
* logging will operate:
* \code
void someFunction()
{
  MLOG_SCOPED_GLOBAL_FUNC_INIT;
  // alternatively one can specify the name with:
  // MLOG_SCOPED_GLOBAL_NAMED_INIT("someName");

  // now it's time to get the logger
  MLOG_SCOPED_GLOBAL_GET_LOGGER(logger);

  MLOG(MLOG_LEVEL_INFO, logger, "log this: " << 944534634578UL);
}
\endcode
* Now, this alone would not be very useful. The true power of global_scoped_logger is that you
* have access of the logger everywhere in your code without having to pass it around!
* Let's say you defined a few functions before "someFunction()":
* \code
void foo()
{
   MLOG_SCOPED_GLOBAL_GET_LOGGER(logger);
   MLOG(MLOG_LEVEL_INFO, logger, "I am in foo!");
}

void bar()
{
   foo(); // nothing needs to be passed here
}
\endcode
* \note Do not forget to include the moost library in your linking or it will not find the external
* for m_pScopedLogging!
*/
class global_scoped_logger
{
public:

   global_scoped_logger(const std::string& name)
   : m_pScopedLogging(pgScopedLogging)
   { m_pScopedLogging.reset( new scoped_logger(name)); }

   template <typename TTimer>
   global_scoped_logger(const std::string& name, TTimer& start)
   : m_pScopedLogging(pgScopedLogging)
   { m_pScopedLogging.reset( new scoped_logger(name, start) ); }

   ~global_scoped_logger()
   { m_pScopedLogging.reset(); }

private:

   global_scoped_logger_t& m_pScopedLogging;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
* A custom deleter that does nothing, used to transform the thread specific pointer
* into a shared pointer.
*/
struct noop_deleter { void operator()(void*) {} };

/**
* Free function to get the global scoped verbose for the current thread.
* \note If possible please use MLOG_SCOPED_GLOBAL_GET_LOGGER(logger) instead
*/
inline LoggerPtr get_global_scoped_logger()
{
   if ( pgScopedLogging.get() == 0 )
      throw std::runtime_error("Global scoped_logger was not set for this thread!");

   // transform the pointer from thread specific to shared_ptr, just make
   // sure shared_ptr does NOT deallocate the object (that's responsibility of
   // thread specific).
   LoggerPtr sharedPtr( pgScopedLogging.get(), noop_deleter() );
   return sharedPtr;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
* Initialize (that is, create the object for the current scope) the
* global scoped logger with the given name.
*/
#define MLOG_SCOPED_GLOBAL_NAMED_DEFINE(name) \
   moost::logging::global_scoped_logger __global_scoped_logger(name)

#define MLOG_SCOPED_GLOBAL_NAMED_TIME_DEFINE(name, time) \
   moost::logging::global_scoped_logger __global_scoped_logger(name, time)

/**
* Initialize (that is, create the object for the current scope) the
* global scoped logger with the name of the current function.
*/
#define MLOG_SCOPED_GLOBAL_FUNC_DEFINE() \
   moost::logging::global_scoped_logger __global_scoped_logger( BOOST_CURRENT_FUNCTION )

#define MLOG_SCOPED_GLOBAL_FUNC_TIME_DEFINE(time) \
   moost::logging::global_scoped_logger __global_scoped_logger( BOOST_CURRENT_FUNCTION, time )

/**
* Get the logger currently registered for the existing thread.
*/
#define MLOG_SCOPED_GLOBAL_GET_LOGGER(logger) \
   moost::logging::LoggerPtr logger = moost::logging::get_global_scoped_logger()

/**
* This is equivalent to:
* moost::multi_timer::scoped_time st(fm303_multi_timer(), "name");
* moost::logging::global_scoped_logger __global_scoped_logger("name", st) );
*/
#define MLOG_SCOPED_GLOBAL_FM303_DEFINE(name) \
   moost::multi_timer::scoped_time st(fm303_multi_timer(), name); \
   MLOG_SCOPED_GLOBAL_NAMED_TIME_DEFINE(name, st)

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Create a default global logger, which basically does nothing.
constructor__(init_global_scoped_logger)
{
   pgScopedLogging.reset(new scoped_logger);
}

}}

#endif // MOOST_LOGGING_GLOBAL_SCOPED_LOGGER_H
