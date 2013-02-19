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

#include "ikvds.hpp"
#include "kvds_integral_type.hpp"

#ifndef MOOST_KVDS_KVDS_HPP__
#define MOOST_KVDS_KVDS_HPP__

namespace moost { namespace kvds {

   typedef boost::shared_ptr<IKvds> ikvds_ptr_t;

   /// Kvds can support any type for serialisation and deserialisation,
   /// but it requires a template template for KvdsTypeKey and KvdsTypeVal
   /// to handle the generic interface for conversion to an from a void *
   /// byte buffer. See kvds_integral_type.hpp for an example and brief
   /// documentation on how to develop additional KvdsType objects.

   /// Provides a nice user friendly adaptor for an ikvds interface.

   /// *** This class is NOT thread safe ***

   template <
      typename keyT, // the type of the key
      typename valT, // the type of the value

      // policy for serialising the key -- default is to treat it as an integeral type
      template <typename> class KvdsTypeKey = KvdsIntegralType,

      // policy for serialising the value -- default is to treat it as an integeral type
      template <typename> class KvdsTypeVal = KvdsIntegralType
   >
   class Kvds
   {
      template <typename kvdsT> friend class KvdsKeyIterator;

   public:
      typedef keyT key_type;
      typedef valT val_type;
      typedef std::vector<val_type> kvds_values_t;

      /// Some syntactic suger to make this more STL like.
      typedef std::pair<key_type, val_type> value_type;
      typedef val_type data_type;

   private:
      typedef KvdsTypeKey<key_type> kvds_key_t;
      typedef KvdsTypeVal<val_type> kvds_val_t;
      typedef KvdsTypeKey<key_type const> kvds_key_const_t;
      typedef KvdsTypeVal<val_type const> kvds_val_const_t;

   public:
      Kvds(ikvds_ptr_t ikvds_ptr) :
         ikvds_ptr_(ikvds_ptr)
         {
            assert(ikvds_ptr_); // catch it in debug!
            if(!ikvds_ptr_) { throw std::runtime_error("Datastore cannot be initialised with a null ikvds"); }
         }

         bool put(kvds_key_const_t const & key, kvds_val_const_t const & val) const
         {
            return ikvds_ptr_->put(
               &key, key.size(),
               &val, val.size()
               );
         }

         bool put(kvds_key_const_t const & key, kvds_values_t const & vals) const
         {
            typename kvds_val_const_t::vector_type kvds_val_vector(vals);

            return ikvds_ptr_->put(
               &key, key.size(),
               &kvds_val_vector, kvds_val_vector.size()
               );
         }

         bool get(kvds_key_const_t const & key, kvds_val_t val) const
         {
            bool found = ikvds_ptr_->get(
               &key, key.size(),
               &val, val.size()
               );

            *val; // Reassemble val

            return found;
         }

         bool get(kvds_key_const_t const & key, kvds_values_t & vals, size_t const count) const
         {
            vals.resize(count);
            typename kvds_val_t::vector_type kvds_val_vector(vals);

            size_t esize = kvds_val_vector.size();

            bool found = esize > 0 ? ikvds_ptr_->get(&key, key.size(), &kvds_val_vector, esize) : false;

            if(found)
            {
               if(esize < kvds_val_vector.size())
               {
                  // read less than the buffer size so before we must resize buffer
                  kvds_val_vector.resize(esize);
               }

               *kvds_val_vector; // Reassemble vals
            }

            return found;
         }

         bool get(kvds_key_const_t const & key, kvds_values_t & vals) const
         {
            typename kvds_val_t::vector_type kvds_val_vector(vals);

            bool found = false;
            size_t esize = 0;

            if(ikvds_ptr_->siz(&key, key.size(), esize))
            {
               kvds_val_vector.resize(esize);

               found = ikvds_ptr_->all(&key, key.size(), &kvds_val_vector, esize);

               if(found)
               {
                  *kvds_val_vector; // Reassemble vals
               }
            }

            return found;
         }

