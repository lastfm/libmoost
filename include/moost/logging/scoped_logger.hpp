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

#ifndef MOOST_LOGGING_SCOPED_LOGGER_HPP
#define MOOST_LOGGING_SCOPED_LOGGER_HPP

#include <string>
#include <sstream>
#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/logger.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/mutex.h>

#include <log4cxx/helpers/resourcebundle.h>
#include <log4cxx/helpers/transcoder.h>

#include <log4cxx/appenderskeleton.h>

#include "../logging.hpp"
#include "../timer.h"

namespace moost { namespace logging {

/**
* \brief handles verbose with log4cxx gracefully.
* scoped_logger takes care of collecting the log information within the scope, then
* spit it out (to whatever it was configured to)
* \note Important: scoped_logger is \b not thread safe.
* \note I must forward everything because the constructor is a bit of a mess (it's generated from
* a factory).
*/
class scoped_logger
{
private:

   struct sLogEntry
   {
      sLogEntry()
         : pLocationInfo(NULL) {}

      log4cxx::LevelPtr level;
      std::string message;
      log4cxx::spi::LocationInfo const* pLocationInfo;
   };

public:

   scoped_logger()
    : m_startTime(boost::posix_time::microsec_clock::local_time())
   {
      init(std::string(), log4cxx::Level::getOff());
   }

   scoped_logger(const std::string& name)
    : m_startTime(boost::posix_time::microsec_clock::local_time())
   {
      init(name);
   }

   scoped_logger(const std::string& name, moost::timer::scoped_time& sc)
      : m_startTime(sc.get_time())
   {
      init(name);
   }

   scoped_logger(const std::string& name, moost::multi_timer::scoped_time& sc)
      : m_startTime(sc.get_time())
   {
      init(name);
   }

   scoped_logger(const std::string& name, const boost::posix_time::ptime& startTime)
      : m_startTime(startTime)
   {
      init(name);
   }

   ~scoped_logger()
   {
      try
      {
         forcedLog();
      }
      catch (...)
      {}
   }

   //////////////////////////////////////////////////////////////////////////
   // scoped_logger custom

   /**
   * Set the timings headers.
   */
   void setTimingsHeader(const std::string& totTimingsHeader = "  Overall time: ", const std::string& subTimingsIndentation = "    -> ")
   {
      m_totTimingsHeader = totTimingsHeader;
      m_subTimingsIndentation = subTimingsIndentation;
   }

   void addSubTime(const std::string& header, int ms)
   {
      m_subTimes[header] += ms;
   }

   //////////////////////////////////////////////////////////////////////////

   void forcedLog(const log4cxx::LevelPtr& level, const std::string& message)
   {
      updateHighestLevel(level);

      m_messages.push_back( sLogEntry() );
      sLogEntry& b = m_messages.back();
      b.level = level;
      b.message = message;
   }

   void forcedLog(const log4cxx::LevelPtr& level, const std::string& message, const log4cxx::spi::LocationInfo& location)
   {
      updateHighestLevel(level);

      m_messages.push_back( sLogEntry() );
      sLogEntry& b = m_messages.back();
      b.level = level;
      b.message = message;
      b.pLocationInfo = &location;
   }

   log4cxx::LevelPtr getLevel() const
   { return m_loggerLevel; }

   virtual void setLevel(const log4cxx::LevelPtr& level)
   { m_loggerLevel = level; }

   //////////////////////////////////////////////////////////////////////////

   void debug(const std::string& msg, const log4cxx::spi::LocationInfo& location)
   {
      if (isDebugEnabled()) {
         forcedLog(log4cxx::Level::getDebug(), msg, location);
      }
   }
   void debug(const std::string& msg)
   {
      if (isDebugEnabled()) {
         forcedLog(log4cxx::Level::getDebug(), msg);
      }
   }

   void error(const std::string& msg, const log4cxx::spi::LocationInfo& location)
   {
      if (isErrorEnabled()) {
         forcedLog(log4cxx::Level::getError(), msg, location);
      }
   }
   void error(const std::string& msg)
   {
      if (isErrorEnabled()) {
         forcedLog(log4cxx::Level::getError(), msg);
      }
   }

   void fatal(const std::string& msg, const log4cxx::spi::LocationInfo& location)
   {
      if (isFatalEnabled()) {
         forcedLog(log4cxx::Level::getFatal(), msg, location);
      }
   }
   void fatal(const std::string& msg)
   {
      if (isFatalEnabled()) {
         forcedLog(log4cxx::Level::getFatal(), msg);
      }
   }

   void info(const std::string& msg, const log4cxx::spi::LocationInfo& location)
   {
      if (isInfoEnabled()) {
         forcedLog(log4cxx::Level::getInfo(), msg, location);
      }
   }
   void info(const std::string& msg)
   {
      if (isInfoEnabled()) {
         forcedLog(log4cxx::Level::getInfo(), msg);
      }
   }

