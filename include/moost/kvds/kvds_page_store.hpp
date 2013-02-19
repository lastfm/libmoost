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

#ifndef MOOST_KVDS_KVDS_PAGESTORE_HPP__
#define MOOST_KVDS_KVDS_PAGESTORE_HPP__

/// \file
/// Page store provides a ikvds interface for a simple key/value page store, where a value is store as a fixed size page
/// in a suitable store from a collection of store where each store has a fixed page size greater than the previous.
/// If the value being store is smaller than the page size padding is added to ensure the page persisted is complete.

#include <vector>
#include <sstream>
#include <fstream>
#include <cassert>
#include <limits>
#include <bitset>
#include <algorithm>
#include <utility>
#include <csignal>

#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cast.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/utility.hpp>

#include "../utils/bits.hpp"
#include "../algorithm/fast_hash.hpp"
#include "../container/sparse_hash_map.hpp"
#include "../container/sparse_hash_set.hpp"

#include "../serialization/hashmap_serializer.hpp"
#include "../serialization/hashset_serializer.hpp"

#include "ikvds.hpp"

/// In debug flush the stream so we can see what's happening (in release this is a noop)
#ifndef NDEBUG
#define KVDS_PAGESTORE_FLUSH_STREAM__(S) S.flush()
#else
#define KVDS_PAGESTORE_FLUSH_STREAM__(S)
#endif

/// TODO [RWC 2009-11-23]: Needs transaction logging for crash recoverly tolerance
///                      : Defrag tool to compact files with holes to save disk space
///                      : Make max value size optional (auto-allocate new page stores)

namespace moost { namespace kvds {

   typedef std::vector<char> byte_array_t;

   /// A pagemap that allows multiple pagestores to share the same pagemap. Useful if you want to store the store entity
   /// in a resource stack to make sure there are not multiple instances of the pagemap being kept in memory. Shared
   /// pagemap simply works by wrapping a pointer to an instance of a pagemap. Different stores can be assigned the
   /// same pagemap by instantiating them with this shared pagemap and then assigning them all the same pointer handle.

   /// *** This class is NOT thread safe ***
   /// NB. There is no point in making this class thread safe since the pagestore that uses it is also not threadsafe!

   template <typename pagemapT>
   class KvdsPageMapShared
   {
   public:
      typedef pagemapT pagemap_t;
      typedef typename pagemap_t::size_type size_type;
      typedef typename pagemap_t::iterator iterator;
      typedef typename pagemap_t::const_iterator const_iterator;
      typedef typename pagemap_t::pageinfo_t pageinfo_t;
      typedef typename pagemap_t::itemid_t itemid_t;
      typedef typename pagemap_t::storage_t storage_t;
      typedef KvdsPageMapShared<pagemapT> this_type;

   private:

      /// This metadata is shared by all instances that share the same pagemap.
      /// It is used to ensure only one shared pagemap is the "owner" to prevent,
      /// for example, multiple loads/saves when the collection of shared pagemaps
      /// is destroyed. Clearly, since they are all sharing the same underlying data
      /// it doesn't make sence to have it loaded/saved multiple times.
      struct metadata
      {
         typedef KvdsPageMapShared<pagemapT> shared_pagemap_t;
         typedef typename shared_pagemap_t::pagemap_t pagemap_t;
         typedef std::vector<shared_pagemap_t *> shared_pagemaps_t;

         metadata(shared_pagemap_t * pshared) :
            isdirty(0), ppagemap(new pagemap_t), pshared_pagemaps(new shared_pagemaps_t)
         {
            typename shared_pagemaps_t::iterator itr = std::find(pshared_pagemaps->begin(), pshared_pagemaps->end(), pshared);

            /// If pshared already exists something is amiss! This should just not be possible.
            /// For this reason I am going to assert it's not to happen but do nothing more to
            /// report it as an error. Instead I'll just code against adding it to the list more
            /// than once. If, later, we notice this asserting we need to figure out why and fix.

            assert(itr == pshared_pagemaps->end());

            /// We're using a vector rather than (for example) a set because the items are maintained
            /// in chronological order. Although this means the search complexity is O(N) the size of the
            /// list is unlikely to be too large (probably 10, 20 or so) so the benefits beat the cons.
            if(itr == pshared_pagemaps->end())
            {
               pshared_pagemaps->push_back(pshared);
            }
         }

         bool is_owner(shared_pagemap_t * pshared)
         {
            // the first item in the list is always considered the owner. If the owner
            // is removed (when it is destroyed) the next item will become the owner.
            return !pshared_pagemaps->empty() && *(pshared_pagemaps->begin()) == pshared;
         }



         /// Remove pshared from the list of shared pagemaps
         void remove(shared_pagemap_t * pshared)
         {
            std::remove(pshared_pagemaps->begin(), pshared_pagemaps->end(), pshared);
         }

         /// If this flag is set we need to perform a save
         volatile std::sig_atomic_t isdirty;

         /// This is the actual pagemap entity we are proxying.
         /// NB. created on the heap so we access indirectly (via a pointer) to prevent issues with x-thread optimisation
         boost::shared_ptr<pagemap_t> ppagemap;

         /// This is a chronoligically ordered list of users of this map, the first (begin) being the owner
         /// NB. created on the heap so we access indirectly (via a pointer) to prevent issues with x-thread optimisation
         boost::shared_ptr<shared_pagemaps_t>  pshared_pagemaps;
      };

