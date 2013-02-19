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

#ifndef MOOST_IO_BLOCK_STORE_HPP__
#define MOOST_IO_BLOCK_STORE_HPP__

#include <algorithm>
#include <string>
#include <fstream>
#include <stdexcept>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/scoped_array.hpp>
#include <boost/thread/mutex.hpp>

#include "../container/resource_stack.hpp"

namespace moost { namespace io {

/** @brief block_store provides block-level storage by presenting a thread-safe pool of streams, for allocing, reading/writing, freeing
*
*/
class block_store
{
private:
   moost::container::resource_stack< std::fstream > m_rstreams;
   size_t                                           m_block_size;
   size_t                                           m_allocated;
   std::vector<size_t>                              m_free_list;
   boost::mutex                                     m_mutex;

   /** free a block at a given index, and don't whine if the index is invalid
   * (don't want to throw on dtors) */
   void free(size_t index)
   {
      boost::mutex::scoped_lock lock(m_mutex);
      if (index >= m_allocated)
         return; // weird
      if (index == m_allocated - 1)
         --m_allocated;
      else
      {
         std::vector<size_t>::iterator it = std::lower_bound(m_free_list.begin(), m_free_list.end(), index);
         if (it == m_free_list.end() || *it != index)
            m_free_list.insert(it, index);
      }
   }

   /** alloc an index and return it */
   size_t alloc()
   {
      size_t ret_val;
      boost::mutex::scoped_lock lock(m_mutex);
      if (m_free_list.empty())
         ret_val = m_allocated++;
      else
      {
         ret_val = m_free_list.back();
         m_free_list.pop_back();
      }
      return ret_val;
   }

   /** given an index, find the associated index within the stream */
   std::streampos getpos(size_t index)
   {
      return static_cast<std::streampos>(sizeof(size_t)) + static_cast<std::streampos>( m_block_size ) * static_cast<std::streampos>( index );
   }

public:

   /** scoped_block secures a block from the block store, and exposes read/write functionality */
   class scoped_block
   {
   private:
      block_store &                                                     m_block_store;
      moost::container::resource_stack< std::fstream >::scoped_resource m_rstream;
      size_t                                                            m_index;
      bool                                                              m_free;
   public:
      /** alloc a new block */
      scoped_block(block_store & block_store_)
         : m_block_store(block_store_),
           m_rstream(block_store_.m_rstreams),
           m_free(false)
      {
         m_index = m_block_store.alloc();
         std::streampos newPos = m_block_store.getpos(m_index);
         m_rstream->seekg( newPos );
         m_rstream->seekp( newPos );
      }
      /** grab a preexisting block */
      scoped_block(block_store & block_store_, size_t index)
         : m_block_store(block_store_),
           m_rstream(block_store_.m_rstreams),
           m_index(index),
           m_free(false)
      {
         std::streampos newPos = m_block_store.getpos(m_index);
         m_rstream->seekg( newPos );
         m_rstream->seekp( newPos );
      }
      ~scoped_block()
      {
         if (m_free)
            m_block_store.free(m_index);
         else
            m_rstream->rdbuf()->pubsync();
      }
      void free()                  { m_free = true; }
      std::fstream & operator * () { return m_rstream.operator* (); }
      std::fstream * operator ->() { return m_rstream.operator->(); }
      size_t index()               { return m_index; }
      size_t block_size()          { return m_block_store.block_size(); }
   };

   /** construct a new block store */
   block_store(const std::string & path,
      size_t block_size,
      size_t num_streams = 8  /* the number of concurrent streams accessing this blocks store */ )
      : m_block_size(block_size),
        m_allocated(0)
   {
      if (!boost::filesystem::exists(boost::filesystem::path(path)))
         std::fstream(path.c_str(), std::ios::binary | std::ios::out | std::ios::app); // poor man's touch
      else
      {
         std::fstream in(path.c_str(), std::ios::binary | std::ios::out | std::ios::in);
         in.read(reinterpret_cast<char *>(&m_allocated), sizeof(size_t));
         in.seekg(getpos(m_allocated));
         size_t free_list_size = 0;
         in.read(reinterpret_cast<char *>(&free_list_size), sizeof(size_t));
         m_free_list.resize(free_list_size);
         if (free_list_size > 0)
            in.read(reinterpret_cast<char *>(&m_free_list[0]), free_list_size * sizeof(size_t));
      }
      while (num_streams-- != 0)
      {
         boost::shared_ptr<std::fstream> pTmpStream( new std::fstream(
                  path.c_str(),
                  std::ios::binary | std::ios::out | std::ios::in));

         if ( ! *pTmpStream )
         {
            throw std::runtime_error("Cannot open output stream for <" + path + ">!");
         }

         m_rstreams.add_resource(pTmpStream);
      }
   }

   /** destroys the block_store
   ** n.b. everyone must have their hands off the block_store before you destroy it */
   ~block_store()
   {
      moost::container::resource_stack< std::fstream >::scoped_resource rstream(m_rstreams);
      rstream->seekp(0);
      rstream->write(reinterpret_cast<const char *>(&m_allocated), sizeof(size_t));
      rstream->seekp(getpos(m_allocated));
      size_t free_list_size = m_free_list.size();
      rstream->write(reinterpret_cast<const char *>(&free_list_size), sizeof(size_t));
      if (free_list_size > 0)
         rstream->write(reinterpret_cast<const char *>(&m_free_list[0]), free_list_size * sizeof(size_t));
      rstream->rdbuf()->pubsync();
   }

   /** returns how many blocks have been allocated, including empty blocks
   * that are in the free list */
   size_t allocated()
   {
      return m_allocated;
   }

   /** returns the block size of this block store */
   size_t block_size()
   {
      return m_block_size;
   }

};

}} // moost::io

#endif // MOOST_IO_BLOCK_STORE_HPP__
