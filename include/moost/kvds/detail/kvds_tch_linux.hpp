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
/// Tokyo Cabinet Hash Table
/// API reference
/// http:///1978th.net/tokyocabinet/spex-en.html#tchdbapi

#ifndef MOOST_KVDS_KVDSTCH_LINUX_HPP__
#define MOOST_KVDS_KVDSTCH_LINUX_HPP__

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <limits>

#if defined (_STDINT_H)
#define STDINT_H_A5C236B9_2336_44a8_AF40_BCA7EA04E940
#undef _STDINT_H
#endif

#include <tchdb.h>

#ifdef STDINT_H_A5C236B9_2336_44a8_AF40_BCA7EA04E940
#undef STDINT_H_A5C236B9_2336_44a8_AF40_BCA7EA04E940
#if !defined (_STDINT_H)
#define _STDINT_H
#endif
#else
#undef _STDINT_H
#endif

#include <boost/cast.hpp>

#include "../ikvds.hpp"

namespace moost { namespace kvds {

   /// *** This class is NOT thread safe ***

   class KvdsTch : public IKvds
   {
   public:
      typedef TCHDB * store_type;

      KvdsTch() :
         pdb_(0), bOpen_(false), pitr_(0), pitr_size_(0)
      {
         pdb_ = tchdbnew();
      }

      ~KvdsTch()
      {
         try
         {
            close();

            if(pdb_)
            {
               tchdbdel(pdb_);
               pdb_ = 0;
            }

            free(pitr_);
         }
         catch(...) { /* ignore */ }
      }

      void open (char const dsname [], bool newdb = false)
      {
         if(bOpen_) { throw std::runtime_error("The store is already open"); }

         int const OMODE = (newdb ? HDBOTRUNC : 0) | (HDBOWRITER | HDBOREADER | HDBOCREAT);

         bOpen_ = tchdbopen(pdb_, dsname, OMODE);

         if(!bOpen_)
         {
            throw_tcexception(tchdbecode(pdb_), "Failed to open TCH file");
         }
      }

      void save() { /* nothihng to do, fully persisted storage */ }

      void close()
      {
         if(bOpen_ && pdb_)
         {
            tchdbclose(pdb_);
            bOpen_ = false;
         }
      }

      /// You can call this to get a refernce to the underlying store. Useful
      /// if you need to do post construction configuration. For example if using
      /// a google hash or spare map you need to specify the delete/erase keys
      store_type & get_store() { return pdb_; }

   private:
      void assert_data_store_open() const
      {
         if(!pdb_) { throw std::runtime_error("Datastore is not open"); }
      }

      void throw_tcexception(int ecode, char const * msg = "Unexpected error")
      {
         std::stringstream oss;
         oss
            << msg << ": " << tchdberrmsg(ecode);

         throw std::runtime_error(oss.str());
      }

      void set_next_itr()
      {
         free(pitr_);
         pitr_ =  tchdbiternext(pdb_, &pitr_size_);
      }

   public: // IKvds interface implementation

      bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         assert_data_store_open();
         return tchdbput(pdb_, pkey, ksize, pval, vsize);
      }

      bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         assert_data_store_open();

         int const esize = tchdbget3(pdb_, pkey, ksize, pval, boost::numeric_cast<int>(vsize));
         bool found = (esize >= 0);

         if(found)
         {
            vsize = std::min(vsize, (size_t) esize);
         }
         else
         {
            vsize = 0;
         }

         return found;
      }

      bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         assert_data_store_open();
         return tchdbputcat(pdb_, pkey, ksize, pval, vsize);
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
         return tchdbout(pdb_, pkey, ksize);
      }

      bool clr()
      {
         assert_data_store_open();
         return tchdbvanish(pdb_);
      }

      bool beg()
      {
         assert_data_store_open();
         bool bOk = tchdbiterinit(pdb_);

         if(bOk)
         {
            set_next_itr();
         }

         return bOk;
      }

      bool nxt(
         void * pkey, size_t & ksize
         )
      {
         assert_data_store_open();

         bool found = false;;

         if((pitr_) && (pitr_size_ >= 0))
         {
            size_t const size = pitr_size_;

            if(size <= ksize)
            {
               memcpy(pkey, pitr_, size);
               set_next_itr();
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
         return 0 == pitr_;
      }

      bool siz(
         void const * pkey, size_t const ksize,
         size_t & vsize
         )
      {
         assert_data_store_open();

         int size = tchdbvsiz(pdb_, pkey, ksize);
         bool found = size >= 0;
         if(found) { vsize = size; }
         return found;
      }

      bool cnt(boost::uint64_t & cnt)
      {
         assert_data_store_open();
         cnt = tchdbrnum(pdb_);
         return true;
      }

      bool nil(bool & isnil)
      {
         boost::uint64_t icnt = 0;
         bool ok = cnt(icnt);
         if(ok) { isnil = (0 == icnt); }
         return ok;
      }

private:
      store_type pdb_;
      bool bOpen_;
      void * pitr_;
      int pitr_size_;
   };

}}

#endif // MOOST_KVDS_KVDSTCH_LINUX_HPP__
