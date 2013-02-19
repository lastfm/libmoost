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

#ifndef MOOST_SCOPED_VERBOSE_H
#define MOOST_SCOPED_VERBOSE_H

#include <sstream>
#include <iostream>
#include <cassert>

#include "timer.h"
#include "terminal_format.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> // timing

#define MOOST_SCOPED_VERBOSE_BOOKMARK_SEP "__scoped__"
#define MOOST_RECURRING_VERBOSE_BOOKMARK_SEP "__recurring__"

// forward declaration
namespace moost
{
   class scoped_verbose;
}

// declaration overriding
namespace std
{
   // free functions
   inline moost::scoped_verbose& endl(moost::scoped_verbose& sv);
   // free functions
   inline moost::scoped_verbose& flush(moost::scoped_verbose& sv);
}

namespace moost {

/**
* \brief handles verbose gracefully.
* It behaves like ostringstream and prints the verbose at the end of the scope.
* Example.
* \code
void myFunction()
{
  moost::scoped_verbose v("myFunction");
  v << "Something.."; // that's optional
  // ..
}
\endcode
* here it will print to cout:
\verbatim
[2009-Jun-18 16:38:09|myFunction] Something..
\endverbatim
*
* It is also possible to wrap the scoped verbose inside a class that will always output
* at the same level. For instance if you are in a function that has only trivial verbose
* instead of using .addTrivial all the time, just do
\code
void trivialFunc(moost::scoped_verbose& v)
{
   moost::trivial_scoped_verbose tv(v);
   tv << "the answer to the question is " << 42;
}
\endcode
* Same is true for error_scoped_verbose and warning_scoped_verbose;
* \note For efficiency reasons if you try to add something to a lower priority than
* the current level, the buffer will \b not be updated, but if you start low
* then switch the priority to high, only the higher will be output, i.e.
\code
void foo()
{
   moost::scoped_verbose v("myFunction", moost::scoped_verbose::VL_WARNING_PRIORITY); // starting medium high verbose
   v << "<hi>"; // this will NOT be recorded since default is VL_LOW_PRIORITY
   v.addWarning("<a warning>"); // this is going to be recorded
   v.changeVerboseLevel(moost::scoped_verbose::VL_HIGH_PRIORITY); // now "<a warning>" will not be printed
   v.addError("<error!>"); // this is the only one at this point that will be output
   v.addWarning("<another warning>"); // not recorded as the current level is VL_HIGH_PRIORITY

   v.changeVerboseLevel(moost::scoped_verbose::VL_LOW_PRIORITY); // we changed our mind again, but what's lost is lost!
} // at the end of the scope it will print: "<a warning><error!>"
\endcode
*
* If verbose is set to everything it will take the bookmark and display it
* in order making a diff from the previous one (with the first one
* using the time in the constructor).
* This allows an easy way of timing differents parts of the code.
* i.e.
* \code
void myFunction()
{
moost::scoped_verbose v("myFunction");
// do something expensive
v.addBookmark("expensive_op1");
// do something else expensive
v.addBookmark("expensive_op2");
// finishing up..
}
\endcode
will result in:
\verbatim
[2009-Jun-18 16:38:09 | myFunction]
expensive_op1: 12ms
expensive_op2: 53ms
total: 68ms
\endverbatim
\param str the name of the expensive op
\note this will work only and only if verbose is set to VL_EVERYTHING
\see ~scoped_verbose
* It is possible to use the timer for only a specific are of the code
* by using the scoped_bookmark, i.e.
\code
moost::scoped_verbose v("foo", moost::scoped_verbose::VL_EVERYTHING); // if not it will not record bookmarks
v << "<hi>";
{ // start a scope
   moost::scoped_verbose::scoped_bookmark sb(sv, "inTheScope"); // timer of the bookmark starts here
   // do stuff
} // end of scope, the timer is stopped and "inTheScope" is assigned
\endcode
If one wants to sum up the times of a recurring event it's possible to do so
by using a recurrurring_bookmark:
\code
moost::scoped_verbose v("foo", moost::scoped_verbose::VL_EVERYTHING); // if not it will not record bookmarks
for (i = 0; i < 10; ++i)
{
   // something here we do not want to measure
   { // start scope for the thing we want to measure and it's repeated
      moost::scoped_verbose::recurring_bookmark sb(sv, "recurring_event"); // timer of the bookmark starts here
      // do something
   }// end of scope, the time measured in the scope is added
}
\endcode
* The recurring bookmark will take 10 measures for the operation in the scope and average
* it out before printing out.
* \note Important: scoped_verbose is \b not thread safe.
*/
class scoped_verbose
{
public:

