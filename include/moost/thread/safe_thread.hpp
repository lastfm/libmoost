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

#ifndef MOOST_THREAD_SAFE_THREAD
#define MOOST_THREAD_SAFE_THREAD


#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/noncopyable.hpp>

#include "detail/safe_thread_default_policy.hpp"

namespace moost { namespace thread {

   // Note: The base class (boost::thread) is NOT polymorphic
   template <typename Policy = default_safe_thread_policy>
   class safe_thread : boost::thread, boost::noncopyable
   {
   public:
      template <typename F>
      boost::thread * create_thread(F f)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), f)); }

      template <typename F, typename A1>
      boost::thread * create_thread(F f, A1 const & a1)
         { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1)))); }

      template <typename F, typename A1, typename A2>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2)))); }

      template <typename F, typename A1, typename A2, typename A3>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a3)))); }

      template <typename F, typename A1, typename A2, typename A3, typename A4>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a3, a4)))); }

      template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a4, a4, a5)))); }

      template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const &a6)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a4, a4, a5, a6)))); }

      template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a4, a4, a5, a6, a7)))); }

      template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a4, a4, a5, a6, a7, a8)))); }

      template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
      boost::thread * create_thread(F f, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const &a6, A7 const & a7, A8 const & a8, A9 const & a9)
      { return boost::thread_group::create_thread(boost::bind<void>(Policy(), boost::protect(boost::bind<void>(f, a1, a2, a4, a4, a5, a6, a7, a8, a9)))); }
   };

}}

#endif
