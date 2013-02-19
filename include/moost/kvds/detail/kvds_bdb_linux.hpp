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
/// Berkeley DB Hash Table
/// API reference
/// http:///www.oracle.com/technology/documentation/berkeley-db/db/index.html
///
/// Windows version download
/// http:///www.oracle.com/technology/software/products/berkeley-db/db/index.html

#ifndef MOOST_KVDS_KVDSBDB_LINUX_HPP__
#define MOOST_KVDS_KVDSBDB_LINUX_HPP__

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <limits>

#include <db_cxx.h>

#include <boost/shared_ptr.hpp>

#include "../../compiler/attributes/unused.hpp"
#include "../ikvds.hpp"

/// Depending on how it was built DBD can either return an error code
/// or throw a DbException object. To ensure we can handle both all
/// calls to the DBD engine are wrapped in this macro. The macro wil
/// always simulate return value semantics.
#define KVDSDBDX__(rval, cmd) \
   try \
   { \
      rval = cmd; \
   } \
   catch(DbException const & e) \
   { \
      rval = e.get_errno(); \
   }

namespace moost { namespace kvds {

   typedef std::vector<char> byte_array_t;

   /// Some API non-get functions malloc memory, ensure we always free this
   /// ie. It's a scoped pointer to free malloc allocated memory
   template <typename T>
   struct ScopedFree
   {
      ScopedFree() : p(0) {}
      ~ScopedFree() { free(p); }

      T * p;
   };

   /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
   /// Unfortunately, Dbt only takes non-const pointers. A const cast is
   /// required here otherwise we'd have to create local copies of key/data.
   /// This should be safe because no modifications are made to key/data
   /// unless the semantics of the BDB function being called demand it and
   /// in these cases the pointers are non-const anyway.
   ///
   /// Below, we encapsulate the three different ways we used Dbt with easy
   /// to recognise class names to (a) make the code more readable and (b)
   /// to ensure the type of Dbt being used is unequivocally clear. Note,
   /// only user memory allocate Dbt allows the data pointer to be changed.
   /// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

   /// User defined data that must NEVER be modified!
   struct ConstDbt : Dbt
   {
      ConstDbt(void const * data, size_t size) :
         Dbt(const_cast<void *>(data), size)
      {
      }

      /// Not allowed to set data on this (via c_tor only)!
      void set_data(void const * data);
   };

   /// User defined data that may be modified!
   struct UsermemDbt : Dbt
   {
      UsermemDbt(void * data, size_t size) :
         Dbt(data, size)
      {
         set_flags(DB_DBT_USERMEM);
      }

      UsermemDbt()
      {
         set_flags(DB_DBT_USERMEM);
      }
   };

   /// Dbd allocated memory that must be released
   struct MallocDbt : Dbt
   {
      MallocDbt()
      {
         set_flags(DB_DBT_MALLOC);
      }

      ~MallocDbt()
      {
         free(get_data());
      }

      /// Not allowed to set data on this!
      void set_data(void const * data);
   };

   /// *** This class is NOT thread safe ***

   /// Only HASH and BTREE are supported, trying to use another type will generate a link error
   template <DBTYPE dbtypeT>
   class KvdsBdb : public IKvds
   {
   public:
      typedef boost::shared_ptr<Db> store_type;

      KvdsBdb() : pitr_(0)
      {
      }

      ~KvdsBdb()
      {
         try
         {
            close();
         }
         catch(...) { /* ignore */ }
      }

      void open (char const dsname [], bool newdb = false)
      {
         if(pdb_) { throw std::runtime_error("The store is already open"); }

         pdb_.reset(new Db(0, 0)); /// We do not support transactions!

         int const OMODE = (newdb ? (DB_TRUNCATE | DB_CREATE) : DB_CREATE);

         int rval = -1;
         KVDSDBDX__(rval, pdb_->open(0, dsname, 0, dbtypeT, OMODE, 0));

         if(rval != 0) { throw std::runtime_error("Failed to open DBD file"); }
      }

      void save() { /* nothing to do, fully persisted storage */ }

      void close()
      {
         int rval unused__ = -1;

         if(pitr_)
         {
            // Close open cursor
            KVDSDBDX__(rval, pitr_->close()); // Failure is not an option!
            pitr_ = 0;
         }

         if(pdb_)
         {
            // Close DB
            KVDSDBDX__(rval, pdb_->close(0)); // Failure is not an option!
            pdb_.reset();
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

      void set_next_itr()
      {
            MallocDbt kt;
            MallocDbt vt;

            int rval = 0;
            KVDSDBDX__(rval, pitr_->get(&kt, &vt, DB_NEXT));

            switch(rval)
            {
            case 0:
               /// all good :)
               break;
            case DB_NOTFOUND:
               {
                  /// End
                  int rrval unused__ = -1;
                  KVDSDBDX__(rrval, pitr_->close());
                  pitr_ = 0;
               }
               break;
            default:
               throw std::runtime_error("Unable to increment DB cursor");
            }
      }

   public: // IKvds interface implementation


      bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         assert_data_store_open();

         ConstDbt kt(pkey, ksize);
         ConstDbt vt(pval, vsize);

         int rval = -1;

         KVDSDBDX__(rval, pdb_->put(0, &kt, &vt, 0));

         return 0 == rval;
      }

      bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         assert_data_store_open();

         ConstDbt kt(pkey, ksize);
         kt.set_ulen(ksize);

         // since we can't do a partial read from Bdb and getting size requires a full read
         // we might as well just read everything and return up to what was requested.

         MallocDbt vt;
         int rval = get(kt, vt);

         if(rval == 0)
         {
            vsize = std::min((size_t)vt.get_size(), vsize);
            memcpy(pval, vt.get_data(), vsize);
         }
         else { vsize = 0; }

         return 0 == rval;
      }

      bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         assert_data_store_open();

         int rval = -1;

         if(vsize > 0)
         {
            // Unfortunatly, BDB provides no way to just append data values to an existing key
            // So we must get the existing value, append the new values and write it back.

            ConstDbt kt(pkey, ksize);
            UsermemDbt evt;

            KVDSDBDX__(rval, pdb_->get(0, &kt, &evt, 0));

            switch(rval)
            {
            case DB_BUFFER_SMALL:
               // Item exists so get it, append new data and write back
               {
                  size_t const esize = evt.get_size();

                  /// Create abuffer big enough for exists + new
                  byte_array_t buf(esize + vsize);
                  evt.set_data(&buf[0]);
                  evt.set_ulen(esize);

                  // Get existing value
                  KVDSDBDX__(rval, pdb_->get(0, &kt, &evt, 0));

                  if(0 == rval)
                  {
                     // Append new value
                     memcpy(&buf[esize], pval, vsize);
                     evt.set_size(buf.size());
                     evt.set_ulen(buf.size());
                     KVDSDBDX__(rval, pdb_->put(0, &kt, &evt, 0));
                  }
               }
               break;
            case DB_NOTFOUND:
               // New item, just add it
               ConstDbt vt(pval, vsize);
               KVDSDBDX__(rval, pdb_->put(0, &kt, &vt, 0));
               break;
            }
         }
         else
         {
            rval = 0;
         }

         return 0 == rval;
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
         assert_data_store_open();

         ConstDbt kt(pkey, ksize);

         int rval = -1;

         KVDSDBDX__(rval, pdb_->exists(0, &kt, 0));

         return 0 == rval;
      }

      bool del(
         void const * pkey, size_t const ksize
         )
      {
         assert_data_store_open();

         ConstDbt kt(pkey, ksize);

         int rval = -1;

         KVDSDBDX__(rval, pdb_->del(0, &kt, 0));

         return 0 == rval;
      }

      bool clr()
      {
         assert_data_store_open();

         int rval = -1;

         unsigned int count = 0;
         KVDSDBDX__(rval, pdb_->truncate(0, &count, 0));

         return 0 == rval;
      }



      bool beg()
      {
         assert_data_store_open();
         if (pitr_ != 0) { pitr_->close(); }
         pdb_->cursor(0, &pitr_, 0);

         if(pitr_)
         {
            set_next_itr();
         }

         return pitr_ != 0;
      }

      bool nxt(
         void * pkey, size_t & ksize
         )
      {
         assert_data_store_open();

         int rval = -1;

         if(pitr_)
         {
            UsermemDbt kt(pkey, ksize);
            kt.set_ulen(ksize);
            MallocDbt vt;

            KVDSDBDX__(rval, pitr_->get(&kt, &vt, DB_CURRENT));

            switch(rval)
            {
            case 0:
               set_next_itr();
               break;
            case DB_BUFFER_SMALL:
               ksize = kt.get_size();
               break;
            }
         }
         else
         {
            ksize = 0;
         }

         return 0 == rval;
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

         ConstDbt kt(pkey, ksize);
         UsermemDbt vt;

         int rval = -1;
         KVDSDBDX__(rval, pdb_->get(0, &kt, &vt, 0));

         switch(rval)
         {
         case DB_BUFFER_SMALL:
            vsize = vt.get_size();
            rval = 0;
            break;
         case 0:
            vsize = 0;
            break;
         }

         return 0 == rval;
      }

      bool cnt(boost::uint64_t & cnt);

      bool nil(bool & isnil)
      {
         boost::uint64_t icnt = 0;
         bool ok = cnt(icnt);
         if(ok) { isnil = (0 == icnt); }
         return ok;
      }

private:

   int get(Dbt & kt, Dbt & vt)
   {
      int rval = -1;
      KVDSDBDX__(rval, pdb_->get(0, &kt, &vt, 0));
      return rval;
   }

   store_type pdb_;
      Dbc * pitr_;
   };

   // Stats for HASH and BTREE are obtained differently, hence the specialisation
   template<>
   inline bool KvdsBdb<DB_HASH>::cnt(boost::uint64_t & cnt)
   {
      assert_data_store_open();

      int rval = -1;

      ScopedFree<DB_HASH_STAT> sf;
      KVDSDBDX__(rval, pdb_->stat(0, (void *)&sf.p, 0));

      if(0 == rval && sf.p)
      {
         cnt = sf.p->hash_nkeys;
      }

      return 0 == rval;
   }

   template<>
   inline bool KvdsBdb<DB_BTREE>::cnt(boost::uint64_t & cnt)
   {
      assert_data_store_open();

      int rval = -1;

      ScopedFree<DB_BTREE_STAT> sf;
      KVDSDBDX__(rval, pdb_->stat(0, (void *)&sf.p, 0));

      if(0 == rval && sf.p)
      {
         cnt = sf.p->bt_nkeys;
      }

      return 0 == rval;
   }

   typedef KvdsBdb<DB_HASH>  KvdsBht; /// Berkeley DB Hash Table
   typedef KvdsBdb<DB_BTREE> KvdsBbt; /// Berkeley DB B-Tree

}}

#endif // MOOST_KVDS_KVDSBDB_LINUX_HPP__