   enum eVerboseLevel
   {
      VL_HIGH_PRIORITY,
      VL_WARNING_PRIORITY,
      VL_LOW_PRIORITY,
      VL_EVERYTHING,
      VL_MAX = VL_EVERYTHING
   };

   class scoped_bookmark
   {
   public:
      scoped_bookmark(scoped_verbose& sv, const std::string& str)
         : m_sv(sv), m_str(str)
      {
         m_sv.addBookmark(MOOST_SCOPED_VERBOSE_BOOKMARK_SEP);
      }

      ~scoped_bookmark()
      {
         m_sv.addBookmark(m_str);
      }

   private:
      scoped_verbose& m_sv;
      std::string m_str;
   };

   class recurrurring_bookmark
   {
   public:
      recurrurring_bookmark(scoped_verbose& sv, const std::string& str)
         : m_sv(sv), m_str(str)
      {
         m_sv.addBookmark(MOOST_RECURRING_VERBOSE_BOOKMARK_SEP);
      }

      ~recurrurring_bookmark()
      {
         m_sv.addBookmark(m_str);
      }

   private:
      scoped_verbose& m_sv;
      std::string m_str;
   };


public:

   scoped_verbose(const std::string& header, eVerboseLevel vl = VL_WARNING_PRIORITY, std::ostream& out = std::cout)
      : m_header(header), m_vl(vl), m_out(out), m_startTime(boost::posix_time::microsec_clock::local_time()),
        m_currLevel(vl)
   { setTimingsHeader(); }

   scoped_verbose(const std::string& header, moost::timer::scoped_time& sc, eVerboseLevel vl = VL_WARNING_PRIORITY, std::ostream& out = std::cout)
      : m_header(header), m_vl(vl), m_out(out), m_startTime(sc.get_time()), m_currLevel(vl)
   { setTimingsHeader(); }

   scoped_verbose(const std::string& header, moost::multi_timer::scoped_time& sc, eVerboseLevel vl = VL_WARNING_PRIORITY, std::ostream& out = std::cout)
      : m_header(header), m_vl(vl), m_out(out), m_startTime(sc.get_time()), m_currLevel(vl)
   { setTimingsHeader(); }

   scoped_verbose(const std::string& header, const boost::posix_time::ptime& time, eVerboseLevel vl = VL_WARNING_PRIORITY, std::ostream& out = std::cout)
      : m_header(header), m_vl(vl), m_out(out), m_startTime(time), m_currLevel(vl)
   { setTimingsHeader(); }

   /**
   * The destructor. It will print the buffer with a timestamp.
   */
   ~scoped_verbose()
   {
      try
      {

         std::ostringstream concatenatedBuf;
         if ( !m_entries.empty() )
         {
            std::vector< std::pair<eVerboseLevel, std::string> >::const_iterator eIt;
            for ( eIt = m_entries.begin(); eIt != m_entries.end(); ++eIt )
            {
               if ( eIt->first <= m_vl )
                  concatenatedBuf << eIt->second;
            }
         }

         if ( m_currLevel <= m_vl )
            concatenatedBuf << m_buf.str();

         long pos = static_cast<long>(concatenatedBuf.tellp());
         if ( pos <= 0 && m_vl != VL_EVERYTHING )
            return; // nothing to output

         using namespace boost::posix_time;
         std::ostringstream oss;
         const time_duration& td = m_startTime.time_of_day();
         char fill_char = '0';
         oss << '['
            << boost::gregorian::to_simple_string_type<char>(m_startTime.date()) << ' '
            << std::setw(2) << std::setfill(fill_char)
            << boost::date_time::absolute_value(td.hours()) << ":"
            << std::setw(2) << std::setfill(fill_char)
            << boost::date_time::absolute_value(td.minutes()) << ":"
            << std::setw(2) << std::setfill(fill_char)
            << boost::date_time::absolute_value(td.seconds())
            << '|' << m_header << "] "
            << concatenatedBuf.str();

         if ( m_vl == VL_EVERYTHING )
         {
            std::vector< std::pair<ptime, std::string> >::const_iterator tIt;
            std::map<std::string, std::pair<double, int> > recurringMaps;

            ptime lastTime = m_startTime;
            oss << '\n';

            std::string scopedSep(MOOST_SCOPED_VERBOSE_BOOKMARK_SEP);
            std::string recurringSep(MOOST_RECURRING_VERBOSE_BOOKMARK_SEP);

            for ( tIt = m_timeBookmarks.begin(); tIt != m_timeBookmarks.end(); ++tIt )
            {
               if ( tIt->second == recurringSep )
               {
                  // that's a recurring bookmark: add the difference to the map
                  // we will print it at theend
                  lastTime = tIt->first;
                  ++tIt;
                  if ( tIt == m_timeBookmarks.end() )
                  {
                     assert(!"Somehow we have only one bit of a recurring verbose!");
                     break; // should never happen
                  }

                  std::pair<double, int>& recurringEntry = recurringMaps[tIt->second];
                  recurringEntry.first += (tIt->first - lastTime).total_milliseconds();
                  ++recurringEntry.second;
               }
               else if ( tIt->second != scopedSep ) // if it's not a bookmark separator
               {
                  // standard entry (or the previous was bookmark)
                  oss << m_subTimingsIndentation << tIt->second << ": " << (tIt->first - lastTime).total_milliseconds() << "ms\n";
               }

               lastTime = tIt->first;
            }

            if ( !recurringMaps.empty() )
            {
               std::map<std::string, std::pair<double, int> >::const_iterator rmIt;
               for ( rmIt = recurringMaps.begin(); rmIt != recurringMaps.end(); ++rmIt )
               {
                  const std::pair<double, int>& recurringEntry = rmIt->second;
                  oss << m_subTimingsIndentation << rmIt->first << ": " << static_cast<int>( recurringEntry.first / recurringEntry.second + 0.5) << "ms\n";
               }

            }

            ptime now = microsec_clock::local_time();
            oss << m_totTimingsHeader << (now - m_startTime).total_milliseconds() << "ms";
         }

         oss << '\n';
         m_out << oss.str() << std::flush;
      }
      catch (...)
      { }
   }

