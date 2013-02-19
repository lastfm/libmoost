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

#ifndef MOOST_POSIX_TIME_TIMESTAMP_HPP__
#define MOOST_POSIX_TIME_TIMESTAMP_HPP__

#include <string>
#include <sstream>

#include <boost/regex.hpp>
#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/operators.hpp>

namespace moost { namespace posix_time {

class universal_timebase
{
public:
   static boost::posix_time::ptime now()
   {
      return boost::posix_time::second_clock::universal_time();
   }

   static boost::posix_time::ptime base()
   {
      return boost::posix_time::ptime(boost::gregorian::date(1970,1,1));
   }
};

template <class TimeBasePolicy>
class basic_timestamp : private boost::less_than_comparable< basic_timestamp<TimeBasePolicy> >
                              , boost::less_than_comparable< basic_timestamp<TimeBasePolicy>, boost::posix_time::ptime >
                              , boost::less_than_comparable< basic_timestamp<TimeBasePolicy>, time_t >
                              , boost::equality_comparable< basic_timestamp<TimeBasePolicy> >
                              , boost::equality_comparable< basic_timestamp<TimeBasePolicy>, boost::posix_time::ptime >
                              , boost::equality_comparable< basic_timestamp<TimeBasePolicy>, time_t >
{
public:
   basic_timestamp()
   {
   }

   explicit basic_timestamp(const boost::posix_time::ptime& t)
      : m_time(t)
   {
   }

   explicit basic_timestamp(time_t t)
   {
      assign(t);
   }

   basic_timestamp(const std::string& str)
   {
      assign(str);
   }

   void assign(time_t t)
   {
      m_time = boost::posix_time::from_time_t(t);
   }

   void assign(const boost::posix_time::ptime& t)
   {
      m_time = t;
   }

   void assign(const std::string& str)
   {
      if (boost::regex_match(str, boost::regex("\\d+")))
      {
         std::istringstream iss(str);
         time_t t;
         iss >> t;
         assign(t);
      }
      else
      {
         boost::smatch what;

         if (boost::regex_match(str, what, boost::regex("([+-]?)(\\d+)\\s*(h(?:ours?)?|d(?:ays?)?|w(?:eeks?)?|m(?:onths?)?|y(?:ears?)?)")))
         {
            std::istringstream iss(what[2]);
            int h;
            iss >> h;

            switch (what.str(3).at(0))
            {
               case 'y': h *= 24*365; break;
               case 'm': h *= 24*30;  break;
               case 'w': h *= 24*7;   break;
               case 'd': h *= 24;     break;
               default:
                  break;
            }

            m_time = TimeBasePolicy::now();

            if (what[1] == "-")
            {
               m_time -= boost::posix_time::hours(h);
            }
            else
            {
               m_time += boost::posix_time::hours(h);
            }
         }
         else
         {
            try
            {
               m_time = boost::posix_time::from_iso_string(str);
            }
            catch (const std::bad_cast&)
            {
               m_time = boost::posix_time::time_from_string(str);
            }
         }
      }
   }

   time_t as_time_t() const
   {
      return static_cast<time_t>((m_time - TimeBasePolicy::base()).total_seconds());
   }

   std::string as_iso_string() const
   {
      return boost::posix_time::to_iso_string(m_time);
   }

   const boost::posix_time::ptime& as_ptime() const
   {
      return m_time;
   }

   void operator=(time_t t)
   {
      assign(t);
   }

   void operator=(const boost::posix_time::ptime& t)
   {
      assign(t);
   }

   void operator=(const std::string& str)
   {
      assign(str);
   }

   bool operator==(const basic_timestamp& ts) const { return m_time == ts.m_time; }
   bool operator<(const basic_timestamp& ts) const { return m_time < ts.m_time; }

   bool operator==(const boost::posix_time::ptime& t) const { return m_time == t; }
   bool operator<(const boost::posix_time::ptime& t) const { return m_time < t; }
   bool operator>(const boost::posix_time::ptime& t) const { return m_time > t; }

   bool operator==(time_t t) const { return m_time == boost::posix_time::from_time_t(t); }
   bool operator<(time_t t) const { return m_time < boost::posix_time::from_time_t(t); }
   bool operator>(time_t t) const { return m_time > boost::posix_time::from_time_t(t); }

private:
   boost::posix_time::ptime m_time;
};

typedef basic_timestamp<universal_timebase> timestamp;

}}

#endif
