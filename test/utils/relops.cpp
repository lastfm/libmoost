/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file relops.cpp
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

#include <boost/test/unit_test.hpp>

#include "../../include/moost/utils/relops.hpp"

using namespace moost::utils;


namespace {
   template<int N>
   struct xnum_;

   template<int N>
      struct num_ : relops<num_<N> >
   {
      static const int val = N;

      // ------------------------------------------
      // Support for comparing with myself
      // ------------------------------------------
      template<int U>
      bool operator == (num_<U> const & rhs) const
      {
         return val == rhs.val;
      }

      template<int U>
      bool operator < (num_<U> const & rhs) const
      {
         return val < rhs.val;
      }
      // ------------------------------------------
      // Support for comparing with xnum_<N>
      // ------------------------------------------
      template<int U>
      bool operator == (xnum_<U> const & rhs) const
      {
         return val == rhs.val;
      }

      template<int U>
      bool operator < (xnum_<U> const & rhs) const
      {
         return val < rhs.val;
      }
      // ------------------------------------------
   };

   // It's safe to construct this here becasuse num_<T> is a POD
   // NB. the constructor won't throw.
   num_<0> _0;
   num_<1> _1;
   num_<2> _2;

   template<int N>
   struct xnum_
   {
      static const int val = N;
   };

   xnum_<0> _x0;
   xnum_<1> _x1;
   xnum_<2> _x2;
}

BOOST_AUTO_TEST_SUITE(moost_utils_relops)

BOOST_AUTO_TEST_CASE(test_equal)
{

   BOOST_CHECK((_0 == _0) == (0 == 0));
   BOOST_CHECK((_0 == _1) == (0 == 1));
   BOOST_CHECK((_0 == _2) == (0 == 2));

   BOOST_CHECK((_1 == _0) == (1 == 0));
   BOOST_CHECK((_1 == _1) == (1 == 1));
   BOOST_CHECK((_1 == _2) == (1 == 2));

   BOOST_CHECK((_2 == _0) == (2 == 0));
   BOOST_CHECK((_2 == _1) == (2 == 1));
   BOOST_CHECK((_2 == _2) == (2 == 2));
}

BOOST_AUTO_TEST_CASE(test_less)
{

   BOOST_CHECK((_0 < _0) == (0 < 0));
   BOOST_CHECK((_0 < _1) == (0 < 1));
   BOOST_CHECK((_0 < _2) == (0 < 2));

   BOOST_CHECK((_1 < _0) == (1 < 0));
   BOOST_CHECK((_1 < _1) == (1 < 1));
   BOOST_CHECK((_1 < _2) == (1 < 2));

   BOOST_CHECK((_2 < _0) == (2 < 0));
   BOOST_CHECK((_2 < _1) == (2 < 1));
   BOOST_CHECK((_2 < _2) == (2 < 2));
}

BOOST_AUTO_TEST_CASE(test_not_equal)
{

   BOOST_CHECK((_0 != _0) == (0 != 0));
   BOOST_CHECK((_0 != _1) == (0 != 1));
   BOOST_CHECK((_0 != _2) == (0 != 2));

   BOOST_CHECK((_1 != _0) == (1 != 0));
   BOOST_CHECK((_1 != _1) == (1 != 1));
   BOOST_CHECK((_1 != _2) == (1 != 2));

   BOOST_CHECK((_2 != _0) == (2 != 0));
   BOOST_CHECK((_2 != _1) == (2 != 1));
   BOOST_CHECK((_2 != _2) == (2 != 2));
}

BOOST_AUTO_TEST_CASE(test_greater)
{

   BOOST_CHECK((_0 > _0) == (0 > 0));
   BOOST_CHECK((_0 > _1) == (0 > 1));
   BOOST_CHECK((_0 > _2) == (0 > 2));

   BOOST_CHECK((_1 > _0) == (1 > 0));
   BOOST_CHECK((_1 > _1) == (1 > 1));
   BOOST_CHECK((_1 > _2) == (1 > 2));

   BOOST_CHECK((_2 > _0) == (2 > 0));
   BOOST_CHECK((_2 > _1) == (2 > 1));
   BOOST_CHECK((_2 > _2) == (2 > 2));
}

BOOST_AUTO_TEST_CASE(test_less_equal)
{

   BOOST_CHECK((_0 <= _0) == (0 <= 0));
   BOOST_CHECK((_0 <= _1) == (0 <= 1));
   BOOST_CHECK((_0 <= _2) == (0 <= 2));

   BOOST_CHECK((_1 <= _0) == (1 <= 0));
   BOOST_CHECK((_1 <= _1) == (1 <= 1));
   BOOST_CHECK((_1 <= _2) == (1 <= 2));

   BOOST_CHECK((_2 <= _0) == (2 <= 0));
   BOOST_CHECK((_2 <= _1) == (2 <= 1));
   BOOST_CHECK((_2 <= _2) == (2 <= 2));
}

