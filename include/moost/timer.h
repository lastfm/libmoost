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

#ifndef MOOST_TIMER_H__
#define MOOST_TIMER_H__

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace moost {

/** timer collects statistics on how many times start()/stop() was called in a second,
* how many milliseconds elapsed on average between a start()/stop(), and best and worst times
*/
class timer
{
public:

   typedef std::vector< std::pair<int, boost::posix_time::ptime> > threshold_times_type;

private:

   boost::mutex mutable       m_mutex;
   std::vector<int>           m_times;
   std::vector<int>::iterator m_times_p;
   std::vector<int>::iterator m_times_end;
   int                        m_min_time;
   int                        m_max_time;
   size_t                     m_count;

   boost::posix_time::ptime   m_start_time;

   int                        m_max_threshold_time_ms;


   threshold_times_type           m_threshold_times;
   threshold_times_type::iterator m_threshold_times_p;
   threshold_times_type::iterator m_threshold_times_end;

public:

  /// used for scoped access to timer
  class scoped_time
  {
  private:
    timer &                  m_timer;
    bool                     m_stopped;
    boost::posix_time::ptime m_time;
  public:
    scoped_time(timer & timer_);
    ~scoped_time();
    void stop();
    boost::posix_time::ptime get_time() const;
  };

  /** Constructs a timer
  * @param resolution how many values to store for calculating the average
  */
  timer(size_t resolution = 4096, int max_threshold_time_ms = (std::numeric_limits<int>::max)(), size_t threshold_resolution = 128);

  /// add a time to the timer
  void time(const boost::posix_time::ptime & start);

  /// get the minimum time recorded
  int min_time() const;

  /// get the average time recorded
  float avg_time() const;

  /// get the average time and standard deviation
  void avg_stddev_time(float& avg, float& std_dev) const;

  /// get the median time
  int median_time() const;

  /// get the maximum time recorded
  int max_time() const;

  /// get the average number of times per second that times were recorded
  double count_per_second() const;

  /// get the count
  size_t count() const;

  int get_threshold_time() const
  { return m_max_threshold_time_ms; }

  /// get all times
  template<typename ForwardIterator>
  void all_times(ForwardIterator out) const
  {
     boost::mutex::scoped_lock lock(m_mutex);
     for (std::vector<int>::const_iterator it = m_times.begin(); it != m_times_end; ++it)
        *out++ = *it;
  }

  threshold_times_type past_threshold_times(int num) const; // will return the last num entries;

  /// reset timing statistics
  void reset();
};

/** multi_timer provides a thread-safe collection of timers indexed by name
 * just a little syntactic confectionary
 */
class multi_timer
{
private:

  size_t m_resolution;

  boost::mutex m_mutex;
  // map is nice in that inserting doesn't invalidate any other iterators
  // we'll take advantage of that by passing timers by ref to their scoped locks
  std::map< std::string, boost::shared_ptr< timer > > m_timers;

public:

  multi_timer(size_t resolution = 4096) : m_resolution(resolution) {}

  typedef std::map< std::string, boost::shared_ptr< timer > >::iterator iterator;
  typedef std::map< std::string, boost::shared_ptr< timer > >::const_iterator const_iterator;

  class scoped_time : public timer::scoped_time
  {
  public:
    scoped_time( multi_timer & mt,
                 const std::string & name,
                 int max_threshold_time_ms = (std::numeric_limits<int>::max)() )
      : timer::scoped_time( mt(name, max_threshold_time_ms) ) {}
  };

  class reassignable_scoped_time
  {
  public:
     reassignable_scoped_time( multi_timer & mt, const std::string & name, int max_threshold_time_ms = (std::numeric_limits<int>::max)());
     ~reassignable_scoped_time();

     void reassign(const std::string& name);
     void stop();
     void discard();
     boost::posix_time::ptime get_time() const;

  private:

     multi_timer&             m_multi_timer;
     boost::shared_ptr<timer> m_timer;
     bool                     m_stopped;
     boost::posix_time::ptime m_time;
     boost::mutex             m_reassign_mutex;
     int                      m_max_threshold_time_ms;
  };


  timer & operator[](const std::string & name)
  {
    boost::mutex::scoped_lock lock(m_mutex);
    boost::shared_ptr< timer > & ptimer = m_timers[name];
    if (!ptimer)
      ptimer.reset(new timer(m_resolution));
    return *ptimer;
  }

  timer & operator()(const std::string & name, int max_threshold_time_ms)
  {
     boost::mutex::scoped_lock lock(m_mutex);
     boost::shared_ptr< timer > & ptimer = m_timers[name];
     if (!ptimer)
        ptimer.reset(new timer(m_resolution, max_threshold_time_ms));
     return *ptimer;
  }

  // grab this before iterating over the collection
  boost::mutex & mutex() { return m_mutex; }

  // iterators:
  iterator begin() { return m_timers.begin(); }
  iterator end() { return m_timers.end(); }
  const_iterator begin() const { return m_timers.begin(); }
  const_iterator end() const { return m_timers.end(); }
};

} // moost

#endif // MOOST_TIMER_H__
