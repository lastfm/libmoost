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

#ifndef MOOST_STRING_LEVENSHTEIN_HPP__
#define MOOST_STRING_LEVENSHTEIN_HPP__

#include <vector>
#include <string>
#include <stdexcept>
#include <climits>

namespace moost { namespace string {

/** @brief Calculates levenshtein distance between two iterator ranges.
 * The levenshtein distance between two strings is given by the minimum number of operations needed
 * to transform one string into the other, where an operation is an insertion, deletion, or substitution
 * of a single character.
 *
 * This is technically damerau-levenshtein, but too long a name for a function.
 */
template <class RandomAccessIterator>
int levenshtein(RandomAccessIterator first1,
                RandomAccessIterator last1,
                RandomAccessIterator first2,
                RandomAccessIterator last2,
                std::vector< std::vector<int> > & d)
{
  const int m = last1 - first1; // length of 1
  const int n = last2 - first2; // length of 2
  int i, j, cost; // cost, iteration

  d.resize(m + 1);
  for (std::vector< std::vector< int> >::iterator it = d.begin(); it != d.end(); ++it)
    it->resize(n + 1);

  for (i = 0; i <= m; ++i)
    d[i][0] = i;
  for (j = 0; j <= n; ++j)
    d[0][j] = j;

  for (i = 1; i <= m; ++i)
  {
    for (j = 1; j <= n; ++j)
    {
      cost = (*(first1 + i - 1) == *(first2 + j - 1) ? 0 : 1);
      d[i][j] = std::min(d[i - 1][j] + 1, // deletion
                std::min(d[i][j - 1] + 1, // insertion
                d[i - 1][j - 1] +   cost)); // substitution

      if (   i > 1
          && j > 1
          && *(first1 + i - 1) == *(first2 + j - 2)
          && *(first1 + i - 2) == *(first2 + j - 1))
        d[i][j] = std::min(d[i][j], d[i - 2][j - 2] + cost); // transposition
    }
  }

  return d[m][n];
}

/** @brief Calculates levenshtein distance between two iterator ranges.
 * The levenshtein distance between two strings is given by the minimum number of operations needed
 * to transform one string into the other, where an operation is an insertion, deletion, or substitution
 * of a single character.
 */
template <class RandomAccessIterator>
int levenshtein(RandomAccessIterator first1,
                RandomAccessIterator last1,
                RandomAccessIterator first2,
                RandomAccessIterator last2)
{
  std::vector< std::vector<int> > d;

  return levenshtein(first1, last1, first2, last2, d);
}

/** @brief Calculates levenshtein distance between two containers of two strings.
 * The levenshtein distance between two strings is given by the minimum number of operations needed
 * to transform one string into the other, where an operation is an insertion, deletion, or substitution
 * of a single character.
 */
int levenshtein(const std::string & first,
                const std::string & second,
                std::vector< std::vector<int> > & d)
{
  return levenshtein(first.begin(), first.end(), second.begin(), second.end(), d);
}

/** @brief Calculates levenshtein distance between two containers of any type, such as strings.
 * The levenshtein distance between two strings is given by the minimum number of operations needed
 * to transform one string into the other, where an operation is an insertion, deletion, or substitution
 * of a single character.
 */
int levenshtein(const std::string & first,
                const std::string & second)
{
  std::vector< std::vector<int> > d;
  return levenshtein(first.begin(), first.end(), second.begin(), second.end(), d);
}

/** @brief Space and time efficient simple levenshtein distance between two iterator ranges.
 * @pre the first string must be no longer than the second
 * @param thresh don't bother computing if the distance is bound to be more than this
 * The levenshtein distance between two strings is given by the minimum number of operations needed
 * to transform one string into the other, where an operation is an insertion, deletion, or substitution
 * of a single character.
 *
 * Note that this is a plain levenshtein distance and not the damereau version of moost::levenshtein()
 * which treats transposition of adjacent characters as a special case.
 */
template <class RandomAccessIterator>
int fast_levenshtein(RandomAccessIterator first1,
                RandomAccessIterator last1,
                RandomAccessIterator first2,
                RandomAccessIterator last2,
                int thresh = INT_MAX)
{
   const int n = last1 - first1; // length of 1
   const int m = last2 - first2; // length of 2

   // check precondition not violated
   if (n > m)
      throw std::runtime_error("fast_levenshtein called with first string longer than second!");

   if ((m - n) > thresh)    // abort, edit dist has to be more than thresh
      return m - n;

   std::vector<int> current(n + 1);
   std::vector<int> previous(n + 1);
   for (int i = 0; i <= n; ++i)
      current[i] = i;

   for (int i = 1; i <= m; ++i)
   {
      std::copy(current.begin(), current.end(), previous.begin());
      std::fill(current.begin(), current.end(), 0);
      current[0] = i;

      for (int j = 1; j <= n; ++j)
      {
         int substitute = previous[j - 1];
         if (*(first1 + j - 1) != *(first2 + i - 1))
            ++substitute;

         // Get minimum of insert, delete and substitute
         current[j] = std::min(std::min(previous[j] + 1, current[j - 1] + 1) , substitute);
      }

      // bail out if already over thresh
      int min_current = current[0];
      for (int j = 1; j <= n; ++j)
         if (current[j] < min_current)
            min_current = current[j];
      if (min_current > thresh)
         return min_current;
   }

   return current[n];
}

/** @brief Space and time efficient simple levenshtein distance between two iterator ranges.
 * @param thresh don't bother computing if the distance is bound to be more than this
 * The levenshtein distance between two strings is given by the minimum number of operations needed
 * to transform one string into the other, where an operation is an insertion, deletion, or substitution
 * of a single character.
 *
 * Note that this is a plain levenshtein distance and not the damereau version of moost::levenshtein()
 * which treats transposition of adjacent characters as a special case.
 */
int fast_levenshtein(const std::string & first,
                const std::string & second,
                int thresh = INT_MAX)
{
  if (first.length() <= second.length())
     return fast_levenshtein(first.begin(), first.end(), second.begin(), second.end(), thresh);
  else
     return fast_levenshtein(second.begin(), second.end(), first.begin(), first.end(), thresh);
}

}} // moost::string

#endif // MOOST_STRING_LEVENSHTEIN_HPP__
