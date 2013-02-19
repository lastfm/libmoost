/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       search.cpp
 * \brief      Test cases for runtime MPL sequence search.
 * \author     Marcus Holland-Moritz (marcus@last.fm)
 * \copyright  Copyright © 2008-2013 Last.fm Limited
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

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include "../../include/moost/mpl/search.hpp"

BOOST_AUTO_TEST_SUITE(mpl_search_test)

namespace {

   typedef boost::mpl::list< long, short > tl_test;

   class finder
   {
   public:
      typedef size_t return_type;

      finder(size_t size)
         : m_size(size)
      {
      }

      template <class T>
      bool test() const
      {
         return m_size == sizeof(T);
      }

      template <class T>
      return_type found() const
      {
         return 2*sizeof(T);
      }

      return_type not_found() const
      {
         return 0;
      }

   private:
      const size_t m_size;
   };

}

BOOST_AUTO_TEST_CASE(mpl_search_policy_test)
{
   BOOST_CHECK_EQUAL(moost::mpl::search<tl_test>(finder(1)), 0);
   BOOST_CHECK_EQUAL(moost::mpl::search<tl_test>(finder(sizeof(short))), 2*sizeof(short));
}

BOOST_AUTO_TEST_SUITE_END()
