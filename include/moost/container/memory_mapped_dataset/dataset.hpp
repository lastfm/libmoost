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

#ifndef MOOST_CONTAINER_MEMORY_MAPPED_DATASET_DATASET_HPP__
#define MOOST_CONTAINER_MEMORY_MAPPED_DATASET_DATASET_HPP__

#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>

#include "config.hpp"

namespace moost { namespace container {

/**
 * Representation of a memory-mapped dataset
 *
 * Each instance of this class represents a memory-mapped dataset.
 * Sections within the dataset can be accessed using accessor classes,
 * e.g. mmd_vector, mmd_hash_multimap or mmd_archive.
 *
 * The dataset class as well as the accessor classes contain writer
 * classes than be used to easily implement methods for writing a
 * dataset to disk.
 */
class memory_mapped_dataset : public boost::noncopyable
{
private:
   static const boost::uint32_t MMD_MAGIC = 0x7473614C;    ///< helps us to identify the file and verify byte order
   static const boost::uint32_t MMD_VERSION = 1;           ///< increment in case of incompatible format changes

   static const size_t MAP_PAGE_SIZE = 4096;               ///< page size used for "warming" the mapping cache

   /**
    * The dataset file header
    *
    * Contains the magic and version as well as the offset and length of
    * the index archive. The index as well as the sections can be placed
    * at an arbitrary position within the dataset, though usually the
    * index will be the last part of the dataset. (As in a good book!)
    */
   struct mmd_header // must be POD
   {
      boost::uint32_t mmd_magic;              ///< magic number, must be set to MMD_MAGIC
      boost::uint32_t mmd_version;            ///< current format version, must be MMD_VERSION
      boost::uint64_t index_offset;           ///< offset of the index archive
      boost::uint64_t index_length;           ///< length of the index archive
   };

public:
   /**
    * Section information
    *
    * This class is used to store and access information about a single
    * section in the dataset, such as the offset at which the section is
    * located, the type of the section and attributes associated with
    * this section.
    *
    * Instances of this class can be serialised and deserialised using
    * boost::archive.
    *
    * This class is usually only used by section accessor classes.
    */
   class section_info
   {
   private:
      typedef std::map<std::string, std::string> attribute_map_type;

   public:
      section_info()
         : m_offset(0)
         , m_alignment(0)
      {
      }

      section_info(const std::string& type, size_t alignment)
         : m_type(type)
         , m_offset(0)
         , m_alignment(alignment)
      {
      }

      boost::uint64_t offset() const
      {
         return m_offset;
      }

      size_t alignment() const
      {
         return m_alignment;
      }

      void set_offset(boost::uint64_t offset)
      {
         m_offset = offset;
      }

      template <typename T>
      void setattr(const std::string& name, const T& value)
      {
         m_attributes[name] = boost::lexical_cast<std::string>(value);
      }

      template <typename T>
      const T getattr(const std::string& name) const
      {
         attribute_map_type::const_iterator it = m_attributes.find(name);

         if (it == m_attributes.end())
         {
            throw std::runtime_error("no such attribute " + name);
         }

         return boost::lexical_cast<T>(it->second);
      }

      const std::string& type() const
      {
         return m_type;
      }

   private:
      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive & ar, const unsigned int /* version */)
      {
         ar & m_type & m_offset & m_attributes;
      }

      std::string m_type;
      boost::uint64_t m_offset;
      const size_t m_alignment;
      attribute_map_type m_attributes;
   };

   typedef std::map<std::string, section_info> section_map_type;

   /**
    * Dataset writer
    *
    * This class is used to aid dataset creation. It handles all writing
    * of data to the dataset file and keeps track of all its sections.
    * It also takes care of aligning sections properly and writing the
    * section index.
    *
    * Most of its methods are supposed to be called only by section
    * creating accessors.
    */
   class writer
   {
   public:
      /**
       * Dataset writer constructor
       *
       * Used to open a dataset file for writing.
       *
       * \param map_file_name       name of the file to write
       * \param dataset_name        name of the dataset, used for validation
       *                            when reading the file
       * \param format_version      dataset format version, used for validation
       *                            when reading the file
       */
      writer(const std::string& map_file_name, const std::string& dataset_name, boost::uint32_t format_version)
         : m_ofs(map_file_name.c_str(), std::ios::binary | std::ios::trunc)
         , m_dataset_name(dataset_name)
         , m_format_version(format_version)
      {
         if (!m_ofs)
         {
            throw std::runtime_error("failed to open file " + map_file_name);
         }

         if (dataset_name.empty())
         {
            throw std::runtime_error("empty dataset name");
         }

         m_header.mmd_magic = MMD_MAGIC;
         m_header.mmd_version = MMD_VERSION;
         m_header.index_offset = 0;
         m_header.index_length = 0;

         write(m_header);
      }

