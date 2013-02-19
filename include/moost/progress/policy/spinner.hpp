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

#ifndef MOOST_UTILS_PROGRESS_POLICY_SPINNER_HPP__
#define MOOST_UTILS_PROGRESS_POLICY_SPINNER_HPP__

#include <vector>
#include <iostream>
#include <iomanip>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/thread_time.hpp>

namespace moost { namespace progress { namespace policy {

   class spinner
   {
   public:
      typedef size_t counter_type;

      spinner(size_t msecs = 500, std::ostream & out = std::cout)
         : counter_(0), msecs_(msecs), out_(out), ptime_(boost::get_system_time())
      {
         frames_.push_back('|');
         frames_.push_back('/');
         frames_.push_back('-');
         frames_.push_back('\\');

         frame_ = frames_.begin();
      }

      counter_type  operator+=(counter_type const incr)
      {
         boost::posix_time::ptime now = boost::get_system_time();

         if(now >= ptime_)
         {
            animate();
            ptime_ = now + boost::posix_time::milliseconds(msecs_);
         }

         return counter_ += incr;
      }

      counter_type operator ++ ()
      {
         return *this += 1;
      }

      counter_type operator ++ (int)
      {
         counter_type const tmp = counter_;
         ++*this;
         return tmp;
      }

      counter_type count() const
      {
         return counter_;
      }

      counter_type expected_count() const
      {
         return counter_;
      }

   private:
      void animate()
      {
         out_ << *frame_++ << '\r' << std::flush;
         if(frame_ == frames_.end())
         {
            frame_ = frames_.begin();
         }
      }

      std::vector<char> frames_;
      std::vector<char>::const_iterator frame_;
      counter_type counter_;
      size_t const msecs_; // the min number of milliseconds before we'll update spinner
      std::ostream & out_;
      boost::posix_time::ptime ptime_;
   };

}}}

#endif // MOOST_UTILS_PROGRESS_POLICY_SPINNER_HPP__
