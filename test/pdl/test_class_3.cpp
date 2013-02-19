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

#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include "test_class_3.h"

extern "C" void pdl_test_c3_load();
extern "C" void pdl_test_c3_unload();
extern "C" void pdl_test_c3_ctor();
extern "C" void pdl_test_c3_dtor();

namespace
{
   struct watcher
   {
      watcher() { pdl_test_c3_load(); }
      ~watcher() { pdl_test_c3_unload(); }
   };

   watcher w;
}

my_test_class::my_test_class()
{
   pdl_test_c3_ctor();
}

my_test_class::~my_test_class()
{
   pdl_test_c3_dtor();
}

int my_test_class::do_it()
{
   boost::this_thread::sleep(boost::posix_time::microseconds(1350));
   return 0;
}

PDL_EXPORT_DYNAMIC_CLASS(my_test_class)