   void log(const log4cxx::LevelPtr& level, const std::string& message,
            const log4cxx::spi::LocationInfo& location)
   {
      if (isEnabledFor(level)) {
         forcedLog(level, message, location);
      }
   }
   void log(const log4cxx::LevelPtr& level, const std::string& message)
   {
      if (isEnabledFor(level)) {
         forcedLog(level, message);
      }
   }

   void warn(const std::string& msg, const log4cxx::spi::LocationInfo& location)
   {
      if (isWarnEnabled()) {
         forcedLog(log4cxx::Level::getWarn(), msg, location);
      }
   }
   void warn(const std::string& msg)
   {
      if (isWarnEnabled()) {
         forcedLog(log4cxx::Level::getWarn(), msg);
      }
   }

   void trace(const std::string& msg, const log4cxx::spi::LocationInfo& location)
   {
      if (isTraceEnabled()) {
         forcedLog(log4cxx::Level::getTrace(), msg, location);
      }
   }
   void trace(const std::string& msg)
   {
      if (isTraceEnabled()) {
         forcedLog(log4cxx::Level::getTrace(), msg);
      }
   }

   //////////////////////////////////////////////////////////////////////////

   void addRef() const
   { /*m_pLogger->addRef(); */}
   void releaseRef() const
   { /*m_pLogger->releaseRef(); */}
   virtual void addAppender(const log4cxx::AppenderPtr& /* newAppender */)
   { /*m_pLogger->addAppender(newAppender);*/ }
   void callAppenders(const log4cxx::spi::LoggingEventPtr& /* event */, log4cxx::helpers::Pool& /* pool */) const
   { /*m_pLogger->callAppenders(event, pool);*/ }
   void closeNestedAppenders()
   { /*m_pLogger->closeNestedAppenders();*/ }

   bool getAdditivity() const
   { return true; /*m_pLogger->getAdditivity();*/ }
   //log4cxx::AppenderList getAllAppenders() const
   //{ return m_pLogger->getAllAppenders(); }
   virtual const log4cxx::LevelPtr& getEffectiveLevel() const
   { return m_loggerLevel; }
   log4cxx::spi::LoggerRepositoryPtr getLoggerRepository() const
   { return log4cxx::Logger::getRootLogger()->getLoggerRepository(); }
   void getName(std::string& name) const
   { log4cxx::helpers::Transcoder::encode(m_name, name); }
   log4cxx::LoggerPtr getParent() const
   { return log4cxx::Logger::getRootLogger(); }
   log4cxx::helpers::ResourceBundlePtr getResourceBundle() const
   { return log4cxx::Logger::getRootLogger()->getResourceBundle(); }

   void removeAllAppenders()
   { log4cxx::Logger::getRootLogger()->removeAllAppenders(); }
   void removeAppender(const log4cxx::AppenderPtr& appender)
   { log4cxx::Logger::getRootLogger()->removeAppender(appender); }
   void setAdditivity(bool additive)
   { log4cxx::Logger::getRootLogger()->setAdditivity(additive); }
   inline void setResourceBundle(const log4cxx::helpers::ResourceBundlePtr& bundle)
   { log4cxx::Logger::getRootLogger()->setResourceBundle(bundle); }
   inline const log4cxx::helpers::Mutex& getMutex() const
   { return log4cxx::Logger::getRootLogger()->getMutex(); }

   //////////////////////////////////////////////////////////////////////////
   // forwarded methods

   bool isAttached(const log4cxx::AppenderPtr& appender) const
   { return log4cxx::Logger::getRootLogger()->isAttached(appender); }
   bool isEnabledFor(const log4cxx::LevelPtr& level) const
   { return level->isGreaterOrEqual(m_loggerLevel); }

   bool isDebugEnabled() const
   { return m_loggerLevel->toInt() <= log4cxx::Level::DEBUG_INT; }
   bool isInfoEnabled() const
   { return m_loggerLevel->toInt() <= log4cxx::Level::INFO_INT; }
   bool isWarnEnabled() const
   { return m_loggerLevel->toInt() <= log4cxx::Level::WARN_INT; }
   bool isErrorEnabled() const
   { return m_loggerLevel->toInt() <= log4cxx::Level::ERROR_INT; }
   bool isFatalEnabled() const
   { return m_loggerLevel->toInt() <= log4cxx::Level::FATAL_INT; }
   bool isTraceEnabled() const
   { return m_loggerLevel->toInt() <= log4cxx::Level::TRACE_INT; }

   //////////////////////////////////////////////////////////////////////////
   //// not implemented

   //AppenderPtr getAppender(const LogString& name) const;