      boost::shared_ptr<metadata> pmetadata_;

   public:

      ~KvdsPageMapShared()
      {
         if(pmetadata_)
         {
            pmetadata_->remove(this);
         }
      }

      /// take metadata from another shared pagemap
      void get_metadata_from(this_type const & rhs)
      {
         // make sure there is something to share
         rhs.validate_metadata_exists();

         // it's good to share
         pmetadata_ = rhs.pmetadata_;
      }

      /// share our metadata with another shared pagemap
      void give_metadata_to(this_type const & rhs)
      {
         // make sure there is something to share
         validate_metadata_exists();

         // it's good to share
         rhs.pmetadata_ = pmetadata_;
      }

      storage_t & get_storage()
      {
         // use responsibly, remember using this function is NOT thread safe!
         create_metadata_ondemand();
         return pmetadata_->ppagemap->get_storage();
      }

      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      /// INTERFACE: You MUST implement the following methods in your pagemap
      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

      iterator begin()
      {
         create_metadata_ondemand();

         // since the iterator is non-const we have to assume the store is now dirty
         pmetadata_->isdirty = 1;
         return pmetadata_->ppagemap->begin();
      }

      iterator end()
      {
         create_metadata_ondemand();
         return pmetadata_->ppagemap->end();
      }

      size_type size() const
      {
         return pmetadata_ ? pmetadata_->ppagemap->size() : 0;
      }

      void resize(size_type const size)
      {
         create_metadata_ondemand();
         pmetadata_->ppagemap->resize(size);
      }

      bool empty() const
      {
         return pmetadata_ ? pmetadata_->ppagemap->empty() : true;
      }

      void clear()
      {
         if(pmetadata_)
         {
            pmetadata_->isdirty = 1;
            pmetadata_->ppagemap->clear();
         }
      }

      iterator find (
         void const * pkey, size_t const ksize
         )
      {
         create_metadata_ondemand();
         iterator itr = pmetadata_->ppagemap->find(pkey, ksize);

         if(itr != end())
         {
            // since the iterator is non-const we have to assume the store is now dirty
            pmetadata_->isdirty = 1;
         }

         return itr;
      }

      void erase(iterator itr)
      {
         if(pmetadata_)
         {
            pmetadata_->isdirty = 1;
            pmetadata_->ppagemap->erase(itr);
         }
      }

      iterator insert(
         void const * pkey, size_t const ksize,
         pageinfo_t const & page_info
         )
      {
         create_metadata_ondemand();
         pmetadata_->isdirty = 1;
         return pmetadata_->ppagemap->insert(pkey, ksize, page_info);
      }

      bool itr2key(
         const_iterator itr,
         void * pkey, size_t & ksize
         ) const
      {
         return pmetadata_ ? pmetadata_->ppagemap->itr2key(itr, pkey, ksize) : false;
      }

      void load(std::string const & fname, bool newdb)
      {
         // if I'm the owner I'll load the data
         if(create_metadata_ondemand() && pmetadata_->is_owner(this))
         {
            pmetadata_->isdirty = !boost::filesystem::exists(fname) || newdb ? 1 : 0;
            pmetadata_->ppagemap->load(fname, newdb);
         }
      }

      void save(std::string const & fname)
      {
         // if there is metadata and we're the store ownder and the store is dirty then save.
         if(pmetadata_ && pmetadata_->is_owner(this) && pmetadata_->isdirty)
         {
            pmetadata_->ppagemap->save(fname);
         }
      }

   private:
      void validate_metadata_exists() const
      {
         if(!pmetadata_) { throw std::runtime_error("shared pagemap metadata not found"); }
      }

      /// Because we need this to act like a normal pagemap we need to create an instance of
      /// the pagemap we're sharing on demand. We don't want to create it at construction in
      /// case we're going to share with another and we can't put a liability on the store to
      /// create it because the store knows nothing about the implementation detail of pagemaps.
      bool create_metadata_ondemand()
      {
         if(!pmetadata_)
         {
            pmetadata_.reset(new metadata(this));
            return true;
         }

         return false;
      }
   };

   /// A intrinsic type pagemap.

   /// *** This class is NOT thread safe ***

   template <typename keyT>
   class KvdsPageMapIntrinsicKey
   {
   public:
      typedef boost::uint8_t storeid_t;
      typedef boost::uint32_t itemid_t; // 4 billion items per store should be enough for anyone.
      typedef std::pair<storeid_t, itemid_t> pageinfo_t;
      typedef keyT key_type;
      typedef moost::container::sparse_hash_map<key_type, pageinfo_t> storage_t;
      typedef typename storage_t::size_type size_type;
      typedef typename storage_t::iterator iterator;
      typedef typename storage_t::const_iterator const_iterator;

   public:
      KvdsPageMapIntrinsicKey()
      {
         // Since we support deletions there must always be a deleted key.
         // By default it'll be the max value that can be represented by
         // the key type. This can be changed by making a call to "get_storage"
         // to get a handle to the underlying storage and then setting a new value.
         storage_.set_deleted_key(std::numeric_limits<key_type>::max());

         // set an arbitrary starting size for the sparse hashmap. Failure to set a
         // reasonable size won't cause a fail but it will cause a performance hit.
         // Since sparse hashmap is very good in terms of memory usage we can affort
         // to push the boat out a little and set a reasonable default starting size.
         storage_.resize(0xFFFFFF);
      }

