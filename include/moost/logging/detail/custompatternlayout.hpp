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

#ifndef MOOST_LOGGING_DETAIL_CUSTOM_PATTERN_LAYOUT_HPP__
#define MOOST_LOGGING_DETAIL_CUSTOM_PATTERN_LAYOUT_HPP__

#include <log4cxx/patternlayout.h>

#include <log4cxx/pattern/loggerpatternconverter.h>
#include <log4cxx/pattern/literalpatternconverter.h>
#include <log4cxx/pattern/classnamepatternconverter.h>
#include <log4cxx/pattern/datepatternconverter.h>
#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/pattern/filelocationpatternconverter.h>
#include <log4cxx/pattern/fulllocationpatternconverter.h>
#include <log4cxx/pattern/integerpatternconverter.h>
#include <log4cxx/pattern/linelocationpatternconverter.h>
#include <log4cxx/pattern/messagepatternconverter.h>
#include <log4cxx/pattern/lineseparatorpatternconverter.h>
#include <log4cxx/pattern/methodlocationpatternconverter.h>
#include <log4cxx/pattern/levelpatternconverter.h>
#include <log4cxx/pattern/relativetimepatternconverter.h>
#include <log4cxx/pattern/threadpatternconverter.h>
#include <log4cxx/pattern/ndcpatternconverter.h>
#include <log4cxx/pattern/propertiespatternconverter.h>
#include <log4cxx/pattern/throwableinformationpatternconverter.h>

#include "highlightlevelpatternconverter.hpp"

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

namespace log4cxx
{

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class LOG4CXX_EXPORT CustomPatternLayout : public PatternLayout
{
public:

   /**
    Does nothing
    */
    CustomPatternLayout() {}

    /**
    Just pass the constructor.
    */
    CustomPatternLayout(const LogString& pattern)
       : PatternLayout() // makes sure it's not calling the other constructor
    {
       setConversionPattern(pattern);
    }

#define RULES_PUT(spec, cls) \
   specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), (PatternConstructor) cls ::newInstance))

protected:
    virtual log4cxx::pattern::PatternMap getFormatSpecifiers()
    {
       using namespace log4cxx::pattern;
       PatternMap specs;
       RULES_PUT("c", LoggerPatternConverter);
       RULES_PUT("logger", LoggerPatternConverter);

       RULES_PUT("C", ClassNamePatternConverter);
       RULES_PUT("class", ClassNamePatternConverter);

       RULES_PUT("d", DatePatternConverter);
       RULES_PUT("date", DatePatternConverter);

       RULES_PUT("F", FileLocationPatternConverter);
       RULES_PUT("file", FileLocationPatternConverter);

       RULES_PUT("l", FullLocationPatternConverter);

       RULES_PUT("L", LineLocationPatternConverter);
       RULES_PUT("line", LineLocationPatternConverter);

       RULES_PUT("m", MessagePatternConverter);
       RULES_PUT("message", MessagePatternConverter);

       RULES_PUT("n", LineSeparatorPatternConverter);

       RULES_PUT("M", MethodLocationPatternConverter);
       RULES_PUT("method", MethodLocationPatternConverter);

       RULES_PUT("p", HighlightLevelPatternConverter);
       RULES_PUT("level", HighlightLevelPatternConverter);

       RULES_PUT("r", RelativeTimePatternConverter);
       RULES_PUT("relative", RelativeTimePatternConverter);

       RULES_PUT("t", ThreadPatternConverter);
       RULES_PUT("thread", ThreadPatternConverter);

       RULES_PUT("x", NDCPatternConverter);
       RULES_PUT("ndc", NDCPatternConverter);

       RULES_PUT("X", PropertiesPatternConverter);
       RULES_PUT("properties", PropertiesPatternConverter);

       RULES_PUT("throwable", ThrowableInformationPatternConverter);
       return specs;
    }

};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

}


#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif // MOOST_LOGGING_CUSTOM_PATTERN_LAYOUT_HPP
