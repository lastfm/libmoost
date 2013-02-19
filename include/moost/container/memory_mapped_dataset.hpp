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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_HPP__

/**
 * \file memory_mapped_dataset.hpp
 *
 * The moost::container::memory_mapped_dataset class help constructing
 * custom datasets that can be easily mapped into memory.
 *
 * There is currently support for vectors of POD types and collections
 * of serialiseable types through the help of boost::archive.
 *
 * Each vector or collection is represented by its own section in the
 * dataset and each dataset can contain an arbitrary number of sections.
 *
 * Support for other section types can be added in the future.
 */

#include "memory_mapped_dataset/vector.hpp"
#include "memory_mapped_dataset/archive.hpp"
#include "memory_mapped_dataset/hash_multimap.hpp"
#include "memory_mapped_dataset/dense_hash_map.hpp"

#endif
