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

#ifndef MOOST_UTILS_BENCHMARK_HPP__
#define MOOST_UTILS_BENCHMARK_HPP__

/**
 * \file benchmark.hpp
 *
 * An easy to use benchmarking statistics class. This isn't yet very
 * sophisticated, but quite capable of producing a nice benchmark
 * statistics for (multithreaded) services with very little code.
 *
 * Usually, you just need to create a benchmark object

\code
moost::utils::benchmark bm("harold", num_threads);
\endcode

 * and then pass a reference to this object to each thread running
 * requests against a service. Within each thread, you can then
 * create a timer object for each request:

\code
moost::utils::benchmark::timer t(bm);
\endcode

 * If the timer object goes out of scope, it will automatically
 * stop the timer and add the elapsed time to the set of times for
 * the default result specified in the constructor (unless explicitly
 * provided, this will be "error"). You can explicity stop the timer
 * with a different result by calling the stop() method on the timer
 * object. The default result when calling stop() explicitly is
 * "success". You're free to use more results that those two.
 *
 * After all threads have been joined, simply call the output()
 * method of the benchmark object:

\code
bm.output(std::cout);
\endcode

 * This will provide you with histograms for each result type as
 * well as a combined histogram and statistical analysis of the
 * distribution of request times.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "foreach.hpp"
#include "histogram.hpp"

namespace moost { namespace utils {

class benchmark
{
private:
   typedef std::map< std::string, std::vector<float> > timings_type;

public:
   class timer
   {
   public:
      timer(benchmark& bm, const std::string& default_result = "error")
         : m_bm(bm)
         , m_default_result(default_result)
      {
         restart();
      }

      ~timer()
      {
         try
         {
            stop(m_default_result);
         }
         catch (...)
         {
         }
      }

      void restart()
      {
         m_running = true;
         m_start = boost::posix_time::microsec_clock::universal_time();
      }

      void stop(const std::string& result = "success")
      {
         if (m_running)
         {
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
            float seconds = 1e-6*(now - m_start).total_microseconds();
            m_bm.add_timing(result, seconds);
            m_running = false;
         }
      }

   private:
      benchmark& m_bm;
      bool m_running;
      boost::posix_time::ptime m_start;
      const std::string m_default_result;
   };

   benchmark(const std::string& name, size_t num_threads = 1)
      : m_name(name)
      , m_num_threads(num_threads)
   {
   }

   void add_timing(const std::string& timing, float time)
   {
      boost::mutex::scoped_lock lock(m_mutex);
      m_timings[timing].push_back(time);
   }

   void output(std::ostream& os, size_t bins = 120, size_t height = 25, float offset = 0.0f, float hi_cut = 0.02f) const
   {
      boost::mutex::scoped_lock lock(m_mutex);

      moost::utils::histogram<float> cumulative("s");
      cumulative.set_display_range(offset, 1.0f - hi_cut);

      os << std::endl << "=== " << m_name << " ===" << std::endl << std::endl;

      foreach (const timings_type::value_type& t, m_timings)
      {
         moost::utils::histogram<float> h("s", bins, height);
         h.set_display_range(offset, 1.0f - hi_cut);

         h.add(t.second.begin(), t.second.end(), t.first, "*");
         cumulative.add(t.second.begin(), t.second.end(), t.first, t.first.substr(0, 1));

         h.draw(os);

         os << std::endl;
      }

      cumulative.draw(os);
      os << std::endl;

      os << "requests per second: " << m_num_threads/cumulative.mean() << std::endl;
   }

private:
   mutable boost::mutex m_mutex;
   timings_type m_timings;
   const std::string m_name;
   const size_t m_num_threads;
};

}}

#endif
