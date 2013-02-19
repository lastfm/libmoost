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

#ifndef MOOST_UTILS_HISTOGRAM_HPP__
#define MOOST_UTILS_HISTOGRAM_HPP__

/**
 * \file histogram.hpp
 *
 * This is the (slightly hacky) implementation of a class that can output an
 * ASCII histogram graph to a stream. Its main focus is ease of use.
 *
 * Usually, all you have to do is to provide the dimensions of your graph,
 * i.e. width and height in characters, and one or more data vectors. Each
 * data vector can be represented by a different character in the graph.
 *
 * The histogram class takes care of scaling the data, using an optimum
 * (well, I guess it could be made even better, but it's good enough for
 * a start) range, drawing nice tick marks and providing some statistics
 * on the data.
 *
 * Here's a short example to show how easy this class is to use. You'll
 * notice that most of the code is actually used to create the data vectors.

\code
#include <iostream>
#include <vector>

#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/normal_distribution.hpp>

#include "histogram.hpp"

template <class ListType, class RngType, class DistributionType>
void random_fill(ListType& list, RngType& rng, DistributionType& dist)
{
   boost::variate_generator<RngType&, DistributionType> die(rng, dist);
   std::generate(list.begin(), list.end(), die);
}

int main()
{
   boost::mt19937 rng;
   std::vector<float> normal(5000), uniform(2000);

   boost::uniform_real<float> ud(2.3e-5, 2.9e-5);
   random_fill(uniform, rng, ud);

   boost::normal_distribution<float> nd(2.0e-5, 0.4e-5);
   random_fill(normal, rng, nd);

   moost::utils::histogram<float> hist("s", 90, 16);
   hist.set_display_range(0.01, 0.99);

   hist.add(uniform.begin(), uniform.end(), "uniform", "#");
   hist.add(normal.begin(), normal.end(), "normal", "/");

   hist.draw(std::cout);

   return 0;
}
\endcode

 * This code will produce the following graph on your console:

\code
-------------------------------------------------------------------------------------------
                                                //
                                                /// / /
                                   /            ///// // /
                             /     /            //////// / /
                             /   / ///  /       ////////////   /
                            ////////// //// /   ////////////// / /
                          //////////////////// /////////////////////
                      // /////////////////////////#/#//#//##///#/#//////
                    /////////////////////////////#####/#/####//#####//##
/                 //////////////////////////////########################
/               ////////////////////////////////########################
/            ///////////////////////////////////########################
/      // / ////////////////////////////////////########################
/      // //////////////////////////////////////########################
/  / ///////////////////////////////////////////########################
////////////////////////////////////////////////########################//  /
-------------------------------------------------------------------------------------------
'   '   '   '   |   '   '   '   '   |   '   '   '   '   |   '   '   '   '   |   '   '   '
                15                  20                  25                  30 us

 [#] uniform (28.57 %): 25.97 ± 1.729 us [23 .. 29 us]
 [/] normal (71.43 %): 19.91 ± 4.042 us [5.541 .. 33.88 us]

 overall: 21.64 ± 4.473 us [5.541 .. 33.88 us]

-------------------------------------------------------------------------------------------
\endcode

 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <limits>

#include "foreach.hpp"

namespace moost { namespace utils {

template <typename FloatType>
class histogram
{
public:
   typedef FloatType float_type;

private:
   struct data_info
   {
      std::vector<float_type> vec;
      std::string id;
      std::string sym;

      data_info(const std::string& id, const std::string& sym)
         : id(id)
         , sym(sym)
      {}
   };

   struct range_info
   {
      float_type offset;
      float_type bin_width;
      float_type disp_factor;
      std::string unit_prefix;
   };

   struct stats
   {
      float_type min;
      float_type max;
      float_type mean;
      float_type dev;
   };

public:
   histogram(const std::string& unit, size_t bins = 120, size_t height = 25)
      : m_count(0)
      , m_unit(unit)
      , m_bins(bins)
      , m_height(height)
      , m_lo_cutoff(0.01)
      , m_hi_cutoff(0.01)
      , m_prec(4)
   {
      if (m_bins < 1)
      {
         throw std::range_error("invalid number of bins");
      }

      if (m_height < 1)
      {
         throw std::range_error("invalid histogram height");
      }
   }

   void set_display_range(float_type min, float_type max)
   {
      if (min >= max || min < 0.0 || max > 1.0)
      {
         throw std::range_error("invalid display range");
      }

      m_lo_cutoff = min;
      m_hi_cutoff = 1.0 - max;
   }

   void set_precision(size_t prec)
   {
      m_prec = prec;
   }

   template <class InputIterator>
   void add(InputIterator first, InputIterator last, const std::string& id, const std::string& sym)
   {
      m_data.push_back(data_info(id, sym));
      std::copy(first, last, std::back_inserter(m_data.back().vec));
      m_count += m_data.back().vec.size();
   }

   float_type mean() const
   {
      stats st;
      get_stats(st, m_data);
      return st.mean;
   }

   void draw(std::ostream& os, bool legend = true) const
   {
      if (m_count == 0)
      {
         // maybe we can do something more useful here?
         return;
      }

      range_info ri;

      optimum_range(ri);

      std::map< std::string, std::vector<size_t> > binvec;
      std::vector<size_t> total(m_bins + 1);

      foreach (const data_info& di, m_data)
      {
         std::vector<size_t>& bv = binvec[di.id];

         bv.resize(m_bins + 1);

         foreach (float_type v, di.vec)
         {
            size_t bin = std::min(m_bins, static_cast<size_t>(std::max(float_type(0.0), (v - ri.offset)/ri.bin_width)));
            ++bv[bin];
            ++total[bin];
         }
      }

      size_t max_total = *std::max_element(total.begin(), total.end());

      typedef std::vector< std::vector<std::string> > mx_t;
      mx_t mx(m_height);

      foreach (std::vector<std::string>& v, mx)
      {
         v.resize(m_bins + 1);
         std::fill(v.begin(), v.end(), " ");
      }

      for (size_t bin = 0; bin <= m_bins; ++bin)
      {
         size_t accu = 0;
         size_t top = 0;

         foreach (const data_info& di, m_data)
         {
            accu += binvec.find(di.id)->second[bin];

            for (size_t new_top = (accu*m_height + max_total/2)/max_total; top < new_top; ++top)
            {
               mx[top][bin] = di.sym;
            }
         }
      }

      std::ostream_iterator<std::string> osi(os);
      std::fill_n(osi, m_bins + 1, "-");
      os << std::endl;

      for (mx_t::reverse_iterator it = mx.rbegin(); it != mx.rend(); ++it)
      {
         std::copy(it->begin(), it->end(), osi);
         os << std::endl;
      }

      std::fill_n(osi, m_bins + 1, "-");
      os << std::endl;

      std::string ticks, labels;
      ticks.resize(m_bins + 1, ' ');
      add_ticks(ticks, ri, 1 + m_bins/2, '\'');
      add_ticks(ticks, ri, 1 + m_bins/8, '|', &labels);

      os << ticks << std::endl;
      os << labels << std::endl;

      if (legend)
      {
         stats st;
         os << std::endl;

         foreach (const data_info& di, m_data)
         {
            get_stats(st, di.vec);

            std::ostringstream oss;
            oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
            oss.precision(2);
            oss << 100.0*di.vec.size()/m_count << " %";

            os << " [" << di.sym << "] " << di.id << " (" << oss.str() << "): " << stats2str(st, ri) << std::endl;
         }

         if (m_data.size() > 1)
         {
            get_stats(st, m_data);
            os << std::endl << " overall: " << stats2str(st, ri) << std::endl;
         }

         os << std::endl;
         std::fill_n(osi, m_bins + 1, "-");
         os << std::endl;
      }
   }

private:
   std::string stats2str(const stats& st, const range_info& ri) const
   {
      std::ostringstream oss;

      oss.precision(m_prec);

      oss << st.mean/ri.disp_factor << " ± " << st.dev/ri.disp_factor << " " << ri.unit_prefix << m_unit
          << " [" << st.min/ri.disp_factor << " .. " <<  st.max/ri.disp_factor<< " " << ri.unit_prefix << m_unit << "]";

      return oss.str();
   }

   static void get_stats(stats& s, const std::vector<data_info>& vdvec)
   {
      std::vector<const std::vector<float_type> *> vv;
      foreach (const data_info& di, vdvec)
      {
         vv.push_back(&di.vec);
      }
      get_stats(s, vv);
   }

   static void get_stats(stats& s, const std::vector<float_type>& vec)
   {
      std::vector<const std::vector<float_type> *> vv;
      vv.push_back(&vec);
      get_stats(s, vv);
   }

   static float_type invalid_value()
   {
      return std::numeric_limits<float_type>::has_quiet_NaN
             ? std::numeric_limits<float_type>::quiet_NaN()
             : float_type(0);
   }

   static void get_stats(stats& s, const std::vector<const std::vector<float_type> *>& vv)
   {
      float_type min = invalid_value();
      float_type max = invalid_value();
      float_type sum = 0;
      float_type sum_of_squares = 0;
      size_t count = 0;

      foreach (const std::vector<float_type> *vec, vv)
      {
         foreach (float_type v, *vec)
         {
            sum += v;
            sum_of_squares += v*v;

            if (count == 0 || v < min)
               min = v;

            if (count == 0 || v > max)
               max = v;

            ++count;
         }
      }

      if (count > 0)
      {
         s.mean = sum/count;

         if (count > 1)
         {
            s.dev = std::sqrt((sum_of_squares - s.mean*sum)/(count - 1));
         }
         else
         {
            s.dev = invalid_value();
         }
      }
      else
      {
         s.mean = invalid_value();
         s.dev = invalid_value();
      }

      s.min = min;
      s.max = max;
   }

   void optimum_range(range_info& ri) const
   {
      const size_t num_lower = m_count*m_lo_cutoff + 1;
      const size_t num_upper = m_count*m_hi_cutoff + 1;

      std::vector<float_type> small(num_lower*m_data.size()), large(num_upper*m_data.size());

      for (size_t i = 0; i < m_data.size(); ++i)
      {
         std::partial_sort_copy(m_data[i].vec.begin(), m_data[i].vec.end(),
                                small.begin() + i*num_lower, small.begin() + (i + 1)*num_lower);
         std::partial_sort_copy(m_data[i].vec.begin(), m_data[i].vec.end(),
                                large.begin() + i*num_upper, large.begin() + (i + 1)*num_upper, std::greater<float_type>());
      }

      std::partial_sort(small.begin(), small.begin() + num_lower, small.end());
      std::partial_sort(large.begin(), large.begin() + num_upper, large.end(), std::greater<float_type>());

      float_type xmin = small[m_count*m_lo_cutoff];
      float_type xmax = large[m_count*m_hi_cutoff];

      float_type delta = nicenum((xmax - xmin)/(1 + m_bins/8), true);

      ri.offset = std::floor(xmin/delta)*delta;
      float_type upper = std::ceil(xmax/delta)*delta;
      ri.bin_width = nice_bin_width((upper - ri.offset)/m_bins);

      float_type exp = std::log10(std::max(std::abs(xmin), std::abs(xmax)));

      if (exp < 0.0)
      {
         const char *prefix[] = { "m", "u", "n", "p" };
         int ix = std::min(3, static_cast<int>((0.0 - exp)/3.0));
         ri.unit_prefix = prefix[ix];
         ri.disp_factor = std::pow(1e-3, ix + 1);
      }
      else if (exp > 3.0)
      {
         const char *prefix[] = { "k", "M", "T", "P" };
         int ix = std::min(3, static_cast<int>((exp - 3.0)/3.0));
         ri.unit_prefix = prefix[ix];
         ri.disp_factor = std::pow(1e3, ix + 1);
      }
      else
      {
         ri.unit_prefix.clear();
         ri.disp_factor = 1.0;
      }
   }

   void add_ticks(std::string& ticks, const range_info& ri, size_t desired_count, char tickmark, std::string *labels = 0) const
   {
      // Adapted from: Andrew S. Glassner "Graphics Gems", p.61

      const size_t min_label_sep = 2;
      const float_type xmin = ri.offset/ri.disp_factor;
      const float_type xmax = (ri.offset + ri.bin_width*m_bins)/ri.disp_factor;
      const float_type range = nicenum(xmax - xmin, false, ri.bin_width/ri.disp_factor);
      const float_type delta = nicenum(range/desired_count, true, ri.bin_width/ri.disp_factor);
      const float_type gmin = std::floor(xmin/delta)*delta;
      const float_type gmax = std::ceil(xmax/delta)*delta;
      const int prec = std::max(-static_cast<int>(std::log10(delta) - 0.8), 0);
      std::string labelstr;

      for (float_type gx = gmin; gx < gmax + 0.5*delta; gx += delta)
      {
         int x = (gx*ri.disp_factor - ri.offset)/ri.bin_width + 0.5;

         if (x >= 0 && x < static_cast<int>(ticks.size()))
         {
            ticks[x] = tickmark;

            std::ostringstream oss;
            oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
            oss.precision(prec);
            oss << gx;

            if (labelstr.size() == 0 || static_cast<size_t>(x) >= labelstr.size() + min_label_sep)
            {
               labelstr.append(static_cast<size_t>(x) - labelstr.size(), ' ');
               labelstr.append(oss.str());
            }
         }
      }

      labelstr.append(" ");
      labelstr.append(ri.unit_prefix);
      labelstr.append(m_unit);

      if (labels)
      {
         labels->swap(labelstr);
      }
   }

   static float_type nicenum(float_type x, bool round, float_type multiple_of = -1.0)
   {
      // Adapted from: Andrew S. Glassner "Graphics Gems", p.61

      float_type exp = std::floor(std::log10(x));
      float_type frac = x/std::pow(10.0, exp);
      float_type nice;
      bool check_multiple = multiple_of > 0.0;

      if (check_multiple)
      {
         multiple_of /= std::pow(10.0, exp);
      }

      const float_type (*nn)[2];

      if (round)
      {
         static const float_type cand[][2] = {
            { 1.5,  1.0},
            { 3.0,  2.0},
            { 7.0,  5.0},
            {-1.0, 10.0},
         };

         nn = &cand[0];
      }
      else
      {
         static const float_type cand[][2] = {
            { 1.0,  1.0},
            { 2.0,  2.0},
            { 5.0,  5.0},
            {-1.0, 10.0},
         };

         nn = &cand[0];
      }

      for (;; ++nn)
      {
         float_type integral;

         if ((*nn)[0] < 0.0 || (frac < (*nn)[0] && (!check_multiple || std::fabs(std::modf((*nn)[1]/multiple_of, &integral)) < 1e-6)))
         {
            nice = (*nn)[1];
            break;
         }
      }

      return nice*std::pow(10.0, exp);
   }

   static float_type nice_bin_width(float_type x)
   {
      float_type exp = std::floor(std::log10(x));
      float_type frac = x/std::pow(10.0, exp);
      float_type nice;

      if (frac < 1.0)
         nice = 1.0;
      else if (frac < 2.0)
         nice = 2.0;
      else if (frac < 2.5)
         nice = 2.5;
      else if (frac < 5.0)
         nice = 5.0;
      else
         nice = 10.0;

      return nice*std::pow(10.0, exp);
   }

   std::vector<data_info> m_data;
   size_t m_count;
   const std::string m_unit;
   const size_t m_bins;
   const size_t m_height;
   float_type m_lo_cutoff;
   float_type m_hi_cutoff;
   size_t m_prec;
};

}}

#endif