      /// Allow the client to apply any specific settings to the underlying storage.
      /// We need to do this to ensure the interface of each storage is consistance
      // and yet different stores may have different requirements. For example the
      /// sparse hash map needs a call to set_deleted to allow deletions.
      storage_t & get_storage()
      {
         return storage_;
      }

      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      /// INTERFACE: You MUST implement the following methods in your storage
      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

      iterator begin() { return storage_.begin(); }

      iterator end() { return storage_.end(); }

      size_type size() const { return storage_.size(); }

      void resize(size_type const size) { storage_.resize(size); }

      bool empty() const { return storage_.empty(); }

      void clear() { storage_.clear(); }

      iterator find (
         void const * pkey, size_t const /*ksize*/
         )
      {
         key_type key = *((key_type*)pkey);
         return storage_.find(key);
      }

      void erase(iterator itr)
      {
         assert(itr != storage_.end());
         storage_.erase(itr);
      }

      iterator insert(
         void const * pkey, size_t const /*ksize*/,
         pageinfo_t const & page_info
         )
      {
         key_type key = *((key_type*)pkey);
         return storage_.insert(std::make_pair(key, page_info)).first;
      }

      /// A generic method to convert an iterator to a key (page store has no idea how to do this)
      bool itr2key(
         const_iterator itr,
         void * pkey, size_t & ksize
         ) const
      {
         bool ok = false;
         size_t const size = sizeof(itr->first);

         if(size <= ksize)
         {
            memcpy(pkey, &itr->first, size);
            ok = true;
         }

         ksize = size;

         return ok;
      }

      void load(std::string const & fname, bool newdb)
      {
         if(newdb)
         {
            boost::filesystem::remove(fname);

         }
         else
         // only try to load it if it exists
         if(boost::filesystem::exists(fname))
         {
            // archive takes care of any io problems for us
            std::ifstream in(fname.c_str(), std::ios::binary);
            boost::archive::binary_iarchive ar(in);
            ar >> storage_;
         }
      }

      void save(std::string const & fname)
      {
         // archive takes care of any io problems for us
         std::ofstream out(fname.c_str(), std::ios::binary);
         boost::archive::binary_oarchive ar(out);
         ar << storage_;
      }

   private:
      storage_t storage_;
   };


   /// Default key hash functor used to hash the byte array key for the keyval index
   struct KvdsPageMapDefaultKeyHashFunctor : public std::unary_function< byte_array_t, size_t >
   {
      size_t operator()(const byte_array_t & key) const
      {
         return moost::algorithm::fast_hash(&key[0], key.size() * sizeof(byte_array_t::value_type));
      }
   };

   /// A non-intrinsic type pagemap, which uses the default key hash functor (above). If you're
   /// not too worried about performance and/or memory usage then this provides a nice simple
   /// way to get up and running with the KVDS Page Store. If you wish to provide a specific
   /// pagemap that is customised for your specific key types you can do so by createing your
   /// own custom pagemap class. This is the default page map unless you specify another.

   /// *** This class is NOT thread safe ***

   template <typename KeyHashFunctorT = KvdsPageMapDefaultKeyHashFunctor>
   class KvdsPageMapNonIntrinsicKey
   {
   public:
      typedef boost::uint8_t storeid_t;
      typedef boost::uint32_t itemid_t; // 4 billion items per store should be enough for anyone.
      typedef std::pair<storeid_t, itemid_t> pageinfo_t;
      typedef KeyHashFunctorT KeyHashFunctor;
      typedef moost::container::sparse_hash_map<byte_array_t, pageinfo_t, KeyHashFunctor> storage_t;
      typedef typename storage_t::key_type key_type;
      typedef typename storage_t::size_type size_type;
      typedef typename storage_t::iterator iterator;
      typedef typename storage_t::const_iterator const_iterator;

   public:
      KvdsPageMapNonIntrinsicKey()
      {
         // Since we support deletions there must always be a deleted key.
         // By default it'll be an empty key (remember a key is just a vector
         // of bytes). This can be changed by making a call to "get_storage"
         // to get a handle to the underlying storage and then setting a new value.
         storage_.set_deleted_key(key_type());

         // set an arbitrary starting size for the sparse hashmap. Failure to set a
         // reasonable size won't cause a fail but it will cause a performance hit.
         // Since sparse hashmap is very good in terms of memory usage we can affort
         // to push the boat out a little and set a reasonable default starting size.
         storage_.resize(0xFFFFFF);
      }

      /// Allow the client to apply any specific settings to the underlying storage.
      /// We need to do this to ensure the interface of each storage is consistance
      /// and yet different stores may have different requirements. For example the
      /// sparse hash map needs a call to set_deleted to allow deletions.
      storage_t & get_storage()
      {
         return storage_;
      }

      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      /// INTERFACE: You MUST implement the following methods in your storage
      /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

      iterator begin() { return storage_.begin(); }

      iterator end() { return storage_.end(); }

      size_type size() const { return storage_.size(); }

      void resize(size_type const size) { storage_.resize(size); }

      bool empty() const { return storage_.empty(); }

      void clear() { storage_.clear(); }

      iterator find (
         void const * pkey, size_t const ksize
         )
      {
         key_type key(ksize);
         memcpy(&key[0], pkey, ksize);
         return storage_.find(key);
      }

      void erase(iterator itr)
      {
         storage_.erase(itr);
      }

      iterator insert(
         void const * pkey, size_t const ksize,
         pageinfo_t const & page_info
         )
      {
         key_type key(ksize);
         memcpy(&key[0], pkey, ksize);
         return storage_.insert(std::make_pair(key, page_info)).first;
      }

