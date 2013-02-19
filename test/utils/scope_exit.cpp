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

// Include boost test framework required headers
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

// Include CRT/STL required header(s)
#include <stdexcept>
#include <csignal>
#include <set>

#include <boost/bind.hpp>

// Include thrift headers

// Include application required header(s)
#include "../../include/moost/utils/scope_exit.hpp"

// Imported required namespace(s)
using namespace moost::utils;

// Name the test suite
BOOST_AUTO_TEST_SUITE( MoostUtilsScopeExitTest )

// Define the test fixture
struct Fixture
{
   // C_tor
   Fixture()
   {
   }

   // D_tor
   ~Fixture()
   {
   }
};

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_set_default_value, Fixture )
{
   bool b = false; // default is false
   {
      scope_exit::type<bool>::set_default_value se(b);
      b = true;
   }
   BOOST_CHECK(!b);

   std::sig_atomic_t sig = 0;
   {
      scope_exit::type<std::sig_atomic_t>::set_default_value se(sig);
      sig = 1;
   }
   BOOST_CHECK(0 == sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_set_default_value_cancel, Fixture )
{
   bool b = false; // default is false
   {
      scope_exit::type<bool>::set_default_value se(b);
      b = true;
      se.cancel();
   }
   BOOST_CHECK(b);

   std::sig_atomic_t sig = 0;
   {
      scope_exit::type<std::sig_atomic_t>::set_default_value se(sig);
      sig = 1;
      se.cancel();
   }
   BOOST_CHECK(1 == sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_set_specific_value, Fixture )
{
   int i = 0; // specific is false
   {
      scope_exit::type<int>::set_specific_value se(i, 10);
      i = 5;
   }
   BOOST_CHECK(i == 10);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_set_specific_value_cancel, Fixture )
{
   int i = 0; // specific is false
   {
      scope_exit::type<int>::set_specific_value se(i, 10);
      i = 5;
      se.cancel();
   }
   BOOST_CHECK(i == 5);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_restore_original_value, Fixture )
{
   bool b = true;
   {
      scope_exit::type<bool>::restore_original_value se(b);
      b = false;
   }
   BOOST_CHECK(b);

   std::sig_atomic_t sig = 1;
   {
      scope_exit::type<std::sig_atomic_t>::restore_original_value se(sig);
      sig = 0;
   }
   BOOST_CHECK(1 == sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_restore_original_value_cancel, Fixture )
{
   bool b = true;
   {
      scope_exit::type<bool>::restore_original_value se(b);
      b = false;
      se.cancel();
   }
   BOOST_CHECK(!b);

   std::sig_atomic_t sig = 1;
   {
      scope_exit::type<std::sig_atomic_t>::restore_original_value se(sig);
      sig = 0;
      se.cancel();
   }
   BOOST_CHECK(0 == sig);
}

namespace
{
   struct Scoped
   {
      Scoped (std::sig_atomic_t sig) : sig(sig)
      {
      }

      void operator()()
      {
         sig = 0;
      }

      std::sig_atomic_t sig;
   };
}


BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_member_function_default_, Fixture )
{
   Scoped scoped = 0;
   {
      scope_exit::type<Scoped>::call_member_function se(scoped);
      scoped.sig = 1;
   }
   BOOST_CHECK(0 == scoped.sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_member_function_default_cancel, Fixture )
{
   Scoped scoped = 0;
   {
      scope_exit::type<Scoped>::call_member_function se(scoped);
      scoped.sig = 1;
      se.cancel();
   }
   BOOST_CHECK(1 == scoped.sig);
}

namespace
{
   struct ScopedReset
   {
      ScopedReset (std::sig_atomic_t sig) : sig(sig)
      {
      }

      void reset()
      {
         sig = 0;
      }

      std::sig_atomic_t sig;
   };
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_member_function, Fixture )
{
   ScopedReset scoped = 0;
   {
      scope_exit::type<ScopedReset>::call_member_function se(scoped, &ScopedReset::reset);
      scoped.sig = 1;
   }
   BOOST_CHECK(0 == scoped.sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_member_function_cancel, Fixture )
{
   ScopedReset scoped = 0;
   {
      scope_exit::type<ScopedReset>::call_member_function se(scoped, &ScopedReset::reset);
      scoped.sig = 1;
      se.cancel();
   }
   BOOST_CHECK(1 == scoped.sig);
}

namespace
{
   struct ScopedPod
   {
      std::sig_atomic_t sig;
   };

   void scope_reset(ScopedPod & scoped)
   {
      scoped.sig = 0;
   }

   struct scope_reset_functor
   {
      void operator() (ScopedPod & scoped) const
      {
         scoped.sig = 0;
      }
   };
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_free_function_with_ref, Fixture )
{
   ScopedPod scoped = { 0 };
   {
      scope_exit::type<ScopedPod>::call_free_function_with_ref se(scoped, scope_reset);
      scoped.sig = 1;
   }
   BOOST_CHECK(0 == scoped.sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_free_function_with_ref_cancel, Fixture )
{
   ScopedPod scoped = { 0 };
   {
      scope_exit::type<ScopedPod>::call_free_function_with_ref se(scoped, scope_reset);
      scoped.sig = 1;
      se.cancel();
   }
   BOOST_CHECK(1 == scoped.sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_free_functor_with_ref, Fixture )
{
   ScopedPod scoped = { 0 };
   {
      scope_exit::type<ScopedPod>::call_free_function_with_ref se(scoped, scope_reset_functor());
      scoped.sig = 1;
   }
   BOOST_CHECK(0 == scoped.sig);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_free_functor_with_ref_cancel, Fixture )
{
   ScopedPod scoped = { 0 };
   {
      scope_exit::type<ScopedPod>::call_free_function_with_ref se(scoped, scope_reset_functor());
      scoped.sig = 1;
      se.cancel();
   }
   BOOST_CHECK(1 == scoped.sig);
}

namespace
{
   int global = 0;

   void set_global(int i_)
   {
      global = i_;
   }

   struct set_global_functor
   {
      void operator()(int i_)
      {
         global = i_;
      }
   };
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_free_function_with_val, Fixture )
{
   global = 0;
   {
      scope_exit::type<int>::call_free_function_with_val se(10, set_global);
      BOOST_CHECK(0 == global);
   }

   BOOST_CHECK(10 == global);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_free_function_with_val_cancel, Fixture )
{
   global = 0;
   {
      scope_exit::type<int>::call_free_function_with_val se(10, set_global);
      BOOST_CHECK(0 == global);
      se.cancel();
   }

   BOOST_CHECK(0 == global);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_functor_with_val, Fixture )
{
   global = 0;
   {
      scope_exit::type<int>::call_free_function_with_val se(10, set_global_functor());
      BOOST_CHECK(0 == global);
   }

   BOOST_CHECK(10 == global);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_functor_with_val_cancel, Fixture )
{
   global = 0;
   {
      scope_exit::type<int>::call_free_function_with_val se(10, set_global_functor());
      BOOST_CHECK(0 == global);
      se.cancel();
   }

   BOOST_CHECK(0 == global);
}

namespace
{
   struct functor
   {
      functor(int & x): x_(x) {}

      void member_func()
      {
         x_ = 10;
      }

      int & x_;
   };
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_functor, Fixture )
{
   int x = 0;

   global = 0;
   {
      scope_exit::call_functor se(boost::bind(&functor::member_func, functor(x)));
      BOOST_CHECK(0 == x);
   }

   BOOST_CHECK(10 == x);
}

BOOST_FIXTURE_TEST_CASE( test_utils_scope_exit_call_functor_cancel, Fixture )
{
   int x = 0;

   global = 0;
   {
      scope_exit::call_functor se(boost::bind(&functor::member_func, functor(x)));
      BOOST_CHECK(0 == x);
      se.cancel();
   }

   BOOST_CHECK(0 == x);
}


// Define end of test suite
BOOST_AUTO_TEST_SUITE_END()
