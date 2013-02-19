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

#ifndef MOOST_ALGORITHM_INPLACE_SET_INTERSECTION_HPP__
#define MOOST_ALGORITHM_INPLACE_SET_INTERSECTION_HPP__

namespace moost { namespace algorithm {

// just like stl set_intersection, these two algorithms have the precondition
// that their input ranges are sorted
// i've looked through gcc's include and it seems like we shouldn't need our own in-place version,
// and that this would be safe:
// std::set_intersection(foo.begin(), foo.end(), bar.begin(), bar.end(), foo.begin());
// however, the preconditions of set_intersection (http://www.sgi.com/tech/stl/set_intersection.html)
// explicitly state that the output range and input ranges should not overlap
// so just to be safe, we'll write our own
template <class ForwardIterator, class InputIterator>
ForwardIterator inplace_set_intersection(ForwardIterator first1, ForwardIterator last1,
                                         InputIterator first2, InputIterator last2)
{
  ForwardIterator result = first1;
  while (   first1 != last1
         && first2 != last2)
  {
    if (*first1 < *first2)
      ++first1;
    else if (*first2 < *first1)
      ++first2;
    else
    {
      *result = *first1;
      ++first1;
      ++first2;
      ++result;
    }
  }
  return result;
}

template <class ForwardIterator, class InputIterator, class StrictWeakOrdering>
ForwardIterator inplace_set_intersection(ForwardIterator first1, ForwardIterator last1,
                                         InputIterator first2, InputIterator last2,
                                         StrictWeakOrdering comp)
{
  ForwardIterator result = first1;
  while (   first1 != last1
         && first2 != last2)
  {
    if (comp(*first1, *first2))
      ++first1;
    else if (comp(*first2, *first1))
      ++first2;
    else
    {
      *result = *first1;
      ++first1;
      ++first2;
      ++result;
    }
  }
  return result;
}

}} // moost::algorithm

#endif // MOOST_ALGORITHM_INPLACE_SET_INTERSECTION_HPP__
