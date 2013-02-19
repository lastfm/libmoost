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

/*!
 * This is NOT a replacement for moost::timer!
 * This is a very simple collection of classes to start, stop and pause a
 * stopwatch so as to get very simple wall-clock metrics. It is not all
 * singing or dancing - it just does what it says on the tin.
 */

#ifndef MOOST_UTILS_STOPWATCH_HPP__
#define MOOST_UTILS_STOPWATCH_HPP__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

namespace moost { namespace utils {

   /*!
    * A simple stopwatch to monitor elapsed wallclock time to microsecond granularity
    */

   class stopwatch : boost::noncopyable
   {
   public:

      typedef boost::int64_t elapsed_t;

      stopwatch()
      {
         restart();
      }

      void restart()
      {
         start_ = boost::posix_time::microsec_clock::universal_time();
      }

      elapsed_t elapsed_ns() const // nanosecs
      {
         boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
         return (now - start_).total_nanoseconds();
      }

      elapsed_t elapsed_us() const // microsecs (default)
      {
         boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
         return (now - start_).total_microseconds();
      }

      elapsed_t elapsed_ms() const // millisecs
      {
         boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
         return (now - start_).total_milliseconds();
      }

      elapsed_t elapsed_secs() const // secs
      {
         boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
         return (now - start_).total_seconds();
      }

      elapsed_t elapsed() const
      {
         return elapsed_us();
      }

   private:
      boost::posix_time::ptime start_;
   };

   /*!
    * A scoped stop watch that will return the elapsed time in a future variable.
    */

   class scoped_stopwatch : boost::noncopyable
   {
   public:
      enum granularity
      {
         nanosecs, microsecs, millisecs, secs
      };

      typedef stopwatch::elapsed_t elapsed_t;

      scoped_stopwatch(
         elapsed_t & future, // This is a future, which will be assigned elapsed on destruction
         granularity const g = microsecs, // This is the granularity of elapsed (default is microsecs)
         bool const a = false // If true add elapsed to future else assign elapsed to future
         )
         : future_(future), granularity_(g), accumulate_(a)
      {
      }

      ~scoped_stopwatch()
      {
         elapsed_t e;

         switch(granularity_)
         {
         case secs:
            e = sw_.elapsed_secs();
            break;
         case millisecs:
            e = sw_.elapsed_ms();
            break;
         case microsecs:
            e = sw_.elapsed_us();
            break;
         case nanosecs:
            e = sw_.elapsed_ns();
            break;
         default:
            e = sw_.elapsed();
            break;
         }

         future_ = e + (accumulate_ ? future_ : 0);
      }

   private:
      stopwatch sw_;
      elapsed_t & future_;
      granularity const granularity_;
      bool const accumulate_;
   };

}}

#endif // MOOST_UTILS_STOPWATCH_HPP__
