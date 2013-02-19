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

#ifndef MOOST_UTILS_PROGRESS_POLICY_SKELETON_HPP__
#define MOOST_UTILS_PROGRESS_POLICY_SKELETON_HPP__

#include <ostream>

/// You can implement your own progress display policy if you wish, you just need to
/// base it on the progress policy concept that is modeled by this skeleton policy.

namespace moost { namespace progress { namespace policy {

   /// use one of the standard policies or roll your own if you wish! A policy can either be bounded to a count and
   /// used to display progress for that fixed window or it can be unbounded and just display some changing output
   /// to reassure the user something is actually happened and your application hasn't frozen during an intensive task.

   /// CounterType is the type that will be used for counter_type. Making this type a template
   /// parameter is strictly option; it can be hard coded into the policy if that is appropriate.
   template<typename CounterType = size_t>
   class skeleton // this is an example policy -- it does absolutely nothing of any use :)
   {
   public:
      /// A policy needs to define the counter_type, which should generally be an integer type.
      typedef CounterType counter_type;

      /// Your policy should implement a constructor appropriate for its needs
      skeleton(std::ostream & out = std::cout)
         : counter_(0), out_(out)
      {
      }

      counter_type  operator+=(counter_type const incr)
      {
         animate();
         return counter_ += incr;
      }

      /// A policy must implement pre and post increment operators that should return a counter_type by **value**.
      /// The value and meaning of the returned value are arbitrary but generally it should be the accumulative
      /// result of the calls to the pre and post increment operators.
      counter_type operator ++ ()
      {
         return *this += 1;
      }

      counter_type operator ++ (int)
      {
         counter_type const tmp = counter_;
         ++*this; // call += to ensure any thing it does (such as animate) is applied
         return tmp;
      }

      /// A policy should implement a count const method to return the current counter_type by **value**
      counter_type count() const
      {
         return counter_;
      }

      /// A policy should implement a expected_count const method to return the expected max value the counter
      /// should reach. If your policy has no concept of an expected count you can return whatever you like but
      /// in general best to return something sensible like 0, -1 or numeric_limits<counter_type>::max()
      counter_type expected_count() const
      {
         return counter_;
      }

   private:
      void animate()
      {
         // TODO: Your progess display animation goes here, which should take into account that incr
         //        could be > 1 so you need to ensure you display an appropriate number of 'frames'.

      }

   private:
      counter_type counter_;
      std::ostream & out_;
   };

}}}

#endif // MOOST_UTILS_PROGRESS_POLICY_SKELETON_HPP__
