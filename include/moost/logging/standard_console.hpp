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

#ifndef MOOST_LOGGING_LEVELS_HPP
#define MOOST_LOGGING_LEVELS_HPP

#include <stdexcept>
#include <iostream>

#include <boost/thread/shared_mutex.hpp>
#include "../terminal_format.hpp"

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/level.h>

#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/logger.h>

#include "detail/custompatternlayout.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace moost { namespace logging {

/**
* Available levels are (defined in log4cxx::Level)
* \code
   enum {
      OFF_INT = INT_MAX,
      FATAL_INT = 50000,
      ERROR_INT = 40000,
      WARN_INT = 30000,
      INFO_INT = 20000,
      DEBUG_INT = 10000,
      TRACE_INT = 5000,
      ALL_INT = INT_MIN
   };
\endcode
*/

class standard_console
{
public:

   /**
   * Constructor. The default layout is:
   * "[%d{yyyy-MMM-dd HH:mm:ss}|%c](%p) %m%n"
   */
   standard_console(log4cxx::LevelPtr threshold = log4cxx::Level::getError(), std::string sLayout = "")
      : m_invalidLevel(new log4cxx::Level(-1, LOG4CXX_STR("INVALID"), 7 ))
   {
      using namespace log4cxx;

      if ( sLayout.empty() )
      {
         sLayout = std::string("[%d{yyyy-MMM-dd HH:mm:ss}|") +
                   moost::terminal_format::color("%c", moost::C_CYAN) +
                   std::string("](%p) %m%n");
      }

      LogManager::getLoggerRepository()->setConfigured(true);
      log4cxx::LoggerPtr root = Logger::getRootLogger();

      // The pattern. For details see
      // http://logging.apache.org/log4cxx/apidocs/classlog4cxx_1_1_pattern_layout.html
      //
      // "[%d{yyyy-MMM-dd HH:mm:ss}|%c]" // [date|"name"]
      // "(%p)" // level of the logging event
      // "%m"   // the message
      // "%n"   // new line
      static LogString lsLayout; // TTCC_CONVERSION_PATTERN(LOG4CXX_STR( layout ));
      helpers::Transcoder::decode(sLayout, lsLayout);

      //LayoutPtr layout(new CustomPatternLayout(lsLayout));
      LayoutPtr layout(new CustomPatternLayout(lsLayout));
      m_pAppender = new ConsoleAppender(layout);

      m_pAppender->setThreshold( threshold );
      root->addAppender(m_pAppender);
   }

   virtual ~standard_console()
   {}

   void enable()
   {
      log4cxx::Logger::getRootLogger()->addAppender(m_pAppender);
   }

   void disable()
   {
      log4cxx::Logger::getRootLogger()->removeAppender(m_pAppender);
   }

   /**
   * Available levels are (defined in log4cxx::Level)
   * \code
      enum {
         OFF_INT = INT_MAX,
         FATAL_INT = 50000,
         ERROR_INT = 40000,
         WARN_INT = 30000,
         INFO_INT = 20000,
         DEBUG_INT = 10000,
         TRACE_INT = 5000,
         ALL_INT = INT_MIN
      };
   \endcode
   */
   bool setThreshold(int val)
   {
      //log4cxx::Level invalidLevel(-1);
      boost::unique_lock<boost::shared_mutex> lock(m_mutex);

      log4cxx::LevelPtr newLevel = log4cxx::Level::toLevel(val, m_invalidLevel);
      if ( newLevel == m_invalidLevel )
         return false;
      else
      {
         m_pAppender->setThreshold(newLevel);
         return true;
      }
   }

   bool setThreshold(const log4cxx::LevelPtr& level)
   {
      boost::unique_lock<boost::shared_mutex> lock(m_mutex);
      m_pAppender->setThreshold(level);
      return true;
   }

   bool setSmallThreshold(int val)
   {
      using namespace log4cxx;

      switch (val)
      {
      case 0:
         return setThreshold(Level::FATAL_INT);
         break;
      case 1:
         return setThreshold(Level::ERROR_INT);
         break;
      case 2:
         return setThreshold(Level::WARN_INT);
         break;
      case 3:
         return setThreshold(Level::INFO_INT);
         break;
      case 4:
         return setThreshold(Level::DEBUG_INT);
         break;
      case 5:
         return setThreshold(Level::TRACE_INT);
         break;
      case 6:
         return setThreshold(Level::ALL_INT);
         break;
      default:
         if ( val > 5 )
            return setThreshold(Level::ALL_INT);

         return false;
      }
      return true;
   }

   bool setThreshold(const std::string& str)
   {
      if ( isAllDigits(str) )
      {
         int val = atoi(str.c_str());
         if ( val < log4cxx::Level::TRACE_INT )
            return setSmallThreshold(val);
         else
         {
            return setThreshold(val);
         }
      }
      else
      {
         log4cxx::LevelPtr newLevel = log4cxx::Level::toLevel(str, m_invalidLevel);
         if ( newLevel == m_invalidLevel )
            return false;
         else
         {
            boost::unique_lock<boost::shared_mutex> lock(m_mutex);
            m_pAppender->setThreshold(newLevel);
            return true;
         }
      }
   }

   int getSmallThreshold()
   {
      using namespace log4cxx;

      int lint = 0;
      {
         boost::shared_lock<boost::shared_mutex> lock(m_mutex);
         lint = m_pAppender->getThreshold()->toInt();
      }

      switch (lint) {
         case Level::ALL_INT:
            return 6;

         case Level::TRACE_INT:
            return 5;

         case Level::DEBUG_INT:
            return 4;

         case Level::INFO_INT:
            return 3;

         case Level::WARN_INT:
            return 2;

         case Level::ERROR_INT:
            return 1;

         case Level::FATAL_INT:
            return 0;

         default:
            return -1;
      }
   }

   int getThreshold()
   {
      boost::shared_lock<boost::shared_mutex> lock(m_mutex);
      return m_pAppender->getThreshold()->toInt();
   }

   void getThreshold(std::string& level)
   {
      boost::shared_lock<boost::shared_mutex> lock(m_mutex);
      m_pAppender->getThreshold()->toString(level);
   }

private:

   bool isAllDigits(const std::string& str)
   {
      for (std::string::const_iterator it = str.begin(); it != str.end(); ++it )
      {
         if ( std::isspace(*it) )
            continue;
         else if ( !std::isdigit(*it) )
            return false;
      }

      return true;
   }

private:

   boost::shared_mutex m_mutex;
   log4cxx::ConsoleAppenderPtr m_pAppender;
   log4cxx::LevelPtr m_invalidLevel;
};

}}

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#endif // MOOST_LOGGING_LEVELS_HPP