   /**
   * Add an error message to the string buffer. This will be printed no matter what.
   * \param str The string to be added
   * \param addErroHeader if true it will add "ERROR: " before the message
   * \param addNewLine if true it will set a new line before adding the message to the buffer
   */
   template <typename T>
   void addError(const T& str, bool addErrorHeader = false, bool addNewLine = true)
   {
      if ( addNewLine )
         addToStream('\n', VL_HIGH_PRIORITY);
      if ( addErrorHeader )
         addToStream(moost::terminal_format::getError() + ": ", VL_HIGH_PRIORITY);

      addToStream(str, VL_HIGH_PRIORITY);
   }

   /**
   * Add a warning message to the string buffer. This will be printed if verbose level
   * is set to VL_WARNING_PRIORITY or higher.
   * \param str The string to be added
   * \param addWarningHeader if true it will add "WARNING: " before the message
   * \param addNewLine if true it will set a new line before adding the message to the buffer
   */
   template <typename T>
   void addWarning(const T& str, bool addWarningHeader = false, bool addNewLine = true)
   {
      if ( m_vl >= VL_WARNING_PRIORITY )
      {
         if ( addNewLine )
            addToStream('\n', VL_WARNING_PRIORITY);
         if ( addWarningHeader )
            addToStream(moost::terminal_format::getWarning() + ": ", VL_WARNING_PRIORITY);

         addToStream(str, VL_WARNING_PRIORITY);
      }
   }

   /**
   * The standard way of adding a string to scoped_verbose is via the << operator.
   * Just use it the same way as cout.
   * \param the string to be stored in the stream buffer.
   */
   template <typename T>
   scoped_verbose& operator<<(const T& str)
   {
      if ( m_vl >= VL_LOW_PRIORITY )
         addToStream(str, VL_LOW_PRIORITY);
      return *(this);
   }

   template <typename T>
   void addTrivial(const T& str)
   {
      if ( m_vl >= VL_EVERYTHING )
         addToStream(str, VL_EVERYTHING);
   }

   template <typename T>
   void add(const T& str, eVerboseLevel lvl)
   {
      if ( m_vl >= lvl )
         addToStream(str, VL_LOW_PRIORITY);
   }

   scoped_verbose& flush()
   { return *this; } // do nothing

   scoped_verbose & operator << (scoped_verbose & (*modifier)(scoped_verbose &))
   { modifier(*this); return *this; };

   /**
   * Set the header that will be printed when going out of scope. Will override the
   * one set on construction.
   */
   void setHeader(const std::string& header)
   { m_header = header; }

