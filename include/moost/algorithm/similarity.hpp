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

#ifndef MOOST_ALGORITHM_SIMILARITY_HPP__
#define MOOST_ALGORITHM_SIMILARITY_HPP__

#include <cmath>

namespace moost { namespace algorithm {

/**
 * Simple adding accumulator policy
 *
 * This is the default policy used by the cosine_similarity template and
 * it will simply sum up products of all item matches.
 */
template <typename FloatType>
struct SimpleAccumulatorPolicy
{
   void operator() (FloatType& accu, const FloatType& val) const
   {
      accu += val;
   }
};

/**
 * Cosine similarity algorithm
 *
 * This function template implements a generic cosine similarity algorithm for two
 * arbitrary dimension feature vectors X and Y.
 *
 * Note that this implementation requires both input vectors to be sorted and that
 * dimensions of both vectors must be less-than comparable.
 *
 * \tparam FloatType             Floating point type used for internal computation and return value.
 * \tparam ForwardIteratorX      Iterator type of vector X.
 * \tparam WeightAccessPolicyX   Policy for accessing the weight of a dimension in X.
 * \tparam ForwardIteratorY      Iterator type of vector Y.
 * \tparam WeightAccessPolicyY   Policy for accessing the weight of a dimension in Y.
 * \tparam AccumulatorPolicy     Policy for summing the feature matches (dot product parts). Defaults
 *                               to SimpleAccumulatorPolicy<FloatType>, but a different policy could
 *                               be used e.g. for counting the number of matches.
 *
 * \param x_beg                  Iterator to the start of vector X.
 * \param x_end                  Iterator to the end of vector X.
 * \param x_weight               Policy to access the weight of a dimension in vector X.
 * \param y_beg                  Iterator to the start of vector Y.
 * \param y_end                  Iterator to the end of vector Y.
 * \param y_weight               Policy to access the weight of a dimension in vector Y.
 * \param accu                   Accumulator policy.
 *
 */
template < typename FloatType,
           class ForwardIteratorX,
           class WeightAccessPolicyX,
           class ForwardIteratorY,
           class WeightAccessPolicyY,
           class AccumulatorPolicy >
FloatType cosine_similarity(ForwardIteratorX x_beg, ForwardIteratorX x_end, WeightAccessPolicyX x_weight,
                            ForwardIteratorY y_beg, ForwardIteratorY y_end, WeightAccessPolicyY y_weight,
                            AccumulatorPolicy& accu)
{
   FloatType sum, norm_x, norm_y;

   sum = norm_x = norm_y = 0.0;

   while (x_beg != x_end && y_beg != y_end)
   {
      if (*x_beg < *y_beg)
      {
         FloatType x = x_weight(*x_beg);
         norm_x += x*x;
         ++x_beg;
      }
      else if (*y_beg < *x_beg)
      {
         FloatType y = y_weight(*y_beg);
         norm_y += y*y;
         ++y_beg;
      }
      else
      {
         FloatType x = x_weight(*x_beg);
         FloatType y = y_weight(*y_beg);
         accu(sum, x*y);
         norm_x += x*x;
         norm_y += y*y;
         ++x_beg;
         ++y_beg;
      }
   }

   if (sum == 0.0)
   {
      return 0.0;
   }

   for (; x_beg != x_end; ++x_beg)
   {
      FloatType x = x_weight(*x_beg);
      norm_x += x*x;
   }

   for (; y_beg != y_end; ++y_beg)
   {
      FloatType y = y_weight(*y_beg);
      norm_y += y*y;
   }

   return sum/std::sqrt(norm_x*norm_y);
}

/* Default implementation using SimpleAccumulatorPolicy */
template < typename FloatType,
           class ForwardIteratorX,
           class WeightAccessPolicyX,
           class ForwardIteratorY,
           class WeightAccessPolicyY >
FloatType cosine_similarity(ForwardIteratorX x_beg, ForwardIteratorX x_end, WeightAccessPolicyX x_weight,
                            ForwardIteratorY y_beg, ForwardIteratorY y_end, WeightAccessPolicyY y_weight)
{
   SimpleAccumulatorPolicy<FloatType> accu;
   return cosine_similarity<FloatType>(x_beg, x_end, x_weight, y_beg, y_end, y_weight, accu);
}

}}

#endif
