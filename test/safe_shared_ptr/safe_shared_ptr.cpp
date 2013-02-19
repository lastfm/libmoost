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

#include <csignal>

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "../../include/moost/safe_shared_ptr.hpp"

using namespace moost;

namespace {

class deref_helper
{
public:
   class impl
   {
   public:
      impl(size_t num)
         : m_num(num)
         , m_accu(new sig_atomic_t(0))
      {
      }

      void call(size_t no)
      {
         boost::this_thread::sleep(boost::posix_time::milliseconds(5));
         *m_accu += m_num + no;
         boost::this_thread::sleep(boost::posix_time::milliseconds(5));
         BOOST_CHECK(m_accu);
      }

      sig_atomic_t read() const
      {
         return *m_accu;
      }

   private:
      const size_t m_num;
      boost::shared_ptr<volatile sig_atomic_t> m_accu;
   };

   deref_helper()
      : m_impl(new impl(0))
      , m_num(0)
   {
   }

   void run_thread()
   {
      boost::thread(boost::bind(&deref_helper::run, this)).swap(m_thread);
   }

   void invalidate()
   {
      for (size_t i = 0; i < 4; ++i)
      {
         boost::this_thread::sleep(boost::posix_time::milliseconds(25));
         BOOST_CHECK(m_impl->read() >= 0);
         m_impl.reset(new impl(++m_num));
      }
   }

   void join()
   {
      m_thread.join();
   }

private:
   void run()
   {
      for (size_t i = 0; i < 10; ++i)
      {
         m_impl->call(i);
      }
   }

   safe_shared_ptr<impl> m_impl;
   boost::thread m_thread;
   size_t m_num;
};

struct int_wrapper
{
   int_wrapper(int value)
      : value(value)
   {
   }

   int value;
};

}

BOOST_AUTO_TEST_SUITE( safe_shared_ptr_test )

BOOST_AUTO_TEST_CASE( test_shared_ptr )
{
  safe_shared_ptr<int_wrapper> p(new int_wrapper(3));

  BOOST_CHECK_EQUAL(p->value, 3);
}

BOOST_AUTO_TEST_CASE( test_shared_ptr_ctors )
{
  boost::shared_ptr<int_wrapper> sp(new int_wrapper(4));

  BOOST_CHECK_EQUAL(sp.use_count(), 1);

  safe_shared_ptr<int_wrapper> p(sp);

  BOOST_CHECK_EQUAL(sp.use_count(), 2);
  BOOST_CHECK_EQUAL(p.use_count(), 2);

  safe_shared_ptr<int_wrapper> p2(p);

  BOOST_CHECK_EQUAL(sp.use_count(), 3);
  BOOST_CHECK_EQUAL(p.use_count(), 3);
  BOOST_CHECK_EQUAL(p2.use_count(), 3);

  safe_shared_ptr<int_wrapper const> p3(p);

  BOOST_CHECK_EQUAL(sp.use_count(), 4);
  BOOST_CHECK_EQUAL(p.use_count(), 4);
  BOOST_CHECK_EQUAL(p2.use_count(), 4);
  BOOST_CHECK_EQUAL(p3.use_count(), 4);

  BOOST_CHECK_EQUAL(p->value, 4);
  BOOST_CHECK_EQUAL(p2->value, 4);
  BOOST_CHECK_EQUAL(p3->value, 4);
}

BOOST_AUTO_TEST_CASE( test_shared_ptr_assign )
{
  safe_shared_ptr<int_wrapper> p(new int_wrapper(3));
  safe_shared_ptr<int_wrapper> p2;

  p2 = p;

  BOOST_CHECK_EQUAL(p2->value, 3);
  BOOST_CHECK_EQUAL(p2.use_count(), 2);

  boost::shared_ptr<int_wrapper> p3(new int_wrapper(4));

  p = p3;

  BOOST_CHECK_EQUAL(p2->value, 3);
  BOOST_CHECK_EQUAL(p->value, 4);
  BOOST_CHECK_EQUAL(p3->value, 4);
  BOOST_CHECK_EQUAL(p2.use_count(), 1);
  BOOST_CHECK_EQUAL(p2.unique(), true);
  BOOST_CHECK_EQUAL(p.use_count(), 2);
  BOOST_CHECK_EQUAL(p.unique(), false);
  BOOST_CHECK_EQUAL(p3.use_count(), 2);

  boost::shared_ptr<int_wrapper> p4 = p.get_shared();

  BOOST_CHECK_EQUAL(p.use_count(), 3);
  BOOST_CHECK_EQUAL(p4.use_count(), 3);
  BOOST_CHECK_EQUAL(p4->value, 4);
}

namespace {
  struct a {
    void x() {}
  };
  struct b : public a {
    void y() {}
  };
}

BOOST_AUTO_TEST_CASE( test_shared_ptr_compare )
{
  safe_shared_ptr<int_wrapper> p(new int_wrapper(3));
  safe_shared_ptr<int_wrapper> p2 = p;
  safe_shared_ptr<int_wrapper> p3(new int_wrapper(3));
  safe_shared_ptr<int_wrapper const> p4(p);
  safe_shared_ptr<int_wrapper const> p5(new int_wrapper(3));

  BOOST_CHECK_EQUAL(p == p2, true);
  BOOST_CHECK_EQUAL(p == p3, false);
  BOOST_CHECK_EQUAL(p == p4, true);
  BOOST_CHECK_EQUAL(p == p5, false);
  BOOST_CHECK_EQUAL(p == p, true);

  safe_shared_ptr<b> pb(new b());
  safe_shared_ptr<a> pa(pb);

  BOOST_CHECK_EQUAL(pa == pb, true);
}

BOOST_AUTO_TEST_CASE( test_shared_ptr_reset )
{
  safe_shared_ptr<int_wrapper> p(new int_wrapper(2));

  BOOST_CHECK_EQUAL(p->value, 2);

  p.reset(new int_wrapper(3));

  BOOST_CHECK_EQUAL(p->value, 3);
}

BOOST_AUTO_TEST_CASE( test_swap )
{
  safe_shared_ptr<int_wrapper> p(new int_wrapper(3));
  safe_shared_ptr<int_wrapper> p2(new int_wrapper(4));

  p2.swap(p);

  BOOST_CHECK_EQUAL(p2->value, 3);
  BOOST_CHECK_EQUAL(p->value, 4);

  p.swap(p);

  BOOST_CHECK_EQUAL(p->value, 4);
}

BOOST_AUTO_TEST_CASE( test_scoped_lock )
{
  safe_shared_ptr<int_wrapper> p(new int_wrapper(3));
  {
    safe_shared_ptr<int_wrapper>::scoped_lock sp(p);

    BOOST_CHECK_EQUAL(sp->value, 3);

    sp->value = 4;
  }
  BOOST_CHECK_EQUAL(p->value, 4);
}

BOOST_AUTO_TEST_CASE( test_deref_thread )
{
  deref_helper dh;

  dh.run_thread();
  dh.invalidate();
  dh.join();
}

BOOST_AUTO_TEST_SUITE_END()
