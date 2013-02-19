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

#include "../include/moost/timer.h"
#include <boost/cstdint.hpp>

using boost::int64_t;
using namespace moost;

timer::scoped_time::scoped_time(timer & timer_)
: m_timer(timer_),
m_stopped(false),
m_time(boost::posix_time::microsec_clock::local_time())
{
}

timer::scoped_time::~scoped_time()
{
   stop();
}

void timer::scoped_time::stop()
{
   if (m_stopped)
      return;
   m_stopped = true;
   m_timer.time(m_time);
}

boost::posix_time::ptime timer::scoped_time::get_time() const
{
   return m_time;
}

timer::timer(size_t resolution /* = 4096 */, int max_threshold_time_ms, size_t threshold_resolution)
: m_max_threshold_time_ms(max_threshold_time_ms)
{
   if (resolution <= 0)
      throw std::runtime_error("resolution must be > 0");
   m_times.resize(resolution);

   if ( max_threshold_time_ms < (std::numeric_limits<int>::max)() )
      m_threshold_times.resize(threshold_resolution);

   reset();
}

void timer::time(const boost::posix_time::ptime & start)
{
   boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
   int total_ms = static_cast<int>((now - start).total_milliseconds());

   boost::mutex::scoped_lock lock(m_mutex);

   // bounds-check the pointers
   if (m_times_p == m_times_end)
   {
      if (m_times_end == m_times.end())
         m_times_p = m_times.begin();
      else
         ++m_times_end;
   }

   *m_times_p++ = total_ms;

   if (m_min_time == -1 || total_ms < m_min_time)
      m_min_time = total_ms;
   if (total_ms > m_max_time)
      m_max_time = total_ms;

   // keep track of the entries that pass the threshold?
   if ( total_ms > m_max_threshold_time_ms )
   {
      if ( m_threshold_times_p == m_threshold_times_end )
      {
         if ( m_threshold_times_end == m_threshold_times.end() )
            m_threshold_times_p = m_threshold_times.begin();
         else
            ++m_threshold_times_end;
      }

      *m_threshold_times_p++ = std::make_pair(total_ms, now);
   }

   ++m_count;
}

int timer::min_time() const
{
   boost::mutex::scoped_lock lock(m_mutex);
   return m_min_time;
}

float timer::avg_time() const
{
   float total = 0;
   boost::mutex::scoped_lock lock(m_mutex);
   if (m_times.begin() == m_times_end)
      return -1.0F;
   for (std::vector<int>::const_iterator it = m_times.begin(); it != m_times_end; ++it)
      total += *it;
   return total / static_cast<float>(m_times_end - m_times.begin());
}

void timer::avg_stddev_time( float& avg, float& std_dev ) const
{
   avg = this->avg_time();
   std_dev = -1;
   if ( avg < 0 )
      return;

   std_dev = 0;
   {
      boost::mutex::scoped_lock lock(m_mutex);
      for (std::vector<int>::const_iterator it = m_times.begin(); it != m_times_end; ++it)
         std_dev += (*it - avg)*(*it - avg);
   }

   std_dev = sqrt( std_dev /
      (m_times_end - m_times.begin())
      );
}

int timer::median_time() const
{
   boost::mutex::scoped_lock lock(m_mutex);

   std::vector<int> m_times_copy(m_times.begin(), std::vector<int>::const_iterator(m_times_end));
   std::vector<int>::iterator it_median = m_times_copy.begin() + (m_times_copy.size() / 2);
   std::nth_element(m_times_copy.begin(), it_median, m_times_copy.end());

   if (it_median == m_times_copy.end())
      return -1;
   else
      return *it_median;
}

int timer::max_time() const
{
   boost::mutex::scoped_lock lock(m_mutex);
   return m_max_time;
}

double timer::count_per_second() const
{
   boost::mutex::scoped_lock lock(m_mutex);

   boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
   int64_t total_ms = (now - m_start_time).total_milliseconds();

   if (total_ms == 0)
      return 0;

   return (m_count * 1000.0) / total_ms;
}

size_t timer::count() const
{
   boost::mutex::scoped_lock lock(m_mutex);
   return m_count;
}

void timer::reset()
{
   boost::mutex::scoped_lock lock(m_mutex);

   m_times_p   = m_times.begin();
   m_times_end = m_times.begin();
   m_count = 0;
   m_min_time = -1;
   m_max_time = -1;

   m_threshold_times_p = m_threshold_times.begin();
   m_threshold_times_end = m_threshold_times.begin();

   m_start_time = boost::posix_time::microsec_clock::local_time();
}

moost::timer::threshold_times_type moost::timer::past_threshold_times( int num ) const/* will return the last num entries */
{
   threshold_times_type ret;
   boost::mutex::scoped_lock lock(m_mutex);

   if ( m_threshold_times_end == m_threshold_times.begin() )
      return ret;

   threshold_times_type::const_iterator it = m_threshold_times_p;
   int i = 0;
   --it;
   for ( ; it != m_threshold_times.begin() && i < num; --it, ++i )
      ret.push_back(*it);

   if ( i < num && it == m_threshold_times.begin() )
   {
      ++i;
      ret.push_back( *m_threshold_times.begin() );
   }

   // roll over?
   if ( i < num && m_threshold_times_end == m_threshold_times.end() )
   {
      it = m_threshold_times.end();
      --it;
      for ( ; it != m_threshold_times_p && i < num; --it, ++i )
         ret.push_back(*it);
      if ( i < num && it == m_threshold_times_p )
         ret.push_back( *it );
   }

   return ret;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

multi_timer::reassignable_scoped_time::reassignable_scoped_time(multi_timer & mt, const std::string & name, int max_threshold_time_ms)
: m_multi_timer(mt),
  m_stopped(false),
  m_time(boost::posix_time::microsec_clock::local_time()),
  m_max_threshold_time_ms(max_threshold_time_ms)
{
   boost::mutex::scoped_lock lock(m_multi_timer.m_mutex);
   multi_timer::iterator it = m_multi_timer.m_timers.find(name);

   if ( it == m_multi_timer.m_timers.end() )
   {
      m_timer.reset(new timer(m_multi_timer.m_resolution, max_threshold_time_ms));
      m_multi_timer.m_timers[name] = m_timer;
   }
   else
      m_timer = it->second;
}

multi_timer::reassignable_scoped_time::~reassignable_scoped_time()
{
   stop();
}

void multi_timer::reassignable_scoped_time::stop()
{
   if (m_stopped)
      return;
   m_stopped = true;

   boost::mutex::scoped_lock lock(m_reassign_mutex);
   if ( m_timer )
      m_timer->time(m_time);
}

void multi_timer::reassignable_scoped_time::reassign(const std::string& name)
{
   boost::mutex::scoped_lock lock(m_multi_timer.m_mutex);
   multi_timer::iterator it = m_multi_timer.m_timers.find(name);
   if ( it == m_multi_timer.m_timers.end() )
   {
      boost::mutex::scoped_lock lock(m_reassign_mutex);
      m_timer.reset(new timer(m_multi_timer.m_resolution, m_max_threshold_time_ms));
      m_multi_timer.m_timers[name] = m_timer;
   }
   else
   {
      boost::mutex::scoped_lock lock(m_reassign_mutex);
      m_timer = it->second;
   }
}

void multi_timer::reassignable_scoped_time::discard()
{
   boost::mutex::scoped_lock lock(m_reassign_mutex);
   m_timer.reset();
}

boost::posix_time::ptime multi_timer::reassignable_scoped_time::get_time() const
{
   return m_time;
}
