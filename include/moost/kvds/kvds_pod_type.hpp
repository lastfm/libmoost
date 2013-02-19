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

/// \file A common interface for all supported key/value data store types.

#include <vector>

#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>

#include "ikvds.hpp"

#ifndef MOOST_KVDS_KVDS_POD_TYPE_HPP__
#define MOOST_KVDS_KVDS_POD_TYPE_HPP__

namespace moost { namespace kvds {

   /// This class represents a generic wrapper for Plain Old Data types.
   /// If you wish to support other types you need to implement your own version
   /// of this class that can serialise and deserialise the object in question
   /// as well as a vector of objects to facilitate support for getall() & putall().
   /// Serialising integral types is simple since we can just read/write to them
   /// directly (assuming endianess isn't a factor) but with custom types it
   /// will be necessary to provide a mutable and non-mutable byte representation
   /// of the actual type that can be written to and read from the database. In the
   /// case of, say, a vector this would just be the internal buffer given the vectors
   /// internal representation guarantees but in the case of, say, a string this
   /// is likely to require a seperate byte buffer be maintained that is used to
   /// reassemble the string from a mutable action. This is likely to be no more
   /// complex than using the vector<char> as a buffer and copying to/from the string.

   /// *** This class is NOT thread safe ***

   template <typename T>
   class KvdsPodType /// KvdsType
   {
   public:
      typedef T kvds_type;

      /// This particular KvdsType only supports pod types
      BOOST_STATIC_ASSERT(boost::is_pod<T>::value);

      /// c_tor that takes a reference to the real type being represented
      KvdsPodType(T & t) : t_(t), size_(sizeof(T)) {}

      /// cc_tor
      KvdsPodType(KvdsPodType const & t) : t_(t.t_), size_(t.size_) {}

      /// Assignment
      KvdsPodType & operator = (KvdsPodType const & rhs)
      {
         t_ = rhs.t_;
         size_ = rhs.size_;
      }

      /// basic comparators
      bool operator == (T const & t) const { return t_ == t; }
      bool operator != (T const & t) const { return !(t_ == t); }
      bool operator == (KvdsPodType<T> const & t) const { return t_ == t.t_; }
      bool operator != (KvdsPodType<T> const & t) const { return !(*this == t); }

      /// Const operations
      ///
      /// These members handle the case where the type being represented is an
      /// immutable reference. In this case all we need do is provide an
      /// immutable byte buffer and size of that buffer, which contains a
      /// serialise form of the represented type for the kvds engine to store.

      /// Return the size of the const byte buffer
      size_t size() const { return size_; }

      /// Return pointer to const byte buffer
      kvds_type const * operator & () const { return &t_; }

      /// Return a reference to the type associated with the const byte buffer
      kvds_type & operator * () const { return t_; }

      /// Non-const operations
      ///
      /// These members handle the case where the type being represented is a
      /// mutable reference. In this case we need to provide a mutable byte
      /// buffer and size of that buffer, which contains a serialise form of
      /// the represented type for the kvds engine to store or modify. In the
      /// case where the buffer is modified the type needs to be reassembled
      /// from the modified buffer so to facilitate this Kvds will call the
      /// * operator, which will give the KvdsType the opportunity to update
      /// the value and size of the actual type being represented.

      /// Return the size of the mutable buffer
      size_t & size() { return size_; }

      /// Return pointer to mutable buffer
      kvds_type * operator & () { return &t_; }

      /// Return a reference to the type associated with the mutable buffer.
      /// If necessary the type should be assembled from the mutable byte buffer.
      /// This will be called after any operations that might have mutated the
      /// buffer to give the KvdsType a chance to assemble the type. If the buffer
      /// size not a divisor equal to sizeof(kvds_type) an exception should thrown.
      /// Obviously, since this operator may be called a number of times it should
      /// only reassemble the buffer if it's been modified. This could have been
      /// called as a seperate function call but if this call was forgotten the
      /// result from this operator would not be as expected and since there is
      /// only a trivial overhead in performing the necessary check to see if
      /// the buffer needs assembling it was deemed worth the cost for the sake
      /// of consistant behavior when calling this operator was worth it.
      kvds_type & operator * () { return t_; }

      /// To support getall and putall we need to handle vectors of kvds_type.
      class vector_type
      {
      public:
         /// Use these helpers to define the correct vector type
         /// dependent upon whether the kvds_type is const or non-const.
         template <typename U>
         struct kvds_vector
         {
            /// We want a nom-const vector with non-const values
            typedef std::vector<U> type;
            private: kvds_vector();
         };

         template <typename U>
         struct kvds_vector<U const>
         {
            /// We want a const vector but with non-const values
            typedef std::vector<typename boost::remove_const<U>::type> const type;
            private: kvds_vector();
         };

         /// Make a friendly typedef for our vector type
         typedef typename kvds_vector<kvds_type>::type kvds_vector_type;

         /// Although not neessary for interal types, during construction
         /// you would create a mutable byte buffer that represents the
         /// contents of the kvds_type vector, so it can be passed as a
         /// pointer and size to the kvds engine for serialisation.
         vector_type(kvds_vector_type & kvt) : kvt_(kvt), kvt_size_(kvt.size() * sizeof(kvds_type)) {}

         /// Resize the mutable byte buffer so it's large enough to accomodate any
         /// incoming buffer data, which will be used. If the size not a divisor of
         /// sizeof(kvds_type) an exception should thrown. In the case of integral
         /// types the mutable buffer is the vector of kvds_type's but for other types
         /// this will probably be a seperate buffer.
         void resize(size_t const size)
         {
            if(0 != (size % sizeof(kvds_type)))
            {
               throw std::runtime_error("size is invalid for kvds_type");
            }

            kvt_size_ = size;
            kvt_.resize(kvt_size_ / sizeof(kvds_type));
         }

         /// Return the size of the mutable byte buffer
         size_t size() const { return kvt_size_; }

         /// Return pointer to mutable byte buffer
         kvds_type * operator & () { return &kvt_[0]; }

         /// Return a reference to the kvds_vector_type associated with the mutable
         /// buffer. If necessary the kvds_type should be assembled from the mutable byte
         /// buffer. This will be called after any operations that might have mutated
         /// the buffer to give the KvdsType a chance to assemble the type. If the
         /// buffer size not a divisor of sizeof(kvds_type) an exception should thrown.
         /// In the case of integral types no assembly is required. Checks should be
         /// performed to only assemble the type if the byte buffer has been modified
         /// since this operator was last called to avoid any unnecessary overhead.
         kvds_vector_type & operator * () { return kvt_; }

      private:
         kvds_vector_type & kvt_;
         size_t kvt_size_;
      };

   private:
      kvds_type & t_;
      size_t size_;
   };

}}

#endif // MOOST_KVDS_KVDS_POD_TYPE_HPP__
