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

#include <boost/cstdint.hpp> // uint64_t
#include <boost/noncopyable.hpp>

#ifndef MOOST_KVDS_IKVDS_HPP__
#define MOOST_KVDS_IKVDS_HPP__

namespace moost { namespace kvds {

   /// Ensure all Kvds objects inherit noncopable semantics from the interface.
   /// Not all data stores are safe to copy without explicit implementation of
   /// a cc_tor and asignment operator. We want each store to be semantically
   /// identical so to make life simple just force them all to be noncopyable.
   ///
   /// Notes:
   ///    - unless stated otherwise functions return true for sucess, else false
   ///    - functions that can't be supported by a datastore return false
   ///    - functions may percolate std::exceptions for unexpected errors
   ///    - the caller is responsible for allocating memory for get operations
   ///    - the caller is responsible for checking result buffer type alignment
   ///    - value iteration is not available because not all db's support this
   ///    - for flexibility none of the interface functions are declared const
   ///    - no function in this interface should be considered thread safe
   ///    - pkey and pval are NOT guarenteed to be null checked
   ///    - if pkey or pval are passed as null the result is undefined
   ///
   /// Common parameters:
   ///
   ///    pkey  : pointer to a key buffer
   ///    vsize : size of the bufer pointed to by pkey
   ///    pval  : pointer to a value buffer
   ///    vsize : size of the bufer pointed to by pval
   ///
   /// NB. Depending upon the function any of these may be in and/or out params
   ///

   struct IKvds : boost::noncopyable
   {

      /// put the key/value into the datastore, overwriting value if the key exists
      virtual bool put(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         ) = 0;

      /// get vsize amount of data for a given key. It is up to the caller to
      /// ensure that the size of the data returned is a multiple of the size of
      /// the actual type being stored and if this is not the case use of the data
      /// is undefined. If the key exists the call will return a value as big as
      /// vsize; however, this may not be the full amount of data stored if vsize
      /// is smaller than the size of the data in the store. If vsize is larger
      /// than the data available only what can be found is returned.
      /// In all success cases vsize will be set to the actual size of the data
      /// being returned. If this function returns false the result values of the
      /// value data and/or vsize are undefined. If the key doesn't exist the result
      /// is false and vsize is set to 0.
      virtual bool get(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         ) = 0;

      /// add the key/value into the datastore, appending value if the key exists
      /// NB. returns false if the datastore doesn't support multi-values for keys
      virtual bool add(
         void const * pkey, size_t const ksize,
         void const * pval, size_t const vsize
         ) = 0;

      /// get all the data for a given key. It is up to the caller to ensure that
      /// the size of the data returned is a multiple of the size of the actual
      /// type being stored and if this is not the case use of the data is
      /// undefined. If the key cannot be found or vsize is too small the function
      /// will fail. If the key doesn't exist pval and vsize will be set to 0.
      /// If the key does exist but vsize is too small then pval will be set to 0
      /// but vsize will be set to the size required. If vsize is greater than
      /// the size require the call will succeed and vsize will be set to the
      /// actual size of the value pointed to by pval.
      virtual bool all(
         void const * pkey, size_t const ksize,
         void * pval, size_t & vsize
         ) = 0;

      /// see if a key/val entry exists in the datastore
      virtual bool xst(
         void const * pkey, size_t const ksize
         ) = 0;

      /// deletes a key/val entry in the datastore
      virtual bool del(
         void const * pkey, size_t const ksize
         ) = 0;

      /// clears the datastore
      virtual bool clr() = 0;

      /// Begin iterating the keys of the datastore (must be called before nxt)
      virtual bool beg() = 0;

      /// get the next key out of the datastore for this iterator session. If the
      /// return value is true a valid key shall be returned. If the retun value
      /// is false this will be either because the iteration has complete or ksize
      /// is too small. If the iteration has completed end() will return true ksize
      /// will be set to 0. If ksize is too small end() will return false and ksize
      /// will be set to the size required. If ksize is larger than required the
      /// call will succeed and ksize will be set to the actual size of the key
      /// pointed to by pkey. The order the keys are iterated is undefined and is
      /// dependent upon the datastore being used. If the datastore is updated
      /// during iteraion the result of a call to nxt() is undefined unless beg()
      /// is called to restart the iteration process.
      virtual bool nxt(
         void * pkey, size_t & ksize
         ) = 0;

      /// returns false if currently in the middle of iterating keys else false
      /// note, false will also be returned if beg() hasn't been called.
      virtual bool end() = 0;

      /// get the size of the value data for a particular key
      virtual bool siz(
         void const * pkey, size_t const ksize,
         size_t & vsize
         ) = 0;

      /// get the key/val gets a count of the number of unique keys stored in the
      /// data store, returning false if this value isn't avaliable.
      virtual bool cnt(boost::uint64_t & cnt) = 0;

      /// isnil is set true if there are nil records in the database, else false.
      /// Returns false if this cannot be established.
      virtual bool nil(bool & isnil) = 0;

      virtual ~ IKvds() {} /// Allow deletion via pointer to base class
   };

}}

#endif /// MOOST_KVDS_IKVDS_HPP__