      /// A generic method to convert an iterator to a key (page store has no idea how to do this)
      bool itr2key(
         const_iterator itr,
         void * pkey, size_t & ksize
         ) const
      {
         bool ok = false;
         size_t const size = itr->first.size();

         if(size <= ksize)
         {
            memcpy(pkey, &itr->first[0], size);
            ok = true;
         }

         ksize = size;

         return ok;
      }

      void load(std::string const & fname, bool newdb)
      {
         if(newdb)
         {
            boost::filesystem::remove(fname);

         }
         else
         {
            std::ifstream in(fname.c_str(), std::ios::binary);
            in.exceptions(std::ios::badbit);

            key_type key;

            // Assume the stream is all ready for reading and exceptions are on
            while(in)
            {
               size_t size = 0;
               in.read(reinterpret_cast<char *>(&size), sizeof(size));

               if(in)
               {
                  key.resize(size);
                  in.read(&key[0], size * sizeof(typename storage_t::key_type::value_type));

                  if(in)
                  {
                     typename storage_t::data_type & val = storage_[key];
                     in.read(reinterpret_cast<char *>(&val.first), sizeof(val.first));
                     in.read(reinterpret_cast<char *>(&val.second), sizeof(val.second));
                  }
               }
            }
         }
      }

      void save(std::string const & fname) const
      {
         std::ofstream out(fname.c_str(), std::ios::binary | std::ios::trunc);
         if(!out) { throw std::runtime_error("Unable to open keyval index for writing"); }

         out.exceptions(std::ios::badbit | std::ios::failbit);

         // Assume the stream is all ready for writing and exceptions are on
         for(typename storage_t::const_iterator itr = storage_.begin() ; itr != storage_.end() ; ++itr)
         {
            /// Write key (size and value)
            size_t size = itr->first.size();
            out.write(reinterpret_cast<char *>(&size), sizeof(size));
            KVDS_PAGESTORE_FLUSH_STREAM__(out);
            out.write(&itr->first[0], size * sizeof(typename storage_t::key_type::value_type));
            KVDS_PAGESTORE_FLUSH_STREAM__(out);

            /// Write 2 part value (1: store, 2: indx)
            out.write(reinterpret_cast<char const *>(&itr->second.first), sizeof(itr->second.first));
            KVDS_PAGESTORE_FLUSH_STREAM__(out);
            out.write(reinterpret_cast<char const *>(&itr->second.second), sizeof(itr->second.second));
            KVDS_PAGESTORE_FLUSH_STREAM__(out);
         }
      }

   private:
      storage_t storage_;
   };

   /// *** This class is NOT thread safe ***

   template <
      typename PageMapT // Either intrinsic (built-in integer types such as int or long) or non-instrinsic key.
   >
   class KvdsPageStore : public IKvds
   {
   private:
      typedef size_t page_size_t;
      typedef PageMapT pagemap_t;
      typedef boost::uint8_t storeid_t;
      typedef typename PageMapT::itemid_t itemid_t;
      typedef typename PageMapT::pageinfo_t pageinfo_t;

      class Store
      {
      public:
         Store(page_size_t page_size) :
            page_size_(page_size), item_cnt_(0), free_list_(0xFFFF) // freelist size set to arbitrary 64K items (sparse map, low cost)
            {
               if(0 == page_size)
               {
                  throw std::invalid_argument("Page size must be greater than zero");
               }

               if(!moost::utils::is_power_of_two(page_size))
               {
                  throw std::invalid_argument("Page size must be a power of two");
               }

               // If we ever hit this number (it's huge) we probably have bigger fish to fry!
               free_list_.set_deleted_key(std::numeric_limits<itemid_t>::max());
            }

            ~Store()
            {
               if(store_.is_open())
               {
                  close();
               }
            }

            page_size_t get_page_size() const { return page_size_; }

            void open(char const dsname [], bool newdb)
            {
               if(page_size_ <= std::numeric_limits<boost::uint8_t>::max())
               {
                  openT<boost::uint8_t>(dsname,  newdb);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint16_t>::max())
               {
                  openT<boost::uint16_t>(dsname,  newdb);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint32_t>::max())
               {
                  openT<boost::uint32_t>(dsname,  newdb);
               }
#ifndef WIN32 // 32 bit Windows will barf at this :(
               else
               if(page_size_ <= std::numeric_limits<boost::uint64_t>::max())
               {
                  openT<boost::uint64_t>(dsname,  newdb);
               }
#endif
               else
               {
                  throw std::runtime_error("open failed, value size is unsupported");
               }
            }

            void save()
            {
               if(store_.is_open())
               {
                  // archive takes care of any io problems for us
                  std::ofstream out(freelist_fname_.c_str(), std::ios::binary);
                  boost::archive::binary_oarchive ar(out);
                  ar << free_list_;
               }
            }

            void close()
            {
               if(store_.is_open())
               {
                  store_.close();
               }
            }

            bool erase(itemid_t itemid)
            {
               bool found = false;

               if(exists(itemid))
               {
                  free_list_.insert(itemid);
                  --item_cnt_;
                  found = true;
               }

               return found;
            }

            bool exists(itemid_t itemid)
            {
               return ((itemid < (free_list_.size() + item_cnt_)) && free_list_.find(itemid) == free_list_.end());
            }

