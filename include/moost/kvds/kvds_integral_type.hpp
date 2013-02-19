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

#include "kvds_pod_type.hpp"

#ifndef MOOST_KVDS_KVDS_INTEGRAL_TYPE_HPP__
#define MOOST_KVDS_KVDS_INTEGRAL_TYPE_HPP__

namespace moost { namespace kvds {

   /// This class represents a generic wrapper for standard C/C++ integral types.
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

   template <typename T> // integral types are also pods but not the other way around!
   class KvdsIntegralType : public KvdsPodType<T> /// KvdsType
   {
   public:
      typedef T kvds_type;

      /// This particular KvdsType only supports standard C/C++ integral types
      BOOST_STATIC_ASSERT(boost::is_integral<T>::value);

      /// c_tor that takes a reference to the real type being represented
      KvdsIntegralType(T & t) : KvdsPodType<T>(t) {}

      /// cc_tor
      KvdsIntegralType(KvdsIntegralType const & t) : KvdsPodType<T>(t) {}

      /// Assignment
      KvdsIntegralType & operator = (KvdsIntegralType const & rhs)
      {
         this->t_ = rhs.t_;
         this->size_ = rhs.size_;
      }
   };

}}

#endif // MOOST_KVDS_KVDS_INTEGRAL_TYPE_HPP__
