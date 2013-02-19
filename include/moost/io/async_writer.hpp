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

#ifndef MOOST_IO_ASYNCWRITER_HPP__
#define MOOST_IO_ASYNCWRITER_HPP__

#include <string>
#include <fstream>
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "../thread/async_worker.hpp"

namespace moost { namespace io {

/** rolls over after a specified number of work items have been written
 */
class count_rollover
{
private:

  size_t m_rollover;
  size_t m_count;

public:

  /** constructs a count_rollover
  * @param rollover how many items written before we create a new file (0 means never roll over)
  */
  count_rollover(size_t rollover = 0)
  : m_rollover(rollover),
    m_count(0)
  {

  }

  /// returns true if it's time to roll over
  bool operator()()
  {
    return (m_rollover != 0 && (m_count++ % m_rollover == 0));
  }

  // returns an pathname related to the type of rollover
  std::string get_path(const std::string & base_path)
  {
    if (m_rollover == 0)
      return base_path;
    std::ostringstream oss;
    oss << base_path << '.' << m_count;
    return oss.str();
  }
};

/** rolls over once at a specified time of day
 */
class timeofday_rollover
{
private:

  boost::posix_time::time_duration m_rollover_timeofday;
  boost::posix_time::ptime         m_next_rollover;

public:

  /** constructs a timeofday_rollover
   * @param hour the hour of the day to roll over
   * @param minute the minute of the day to roll over
   * @param second the second of the day to roll over
   */
  timeofday_rollover(int hour = 0, int minute = 0, int second = 0)
  : m_rollover_timeofday(hour, minute, second, 0)
  {
    boost::posix_time::ptime now(boost::posix_time::second_clock::universal_time());
    if ( now.time_of_day() < m_rollover_timeofday )
      m_next_rollover = boost::posix_time::ptime(now.date(), m_rollover_timeofday);
    else
      m_next_rollover = boost::posix_time::ptime((now + boost::gregorian::days(1)).date(), m_rollover_timeofday);
  }

  /// returns true if it's time to roll over
  bool operator()()
  {
    boost::posix_time::ptime now(boost::posix_time::second_clock::universal_time());
    if (now >= m_next_rollover)
    {
      m_next_rollover = boost::posix_time::ptime((m_next_rollover + boost::gregorian::days(1)).date(), m_rollover_timeofday);
      return true;
    }
    return false;
  }

  // returns an pathname related to the type of rollover
  std::string get_path(const std::string & base_path, boost::gregorian::date * path_date = NULL)
  {
    boost::gregorian::date now;
    if (path_date == NULL)
    {
      now = (boost::posix_time::second_clock::universal_time()).date();
      path_date = &now;
    }
    std::ostringstream oss;
    oss << base_path << '.' << boost::gregorian::to_iso_extended_string(*path_date);
    return oss.str();
  }
};

/** @brief async_writer is a class that simplifies the mechanics of writing to a file asynchronously.
 *
 * To use it, you must provide a template parameter, and the template type must implement the method:
 * write(std::ostream & out)
 */
template<typename TWork, class TRolloverPolicy = count_rollover>
class async_writer : public moost::thread::async_worker<TWork>
{
private:

  std::string     m_base_path;
  TRolloverPolicy m_rollover_policy;
  std::ofstream   m_out;

  /// @brief open a new file to write
  void reload_out()
  {
    // get a suitable filename
    boost::filesystem::path p;
    std::string path_name = m_rollover_policy.get_path(m_base_path);

    for (int i = 0; ;++i)
    {
      std::ostringstream oss;
      oss << path_name;
      if (i != 0)
        oss << '.' << i;
      p = boost::filesystem::path(oss.str());
      if (!boost::filesystem::exists(p))
        break;
    }

    m_out.close();
    m_out.clear();
    m_out.open(p.string().c_str(), std::ios::binary);
  }

protected:

  void do_work(TWork & work)
  {
    if (m_rollover_policy() || !m_out.is_open())
      reload_out();
    work.write(m_out);
  }

public:

  /** @brief Constructs an async_writer.
   * @param base_path the base path name for file creation (unix time is appended)

   * @param max_queue the maximum length the queue may grow before further enqueue's begin to wait (default don't wait)
   * @param enqueue_timeout the longest amount of time an enqueue may wait (-1 means wait forever)
   */
  async_writer(const std::string & base_path,
               const TRolloverPolicy & rollover_policy = TRolloverPolicy(),
               size_t max_queue = 0,
               size_t enqueue_timeout = 0)
  : moost::thread::async_worker<TWork>(1, max_queue, enqueue_timeout),
    m_base_path(base_path),
    m_rollover_policy(rollover_policy)
  {
  }

  /** Destroys the async_writer
   *
   * It's safe to destroy the async_writer without first calling stop().  It will shut down cleanly.
   */
  ~async_writer()
  {
    stop();
  }

  void stop()
  {
    moost::thread::async_worker<TWork>::stop();
    m_out.close();
  }

};

}} // moost::io

#endif // MOOST_IO_ASYNCWRITER_HPP__
