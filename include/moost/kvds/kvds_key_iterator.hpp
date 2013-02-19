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

/// \file A common interface for all supported key/value data stores.
#include <cassert>
#include <stdexcept>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>

#ifndef MOOST_KVDS_KVDS_ITERATOR_HPP__
#define MOOST_KVDS_KVDS_ITERATOR_HPP__

namespace moost { namespace kvds {

   /// Unfortunately, not all datastores support 'cursors' which could be used
   /// to encapsulate the iterator semantics inside the iterator and would have
   /// meant begin() and end() methods could have been provided on Kvds. Instead,
   /// the semantics of this iterator are very similar to an istream_iterator
   /// in so far as you must provide it a source to iterate over. The iterator
   /// process may modify the state of the source. Since this is the case, it must
   /// be noted that only one iterator can sucessfully operate on the source at
   /// any one time and any operation that modifies the source other than by
   /// calling the iterator shall leave the iterator in an invalid state.
   /// This iterator is a sequencial forward reading, read-only iterator. Upon
   /// construction, the iterator needs to be passed a kvds source. It will ensure
   /// The source is reading to start reading from the very first key in the store.
   /// Keys are not iterated in any specific order since some of the datastores do
   /// support this. The special case of an iterator constructed without a source
   /// represents the end iterator (again, just like istream_iterator).

   /// This iterator returns keys, oyou must operform a seperate lookup using
   /// that key if you wish to obtain the value(s) associated with it.

   /// *** This class is NOT thread safe ***

   template <typename kvdsT>
   class KvdsKeyIterator
   {
   public:
      typedef typename kvdsT::key_type key_type;

   private:
      typedef typename kvdsT::kvds_key_t kvds_key_t;

      void get_next_key()
      {
         assert(ikvds_ptr_);

         if(ikvds_ptr_)
         {
            if(ikvds_ptr_->nxt(&key_, key_.size()))
            {
               *key_; /// Assemble key
               ++pos_;
            }
            else
            {
               if(key_.size() != 0)
               {
                  throw std::runtime_error("unexpected key size");
               }

               ikvds_ptr_.reset();
            }
         }
      }

      void init()
      {
         assert(ikvds_ptr_);

         if(!ikvds_ptr_ || !ikvds_ptr_->beg())
         {
            throw std::runtime_error("failed to initialise iterator");
         }

         get_next_key();
      }

   public:
      KvdsKeyIterator() : key__(0), key_(key__), pos_(0) {}

      KvdsKeyIterator(kvdsT const & kvds) :
         key__(0), key_(key__), ikvds_ptr_(kvds.get_ikvds_ptr())
      {
         init();
      }

      KvdsKeyIterator(ikvds_ptr_t ikvds_ptr) :
         key__(0), key_(key__), pos_(0), ikvds_ptr_(ikvds_ptr)
      {
         init();
      }

      KvdsKeyIterator(KvdsKeyIterator const & rhs) :
         key__(rhs.key__), key_(key__), pos_(rhs.pos_), ikvds_ptr_(rhs.ikvds_ptr_)
      {
      }

      KvdsKeyIterator & operator = (KvdsKeyIterator const & rhs)
      {
         if(this != &rhs)
         {
            ikvds_ptr_ = rhs.ikvds_ptr_;
            key__ = rhs.key__;
            pos_ = rhs.pos_;
         }

         return *this;
      }

      KvdsKeyIterator & operator ++ () // prefix
      {
         if(!ikvds_ptr_)
         {
            assert(false);
            throw std::runtime_error("invalid iterator");
         }

         get_next_key();

         return *this;
      }

      KvdsKeyIterator operator ++ (int) // postfix
      {
         KvdsKeyIterator tmp = *this;

         if(!ikvds_ptr_)
         {
            assert(false);
            throw std::runtime_error("invalid iterator");
         }

         get_next_key();

         return tmp;
      }

      bool operator == (KvdsKeyIterator const & rhs) const
      {
         if(!ikvds_ptr_ && !rhs.ikvds_ptr_) { return true; }
         if(ikvds_ptr_ && rhs.ikvds_ptr_) { return pos_ == rhs.pos_; }
         return false;
      }

      bool operator != (KvdsKeyIterator const & rhs) const
      {
         return !(*this == rhs);
      }

      key_type const & operator * () const
      {
         if(!ikvds_ptr_)
         {
            assert(false);
            throw std::runtime_error("invalid iterator");
         }

         return *key_;
      }

   private:
      key_type key__;
      kvds_key_t key_;
      size_t pos_;
      ikvds_ptr_t ikvds_ptr_;
   };


}}

#endif // MOOST_KVDS_KVDS_ITERATOR_HPP__
