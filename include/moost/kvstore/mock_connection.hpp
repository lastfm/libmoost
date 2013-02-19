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
/// Mock key value store connections for testing.
/// Doesn't persist anything but multiple mock connections
/// will access the same underlying singleton in-memory store.

#ifndef FM_LAST_KVSTORE_MOCK_CONNECTION_HPP
#define FM_LAST_KVSTORE_MOCK_CONNECTION_HPP

#include "i_kyoto_tycoon_connection.h"

#include <string>
#include <vector>
#include <map>
#include <ctime>

#include <boost/thread/mutex.hpp>

namespace moost { namespace kvstore {

class DefaultAccessPolicy
{
public:
   template <class StoreT>
   static boost::shared_array<char> get(StoreT& store, const char* pkey, size_t ksize, size_t& vsize)
   {
      std::string key(pkey, ksize);
      return store.get(key, vsize);
   }

   // throws on failure, supply key "fail" to force failure
   template <class StoreT>
   static void set(StoreT& store, const char* pkey, size_t ksize, const char* pval, size_t vsize)
   {
      std::string key(pkey, ksize);
      store.set(key, pval, vsize);
   }

   // throws on failure, supply key "fail" to force failure
   template <class StoreT>
   static void cache(StoreT& store, const char* pkey, size_t ksize, const char* pval, size_t vsize, int expiryTime)
   {
      std::string key(pkey, ksize);
      store.cache(key, pval, vsize, expiryTime);
   }
};

template <class AccessPolicy = DefaultAccessPolicy>
class MockKyotoTycoonConnection : public IKyotoTycoonConnection
{
public:

   MockKyotoTycoonConnection() : isOpen_(false) { }

   ~MockKyotoTycoonConnection() { close(); }

   void open(const std::string& /*host*/, int /*port*/, int /*timeoutMs*/)
   {
      if (isOpen_) { throw std::runtime_error("The connection is already open"); }
      // initialize singleton
      store();
      isOpen_ = true;
   }

   void close()
   {
      isOpen_ = false;
   }

public:

   // if key is found then return value is a C-style string
   // of length vsize (not including the trailing null byte)
   boost::shared_array<char> get(const void* pkey, size_t ksize, size_t& vsize) const
   {
      assert_data_store_open();
      return AccessPolicy::get(store(), reinterpret_cast<const char *>(pkey), ksize, vsize);
   }

   // throws on failure, supply key "fail" to force failure
   virtual void set(const void* pkey, size_t ksize, const void* pval, size_t vsize) const
   {
      assert_data_store_open();
      AccessPolicy::set(store(), reinterpret_cast<const char *>(pkey), ksize, reinterpret_cast<const char *>(pval), vsize);
   }

   // throws on failure, supply key "fail" to force failure
   virtual void cache(const void* pkey, size_t ksize, const void* pval, size_t vsize, boost::int64_t expirySecs) const
   {
      assert_data_store_open();
      boost::int64_t expiryTime = time(0) + expirySecs;
      AccessPolicy::cache(store(), reinterpret_cast<const char *>(pkey), ksize, reinterpret_cast<const char *>(pval), vsize, expiryTime);
   }

private:

   void assert_data_store_open() const
   {
      assert(isOpen_);
      if (!isOpen_)
         throw std::runtime_error("Mock Kyoto Tycoon connection not open");
   }

private:

   class Store
   {
   private:

      typedef std::vector<char> entry_t;
      typedef std::map<std::string, entry_t> store_t;
      typedef std::map<std::string, time_t> expiry_store_t;

      mutable boost::mutex mutex_;  // synchronize access to store and expiry store
      store_t db_;
      expiry_store_t expiry_;

   public:

      // return a copy of the stored char*
      // or 0 if not found
      boost::shared_array<char> get(const std::string& key, size_t& vsize) const
      {
         boost::shared_array<char> val;
         vsize = 0;
         time_t expiry;

         boost::mutex::scoped_lock lock(mutex_);

         if (!get(key, expiry, expiry_) || expiry >= time(0))
         {
            entry_t entry;
            if (get(key, entry, db_))
            {
               vsize = entry.size();
               val.reset(new char[vsize+1]);
               memcpy(val.get(), &entry[0], entry.size());
               val[vsize] = 0;
            }
         }

         return val;
      }

      void set(const std::string& key, const char* val, size_t vsize)
      {
         boost::mutex::scoped_lock lock(mutex_);
         db_[key].assign(val, val + vsize);
         // remove any existing expiry time for key
         expiry_store_t::iterator it = expiry_.find(key);
         if (it != expiry_.end())
            expiry_.erase(it);
      }

     void cache(const std::string& key, const char* val, size_t vsize, boost::int64_t expirySecs)
      {
         boost::mutex::scoped_lock lock(mutex_);
         db_[key].assign(val, val + vsize);
         expiry_[key] = time(0) + static_cast<time_t>(expirySecs);
      }

   private:

      template <typename TKey, typename TVal, typename TMap>
      bool get(const TKey& key, TVal& val, TMap& store) const
      {
         typename TMap::const_iterator it = store.find(key);
         bool found = (it != store.end());
         if (found)
            val = it->second;
         return found;
      }
   };

   bool isOpen_;

   // singleton store
   static Store& store()
   {
      static Store store_;
      return store_;
   }
};

}}  // end namespace

#endif