   //void forcedLogLS( const LevelPtr& level, const log4cxx::LogString& message,
   //                  const log4cxx::spi::LocationInfo& location) const;
   // static LoggerPtr getLoggerLS(const LogString& name);
   //static LoggerPtr getLoggerLS(const LogString& name,
   //                             const log4cxx::spi::LoggerFactoryPtr& factory);
   //void removeAppender(const LogString& name)
   //void l7dlog(const LevelPtr& level, const LogString& key,
   //            const log4cxx::spi::LocationInfo& locationInfo,
   //            const std::vector<LogString>& values) const;
   //void logLS(const LevelPtr& level, const LogString& message,
   //           const log4cxx::spi::LocationInfo& location) const;
   //const LogString getName() const { return name; }

private:

   void forcedLog()
   {
      if ( !m_highestMessageLevel )
         m_highestMessageLevel = m_loggerLevel;

      if ( isTraceEnabled() )
      {
         std::ostringstream buf;
         buf << "\n";

         // get subtimes
         if ( !m_subTimes.empty() )
         {
            std::map<std::string, int>::const_iterator it;
            for ( it = m_subTimes.begin(); it != m_subTimes.end(); ++it )
               buf << ">>> " << m_subTimingsIndentation << it->first << ": " <<  it->second << "ms\n";

         }

         // get overall time
         int timeMs  = static_cast<int> (
            (  boost::posix_time::microsec_clock::local_time() -
            m_startTime
            ).total_milliseconds() );

         buf << "   " << m_totTimingsHeader << timeMs << "ms\n";

         m_messages.push_back( sLogEntry() );
         sLogEntry& b = m_messages.back();
         b.level = log4cxx::Level::getDebug();
         b.message = buf.str();
      }

      std::vector< std::pair<log4cxx::LevelPtr, log4cxx::AppenderPtr> >::iterator aIt;
      std::vector<sLogEntry>::const_iterator mIt;

      std::string message;
      log4cxx::helpers::Pool p;
      log4cxx::LogString msg;

      for ( aIt = m_appenders.begin(); aIt != m_appenders.end(); ++aIt )
      {
         const log4cxx::LevelPtr& appenderLevel = aIt->first;
         log4cxx::AppenderPtr& appender = aIt->second;

         std::ostringstream buf;
         log4cxx::spi::LocationInfo const* pLocationInfo = NULL;

         if(!m_messages.empty())
         {
            buf << "\n";

            for ( mIt = m_messages.begin(); mIt != m_messages.end(); ++mIt )
            {
               if ( mIt->level->isGreaterOrEqual( appenderLevel ) )
               {
                  buf << "   " << mIt->message << "\n";
                  pLocationInfo = mIt->pLocationInfo;
               }
            }
         }

         message = buf.str();
         if ( message.empty() )
            continue;

         LOG4CXX_DECODE_CHAR(msg, message);

         if ( pLocationInfo )
         {
            log4cxx::spi::LoggingEventPtr event(new log4cxx::spi::LoggingEvent( m_name, m_highestMessageLevel, msg,
               *pLocationInfo ));
            appender->doAppend(event, p);
         }
         else
         {
            log4cxx::spi::LoggingEventPtr event(new log4cxx::spi::LoggingEvent( m_name, m_highestMessageLevel, msg,
               log4cxx::spi::LocationInfo::getLocationUnavailable()));
            appender->doAppend(event, p);
         }

      }
   }

   void updateHighestLevel(const log4cxx::LevelPtr& level)
   {
      if ( !m_highestMessageLevel || // no level was set
         ( !level->equals(m_highestMessageLevel) && level->isGreaterOrEqual(m_highestMessageLevel) ) // it's a higher level!
         )
      {
         m_highestMessageLevel = level;
      }
   }

public:
   /**
   * Initialize the logger.
   * It will go through all the appenders currently registered and check the lowest threshold,
   * then set its own level to that and keep track of the appenders.
   */
   void init(const std::string& name, log4cxx::LevelPtr level = log4cxx::Level::getFatal())
   {
      setTimingsHeader();
      log4cxx::helpers::Transcoder::decode(name, m_name);

      log4cxx::LoggerPtr pRoot = log4cxx::Logger::getRootLogger();
      m_loggerLevel = level;

      //const log4cxx::helpers::Mutex& mutex = pRoot->getMutex();
      //log4cxx::helpers::synchronized s(mutex);

      // go through the list of appenders and set the lowest threshold!
      log4cxx::AppenderList al = pRoot->getAllAppenders();
      log4cxx::AppenderList::const_iterator it;
      for ( it = al.begin(); it != al.end(); ++it )
      {
         // Looks ugly, but works quite nicely (TM)
         // The static_cast<> invokes the overloaded cast operator of the ObjectPtrT<> template and turns
         // the AppenderPtr into and Appender *, which we can easily downcast to an AppenderSkeleton *.
         log4cxx::AppenderSkeleton *pSkeletonAppender = dynamic_cast<log4cxx::AppenderSkeleton*>(static_cast<log4cxx::Appender *>(*it));
         if ( !pSkeletonAppender )
            continue;

         const log4cxx::LevelPtr& pLvl = pSkeletonAppender->getThreshold();

         if ( !pLvl )
            continue;

         m_appenders.push_back( std::make_pair(pLvl, *it) );

         if ( m_loggerLevel->isGreaterOrEqual(pLvl) )
         {
            m_loggerLevel = pLvl;
         }
      }
   }

private:

