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

#ifndef MOOST_TERMINAL_FORMAT_H__
#define MOOST_TERMINAL_FORMAT_H__

#include <string>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> // timing

namespace moost {

enum eColor
{
   C_BLACK = 30,
   C_RED = 31,
   C_GREEN = 32,
   C_YELLOW = 33,
   C_BLUE = 34,
   C_MAGENTA = 35,
   C_CYAN = 36,
   C_WHITE = 37,

   BGC_BLACK = 40,
   BGC_RED = 41,
   BGC_GREEN = 42,
   BGC_YELLOW = 43,
   BGC_BLUE = 44,
   BGC_MAGENTA = 45,
   BGC_CYAN = 46,
   BGC_WHITE = 47
};

enum eMask {    // numeric values only as example of possible bitmask values:
   tf_standard=1<<0,
   tf_bold=1<<1,
   tf_italic=1<<2,
   tf_underline=1<<3,
   tf_blinking=1<<4,
   tf_reverse=1<<5,

   tf_black=1<<6,
   tf_red=1<<7,
   tf_green=1<<8,
   tf_blue=1<<9,
   tf_magenta=1<<10,
   tf_cyan=1<<11,
   tf_white=1<<12,

   tf_bgblack=1<<13,
   tf_bgred=1<<14,
   tf_bggreen=1<<15,
   tf_bgblue=1<<16,
   tf_bgmagenta=1<<17,
   tf_bgcyan=1<<18,
   tf_bgwhite=1<<19
};

// -----------------------------------------------------------------------------

class null_terminal_format
{
public:
   static std::string bold()                          { return ""; }
   static std::string bold(const std::string& str)    { return str; }

   static std::string italic()                        { return ""; }
   static std::string italic(const std::string& str)  { return str; }

   static std::string underline()                        { return ""; }
   static std::string underline(const std::string& str)  { return str; }

   static std::string blinking()                       { return ""; }
   static std::string blinking(const std::string& str) { return str; }

   static std::string reverse()                        { return ""; }
   static std::string reverse(const std::string& str)  { return str; }

   static std::string color(eColor /*c*/)                         { return ""; }
   static std::string color(const std::string& str, eColor /*c*/) { return str; }

   static std::string reset(){ return ""; }

   static std::string get(const std::string& str, eMask /*mask*/)
   { return str; }

   static std::string getWarning(const std::string& warnStr = "WARNING") { return warnStr; }
   static std::string getError(const std::string& errStr = "ERROR") { return errStr; }
   static std::string getFailed(const std::string& errStr = "FAILED") { return errStr; }
   static std::string getOkay(const std::string& okayStr = "OK") { return okayStr; }

   static std::string getTimeStamp(bool enclose = true)
   {
      boost::posix_time::ptime currTime(boost::posix_time::second_clock::local_time());
      const boost::posix_time::time_duration& td = currTime.time_of_day();
      std::ostringstream oss;
      char fill_char = '0';

      if (enclose)
         oss << '[';

      oss << boost::gregorian::to_simple_string_type<char>(currTime.date()) << ' '
          << std::setw(2) << std::setfill(fill_char)
          << boost::date_time::absolute_value(td.hours()) << ":"
          << std::setw(2) << std::setfill(fill_char)
          << boost::date_time::absolute_value(td.minutes()) << ":"
          << std::setw(2) << std::setfill(fill_char)
          << boost::date_time::absolute_value(td.seconds())
          ;

      if ( enclose )
         oss << "] ";
      return oss.str();
   }
};

// -----------------------------------------------------------------------------

class vt_100_terminal_format
{
public:

   static std::string bold()                          { return "\033[1m"; }
   static std::string bold(const std::string& str)    { return bold() + str + reset(); }

   static std::string italic()                        { return "";  } // no italic in vt100
   static std::string italic(const std::string& str)  { return str; }

   static std::string underline()                        { return "\033[4m"; }
   static std::string underline(const std::string& str)  { return underline() + str + reset() ; }

   static std::string blinking()                       { return "\033[5m"; }
   static std::string blinking(const std::string& str) { return blinking() + str + reset(); }

   static std::string reverse()                        { return "\033[7m"; }
   static std::string reverse(const std::string& str)  { return reverse() + str + reset(); }

   static std::string color(eColor c)
   { return "\033[" + boost::lexical_cast<std::string>(c) + "m"; }
   static std::string color(const std::string& str, eColor c)
   { return color(c) + str + reset(); }

   static std::string reset()
   { return "\033[0m"; }

   static std::string get(const std::string& str, eMask mask)
   {
      std::ostringstream oss;

      if ( mask & tf_bold )
         oss << bold();
      //if ( mask & tf_italic )
      //   oss << italic();
      if ( mask & tf_underline )
         oss << underline();
      if ( mask & tf_blinking )
         oss << blinking();
      if ( mask & tf_reverse )
         oss << reverse();

      if ( mask & tf_black )
         oss << color(C_BLACK);
      if ( mask & tf_red )
         oss << color(C_RED);
      if ( mask & tf_green )
         oss << color(C_GREEN);
      if ( mask & tf_blue )
         oss << color(C_BLACK);
      if ( mask & tf_magenta )
         oss << color(C_MAGENTA);
      if ( mask & tf_cyan )
         oss << color(C_CYAN);

      if ( mask & tf_bgblack )
         oss << color(BGC_BLACK);
      if ( mask & tf_bgred )
         oss << color(BGC_RED);
      if ( mask & tf_bggreen )
         oss << color(BGC_GREEN);
      if ( mask & tf_bgblue )
         oss << color(BGC_BLACK);
      if ( mask & tf_bgmagenta )
         oss << color(BGC_MAGENTA);
      if ( mask & tf_bgcyan )
         oss << color(BGC_CYAN);

      oss << str << reset();
      return oss.str();
   }

   static std::string getWarning(const std::string& warnStr = "WARNING")
   { return bold() + color(C_YELLOW) + warnStr + reset(); }
   static std::string getError(const std::string& errStr = "ERROR")
   { return bold() + color(C_RED) + errStr + reset(); }
   static std::string getFailed(const std::string& errStr = "FAILED")
   { return bold() + color(C_RED) + errStr + reset(); }
   static std::string getOkay(const std::string& okayStr = "OK")
   { return bold() + color(C_GREEN) + okayStr + reset(); }

   static std::string getTimeStamp(bool enclose = true)
   { return null_terminal_format::getTimeStamp(enclose); }

};

// -----------------------------------------------------------------------------

#ifdef _WIN32
typedef null_terminal_format terminal_format;
#else
typedef vt_100_terminal_format terminal_format;
#endif

// -----------------------------------------------------------------------------

class scoped_format
{
public:
   scoped_format(std::ostream& s) : m_s(s) {}

   ~scoped_format()
   { m_s << terminal_format::reset(); }

private:
   std::ostream& m_s;
};

// -----------------------------------------------------------------------------


} // moost

#endif // MOOST_TERMINAL_FORMAT_H__
