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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_SECTION_WRITER_BASE_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_SECTION_WRITER_BASE_HPP__

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/type_traits/is_pod.hpp>
#include <boost/noncopyable.hpp>

#include "dataset.hpp"

namespace moost { namespace container {

/**
 * Base class for section writing accessors
 */
class mmd_section_writer_base : public boost::noncopyable
{
public:
   /**
    * Constructor for section writer
    *
    * \param wr          reference to a dataset writer
    * \param name        section name
    * \param type        section type
    * \param alignment   section alignment (section will always start at an
    *                    offset that is a multiple of this value); must be
    *                    a power of 2
    */
   mmd_section_writer_base(memory_mapped_dataset::writer& wr, const std::string& name, const std::string& type, size_t alignment)
      : m_name(name)
      , m_wr(wr)
      , m_committed(false)
   {
      wr.create_section(name, type, alignment);
   }

   virtual ~mmd_section_writer_base()
   {
      try
      {
         commit();
      }
      catch (...)
      {
      }
   }

   /**
    * Commit the section
    *
    * All data has been written to the section and the accessor can
    * now finish writing the section to the dataset and updating the
    * index.
    *
    * This method should always be called explicitly in order to avoid
    * hidden exceptions when it is called by the destructor.
    */
   void commit()
   {
      if (!m_committed)
      {
         pre_commit();
         m_wr.commit_section(m_name);
         m_committed = true;
      }
   }

protected:
   void rollback()
   {
      m_wr.uncreate_section(m_name);
   }

   template <typename T>
   void setattr(const std::string& attr, const T& value)
   {
      m_wr.setattr(m_name, attr, value);
   }

   template <typename T>
   void write(const std::vector<T>& vec)
   {
      BOOST_STATIC_ASSERT_MSG(boost::is_pod<T>::value, "only POD types can be written to a dataset");
      write(reinterpret_cast<const char *>(&vec[0]), vec.size()*sizeof(T));
   }

   template <typename T>
   void write(const T& value)
   {
      BOOST_STATIC_ASSERT_MSG(boost::is_pod<T>::value, "only POD types can be written to a dataset");
      write(reinterpret_cast<const char *>(&value), sizeof(value));
   }

   void write(const std::string& value)
   {
      write(&value[0], value.size());
   }

   void write(const char *value, size_t size)
   {
      if (m_committed)
      {
         throw std::runtime_error("write access to committed section");
      }
      m_wr.write(m_name, value, size);
   }

   virtual void pre_commit()
   {
   }

private:
   const std::string m_name;
   memory_mapped_dataset::writer& m_wr;
   bool m_committed;
};

}}

#endif
