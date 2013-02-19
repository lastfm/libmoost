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

#ifndef MOOST_UTILS_PROGRESS_DISPLAY_HPP__
#define MOOST_UTILS_PROGRESS_DISPLAY_HPP__

// Like boost progress only more flexible since you can specify different process policy types

namespace moost { namespace progress {

   template < typename policyT >
   class display
   {
   public:
      typedef policyT policy_t;
      typedef display<policy_t> this_type;
      typedef typename policy_t::counter_type counter_type;

      display()
      {
      }

      display(policy_t const & policy) : policy_(policy)
      {
      }

      counter_type operator ++ ()
      {
         return ++policy_;
      }

      counter_type operator ++ (int)
      {
         return policy_++;
      }

   private:
      policy_t policy_;
   };

}}

#endif // MOOST_UTILS_PROGRESS_DISPLAY_HPP__
