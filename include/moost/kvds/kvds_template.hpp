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
/// This is a template for implementing a new kvds data store.
/// This template should be copied and NOT modified directly!

#include "ikvds.hpp"

#ifndef MOOST_KVDS_KVDS_TEMPLATE_HPP__
#define MOOST_KVDS_KVDS_TEMPLATE_HPP__

namespace moost { namespace kvds {

   /// *** This class is NOT thread safe ***

   class KvdsTemplate : public IKvds
   {
   public:
      /// Change this to whatever the correct type is (or remeove if unused)
      typedef void * store_type;

      /// Set store to null pointer for now, replace as appropriate
      KvdsTemplate() : store_(0)
      {
         // Construction code goes here
      }

      ~KvdsTemplate()
      {
         if(!dsname_.empty())
         {
            close(); // Ensure DB is closed
         }
      }

      void open(char const dsname [], bool newdb = false)
      {
         dsname_ = dsname;

         if(newdb)
         {
            // Truncate database if it already exists
         }
         else
         {
            // Open database
         }
      }

      void save()
      {
         // Add save code here
      }

      void close()
      {
         if(!dsname_.empty())
         {
            save();

            // Add close code

            dsname_.clear(); // Make sure closed isn't called mutliple times
         }
      }

   public:
      /// You can call this to get a refernce to the underlying store. Useful
      /// if you need to do post construction configuration. For example if using
      /// a google hash or spare map you need to specify the delete/erase keys

      store_type & get_store() { return store_; } // Remove if not applicable

   public:
      // IKvds interface implementation

      bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         return false;
      }

      bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         return false;
      }

      bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         )
      {
         return false;
      }

      bool all(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         )
      {
         return false;
      }

      bool xst(
         void const * pkey, size_t const ksize
         )
      {
         return false;
      }

      bool del(
         void const * pkey, size_t const ksize
         )
      {
         return false;

      }

      bool clr()
      {
         return true;
      }

      bool beg()
      {
         return false;
      }

      bool nxt(
         void * pkey, size_t & ksize
         )
      {
         return false;

      }

      bool end()
      {
         return true;
      }

      bool siz(
         void const * pkey, size_t const ksize,
         size_t & vsize
         )

      {
         return false;
      }

      bool cnt(boost::uint64_t & cnt)
      {
         return false;
      }

      bool nil(bool & isnil)
      {
         isnil = true;
         return true;
      }

   private:
      store_type store_;
      std::string dsname_;
   };
}}

#endif // MOOST_KVDS_KVDS_TEMPLATE_HPP__
