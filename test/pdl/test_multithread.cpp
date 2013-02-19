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
#include <vector>
#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "../../include/moost/pdl/dynamic_library.h"
#include "../../include/moost/safe_shared_ptr.hpp"

#include "test_interface.h"

BOOST_AUTO_TEST_SUITE(moost_pdl_multithread)

namespace
{

// mmmmh, maybe we can turn this into a stand-alone template?
template <typename T>
class atomic
{
public:
   atomic()
      : m_value(0)
   {
   }

   atomic& operator++ ()
   {
      atomic_add_and_fetch(1);
      return *this;
   }

   T read()
   {
      return atomic_add_and_fetch(0);
   }

private:
   volatile T m_value;

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
   T atomic_add_and_fetch(T x)
   {
      return __sync_add_and_fetch(&m_value, x);
   }
#else
   boost::mutex m_mutex;

   T atomic_add_and_fetch(T x)
   {
      boost::mutex::scoped_lock lock(m_mutex);
      m_value += x;
      return m_value;
   }
#endif
};

atomic<unsigned> c3_load;
atomic<unsigned> c3_unload;
atomic<unsigned> c3_ctor;
atomic<unsigned> c3_dtor;

atomic<unsigned> c4_load;
atomic<unsigned> c4_unload;
atomic<unsigned> c4_ctor;
atomic<unsigned> c4_dtor;

class multithread_pdl_test
{
public:
   multithread_pdl_test(const std::vector<std::string>& libs)
      : m_finish(0)
      , m_lib_counter(0)
      , m_libs(libs)
   {
      reload(true);
   }

   void reload(bool force)
   {
      if (force)
      {
         m_dl.open(m_libs[m_lib_counter++ % m_libs.size()]);
      }

      m_inst = m_dl.create<my_test_interface>("my_test_class");
   }

   void process()
   {
      boost::shared_ptr<my_test_interface> inst = m_inst.get_shared();

      for (size_t i = 0; i < 10; ++i)
      {
         inst->do_it();
      }
   }

   void processor_thread()
   {
      while (!m_finish)
      {
         process();
      }
   }

   void run(size_t num_threads = 37)
   {
      boost::thread_group tg;

      for (size_t i = 0; i < num_threads; ++i)
      {
         tg.create_thread(boost::bind(&multithread_pdl_test::processor_thread, this));
      }

      for (size_t i = 0; i < 2000; ++i)
      {
         boost::this_thread::sleep(boost::posix_time::microseconds(500));
         reload(i % 5 == 0);
      }

      m_finish = 1;

      tg.join_all();
   }

private:
   volatile sig_atomic_t m_finish;
   size_t m_lib_counter;
   std::vector<std::string> m_libs;
   moost::pdl::dynamic_library m_dl;
   moost::safe_shared_ptr<my_test_interface> m_inst;
};

}

extern "C" void pdl_test_c3_load() { ++c3_load; }
extern "C" void pdl_test_c3_unload() { ++c3_unload; }
extern "C" void pdl_test_c3_ctor() { ++c3_ctor; }
extern "C" void pdl_test_c3_dtor() { ++c3_dtor; }

extern "C" void pdl_test_c4_load() { ++c4_load; }
extern "C" void pdl_test_c4_unload() { ++c4_unload; }
extern "C" void pdl_test_c4_ctor() { ++c4_ctor; }
extern "C" void pdl_test_c4_dtor() { ++c4_dtor; }

BOOST_AUTO_TEST_CASE(pdl_thread)
{
   // Check behaviour in a multi-threaded environment

   std::vector<std::string> libs;

   libs.push_back("../lib/test_module3");
   libs.push_back("../lib/test_module4");

   multithread_pdl_test(libs).run();

   BOOST_CHECK(c3_load.read() > 0);
   BOOST_CHECK(c3_ctor.read() > 0);

   BOOST_CHECK(c4_load.read() > 0);
   BOOST_CHECK(c4_ctor.read() > 0);

   BOOST_CHECK_EQUAL(c3_load.read(), c3_unload.read());
   BOOST_CHECK_EQUAL(c3_ctor.read(), c3_dtor.read());

   BOOST_CHECK_EQUAL(c4_load.read(), c4_unload.read());
   BOOST_CHECK_EQUAL(c4_ctor.read(), c4_dtor.read());
}

BOOST_AUTO_TEST_SUITE_END()
