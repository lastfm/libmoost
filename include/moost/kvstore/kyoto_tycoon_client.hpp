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
/// Client class for Kyoto Tycoon key value stores.
/// Provides methods to allow reading and writing of
/// arbitrary C++ types.
/// User classes can then treat this Client as a policy class
/// e.g.:
///
/// template <typename ClientType>
/// class FooStore
/// {
/// public:
///    bool get(typename ClientType::connection_type& conn, const std::string& key, std::vector<Foo>& foos) const
///    {
///       return ClientType(conn).get(key, foos);
///    }
///    // etc.
/// };

#ifndef MOOST_KVSTORE_KYOTO_TYCOON_CLIENT_HPP
#define MOOST_KVSTORE_KYOTO_TYCOON_CLIENT_HPP

#include <stdexcept>
#include <vector>

#include "i_kyoto_tycoon_connection.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pod.hpp>

namespace moost { namespace kvstore {

class KyotoTycoonClient
{
public:
   typedef IKyotoTycoonConnection connection_type;

private:
   connection_type& m_conn;

public:
   KyotoTycoonClient(connection_type& conn)
     : m_conn(conn)
   {
   }

   // vector values, returns false if key not found, throws if data alignment is wrong
   template <typename TKey, typename TVal>
   bool get(const TKey& key, std::vector<TVal>& val) const
   {
      return get(&key, sizeof(TKey), val);
   }

   // generic values, returns false if key not found, throws if value is wrong size
   template <typename TKey, typename TVal>
   bool get(const TKey& key, TVal& val) const
   {
      return get(&key, sizeof(TKey), val);
   }

   // override for string key
   template <typename TVal>
   bool get(const std::string& key, TVal& val) const
   {
      return get(key.data(), key.size(), val);
   }

   // override for string key
   template <typename TVal>
   bool get(const std::string& key, std::vector<TVal>& val) const
   {
      return get(key.data(), key.size(), val);
   }

private:

   // vector values, returns false if key not found, throws if data alignment is wrong
   template <typename TVal>
   bool get(const void* pkey, size_t ksize, std::vector<TVal>& val) const
   {
      size_t vsize;
      boost::shared_array<char> sa = m_conn.get(pkey, ksize, vsize);
      TVal *pv = reinterpret_cast<TVal *>(sa.get());
      bool found = (0 != pv);

      if (found)
      {
         if (vsize % sizeof(TVal) == 0)
         {
            val.assign(pv, pv + vsize/sizeof(TVal));
         }
         else
         {
            throw std::runtime_error("Retrieved value has incorrect data alignment");
         }
      }

      return found;
   }

   // generic values, returns false if key not found, throws if value is wrong size
   template <typename TVal>
   bool get(const void* pkey, size_t ksize, TVal& val) const
   {
      size_t vsize;
      boost::shared_array<char> sa = m_conn.get(pkey, ksize, vsize);
      TVal *pv = reinterpret_cast<TVal *>(sa.get());
      bool found = (0 != pv);

      if (found)
      {
         if (vsize == sizeof(TVal))
         {
            val = *pv;
         }
         else
         {
            throw std::runtime_error("Retrieved value has incorrect size");
         }
      }

      return found;
   }

public:

   // vector values, throws on failure
   template <typename TKey, typename TVal>
   void set(const TKey& key, const std::vector<TVal>& val) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TKey>::value));
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.set(&key, sizeof(key), &val[0], sizeof(TVal) * val.size());
   }

   // generic values, throws on failure
   template <typename TKey, typename TVal>
   void set(const TKey& key, const TVal& val) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TKey>::value));
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.set(&key, sizeof(key), &val, sizeof(val));
   }

   // vector values, throws on failure
   template <typename TKey, typename TVal>
   void cache(const TKey& key, const std::vector<TVal>& val, boost::int64_t expirySecs) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TKey>::value));
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.cache(&key, sizeof(key), &val[0], sizeof(TVal) * val.size(), expirySecs);
   }

   // generic values, throws on failure
   template <typename TKey, typename TVal>
   void cache(const TKey& key, const TVal& val, boost::int64_t expirySecs) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TKey>::value));
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.cache(&key, sizeof(key), &val, sizeof(val), expirySecs);
   }

   // vector values, throws on failure (std::string key override)
   template <typename TVal>
   void set(const std::string& key, const std::vector<TVal>& val) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.set(key.data(), key.size(), &val[0], sizeof(TVal) * val.size());
   }

   // generic values, throws on failure (std::string key override)
   template <typename TVal>
   void set(const std::string& key, const TVal& val) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.set(key.data(), key.size(), &val, sizeof(val));
   }

   // vector values, throws on failure (std::string key override)
   template <typename TVal>
   void cache(const std::string& key, const std::vector<TVal>& val, boost::int64_t expirySecs) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.cache(key.data(), key.size(), &val[0], sizeof(TVal) * val.size(), expirySecs);
   }

   // generic values, throws on failure (std::string key override)
   template <typename TVal>
   void cache(const std::string& key, const TVal& val, boost::int64_t expirySecs) const
   {
      BOOST_STATIC_ASSERT((boost::is_pod<TVal>::value));

      m_conn.cache(key.data(), key.size(), &val, sizeof(val), expirySecs);
   }
};

}}  // end namespace

#endif
