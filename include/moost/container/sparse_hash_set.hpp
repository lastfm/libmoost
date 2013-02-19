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

#ifndef MOOST_CONTAINER_SPARSE_HASH_SET_HPP__
#define MOOST_CONTAINER_SPARSE_HASH_SET_HPP__

#if defined (_WIN32) && defined (WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN_3C996A98_ABAB_4d71_AC4B_633055150696
#undef WIN32_LEAN_AND_MEAN
#endif

#include <google/sparse_hash_set>

namespace moost { namespace container {
   using google::sparse_hash_set;
}}

#ifdef _WIN32
#ifdef WIN32_LEAN_AND_MEAN_3C996A98_ABAB_4d71_AC4B_633055150696
#undef WIN32_LEAN_AND_MEAN_3C996A98_ABAB_4d71_AC4B_633055150696
#define WIN32_LEAN_AND_MEAN
#else
#undef WIN32_LEAN_AND_MEAN
#endif
#endif

#endif // MOOST_CONTAINER_SPARSE_HASH_SET_HPP__
