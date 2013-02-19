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
/// Kyoto Hash Table
/// API reference
/// http://fallabs.com/kyotocabinet/spex.html

#ifndef MOOST_KVDS_KVDSKCH_LINUX_HPP__
#define MOOST_KVDS_KVDSKCH_LINUX_HPP__

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <limits>

#include <boost/scoped_array.hpp>

#if defined (_STDINT_H)
#define STDINT_H_3C996A98_ABAB_4d71_AC4B_633055150696
#undef _STDINT_H
#endif

#include <kchashdb.h>

#ifdef STDINT_H_3C996A98_ABAB_4d71_AC4B_633055150696
#undef STDINT_H_3C996A98_ABAB_4d71_AC4B_633055150696
#if !defined (_STDINT_H)
#define _STDINT_H
#endif
#else
#undef _STDINT_H
#endif

#include <boost/cast.hpp>

#include "../ikvds.hpp"

namespace moost { namespace kvds {

   class KvdsKch : public IKvds
   {
   public:
      typedef kyotocabinet::HashDB store_type;

      KvdsKch() : bOpen_(false), pcursor_(0)
      {
      }

      ~KvdsKch()
      {
         try
         {
            close();
         }
         catch(...) { /* ignore */ }
      }

      void open (char const dsname [], bool newdb = false)
      {
         if(bOpen_) { throw std::runtime_error("The store is already open"); }

         uint32_t const OMODE = (newdb ? kyotocabinet::HashDB::OTRUNCATE : 0)
            | (kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OREADER | kyotocabinet::HashDB::OCREATE);

         bOpen_ = db_.open(dsname, OMODE);

         if(!bOpen_)
         {
            throw_kch_exception("Failed to open KCH file");
         }
      }

      void save() { /* nothing to do, fully persisted storage */ }

      void close()
      {
         release_cursor();

         if(bOpen_)
         {
            if (!db_.close())
                throw_kch_exception();
            bOpen_ = false;
         }
      }

      /// You can call this to get a reference to the underlying store. Useful
      /// if you need to do post construction configuration. For example if using
      /// a google hash or spare map you need to specify the delete/erase keys
      store_type & get_store() { return db_; }

   private:
      bool set_cursor()
      {
         release_cursor();
         pcursor_ = db_.cursor();
         // move to first record if there is one
         bool ok = true;
         if (!pcursor_->jump())
         {
            release_cursor();
            ok = false;
         }
         return ok;
      }

      void release_cursor()
      {
         delete pcursor_;
         pcursor_ = 0;
      }

      void throw_kch_exception(const std::string& msg) const
      {
         throw std::runtime_error(msg + ": " + db_.error().name());
      }

      void throw_kch_exception() const
      {
         throw_kch_exception("ERROR");
      }

      void assert_data_store_open() const
      {
         assert(bOpen_);
      }

   public: // IKvds interface implementation

      bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         assert_data_store_open();
         return db_.set(static_cast<const char*>(pkey), ksize, static_cast<const char*>(pval), vsize);
      }

      bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         assert_data_store_open();

         int32_t const esize = db_.get((char *)pkey, ksize, (char *)pval, vsize);

         bool found = (esize >= 0);

         if(found)
         {
            vsize = std::min(vsize, (size_t) esize);
         }
         else { vsize = 0; }

         return found;
      }

      bool all(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         assert_data_store_open();

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

      bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         assert_data_store_open();
         return db_.append(static_cast<const char*>(pkey), ksize, static_cast<const char*>(pval), vsize);
      }

      bool xst(
         void const * pkey, size_t const ksize
         )
      {
         size_t size; /// Unused
         return siz(pkey, ksize, size) > 0;
      }

      bool del(
         void const * pkey, size_t const ksize
         )
      {
         assert_data_store_open();
         return db_.remove(static_cast<const char*>(pkey), ksize);
      }

      bool clr()
      {
         assert_data_store_open();
         return db_.clear();
      }

      bool beg()
      {
         assert_data_store_open();
         return bOpen_ && set_cursor();
      }

      bool nxt(
         void * pkey, size_t & ksize
         )
      {
         assert(bOpen_);

         bool found = false;;

         if (pcursor_)
         {
            size_t size;
            boost::scoped_array<char> pk(pcursor_->get_key(&size)); // alloc'd via new []

            if (size <= ksize)
            {
               memcpy(pkey, pk.get(), size);
               found = true;
               if (!pcursor_->step())
                  release_cursor();
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
         return 0 == pcursor_;
      }

      bool siz(
         void const * pkey, size_t const ksize,
         size_t & vsize
         )
      {
         assert_data_store_open();

         boost::scoped_array<char> pval(db_.get(static_cast<const char*>(pkey), ksize, &vsize)); // alloc'd via new []
         return (bool)pval; // true if set else false
      }

      bool cnt(boost::uint64_t & cnt)
      {
         assert_data_store_open();
         cnt = db_.count();
         return cnt < boost::uint64_t(-1);
      }

      bool nil(bool & isnil)
      {
         boost::uint64_t icnt = 0;
         bool ok = cnt(icnt);
         if(ok) { isnil = (0 == icnt); }
         return ok;
      }

private:
      store_type db_;
      bool bOpen_;
      kyotocabinet::HashDB::Cursor* pcursor_;
   };

}}

#endif // MOOST_KVDS_KVDSKCH_LINUX_HPP__
