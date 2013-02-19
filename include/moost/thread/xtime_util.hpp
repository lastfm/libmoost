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

#ifndef MOOST_THREAD_XTIME_UTIL_HPP__
#define MOOST_THREAD_XTIME_UTIL_HPP__

#include <boost/thread/xtime.hpp>

namespace moost { namespace thread {

/// @brief Some xtime utils to hide ugly operations.
struct xtime_util
{
  static const int NSECS_PER_SEC = 1000000000;
  static const int NSECS_PER_MILLISEC = 1000000;


  /// @brief Returns now in boost::xtime.
  static boost::xtime now()
  {
    boost::xtime ret_val;
    xtime_get(&ret_val, boost::TIME_UTC);

    return ret_val;
  }

  /// @brief Adds the specified number of milliseconds to a boost::xtime.
  static boost::xtime add_ms(const boost::xtime & time, int milliseconds)
  {
    boost::xtime ret_val;

    if ( milliseconds < NSECS_PER_SEC / NSECS_PER_MILLISEC )
    {
       ret_val.sec  = time.sec;
       ret_val.nsec = time.nsec + milliseconds * NSECS_PER_MILLISEC;
    }
    else
    {
       ret_val.sec  = time.sec + milliseconds / 1000;
       ret_val.nsec = time.nsec + milliseconds % NSECS_PER_SEC;
    }

    if (ret_val.nsec >= NSECS_PER_SEC)
    {
      ret_val.sec += ret_val.nsec / NSECS_PER_SEC;
      ret_val.nsec = ret_val.nsec % NSECS_PER_SEC;
    }

    return ret_val;
  }

  /// @brief Adds the specified number of seconds to a boost::xtime.
  static boost::xtime add_sec(const boost::xtime & time, int seconds)
  {
    boost::xtime ret_val;

    ret_val.sec = time.sec + seconds;
    ret_val.nsec = time.nsec;

    return ret_val;
  }
};

}} // moost::thread

#endif // MOOST_THREAD_XTIME_UTIL_HPP__
