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
/// Memory bound key/value store provides a ikvds interface for a simple key/value store memory bound container.
/// The backing container is specified as a template parameter but MUST model the same interface as a STL [multi]map
/// and the key and value types MUST be of type kvds::byte_array_t as they're used as a byte buffer for storage.
/// To ensure this is the case the types are statically asserted. Unfortunately, we can't use a template template type
/// because this would be far to restrictive as different container types have different template signatures!
/// The backing container must be default constructable but can be configured post contruction.

/// NB. The main purpose of this implementation is to allow code that uses a
///     platform specific k/v store to still compile and run on other platforms
///     for debugging purposes. It is not really aimed at production use (hence
///     is functional but not necessarily efficient -- this is by design!!!

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <map>

#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/cast.hpp>

#include "ikvds.hpp"

#ifndef MOOST_KVDS_KVDSMEM_HPP__
#define MOOST_KVDS_KVDSMEM_HPP__

namespace moost { namespace kvds {

   typedef std::vector<char> byte_array_t;

   /// *** This class is NOT thread safe ***

   template <typename T>
   class KvdsMem : public IKvds
   {
   public:
      typedef T store_type;
      typedef typename T::key_type key_type;
      typedef typename T::mapped_type val_type;

      KvdsMem() : itr_(store_.end()){}
      ~KvdsMem()
      {
         if(!dsname_.empty())
         {
            close();
         }
      }

      void open (char const dsname [], bool newdb = false)
      {
         if(!dsname_.empty()) { throw std::runtime_error("The store is already open"); }

         dsname_ = dsname;

         if(newdb)
         {
            // Truncate database if it already exists
            std::ofstream(dsname_.c_str(), std::ios::binary | std::ios::trunc);
         }
         else
         {
            std::ifstream in(dsname_.c_str(), std::ios::binary);
            in.exceptions(std::ios::badbit);

            try
            {
               if(in.is_open()) // Failure to open is fine, this could be a new store
               {
                  key_type key;

                  while(in)
                  {
                     size_t key_size = 0;
                     in.read(reinterpret_cast<char *>(&key_size), sizeof(key_size));

                     if(key_size > 0)
                     {
                        key.resize(key_size / sizeof(typename key_type::value_type));
                        in.read(reinterpret_cast<char *>(&key[0]), boost::numeric_cast<std::streamsize>(key_size));

                        size_t val_size = 0;
                        in.read(reinterpret_cast<char *>(&val_size), sizeof(val_size));

                        if(val_size > 0)
                        {
                           val_type & val = store_[key];
                           val.resize(val_size / sizeof(typename val_type::value_type));

                           if(in)
                           {
                              in.read(reinterpret_cast<char *>(&val[0]), boost::numeric_cast<std::streamsize>(val_size));
                           }
                        }
                     }
                  }
               }
            }
            catch(...)
            {
               dsname_.clear(); // On close, ensure we don't save bad data and cause potential corruption!
               throw;
            }
         }
      }


      void save()
      {
         if(!dsname_.empty())
         {
            std::ofstream out(dsname_.c_str(), std::ios::binary);
            out.exceptions(std::ios::badbit | std::ios::failbit);

            for(typename store_type::const_iterator itr = store_.begin() ; itr != store_.end() ; ++itr)
            {
               size_t key_size = itr->first.size();
               out.write(reinterpret_cast<char const *>(&key_size), sizeof(key_size));
               out.write(reinterpret_cast<char const *>(&itr->first[0]), boost::numeric_cast<std::streamsize>(key_size));

               size_t val_size = itr->second.size();
               out.write(reinterpret_cast<char const *>(&val_size), sizeof(val_size));
               out.write(reinterpret_cast<char const *>(&itr->second[0]), boost::numeric_cast<std::streamsize>(val_size));
            }

            dsname_.clear();
         }
      }

      void close()
      {
         save();
      }

   private:
      std::string dsname_;

   private:
      /// If the key/val types are not byte_array_t types fail compilation
      BOOST_STATIC_ASSERT((boost::is_same<key_type, byte_array_t>::value));
      BOOST_STATIC_ASSERT((boost::is_same<val_type, byte_array_t>::value));

   public:
      /// You can call this to get a refernce to the underlying store. Useful
      /// if you need to do post construction configuration. For example if using
      /// a google hash or spare map you need to specify the delete/erase keys
      store_type & get_store() { return store_; }

   public: /// IKvds interface implementation
      bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         key_type key(ksize);
         memcpy (&key[0], pkey, ksize);

         val_type & vt = store_[key];

         vt.resize(vsize);
         memcpy (&vt[0], pval, vsize);

         return true;
      }

      bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         key_type key(ksize);
         memcpy (&key[0], pkey, ksize);

         typename store_type::const_iterator itr = store_.find(key);

         bool found = itr != store_.end();

         if(found)
         {
            vsize = std::min(vsize, itr->second.size());
            memcpy(pval, &itr->second[0], vsize);
         }
         else { vsize = 0; }

         return found;
      }

      bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         key_type key(ksize);
         memcpy (&key[0], pkey, ksize);

         val_type & vt = store_[key];

         size_t const osize = vt.size();
         vt.resize(osize + vsize);
         memcpy (&vt[osize], pval, vsize);

         return true;
      }

      bool all(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         bool found = false;
         size_t esize = 0;

         if(siz(pkey, ksize, esize))
         {
            if(esize <= vsize)
            {
               found = get(
                  pkey, ksize,
                  pval, esize
                  );
            }

            vsize = esize;
         }
         else { vsize = 0; }

         return found;
      }

      bool xst(
         void const * pkey, size_t const ksize
         )
      {
         key_type key(ksize);
         memcpy (&key[0], pkey, ksize);
         return store_.find(key) != store_.end();
      }

      bool del(
         void const * pkey, size_t const ksize
         )
      {
         bool removed = false;

         key_type key(ksize);
         memcpy (&key[0], pkey, ksize);

         typename store_type::iterator itr = store_.find(key);

         if(itr != store_.end())
         {
            store_.erase(itr);
            removed = true;
         }

         return removed;
      }

      bool clr()
      {
         store_.clear();
         return true;
      }

      bool beg()
      {
         itr_ = store_.begin();
         return true;
      }

      bool nxt(
         void * pkey, size_t & ksize
         )
      {
         bool found = false;

         if(itr_ != store_.end())
         {
            size_t const size = itr_->first.size();

            if(size <= ksize)
            {
               memcpy(pkey, &itr_->first[0], size);
               ++itr_;
               found = true;
            }

            ksize = size;
         }
         else
         {
            ksize = 0;
         }

         return found;
      }

      bool end()
      {
         return itr_ == store_.end();
      }

      bool siz(
         void const * pkey, size_t const ksize,
         size_t & vsize
         )

      {
         key_type key(ksize);
         memcpy (&key[0], pkey, ksize);

         typename store_type::const_iterator itr = store_.find(key);

         bool found = itr != store_.end();

         if(itr != store_.end())
         {
            vsize = itr->second.size();
         }

         return found;
      }

      bool cnt(boost::uint64_t & cnt)
      {
         cnt = store_.size();
         return true;
      }

      bool nil(bool & isnil)
      {
         isnil = store_.empty();
         return true;
      }

   private:
      store_type store_;
      typename store_type::const_iterator itr_;

   };

   /// This is defined for convenience but feel free to use your own container!
   typedef KvdsMem<std::map<byte_array_t, byte_array_t> > KvdsMemMap;

}}

#endif /// MOOST_KVDS_KVDSMEM_HPP__
