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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_VECTOR_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_VECTOR_HPP__

#include <string>
#include <stdexcept>
#include <iterator>

#include <boost/type_traits/is_pod.hpp>
#include <boost/noncopyable.hpp>

#include "section_writer_base.hpp"

namespace moost { namespace container {

/**
 * Memory-mapped dataset section representing a POD vector
 *
 * This is the most useful type of section as it allows using huge
 * vectors (millions of elements) of data without loading the entire
 * data to memory.
 *
 * A mmd_vector can be used more or less like a standard vector, i.e.
 * it supports access through iterators as well as indexed access.
 */
template <typename T>
class mmd_vector : public boost::noncopyable
{
   BOOST_STATIC_ASSERT_MSG(boost::is_pod<T>::value, "mmd_vector<> template can only handle POD types");

public:
   static const size_t MMD_VECTOR_ALIGNMENT = 16;

   typedef const T& const_reference;
   typedef const T *const_iterator;
   typedef size_t size_type;
   typedef T value_type;
   typedef const T* const_pointer;
   typedef std::random_access_iterator_tag iterator_category;

   class writer : public mmd_section_writer_base
   {
   public:
      // required to make push_back work with std::back_inserter()
      typedef const T& const_reference;
      typedef size_t size_type;
      typedef T value_type;

      writer(memory_mapped_dataset::writer& wr, const std::string& name, size_t alignment = MMD_VECTOR_ALIGNMENT)
         : mmd_section_writer_base(wr, name, "mmd_vector", alignment)
         , m_size(0)
      {
         setattr("elem_size", sizeof(value_type));
      }

      writer& operator<< (const_reference e)
      {
         push_back(e);
         return *this;
      }

      void push_back(const_reference e)
      {
         write(e);
         ++m_size;
      }

      size_type size() const
      {
         return m_size;
      }

   protected:
      void pre_commit()
      {
         setattr("size", m_size);
      }

   private:
      size_type m_size;
   };

   mmd_vector()
      : m_begin(0)
      , m_end(0)
   {
   }

   mmd_vector(const memory_mapped_dataset& mmd, const std::string& name)
   {
      set(mmd, name);
   }

   void set(const memory_mapped_dataset& mmd, const std::string& name)
   {
      const memory_mapped_dataset::section_info& info = mmd.find(name, "mmd_vector");

      if (info.getattr<size_t>("elem_size") != sizeof(T))
      {
         throw std::runtime_error("wrong element size for vector " + name + " in dataset " + mmd.description());
      }

      size_type size = info.getattr<size_type>("size");
      m_begin = mmd.data<T>(info.offset(), size);
      m_end = m_begin + size;
   }

   void warm_cache() const
   {
      memory_mapped_dataset::warm_cache(m_begin, m_end);
   }

   const_iterator begin() const
   {
      return m_begin;
   }

   const_iterator end() const
   {
      return m_end;
   }

   size_type size() const
   {
      return m_end - m_begin;
   }

   bool empty() const
   {
      return size() == 0;
   }

   const_reference operator[] (size_type ix) const
   {
      return m_begin[ix];
   }

private:
   const_iterator m_begin;
   const_iterator m_end;
};

}}

#endif
