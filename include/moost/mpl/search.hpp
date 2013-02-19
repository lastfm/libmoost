/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       search.hpp
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

#ifndef MOOST_MPL_SEARCH_HPP
#define MOOST_MPL_SEARCH_HPP

#include <boost/mpl/assert.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/next.hpp>

namespace moost { namespace mpl {

namespace detail {

template <bool = true>
struct search_impl
{
   template <typename Iterator, typename End, typename Policy>
   static typename Policy::return_type exec(const Policy& policy)
   {
      return policy.not_found();
   }
};

template <>
struct search_impl<false>
{
   template <typename Iterator, typename End, typename Policy>
   static typename Policy::return_type exec(const Policy& policy)
   {
      typedef typename boost::mpl::deref<Iterator>::type item;
      typedef typename boost::mpl::next<Iterator>::type iter;

      return policy.template test<item>()
                ? policy.template found<item>()
                : search_impl<boost::is_same<iter, End>::value>::template exec<iter, End>(policy);
   }
};

}

/**
 * Search type in a type list according to a policy
 *
 * This template attempts to find a type in a type list using the
 * policy's test() method and then either calls the policy's found()
 * or not_found() method.
 *
 * Here's an example policy for implementing an object factory:

\code
class factory_policy
{
public:
   typedef base *return_type;

   factory_policy(const std::string& name)
      : m_name(name)
   {
   }

   template <class T>
   bool test() const
   {
      return m_name == T::name();
   }

   template <class T>
   return_type found() const
   {
      return new T();
   }

   return_type not_found() const
   {
      throw std::runtime_error("unknown class " + m_name);
   }

private:
   const std::string& m_name;
};
\endcode

 * You could then use the policy like this:

\code
factory_policy policy(name);
base *obj = search<class_list>(policy);
\endcode

 */
template <typename Sequence, typename Policy>
inline typename Policy::return_type search(const Policy& policy)
{
   BOOST_MPL_ASSERT((boost::mpl::is_sequence<Sequence>));

   typedef typename boost::mpl::begin<Sequence>::type first;
   typedef typename boost::mpl::end<Sequence>::type last;

   return detail::search_impl<boost::is_same<first, last>::value>::template exec<first, last>(policy);
}

}}

#endif