         bool insert(value_type const & kvp) // Just to be more STL map like
         {
            return put(kvp.first, kvp.second);
         }

         bool add(kvds_key_const_t const & key, kvds_val_const_t const & val) const
         {
            return ikvds_ptr_->add(
               &key, key.size(),
               &val, val.size()
               );
         }

         bool exists(kvds_key_const_t const & key) const
         {
            return ikvds_ptr_->xst(&key, key.size());
         }

         bool erase(kvds_key_const_t const & key) const
         {
            return ikvds_ptr_->del(&key, key.size());
         }

         bool clear() const
         {
            return ikvds_ptr_->clr();
         }

         bool size(kvds_key_const_t const & key, size_t & size) const
         {
            bool found = ikvds_ptr_->siz(&key, key.size(), size);

            if(found && (0 != (size % sizeof(typename kvds_values_t::value_type))))
            {
               throw std::runtime_error("value type boundary error");
            }

            size /= sizeof(typename kvds_values_t::value_type);

            return found;
         }

         size_t size(kvds_key_const_t const & key) const
         {
            size_t size = 0;

            if(!this->size(key, size))
            {
               throw std::runtime_error("item not found");
            }

            return size;
         }

         bool count(boost::uint64_t & cnt) const
         {
            return ikvds_ptr_->cnt(cnt);
         }

         boost::uint64_t size() const // Just to be more STL like
         {
            boost::uint64_t cnt = 0;

            if(!count(cnt))
            {
                throw std::runtime_error("size not available");
            }

            return cnt;
         }

         bool empty() const
         {
            bool isnil = false;

            if(!ikvds_ptr_->nil(isnil))
            {
                throw std::runtime_error("empty state is not available");
            }

            return isnil;
         }

         /// Just in case you need access to the low level interface, but why would you?
         IKvds & get_ikvds() const
         {
            /// Mess with this at your own risk!!!
            if(!ikvds_ptr_) { throw std::runtime_error("Invalid kvds interface"); }
            return *ikvds_ptr_;
         }

         ikvds_ptr_t get_ikvds_ptr() const
         {
            /// Mess with this at your own risk!!!
            return ikvds_ptr_;
         }

   private:
      /// Index operators normally return a reference to a value held within
      /// and when you then assign to an inderer you are actually assigning
      /// to the reference that has been returned. This won't work for a kvds
      /// because the reference returned would not update the store if it was
      /// modified. To get around this we return a value that is wrapped in
      /// a special class that implements an operator = and a cast operator.
      /// On return, if an assignment is made it will invoke the operator =
      /// which will update the value and the kvds store. If the result is
      /// assigned to a val_type the cast operator is invoked to return a
      /// copy of the value_type. The cost of this convenience is that a copy
      /// of the value type must be taken. In relality most compilers will
      /// optimise this away using RVO (Return Value Optimisation).
      /// http:///en.wikipedia.org/wiki/Return_value_optimization

      class Iow // Indexed object wrapper (needs no external visibility!)
      {
      public:
         Iow(
            Kvds & kvds,
            kvds_key_const_t const & key
            ) : kvds_(kvds), key_(key), val_(val_type()) {}

         Iow & operator = (val_type const & val)
         {
            val_ = val;

            if(!kvds_.put(key_, val_))
            {
               throw std::runtime_error("Write to datastore failed");
            }

            return *this;
         }

         val_type & operator * () { return val_; }

         operator val_type const () { return val_; }

      private:
         Kvds & kvds_;
         kvds_key_const_t const & key_;
         val_type val_;
      };
   public:
      /// Note that the indexer is semantically the same as a put
      /// meaning that any existing values will be overwritten!
      Iow operator [](kvds_key_const_t const & key)
      {
         Iow iow(*this, key);

         if(!get(key, *iow))
         {
            // It doesn't exist, create one
            iow = val_type();
         }

         return iow;
      }

   private:
      ikvds_ptr_t ikvds_ptr_;
   };

}}

#endif /// MOOST_KVDS_KVDS_HPP__
