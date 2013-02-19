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

/// \file
/// Connection interface for a Kyoto Tycoon key value store.

#ifndef MOOST_KVSTORE_KYOTO_TYCOON_CONNECTION_INTERFACE_H
#define MOOST_KVSTORE_KYOTO_TYCOON_CONNECTION_INTERFACE_H

#include <boost/shared_array.hpp>
#include <boost/cstdint.hpp>

#include "connection.h"

namespace moost { namespace kvstore {

   class IKyotoTycoonConnection : public IConnection
   {
   public:

      virtual ~IKyotoTycoonConnection() { }

   public:

      // native store methods

      // return value is a C-style string of length vsize
      // (not including the trailing null byte)
      virtual boost::shared_array<char> get(const void* pkey, size_t ksize, size_t& vsize) const = 0;

      // throws on failure
      virtual void set(const void* pkey, size_t ksize, const void* pval, size_t vsize) const = 0;

      // throws on failure
      virtual void cache(const void* pkey, size_t ksize, const void* pval, size_t vsize, boost::int64_t expirySecs) const = 0;
   };

}}  // end namespace

#endif
