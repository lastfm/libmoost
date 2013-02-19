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

#include "../../include/moost/kvstore/kyoto_tycoon_connection.h"

// below is just the linux specific implementation

#ifndef WIN32

#if defined (_STDINT_H)
#define STDINT_H_3C996A98_ABAB_4d71_AC4B_633055150696
#undef _STDINT_H
#endif

#include <ktremotedb.h>

#ifdef STDINT_H_3C996A98_ABAB_4d71_AC4B_633055150696
#undef STDINT_H_3C996A98_ABAB_4d71_AC4B_633055150696
#if !defined (_STDINT_H)
#define _STDINT_H
#endif
#else
#undef _STDINT_H
#endif

namespace moost { namespace kvstore { namespace detail {

   KyotoTycoonConnection::~KyotoTycoonConnection()
   {
      try
      {
         close();
      }
      catch(...)
      {
         // ignore
      }
   }

   void KyotoTycoonConnection::open(const std::string& host, int port, int timeoutMs)
   {
      if (isOpen_)
      {
         throw std::runtime_error("The connection is already open");
      }

      if (!pDb_)
      {
         pDb_.reset(new kyototycoon::RemoteDB);
      }

      isOpen_ = pDb_->open(host, port, timeoutMs / 1000.0f);

      if (!isOpen_)
      {
         std::ostringstream oss;
         oss << "Couldn't open connection to " << host << ":" << port;
         throw_kt_exception(oss.str());
      }
   }

   void KyotoTycoonConnection::close()
   {
      if(isOpen_)
      {
         if (!pDb_->close())
            throw_kt_exception("Failed to close remote datastore");
         isOpen_ = false;
      }
   }

   boost::shared_array<char> KyotoTycoonConnection::get(const void* pkey, size_t ksize, size_t& vsize) const
   {
      assert_data_store_open();
      return boost::shared_array<char>(pDb_->get(reinterpret_cast<const char *>(pkey), ksize, &vsize));
   }

   void KyotoTycoonConnection::set(const void* pkey, size_t ksize, const void* pval, size_t vsize) const
   {
      assert_data_store_open();
      if (!pDb_->set(reinterpret_cast<const char *>(pkey), ksize, reinterpret_cast<const char *>(pval), vsize))
      {
         throw_kt_exception("Failed to set in remote datastore");
      }
   }

   void KyotoTycoonConnection::cache(const void* pkey, size_t ksize, const void* pval, size_t vsize, int64_t expirySecs) const
   {
      assert_data_store_open();
      if (!pDb_->set(reinterpret_cast<const char *>(pkey), ksize, reinterpret_cast<const char *>(pval), vsize, expirySecs))
      {
         throw_kt_exception("Failed to cache in remote datastore");
      }
   }

   void KyotoTycoonConnection::throw_kt_exception(const std::string& msg) const
   {
      throw std::runtime_error(msg + ": " + pDb_->error().name() + " (" + pDb_->error().message() + ")");
   }

   void KyotoTycoonConnection::assert_data_store_open() const
   {
      assert(isOpen_);
      if (!isOpen_)
         throw std::runtime_error("Kyoto Tycoon connection not open");
   }

}}}  // end namespace

#endif
