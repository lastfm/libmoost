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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_POD_PAIR_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_POD_PAIR_HPP__

#include "config.hpp"

namespace moost { namespace container {

/**
 * Helper struct for memory-mapped map types
 *
 * Unfortunately, std::pair<> itself isn't POD even when used with only
 * POD types. Thus we need this std::pair<> look-a-like that is POD.
 */
template <typename T1, typename T2>
struct pod_pair
{
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<T1>::value, "pod_pair<> template can only handle POD types (T1)");
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<T2>::value, "pod_pair<> template can only handle POD types (T2)");

   T1 first;
   T2 second;
};

}}

#endif