   /**
   * The level of the logger. Below this level nothing is stored.
   * This is generally initialized with the smallest level of all the appenders
   * attached to root.
   */
   log4cxx::LevelPtr m_loggerLevel;

   /**
   * The highest level of the message received. It's used when sending the actual entry
   * to the appender.
   */
   log4cxx::LevelPtr m_highestMessageLevel;

   /**
   * The name of the log.
   */
   log4cxx::LogString m_name;

   /**
   * The messages stored.
   */
   std::vector<sLogEntry> m_messages;

   /**
   * The list of available appenders.
   */
   std::vector< std::pair<log4cxx::LevelPtr, log4cxx::AppenderPtr> > m_appenders;

   boost::posix_time::ptime m_startTime;
   std::map<std::string, int> m_subTimes;

   std::string m_totTimingsHeader;
   std::string m_subTimingsIndentation;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef boost::shared_ptr<moost::logging::scoped_logger> LoggerPtr;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class scoped_timing
{
public:
   scoped_timing( LoggerPtr& logger, const std::string& str)
      : m_pLogger(logger), m_str(str),
        m_startTime(boost::posix_time::microsec_clock::local_time())
   {}

   ~scoped_timing()
   {
      try
      {
         int timeMs  = static_cast<int> (
            (  boost::posix_time::microsec_clock::local_time() -
               m_startTime
            ).total_milliseconds() );

         m_pLogger->addSubTime( m_str, timeMs);
      }
      catch (...)
      {}
   }

private:

   LoggerPtr& m_pLogger;
   std::string m_str;
   boost::posix_time::ptime m_startTime;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define MLOG_SCOPED_NAMED_DEFINE(logger, name) \
   moost::logging::LoggerPtr logger( new moost::logging::scoped_logger( name ) )

#define MLOG_SCOPED_FUNC_DEFINE(logger) \
   moost::logging::LoggerPtr logger( new moost::logging::scoped_logger( MLOG_FUNC_NAME() ) )

#define MLOG_SCOPED_NAMED_TIME_DEFINE(logger, name, time) \
   moost::logging::LoggerPtr logger( new moost::logging::scoped_logger( name, time ) )

#define MLOG_SCOPED_FUNC_TIME_DEFINE(logger, time) \
   moost::logging::LoggerPtr logger( new moost::logging::scoped_logger( MLOG_FUNC_NAME(), time ) )

/**
* This is equivalent to:
* moost::multi_timer::scoped_time st(fm303_multi_timer(), "name");
* moost::logging::LoggerPtr logger( new moost::logging::scoped_logger("name", st) );
*/
#define MLOG_SCOPED_FM303_DEFINE(logger, name) \
   moost::multi_timer::scoped_time st(fm303_multi_timer(), name); \
   MLOG_SCOPED_NAMED_TIME_DEFINE(logger, name, st)

/**
* This is equivalent to:
* moost::multi_timer::scoped_time st(fm303_multi_timer(), MLOG_FUNC_NAME());
* moost::logging::LoggerPtr logger( new moost::logging::scoped_logger(MLOG_FUNC_NAME(), st) );
*/
#define MLOG_SCOPED_FM303_FUNCTION_DEFINE(logger) \
   moost::multi_timer::scoped_time st(fm303_multi_timer(), MLOG_FUNC_NAME()); \
   MLOG_SCOPED_NAMED_TIME_DEFINE(logger, MLOG_FUNC_NAME(), st)

/**
* This is equivalent to:
* moost::multi_timer::scoped_time st(fm303_multi_timer(), typeid(*this).name());
* moost::logging::LoggerPtr logger( new moost::logging::scoped_logger(typeid(*this).name(), st) );
*
*/
#define MLOG_SCOPED_FM303_CLASS_DEFINE(logger) \
   moost::multi_timer::scoped_time st(fm303_multi_timer(), typeid(*this).name()); \
   MLOG_SCOPED_NAMED_TIME_DEFINE(logger, typeid(*this).name(), st)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

}}

#endif // #define MOOST_LOGGING_SCOPED_LOGGER_HPP
