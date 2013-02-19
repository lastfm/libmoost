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

/**
 * @file spline_interpolation.hpp
 * @brief generate a cubic spline with natural boundary conditions
 * @author Ricky Cormier (thanks to Marcus for the suggestion)
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-04-26
 */

#ifndef MOOST_ALGORITHM_SPLINE_INTERPOLATION_HPP
#define MOOST_ALGORITHM_SPLINE_INTERPOLATION_HPP

#include <cassert>
#include <vector>
#include <algorithm>

#include <gsl/gsl_spline.h> // gnu scientific library

#include <boost/noncopyable.hpp>

#include "../utils/scope_exit.hpp"


namespace moost { namespace algorithm {

   /**
    * @brief Generate a spline interpolation of X and Y data points using
    *        the GNU Scientific Library.
    *
    * http://www.gnu.org/software/gsl/manual/html_node/Interpolation.html
    *
    */

   class spline_interpolation : private boost::noncopyable
   {
      private:
         typedef moost::utils::scope_exit::type<
            gsl_interp *>::call_free_function_with_val interp_t;

         typedef moost::utils::scope_exit::type<
            gsl_interp_accel *>::call_free_function_with_val accel_t;

      public:


         /**
          * @brief create a spline interpolation object
          *
          * @param x : A vector of X data points
          * @param y : A vector of Y data points
          *
          * @note The X vector MUST be sorted in ascending order.
          *
          * Given a vector of X/Y data points, this class will interpolate a
          * value of Y for any value of X providing the value of X is given
          * within the range of X_begin to X_end.
          */

         spline_interpolation(
               std::vector<double> const & x,
               std::vector<double> const & y)
            : interp_(gsl_interp_alloc(gsl_interp_cspline, x.size()), &gsl_interp_free)
              , accel_(gsl_interp_accel_alloc(), &gsl_interp_accel_free)
              , x_(x)
              , y_(y)

         {
            validate_construction(); // throws on fail
            gsl_interp_init(interp_->get(), &x_[0], &y_[0], x_.size());
         }


         /**
          * @brief Given a value of X return Y (or Z if X is out of range)
          *
          * @param x An X data point
          * @param y An interpolated Y data point
          * @param z Used as the default if X is out of range
          *
          * @note the value of X doesn't have to be one of the original values
          *       of the X vector, it just has to be within the range of the
          *       lowest to the highest X value (hence, Y is an interpolation).
          *
          * @return Interpolation of Y or Z if X is out of range
          */

         bool operator () (double const x, double & y, double const z = 0) const
         {
            bool ok =  0 == gsl_interp_eval_e(
                  interp_->get(),
                  &x_[0], &y_[0], x,
                  accel_->get(), &y
                  );

            if(!ok) { y = z;}

            return ok;
         }

         /**
          * @brief Given a value of X return Y (or throw if out of range)
          *
          * @param x An X data point
          *
          * @note the value of X doesn't have to be one of the original values
          *       of the X vector, it just has to be within the range of the
          *       lowest to the highest X value (hence, Y is an interpolation).
          *
          * @return An interpolation of Y data point
          */
         double operator () (double const x) const
         {
            double y = 0.0;

            if(!(*this)(x, y))
            {
               throw std::range_error("out of range");
            }

            return y;
         }

      private:

         void validate_construction() const
         {
            // x and y must be the same size
            assert(x_.size() == y_.size());
            if(x_.size() != y_.size())
            {
               throw std::runtime_error("the size of x and y must be the same");
            }

            // x must be sorted by-asc (not my rule, gsl demands it)
            bool const sorted = (std::adjacent_find(x_.begin(), x_.end(), std::greater_equal<double>()) == x_.end());
            assert(sorted);
            if(!sorted)
            {
               throw std::runtime_error("x must be sorted in assending order");
            }
         }

      private:
         interp_t interp_;
         accel_t  accel_;
         std::vector<double> const & x_;
         std::vector<double> const & y_;
   };


}}

#endif // MOOST_ALGORITHM_SPLINE_INTERPOLATION_HPP