BOOST_AUTO_TEST_CASE(test_greater_equal)
{

   BOOST_CHECK((_0 >= _0) == (0 >= 0));
   BOOST_CHECK((_0 >= _1) == (0 >= 1));
   BOOST_CHECK((_0 >= _2) == (0 >= 2));

   BOOST_CHECK((_1 >= _0) == (1 >= 0));
   BOOST_CHECK((_1 >= _1) == (1 >= 1));
   BOOST_CHECK((_1 >= _2) == (1 >= 2));

   BOOST_CHECK((_2 >= _0) == (2 >= 0));
   BOOST_CHECK((_2 >= _1) == (2 >= 1));
   BOOST_CHECK((_2 >= _2) == (2 >= 2));
}

BOOST_AUTO_TEST_CASE(test_equalx)
{

   BOOST_CHECK((_0 == _x0) == (0 == 0));
   BOOST_CHECK((_0 == _x1) == (0 == 1));
   BOOST_CHECK((_0 == _x2) == (0 == 2));

   BOOST_CHECK((_1 == _x0) == (1 == 0));
   BOOST_CHECK((_1 == _x1) == (1 == 1));
   BOOST_CHECK((_1 == _x2) == (1 == 2));

   BOOST_CHECK((_2 == _x0) == (2 == 0));
   BOOST_CHECK((_2 == _x1) == (2 == 1));
   BOOST_CHECK((_2 == _x2) == (2 == 2));
}

BOOST_AUTO_TEST_CASE(test_lessx)
{

   BOOST_CHECK((_0 < _x0) == (0 < 0));
   BOOST_CHECK((_0 < _x1) == (0 < 1));
   BOOST_CHECK((_0 < _x2) == (0 < 2));

   BOOST_CHECK((_1 < _x0) == (1 < 0));
   BOOST_CHECK((_1 < _x1) == (1 < 1));
   BOOST_CHECK((_1 < _x2) == (1 < 2));

   BOOST_CHECK((_2 < _x0) == (2 < 0));
   BOOST_CHECK((_2 < _x1) == (2 < 1));
   BOOST_CHECK((_2 < _x2) == (2 < 2));
}

BOOST_AUTO_TEST_CASE(test_not_equalx)
{

   BOOST_CHECK((_0 != _x0) == (0 != 0));
   BOOST_CHECK((_0 != _x1) == (0 != 1));
   BOOST_CHECK((_0 != _x2) == (0 != 2));

   BOOST_CHECK((_1 != _x0) == (1 != 0));
   BOOST_CHECK((_1 != _x1) == (1 != 1));
   BOOST_CHECK((_1 != _x2) == (1 != 2));

   BOOST_CHECK((_2 != _x0) == (2 != 0));
   BOOST_CHECK((_2 != _x1) == (2 != 1));
   BOOST_CHECK((_2 != _x2) == (2 != 2));
}

BOOST_AUTO_TEST_CASE(test_greaterx)
{

   BOOST_CHECK((_0 > _x0) == (0 > 0));
   BOOST_CHECK((_0 > _x1) == (0 > 1));
   BOOST_CHECK((_0 > _x2) == (0 > 2));

   BOOST_CHECK((_1 > _x0) == (1 > 0));
   BOOST_CHECK((_1 > _x1) == (1 > 1));
   BOOST_CHECK((_1 > _x2) == (1 > 2));

   BOOST_CHECK((_2 > _x0) == (2 > 0));
   BOOST_CHECK((_2 > _x1) == (2 > 1));
   BOOST_CHECK((_2 > _x2) == (2 > 2));
}

BOOST_AUTO_TEST_CASE(test_less_equalx)
{

   BOOST_CHECK((_0 <= _x0) == (0 <= 0));
   BOOST_CHECK((_0 <= _x1) == (0 <= 1));
   BOOST_CHECK((_0 <= _x2) == (0 <= 2));

   BOOST_CHECK((_1 <= _x0) == (1 <= 0));
   BOOST_CHECK((_1 <= _x1) == (1 <= 1));
   BOOST_CHECK((_1 <= _x2) == (1 <= 2));

   BOOST_CHECK((_2 <= _x0) == (2 <= 0));
   BOOST_CHECK((_2 <= _x1) == (2 <= 1));
   BOOST_CHECK((_2 <= _x2) == (2 <= 2));
}

