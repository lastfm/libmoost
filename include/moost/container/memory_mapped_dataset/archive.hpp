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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_ARCHIVE_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_ARCHIVE_HPP__

#include <string>
#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/noncopyable.hpp>

#include "section_writer_base.hpp"

namespace moost { namespace container {

struct TextArchivePolicy
{
   static const char *archive_type()
   {
      return "text";
   }

   typedef boost::archive::text_oarchive oarchive_t;
   typedef boost::archive::text_iarchive iarchive_t;
};

struct BinaryArchivePolicy
{
   static const char *archive_type()
   {
      return "binary";
   }

   typedef boost::archive::binary_oarchive oarchive_t;
   typedef boost::archive::binary_iarchive iarchive_t;
};

/**
 * Memory-mapped dataset section representing an archive of items
 *
 * This section can be used to store meta information for a dataset.
 * It needs to be deserialised to RAM in order to be used, so it's
 * not useful for storing large amounts of data.
 */
template <class ArchivePolicy>
class mmd_generic_archive : public boost::noncopyable
{
public:
   class writer : public mmd_section_writer_base
   {
   public:
      writer(memory_mapped_dataset::writer& wr, const std::string& name)
         : mmd_section_writer_base(wr, name, std::string("mmd_archive:") + ArchivePolicy::archive_type(), 1)
         , m_oss()
         , m_oa(m_oss)
      {
      }

      template <typename T>
      writer& operator<< (const T& e)
      {
         m_oa << e;
         return *this;
      }

   protected:
      void pre_commit()
      {
         write(m_oss.str());
         setattr("size", m_oss.str().size());
      }

   private:
      std::ostringstream m_oss;
      typename ArchivePolicy::oarchive_t m_oa;
   };

   mmd_generic_archive()
      : m_ia(m_iss)
   {
   }

   mmd_generic_archive(const memory_mapped_dataset& mmd, const std::string& name)
      : m_iss(archive_data(mmd, name))
      , m_ia(m_iss)
   {
   }

   void set(const memory_mapped_dataset& mmd, const std::string& name)
   {
      m_iss.str(archive_data(mmd, name));
   }

   template <typename T>
   mmd_generic_archive& operator>> (T& value)
   {
      m_ia >> value;
      return *this;
   }

private:
   static std::string archive_data(const memory_mapped_dataset& mmd, const std::string& name)
   {
      const memory_mapped_dataset::section_info& info = mmd.find(name, std::string("mmd_archive:") + ArchivePolicy::archive_type());
      size_t size = info.getattr<size_t>("size");
      return std::string(mmd.data<char>(info.offset(), size), size);
   }

   std::istringstream m_iss;
   typename ArchivePolicy::iarchive_t m_ia;
};

typedef mmd_generic_archive<TextArchivePolicy> mmd_text_archive;
typedef mmd_generic_archive<BinaryArchivePolicy> mmd_binary_archive;
typedef mmd_text_archive mmd_archive;

}}

#endif