      ~writer()
      {
         try
         {
            close();
         }
         catch (...)
         {
         }
      }

      /**
       * Close the dataset file
       *
       * This method should always be called explicitly in order to avoid
       * hidden exceptions when it is called by the destructor.
       */
      void close()
      {
         if (m_ofs.is_open())
         {
            m_header.index_offset = m_ofs.tellp();
            boost::archive::text_oarchive oa(m_ofs);
            oa << m_dataset_name << m_format_version << m_section_map;
            m_header.index_length = static_cast<boost::uint64_t>(m_ofs.tellp()) - m_header.index_offset;
            m_ofs.seekp(0);
            write(m_header);
            m_ofs.close();
         }
      }

      void create_section(const std::string& name, const std::string& type, size_t alignment)
      {
         if (name.empty())
         {
            throw std::runtime_error("invalid empty section name");
         }

         if (type.empty())
         {
            throw std::runtime_error("invalid empty section type");
         }

         if (alignment == 0 || (alignment & (alignment - 1)) != 0)
         {
            throw std::runtime_error("alignment must be a power of 2");
         }

         std::pair<section_map_type::iterator, bool> rv = m_section_map.insert(std::make_pair(name, section_info(type, alignment)));

         if (!rv.second)
         {
            throw std::runtime_error("attempt to create duplicate section " + name);
         }
      }

      void uncreate_section(const std::string& name)
      {
         if (name.empty())
         {
            throw std::runtime_error("invalid empty section name");
         }

         section_map_type::iterator it = m_section_map.find(name);

         if (it == m_section_map.end())
         {
            throw std::runtime_error("attempt to uncreate non-existent section " + name);
         }

         if (it->second.offset())
         {
            throw std::runtime_error("cannot uncreate section " + name + " that has already been written to");
         }

         m_section_map.erase(it);
      }

      void write(const std::string& section, const char *data, size_t size)
      {
         set_active_section(section);
         m_ofs.write(data, size);
      }

      void commit_section(const std::string& section)
      {
         set_active_section(section);
      }

      template <typename T>
      void setattr(const std::string& section, const std::string& attr, const T& value)
      {
         find(section).setattr(attr, value);
      }

   private:
      section_info& find(const std::string& section)
      {
         section_map_type::iterator it = m_section_map.find(section);

         if (it == m_section_map.end())
         {
            throw std::runtime_error("no such section " + section);
         }

         return it->second;
      }

      template <typename T>
      void write(const T& data)
      {
         m_ofs.write(reinterpret_cast<const char *>(&data), sizeof(data));
      }

      void set_active_section(const std::string& section)
      {
         // Yeah, this is slightly inefficient, but it's only being used at
         // dataset creation time.
         if (section != m_active_section)
         {
            section_info& sec = find(section);

            // We have to ensure that only one section is written at a time.
            if (sec.offset() > 0)
            {
               throw std::runtime_error("interleaved write access to section " + section);
            }

            align_stream(sec.alignment());
            sec.set_offset(m_ofs.tellp());
            m_active_section = section;
         }
      }

      void align_stream(size_t alignment)
      {
         while (m_ofs.tellp() % alignment)
         {
            m_ofs.put(0);
         }
      }

      section_map_type m_section_map;
      std::string m_active_section;
      std::ofstream m_ofs;
      const std::string m_dataset_name;
      const boost::uint32_t m_format_version;
      mmd_header m_header;
   };

