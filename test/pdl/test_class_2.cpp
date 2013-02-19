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

#include <boost/lexical_cast.hpp>

#include "test_class_2.h"

namespace { so_load_watcher lw("test_module2"); }

int my_test_class1::inst = 1;
int my_test_class2::inst = 1;

my_test_class1::my_test_class1()
   : m_inst(boost::lexical_cast<std::string>(inst++))
{
   pdl_test_event("ctor test_module2:my_test_class1 " + m_inst);
}

my_test_class1::~my_test_class1()
{
   pdl_test_event("dtor test_module2:my_test_class1 " + m_inst);
}

int my_test_class1::do_it()
{
   return 3;
}

my_test_class2::my_test_class2()
   : m_inst(boost::lexical_cast<std::string>(inst++)), m_ctr(0)
{
   pdl_test_event("ctor test_module2:my_test_class2 " + m_inst);
}

my_test_class2::~my_test_class2()
{
   pdl_test_event("dtor test_module2:my_test_class2 " + m_inst);
}

int my_test_class2::do_it()
{
   return 4 + 10*m_ctr++;
}

PDL_EXPORT_DYNAMIC_CLASS(my_test_class1)
PDL_EXPORT_DYNAMIC_CLASS(my_test_class2)