BOOST_AUTO_TEST_CASE(test_greater_equalx)
{

   BOOST_CHECK((_0 >= _x0) == (0 >= 0));
   BOOST_CHECK((_0 >= _x1) == (0 >= 1));
   BOOST_CHECK((_0 >= _x2) == (0 >= 2));

   BOOST_CHECK((_1 >= _x0) == (1 >= 0));
   BOOST_CHECK((_1 >= _x1) == (1 >= 1));
   BOOST_CHECK((_1 >= _x2) == (1 >= 2));

   BOOST_CHECK((_2 >= _x0) == (2 >= 0));
   BOOST_CHECK((_2 >= _x1) == (2 >= 1));
   BOOST_CHECK((_2 >= _x2) == (2 >= 2));
}

BOOST_AUTO_TEST_CASE(test_xequal)
{

   BOOST_CHECK((_x0 == _0) == (0 == 0));
   BOOST_CHECK((_x0 == _1) == (0 == 1));
   BOOST_CHECK((_x0 == _2) == (0 == 2));

   BOOST_CHECK((_x1 == _0) == (1 == 0));
   BOOST_CHECK((_x1 == _1) == (1 == 1));
   BOOST_CHECK((_x1 == _2) == (1 == 2));

   BOOST_CHECK((_x2 == _0) == (2 == 0));
   BOOST_CHECK((_x2 == _1) == (2 == 1));
   BOOST_CHECK((_x2 == _2) == (2 == 2));
}

BOOST_AUTO_TEST_CASE(test_xless)
{

   BOOST_CHECK((_x0 < _0) == (0 < 0));
   BOOST_CHECK((_x0 < _1) == (0 < 1));
   BOOST_CHECK((_x0 < _2) == (0 < 2));

   BOOST_CHECK((_x1 < _0) == (1 < 0));
   BOOST_CHECK((_x1 < _1) == (1 < 1));
   BOOST_CHECK((_x1 < _2) == (1 < 2));

   BOOST_CHECK((_x2 < _0) == (2 < 0));
   BOOST_CHECK((_x2 < _1) == (2 < 1));
   BOOST_CHECK((_x2 < _2) == (2 < 2));
}

BOOST_AUTO_TEST_CASE(test_xnot_equal)
{

   BOOST_CHECK((_x0 != _0) == (0 != 0));
   BOOST_CHECK((_x0 != _1) == (0 != 1));
   BOOST_CHECK((_x0 != _2) == (0 != 2));

   BOOST_CHECK((_x1 != _0) == (1 != 0));
   BOOST_CHECK((_x1 != _1) == (1 != 1));
   BOOST_CHECK((_x1 != _2) == (1 != 2));

   BOOST_CHECK((_x2 != _0) == (2 != 0));
   BOOST_CHECK((_x2 != _1) == (2 != 1));
   BOOST_CHECK((_x2 != _2) == (2 != 2));
}

BOOST_AUTO_TEST_CASE(test_xgreater)
{

   BOOST_CHECK((_x0 > _0) == (0 > 0));
   BOOST_CHECK((_x0 > _1) == (0 > 1));
   BOOST_CHECK((_x0 > _2) == (0 > 2));

   BOOST_CHECK((_x1 > _0) == (1 > 0));
   BOOST_CHECK((_x1 > _1) == (1 > 1));
   BOOST_CHECK((_x1 > _2) == (1 > 2));

   BOOST_CHECK((_x2 > _0) == (2 > 0));
   BOOST_CHECK((_x2 > _1) == (2 > 1));
   BOOST_CHECK((_x2 > _2) == (2 > 2));
}

BOOST_AUTO_TEST_CASE(test_xless_equal)
{

   BOOST_CHECK((_x0 <= _0) == (0 <= 0));
   BOOST_CHECK((_x0 <= _1) == (0 <= 1));
   BOOST_CHECK((_x0 <= _2) == (0 <= 2));

   BOOST_CHECK((_x1 <= _0) == (1 <= 0));
   BOOST_CHECK((_x1 <= _1) == (1 <= 1));
   BOOST_CHECK((_x1 <= _2) == (1 <= 2));

   BOOST_CHECK((_x2 <= _0) == (2 <= 0));
   BOOST_CHECK((_x2 <= _1) == (2 <= 1));
   BOOST_CHECK((_x2 <= _2) == (2 <= 2));
}

BOOST_AUTO_TEST_CASE(test_xgreater_equal)
{

   BOOST_CHECK((_x0 >= _0) == (0 >= 0));
   BOOST_CHECK((_x0 >= _1) == (0 >= 1));
   BOOST_CHECK((_x0 >= _2) == (0 >= 2));

   BOOST_CHECK((_x1 >= _0) == (1 >= 0));
   BOOST_CHECK((_x1 >= _1) == (1 >= 1));
   BOOST_CHECK((_x1 >= _2) == (1 >= 2));

   BOOST_CHECK((_x2 >= _0) == (2 >= 0));
   BOOST_CHECK((_x2 >= _1) == (2 >= 1));
   BOOST_CHECK((_x2 >= _2) == (2 >= 2));
}

BOOST_AUTO_TEST_SUITE_END()