   /**
    * Memory-mapped dataset constructor
    *
    * Used to map an existing on-disk dataset into memory. Performs a whole
    * variety of checks to ensure the dataset is consistent and matches the
    * specification given by dataset_name and format_version.
    *
    * \param map_file_name       name of the dataset file to map into memory
    * \param dataset_name        name of the dataset, used for validation
    * \param format_version      dataset format version, used for validation
    */
   memory_mapped_dataset(const std::string& map_file_name,
                         const std::string& dataset_name,
                         boost::uint32_t format_version)
      : m_file(map_file_name)
      , m_format(dataset_name)
   {
      try
      {
         m_map.open(m_file, boost::iostreams::mapped_file::readonly);
      }
      catch (const BOOST_IOSTREAMS_FAILURE& fail)
      {
         // otherwise it's a real pain to figure out which file it's actually complaining about
         throw BOOST_IOSTREAMS_FAILURE(m_file + ": " + fail.what());
      }

      const mmd_header *hdr = data<mmd_header>();

      if (hdr->mmd_magic != MMD_MAGIC)
      {
         throw std::runtime_error(m_file + ": invalid magic");
      }

      if (hdr->mmd_version != MMD_VERSION)
      {
         throw std::runtime_error(m_file + ": unsupported version");
      }

      if (hdr->index_offset == 0 || hdr->index_length == 0)
      {
         throw std::runtime_error(m_file + ": corrupted file");
      }

      std::string indexstr(data<char>(hdr->index_offset, hdr->index_length), hdr->index_length);
      std::istringstream iss(indexstr);

      std::string dset_name;
      boost::uint32_t fmt_version;

      boost::archive::text_iarchive ia(iss);
      ia >> dset_name >> fmt_version >> m_section_map;

      if (dset_name != dataset_name)
      {
         throw std::runtime_error(m_file + ": unexpected format name: " + dset_name + " (expected " + dataset_name + ")");
      }

      if (fmt_version != format_version)
      {
         std::ostringstream oss;
         oss << m_file << ": unsupported format version: " << fmt_version << " (expected " << format_version << ")";
         throw std::runtime_error(oss.str());
      }
   }

   std::string description() const
   {
      return m_format + " (" + m_file + ")";
   }

   /**
    * Find a section in the dataset's index
    *
    * Will throw an exception if the section cannot be found or is corrupt.
    *
    * \param section        section name
    * \param type           section type
    *
    * \returns reference to a section_info object describing the section
    */
   const section_info& find(const std::string& section, const std::string& type) const
   {
      section_map_type::const_iterator it = m_section_map.find(section);

      if (it == m_section_map.end())
      {
         throw std::runtime_error(m_file + ": no such section " + section);
      }

      if (it->second.offset() == 0)
      {
         throw std::runtime_error(m_file + ": corrupt section " + section);
      }

      if (it->second.type() != type)
      {
         throw std::runtime_error(m_file + ": invalid section type " + it->second.type() + " (expected " + type + ")");
      }

      return it->second;
   }

   /**
    * Access mapped data
    *
    * \tparam T            type to access
    *
    * \param offset        offset relative to start of mapped file
    * \param count         number of items of type T to access
    *
    * \returns a pointer to the requested type at a certain offset relative to
    *          the start of the mapped file.
    */
   template <typename T>
   const T *data(size_t offset = 0, size_t count = 1) const
   {
      BOOST_STATIC_ASSERT_MSG(boost::is_pod<T>::value, "data<>() called on non-POD type");

      if (offset + count*sizeof(T) > m_map.size())
      {
         throw std::runtime_error(m_file + ": potential attempt to access data beyond end of mapping");
      }

      return reinterpret_cast<const T *>(m_map.const_data() + offset);
   }

   /**
    * Return name of mapped file
    */
   const std::string& filename() const
   {
      return m_file;
   }

   /**
    * Warm the cache for a particular data structure
    *
    * This is intended to be called by the individual data structure's
    * cache warming method. Cache warming is useful _only_ if there's a
    * chance for the particular data structure to fit into memory and
    * it's most effective when the data structure in question is randomly
    * accessed very frequently (i.e. several thousand times per second).
    */
   static void warm_cache(const void *beg, const void *end)
   {
      const char *b = reinterpret_cast<const char *>(beg);
      const char *e = reinterpret_cast<const char *>(end);
      char buf[MAP_PAGE_SIZE];
      size_t page_off = static_cast<size_t>(b - static_cast<const char *>(0))%sizeof(buf);

      if (page_off)
      {
         size_t len = std::min(sizeof(buf) - page_off, static_cast<size_t>(e - b));
         std::memcpy(buf, beg, len);
         b += len;
      }

      while (b < e)
      {
         size_t len = std::min(sizeof(buf), static_cast<size_t>(e - b));
         std::memcpy(buf, b, len);
         b += len;
      }
   }

private:
   const std::string m_file;
   const std::string m_format;
   boost::iostreams::mapped_file m_map;
   section_map_type m_section_map;
};

}}

#endif