   /**
   * Return a string with a description of the passed verbosity level
   * \param lvl the passed verbosity level
   */
   static std::string stringize(eVerboseLevel lvl)
   {
      switch (lvl)
      {
      case VL_LOW_PRIORITY:
         return "LOW PRIORITY";
      case VL_WARNING_PRIORITY:
         return "WARNING PRIORITY";
      case VL_HIGH_PRIORITY:
         return "HIGH PRIORITY";
      case VL_EVERYTHING:
         return "EVERYTHING";
      }
      return "UNKNOWN";
   }

   /**
   * Return a string with the description of the current verbosity level.
   */
   std::string stringize()
   {
      return stringize(m_vl);
   }

   /**
   * \brief Add a time bookmark.
   * If verbose is set to everything it will take the bookmark and display it
   * in order making a diff from the previous one (with the first one
   * using the time in the constructor).
   * This allows an easy way of timing differents parts of the code.
   * i.e.
   * \code
   void myFunction()
   {
      moost::scoped_verbose v("myFunction");
      // do something expensive
      v.addBookmark("expensive_op1");
      // do something else expensive
      v.addBookmark("expensive_op2");
      // finishing up..
   }
   \endcode
   will result in:
   \verbatim
[2009-Jun-18 16:38:09 | myFunction]
   expensive_op1: 12ms
   expensive_op2: 53ms
   total: 68ms
\endverbatim
   \param str the name of the expensive op
   \note this will work only and only if verbose is set to VL_EVERYTHING
   \see ~scoped_verbose
   */
   void addBookmark(const std::string& str )
   {
      if ( m_vl < VL_EVERYTHING )
         return; // ignore unless we're verbosing everything!

      m_timeBookmarks.push_back(
         make_pair( boost::posix_time::microsec_clock::local_time(),
                    str ) )
      ;
   }

   /**
   * Change the level of verbosity.
   * \param vl the new verbosity level.
   */
   void changeVerboseLevel(eVerboseLevel vl = VL_EVERYTHING)
   { m_vl = vl; }

   /**
   * Returns the current verbose level
   */
   eVerboseLevel getVerboseLevel() const
   { return m_vl; }

   void setTimingsHeader(const std::string& totTimingsHeader = "  Overall time: ", const std::string& subTimingsIndentation = "    -> ")
   {
      m_totTimingsHeader = totTimingsHeader;
      m_subTimingsIndentation = subTimingsIndentation;
   }

protected:

   template <typename T>
   void addToStream(const T& str, eVerboseLevel toLevel)
   {
      if ( toLevel != m_currLevel )
      {
         m_entries.push_back( make_pair(m_currLevel, m_buf.str()) );

         m_buf.str(""); // reset
         m_currLevel = toLevel;
      }

      m_buf << str;
      //*((std::ostringstream*)this) << str;
   }

   std::string   m_header; //< header of the message

   eVerboseLevel m_vl; //< the verbose level
   std::ostream& m_out; //< where it will be eventually output

   std::ostringstream m_buf;

   std::vector< std::pair<eVerboseLevel, std::string> > m_entries;

   boost::posix_time::ptime m_startTime; //< when it started

   std::string m_totTimingsHeader; //< the indentation string used while output the total timing
   std::string m_subTimingsIndentation; //< the indentation string used while output bookmarked timings

   std::vector< std::pair<boost::posix_time::ptime, std::string> > m_timeBookmarks; //< the time bookmarks

   eVerboseLevel m_currLevel;
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define LEVEL_SCOPED_VERBOSE( level, lvl_enum )         \
class level##_scoped_verbose                            \
{                                                       \
public:                                                 \
   level##_scoped_verbose(scoped_verbose& sv)           \
   : m_sv(sv) {}                                        \
                                                        \
   template <typename T>                                \
   level##_scoped_verbose& operator<<(const T& str)     \
   {                                                    \
      m_sv.add(str, lvl_enum);                          \
      return *(this);                                   \
   }                                                    \
                                                        \
private:                                                \
                                                        \
   scoped_verbose& m_sv;                                \
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

LEVEL_SCOPED_VERBOSE( error, scoped_verbose::VL_HIGH_PRIORITY )
LEVEL_SCOPED_VERBOSE( warning, scoped_verbose::VL_WARNING_PRIORITY )
LEVEL_SCOPED_VERBOSE( trivial, scoped_verbose::VL_EVERYTHING )

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// implementation of the free functions
namespace std
{
   // free functions
   moost::scoped_verbose& endl(moost::scoped_verbose& sv)
   { return sv << '\n'; } // no flush

   // free functions
   moost::scoped_verbose& flush(moost::scoped_verbose& sv)
   { return sv; } // do nothing
}


#endif // #define MOOST_SCOPED_VERBOSE_H
