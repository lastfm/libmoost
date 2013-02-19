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

#ifndef MOOST_LOGGING_DETAIL_HIGHLIGHT_LEVEL_PATTERN_CONVERTER_HPP__
#define MOOST_LOGGING_DETAIL_HIGHLIGHT_LEVEL_PATTERN_CONVERTER_HPP__

#include <log4cxx/pattern/loggingeventpatternconverter.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/spi/loggingevent.h>
#include "../../terminal_format.hpp"

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

namespace log4cxx { namespace pattern {

class HighlightLevelPatternConverter : public LoggingEventPatternConverter
{

   HighlightLevelPatternConverter()
      :   LoggingEventPatternConverter( LOG4CXX_STR("Level"),
                                        LOG4CXX_STR("level")) {}


public:

   /**
   * Obtains an instance of pattern converter.
   * @param options options, may be null.
   * @return instance of pattern converter.
   */
  static PatternConverterPtr newInstance(const std::vector<LogString>& /*options*/)
  {
     static PatternConverterPtr def(new HighlightLevelPatternConverter());
     return def;
  }

  void format( const log4cxx::spi::LoggingEventPtr& event,
               LogString& toAppendTo,
               log4cxx::helpers::Pool& /*p*/) const
  {
      LevelPtr level = event->getLevel();
      switch ( level->toInt()  )
      {
      case Level::FATAL_INT:
      case Level::ERROR_INT:
         {
            std::string lvl;
            level->toString(lvl);
            lvl = moost::terminal_format::getError(lvl);
            LogString wlvl;
            helpers::Transcoder::decode(lvl, wlvl);
            toAppendTo.append(wlvl);
         }
         break;

      case Level::WARN_INT:
         {
            std::string lvl;
            level->toString(lvl);
            lvl = moost::terminal_format::getWarning(lvl);
            LogString wlvl;
            helpers::Transcoder::decode(lvl, wlvl);
            toAppendTo.append(wlvl);
         }

         break;

      default:
         toAppendTo.append(event->getLevel()->toString());
         break;
      }
  }

  LogString getStyleClass(const log4cxx::helpers::ObjectPtr& obj) const
  {
     log4cxx::spi::LoggingEventPtr e(obj);
     if (e != NULL) {
        int lint = e->getLevel()->toInt();

        switch (lint) {
         case Level::TRACE_INT:
            return LOG4CXX_STR("level trace");

         case Level::DEBUG_INT:
            return LOG4CXX_STR("level debug");

         case Level::INFO_INT:
            return LOG4CXX_STR("level info");

         case Level::WARN_INT:
            return LOG4CXX_STR("level warn");

         case Level::ERROR_INT:
            return LOG4CXX_STR("level error");

         case Level::FATAL_INT:
            return LOG4CXX_STR("level fatal");

         default:
            return LogString(LOG4CXX_STR("level ")) + e->getLevel()->toString();
        }
     }

     return LOG4CXX_STR("level");

  }

};

}}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif // MOOST_LOGGING_CUSTOM_PATTERN_LAYOUT