            size_t size() const
            {
               return item_cnt_;
            }

            bool empty()
            {
               return 0 == item_cnt_;
            }

            void clear()
            {
               // Clear indexes
               item_cnt_ = 0;
               free_list_.clear();

               // Close store and re-open as new (thus, truncating the store)
               store_.close();
               open(dsname_.c_str(), true);
            }

            bool size(itemid_t itemid, size_t & size)
            {
               bool ok = false;

               if(page_size_ <= std::numeric_limits<boost::uint8_t>::max())
               {
                  ok = sizeT<boost::uint8_t>(itemid, size);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint16_t>::max())
               {
                  ok = sizeT<boost::uint16_t>(itemid, size);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint32_t>::max())
               {
                  ok = sizeT<boost::uint32_t>(itemid, size);
               }
#ifndef WIN32 // 32 bit Windows will barf at this :(
               else
               if(page_size_ <= std::numeric_limits<boost::uint64_t>::max())
               {
                  ok = sizeT<boost::uint64_t>(itemid, size);
               }
#endif
               else
               {
                  throw std::runtime_error("size failed, value size is unsupported");
               }

               return ok;
            }

            bool read(void * data, page_size_t size, itemid_t itemid)
            {
               bool ok = false;

               if(page_size_ <= std::numeric_limits<boost::uint8_t>::max())
               {
                  ok = readT<boost::uint8_t>(data, size, itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint16_t>::max())
               {
                  ok = readT<boost::uint16_t>(data, size, itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint32_t>::max())
               {
                  ok = readT<boost::uint32_t>(data, size, itemid);
               }
#ifndef WIN32 // 32 bit Windows will barf at this :(
               else
               if(page_size_ <= std::numeric_limits<boost::uint64_t>::max())
               {
                  ok = readT<boost::uint64_t>(data, size, itemid);
               }
#endif
               else
               {
                  throw std::runtime_error("read failed, size is unsupported");
               }

               return ok;
            }

            void write(void const * data, page_size_t size, itemid_t itemid)
            {
               if(page_size_ <= std::numeric_limits<boost::uint8_t>::max())
               {
                  writeT<boost::uint8_t>(data, size, itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint16_t>::max())
               {
                  writeT<boost::uint16_t>(data, size, itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint32_t>::max())
               {
                  writeT<boost::uint32_t>(data, size, itemid);
               }
#ifndef WIN32 // 32 bit Windows will barf at this :(
               else
               if(page_size_ <= std::numeric_limits<boost::uint64_t>::max())
               {
                  writeT<boost::uint64_t>(data, size, itemid);
               }
#endif
               else
               {
                  throw std::runtime_error("write failed, value size is unsupported");
               }
            }

            void append(void const * data, page_size_t size, itemid_t itemid)
            {
               if(page_size_ <= std::numeric_limits<boost::uint8_t>::max())
               {
                  appendT<boost::uint8_t>(data, size, itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint16_t>::max())
               {
                  appendT<boost::uint16_t>(data, size, itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint32_t>::max())
               {
                  appendT<boost::uint32_t>(data, size, itemid);
               }
#ifndef WIN32 // 32 bit Windows will barf at this :(
               else
               if(page_size_ <= std::numeric_limits<boost::uint64_t>::max())
               {
                  appendT<boost::uint64_t>(data, size, itemid);
               }
#endif
               else
               {
                  throw std::runtime_error("append failed, size is unsupported");
               }
            }

            /// This function only gets what the next free item will be
            /// but it doesn't actually reserve it just in case there is
            /// an error before it's been used and we end up losing a page.
            /// It is, therefore, up to the caller to ensure that all the
            /// code from getting the next free ID to using (or disgarding)
            /// it is an atomic action.
            itemid_t get_next_free_id()
            {
               itemid_t itemid = boost::numeric_cast<itemid_t>(size());

               if(!free_list_.empty()) /// If there's a free page use that instead
               {
                  itemid = *free_list_.begin();
               }

               if(itemid >= std::numeric_limits<itemid_t>::max())
               {
                  /// Arrrrr.... no more space in the store!!!
                  std::stringstream ss;
                  ss << store_fname_ << " store is full";
                  throw std::runtime_error(ss.str());
               }

               return itemid;
            }

            std::streampos get_item_pos(itemid_t itemid) const
            {
               std::streampos pos;

               if(page_size_ <= std::numeric_limits<boost::uint8_t>::max())
               {
                  pos = get_item_posT<boost::uint8_t>(itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint16_t>::max())
               {
                  pos = get_item_posT<boost::uint16_t>(itemid);
               }
               else
               if(page_size_ <= std::numeric_limits<boost::uint32_t>::max())
               {
                  pos = get_item_posT<boost::uint32_t>(itemid);
               }
#ifndef WIN32 // 32 bit Windows will barf at this :(
               else
               if(page_size_ <= std::numeric_limits<boost::uint64_t>::max())
               {
                  pos = get_item_posT<boost::uint64_t>(itemid);
               }
#endif
               else
               {
                  throw std::runtime_error("get_item_pos failed, value size is unsupported");
               }

               return pos;
            }

      private:

         template <typename valsizeT>
         void openT(char const dsname [], bool newdb)
         {
            if(store_.is_open()) { throw std::runtime_error("The store is already open"); }

            dsname_ = dsname;

            std::stringstream ssStore;
            ssStore << dsname_ << "_data." << page_size_;
            store_fname_ = ssStore.str();

            std::stringstream ssFreelist;
            ssFreelist << dsname_ << "_idx." << page_size_;
            freelist_fname_ = ssFreelist.str();

            if(!newdb && boost::filesystem::exists(store_fname_))
            {
               // Use existing store, sanity check size
               boost::uintmax_t const storefilesize = boost::filesystem::file_size(store_fname_);
               if(storefilesize % (sizeof(valsizeT) + page_size_)) {
                  throw std::runtime_error(std::string("Page size for store is invalid: ") + store_fname_);
               }
               item_cnt_ = boost::numeric_cast<size_t>((storefilesize / (sizeof(valsizeT) + page_size_)));
            }
            else
            {
               /// Truncate stores and associated free list index (they must exist or open 'in' fails)
               { std::ofstream(store_fname_.c_str(), std::ios::binary | std::ios::trunc); }
               { std::ofstream(freelist_fname_.c_str(), std::ios::binary | std::ios::trunc); }
            }

            // Open store
            store_.open(store_fname_.c_str(), std::ios::binary | std::ios::out | std::ios::in);
            if(!store_) { throw std::runtime_error(std::string("Unable to open store: ") + store_fname_); }
            store_.exceptions(std::ios::badbit | std::ios::failbit); /// IO errors will generate an exception

            if(boost::filesystem::exists(freelist_fname_))
            {
               std::streamsize const freelistsize = boost::numeric_cast<std::streamsize>(boost::filesystem::file_size(freelist_fname_));

               if(freelistsize < 0) { throw std::runtime_error(std::string("Invalid free list size: ") + freelist_fname_); }
               else
               if(freelistsize > 0) // if the file has 0 bytes we can assume there was nothing in the free list
               {
                  // archive takes care of any io problems for us
                  std::ifstream in(freelist_fname_.c_str(), std::ios::binary);
                  boost::archive::binary_iarchive ar(in);
                  ar >> free_list_;
                  item_cnt_ -= free_list_.size(); // item count is less free list size.
               }
            }
         }

         template <typename valsizeT>
         bool sizeT(itemid_t itemid, size_t & size)
         {
            size = 0;

            bool found = exists(itemid);

            if(found)
            {
               std::streampos pos = get_item_pos(itemid);

               store_.clear();
               store_.seekg(pos);

               valsizeT esize = 0;
               store_.read(reinterpret_cast<char *>(&esize), sizeof(esize));
               size = esize;
            }

            return found;
         }

         template <typename valsizeT>
         bool readT(void * data, page_size_t size, itemid_t itemid)
         {
            bool found = exists(itemid);

            if(found)
            {
               if(size > page_size_) {
                  throw std::invalid_argument("Read size cannot be greater than page size");
               }

               std::streampos pos = get_item_pos(itemid);

               store_.clear();
               store_.seekg(pos);

               valsizeT esize = 0;
               store_.read(reinterpret_cast<char *>(&esize), sizeof(esize));
               if(size > esize) { throw std::invalid_argument("Read size cannot be greater than value size"); }

               store_.read(reinterpret_cast<char *>(data), size);
            }

            return found;
         }

         template <typename valsizeT>
         void writeT(void const * data, size_t size, itemid_t itemid)
         {
            if(size > page_size_) { throw std::invalid_argument("Write size cannot be greater than page size"); }

            valsizeT const tsize = boost::numeric_cast<valsizeT>(size);

            store_.clear();
            store_.seekp(get_item_pos(itemid));

            store_.write(reinterpret_cast<char const *>(&tsize), sizeof(tsize));
            KVDS_PAGESTORE_FLUSH_STREAM__(store_);

            store_.write(reinterpret_cast<char const *>(data), size);
            KVDS_PAGESTORE_FLUSH_STREAM__(store_);

            // If we're appending a new item we need to add padding
            if(itemid == (free_list_.size() + item_cnt_))
            {
               pad(size);
            }

            typename free_list_t::iterator itr = free_list_.find(itemid);

            if((itr != free_list_.end()) || (itemid == (free_list_.size() + item_cnt_)))
            {
               ++item_cnt_;
            }

            if(itr != free_list_.end())
            {
               free_list_.erase(itr);
            }
         }

         template <typename valsizeT>
         void appendT(void const * data, size_t size, itemid_t itemid)
         {
            size_t esize = 0;
            this->size(itemid, esize);
            valsizeT nsize = boost::numeric_cast<valsizeT>(esize + size);

            if(nsize > page_size_) { throw std::invalid_argument("Append size cannot be greater than page size"); }

            store_.clear();
            store_.seekp(get_item_pos(itemid));

            store_.write(reinterpret_cast<char const *>(&nsize), sizeof(nsize));
            KVDS_PAGESTORE_FLUSH_STREAM__(store_);

            store_.seekp(boost::numeric_cast<std::streamoff>(esize), std::ios::cur);

            store_.write(reinterpret_cast<char const *>(data), size);
            KVDS_PAGESTORE_FLUSH_STREAM__(store_);

            // If we're appending a new item we need to add padding
            if(itemid == (free_list_.size() + item_cnt_))
            {
               pad(nsize);
            }

            typename free_list_t::iterator itr = free_list_.find(itemid);

            if((itr != free_list_.end()) || (itemid == (free_list_.size() + item_cnt_)))
            {
               ++item_cnt_;
            }

            if(itr != free_list_.end())
            {
               free_list_.erase(itr);
            }
         }

         template <typename valsizeT>
         std::streampos get_item_posT(itemid_t itemid) const
         {
            return boost::numeric_cast<std::streampos>(itemid * (sizeof(valsizeT) + page_size_));
         }

      private:
         enum { PADSIZE = 0xFF };
         static char const * GetPadding()
         {
            static char const PADBUF [PADSIZE] = {0};
            return PADBUF;
         }

         void pad(page_size_t size) // Write padding in 0xFF byte blocks.
         {
            page_size_t pad_size = (page_size_ - size);

            if(pad_size)
            {
               static char const * const padbuf = GetPadding();
               page_size_t topad = 0;

               while(pad_size)
               {
                  topad = std::min(pad_size, (size_t)PADSIZE);
                  if(!store_.write(padbuf, topad)) { throw std::runtime_error("Unable to write padding"); }
                  KVDS_PAGESTORE_FLUSH_STREAM__(store_);
                  pad_size -= topad;
               }
            }
         }

      private:
         page_size_t const page_size_;
         size_t item_cnt_;
         typedef moost::container::sparse_hash_set<itemid_t> free_list_t;
         free_list_t free_list_;
         std::fstream store_;
         std::string store_fname_;
         std::string dsname_;
         std::string freelist_fname_;
      };

      typedef boost::shared_ptr<Store> store_t;
      typedef moost::container::sparse_hash_map<storeid_t, store_t> store_index_t;
      typedef std::bitset<sizeof(page_size_t) * 8> store_inventory_t;

   public:
      typedef pagemap_t store_type;

      KvdsPageStore() : iterating_(false) { }

      ~KvdsPageStore()
      {
         close();
      }

      // Allow the client to apply any specific settings to the underlying pagemap.
      pagemap_t & get_pagemap()
      {
         return pagemap_;
      }

      void open(
         char const dsname [],
         bool newdb = false
         )
      {
         if(!dsname_.empty()) { throw std::runtime_error("The store is already open"); }

         dsname_ = dsname;

         std::stringstream ssStoreInv;
         ssStoreInv << dsname_ << "_inv";
         storeinv_fname_ = ssStoreInv.str();

         if(newdb)
         {
            boost::filesystem::remove(pagemap_fname_);
         }
         else
         {
            std::ifstream in(storeinv_fname_.c_str());
            if(in.is_open())
            {
               in.exceptions(std::ios::badbit);

               // The inventory bitmap is a text file of 1's and 0's (eg. "0000010011010001")
               std::string s;
               in >> s;

               // create the store inventory bitset
               store_inventory_ = store_inventory_t(trim_inventory_string(s));

               // The number of 1's in s should correspond to the number of bits set in the store inventory bitset
               size_t expectedBitCount = std::count(s.begin(), s.end(), '1');
               if(expectedBitCount != store_inventory_.count())
               {
                  throw std::runtime_error("the format of the inventory file is incorrect");
               }

               // we seem to have a valid inventory so use it to create the stores
               for(page_size_t page_size = 1 ; page_size ; page_size <<= 1)
               {
                  storeid_t const storeid = moost::utils::msb_set(page_size);

                  if(store_inventory_[storeid])
                  {
                     store_t store(new Store(page_size));
                     store->open(dsname, newdb);
                     store_index_[storeid] = store;
                  }
               }
            }
         }

         // now the stores are open all that remains is to open the store index
         std::stringstream ssKeyValIndex;
         ssKeyValIndex << dsname_ << "_idx";
         pagemap_fname_ = ssKeyValIndex.str();
         pagemap_.load(pagemap_fname_, newdb);
      }

      void save()
      {
         save_impl();
      }

      void close()
      {
         if(!dsname_.empty())
         {
            save_impl(true);
            dsname_.clear();
         }
      }

   private:

      std::string trim_inventory_string(std::string s) const
      {
         std::string::size_type pos = s.find('1');
         if(pos != std::string::npos)
         {
            s = s.substr(pos);
         }

         return s;
      }

      void save_impl(bool bClose = false)
      {
         for(typename store_index_t::iterator itr = store_index_.begin() ; itr != store_index_.end() ; ++itr)
         {
            itr->second->save();
            if(bClose) { itr->second->close(); }
         }

         pagemap_.save(pagemap_fname_);
      }

      storeid_t const * get_storeid(page_size_t page_size)
      {
         page_size = moost::utils::next_power_of_two(page_size);
         storeid_t const storeid = moost::utils::msb_set(page_size);
         typename store_index_t::const_iterator itr = store_index_.find(storeid);

         if(itr == store_index_.end())
         {
            store_t store(new Store(page_size));
            store->open(dsname_.c_str(), true);
            itr = store_index_.insert(std::make_pair(storeid, store)).first;

            store_inventory_.set(storeid);
            std::ofstream out(storeinv_fname_.c_str());
            out << trim_inventory_string(store_inventory_.to_string());
         }

         return (itr == store_index_.end()) ? 0 : &itr->first;
      }

      store_t get_store(storeid_t const id) const
      {
         typename store_index_t::const_iterator itr = store_index_.find(id);

         assert(itr != store_index_.end() && itr->second);

         if(itr == store_index_.end() || !itr->second)
         {
            throw std::runtime_error("error retrieving store");
         }

         return itr->second;
      }

   public:
      /// For the purposes of external configuration we return a reference to the pagemap so
      /// the any pagemap specific settings (such as itemcnt, deleted or erased key) can be set
      store_type & get_store() { return pagemap_; }

   public: // IKvds interface implementation

      /// Overwrites existing values.
      bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         bool ok = false;

         storeid_t const * pstoreid = get_storeid(vsize);

         if(pstoreid)
         {
            typename pagemap_t::iterator itr = pagemap_.find(pkey, ksize);

            if(itr != pagemap_.end() && (itr->second.first != *pstoreid))
            {
               get_store(itr->second.first)->erase(itr->second.second);
               pagemap_.erase(itr);
               itr = pagemap_.end();
            }

            store_t store = get_store(*pstoreid);
            itemid_t itemid = (itr == pagemap_.end()) ? store->get_next_free_id() : itr->second.second;
            store->write(pval, vsize, itemid);

            if(itr == pagemap_.end())
            {
               pagemap_.insert(pkey, ksize, std::make_pair(*pstoreid, itemid));
            }

            ok = true;
         }

         return ok;
      }

      bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         bool found = false;

         typename pagemap_t::const_iterator itr = pagemap_.find(pkey, ksize);

         page_size_t esize = 0;
         if(itr != pagemap_.end())
         {
            store_t store = get_store(itr->second.first);

            if(store->size(itr->second.second, esize))
            {
               vsize = std::min(vsize, esize);
               found = store->read(pval, vsize, itr->second.second);
            }
         }

         if(!found) { vsize = 0; }

         return found;
      }

      /// Appends to existing values
      bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         bool ok = false;

         page_size_t esize = 0;
         page_size_t nsize = vsize;

         store_t store;

         typename pagemap_t::iterator itr = pagemap_.find(pkey, ksize);
         if(itr != pagemap_.end())
         {
            store = get_store(itr->second.first);
            store->size(itr->second.second, esize);
            nsize += esize;
         }

         storeid_t const * pstoreid = get_storeid(nsize);

         if(pstoreid)
         {
            store_t promoted_store = get_store(*pstoreid);

            if(itr != pagemap_.end() && (itr->second.first != *pstoreid))
            {
               itemid_t itemid = promoted_store->get_next_free_id();

               byte_array_t buf(esize);
               if(store->read(&buf[0], esize, itr->second.second))
               {
                  promoted_store->write(&buf[0], esize, itemid);
               }

               store->erase(itr->second.second);
               itr->second.first = (*pstoreid);
               itr->second.second = itemid;

               store = get_store(itr->second.first);
            }

            if(itr == pagemap_.end())
            {
               itr = pagemap_.insert(pkey, ksize, std::make_pair(*pstoreid, promoted_store->get_next_free_id()));

               store = get_store(itr->second.first);
            }

            store->append(pval, vsize, itr->second.second);

            ok = true;
         }

         return ok;
      }

      bool all(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         bool found = false;

         typename pagemap_t::const_iterator itr = pagemap_.find(pkey, ksize);

         page_size_t esize = 0;
         if((itr != pagemap_.end()) && (get_store(itr->second.first)->size(itr->second.second, esize)))
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
         bool found = false;

         typename pagemap_t::const_iterator itr = pagemap_.find(pkey, ksize);
         if(itr != pagemap_.end())
         {
            found = get_store(itr->second.first)->exists(itr->second.second);
         }

         return found;
      }

      bool del(
         void const * pkey, size_t const ksize
         )
      {
         bool found = false;

         typename pagemap_t::iterator itr = pagemap_.find(pkey, ksize);
         if(itr != pagemap_.end())
         {
            get_store(itr->second.first)->erase(itr->second.second);
            pagemap_.erase(itr);
            found = true;
         }

         return found;
      }

      bool clr()
      {
         // Truncates all filestores and indexes (this CANNOT be undone!)

         for(typename store_index_t::const_iterator itr = store_index_.begin() ; itr != store_index_.end() ; ++itr)
         {
            itr->second->clear();
         }

         pagemap_.clear();

         { std::ofstream(pagemap_fname_.c_str(), std::ios::binary | std::ios::trunc); }

         return true;
      }

      bool beg()
      {
         itr_ = pagemap_.begin();
         return iterating_ = true;
      }

      bool nxt(
         void * pkey, size_t & ksize
         )
      {
         bool found = false;

         if(iterating_ && (itr_ != pagemap_.end()))
         {
            found = pagemap_.itr2key(itr_, pkey, ksize);
            if(found) { ++itr_; }
         }
         else
         {
            ksize = 0;
            iterating_ = false;
         }

         return found;
      }

      bool end()
      {
         return iterating_ ? itr_ == pagemap_.end() : true;
      }

      bool siz(
         void const * pkey, size_t const ksize,
         size_t & vsize
         )

      {
         bool found = false;

         typename pagemap_t::const_iterator itr = pagemap_.find(pkey, ksize);

         if(itr != pagemap_.end())
         {
            found = get_store(itr->second.first)->size(itr->second.second, vsize);
         }

         return found;
      }

      bool cnt(boost::uint64_t & cnt)
      {
         cnt = pagemap_.size();
         return true;
      }

      bool nil(bool & isnil)
      {
         isnil =  pagemap_.empty();
         return true;
      }

   private:
      std::string dsname_;
      std::string pagemap_fname_;
      std::string storeinv_fname_;
      pagemap_t pagemap_;
      typename pagemap_t::const_iterator itr_;
      store_index_t store_index_;
      store_inventory_t store_inventory_;
      bool iterating_;
   };

}}

#endif // MOOST_KVDS_KVDS_PAGESTORE_HPP__
