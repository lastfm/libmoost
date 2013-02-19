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

#ifndef MOOST_IO_MAP_STORE_HPP__
#define MOOST_IO_MAP_STORE_HPP__

#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>

#include "../container/sparse_hash_map.hpp"

#include "variable_store.hpp"

namespace moost { namespace io {

/** @brief map_store provides storage by presenting a thread-safe scoped_stream given a key and requested size
 */
template<class Key, class HashFcn = boost::hash< Key > >
class map_store
{
private:

  struct location
  {
    location(size_t block_size_, size_t index_) : block_size(block_size_), index(index_) {}
    location() : block_size(0), index(0) {}
    size_t block_size;
    size_t index;
  };

  typedef moost::container::sparse_hash_map<Key, location, HashFcn > hm_key_location;

  std::string     m_key_location_path;
  variable_store  m_variable_store;
  hm_key_location m_key_location;
  boost::mutex    m_mutex;

  /** gets a scoped_block given a key, or leaves empty
   ** if the key is not found */
  void get(boost::scoped_ptr< variable_store::scoped_block > & p,
           const Key & key)
  {
    boost::mutex::scoped_lock lock(m_mutex);
    // if key is found, loads index into the scoped block
    typename hm_key_location::const_iterator it = m_key_location.find(key);

    if (it == m_key_location.end())
      return;

    p.reset( new variable_store::scoped_block(m_variable_store, it->second.block_size, it->second.index) );
  }

  /** allocs a scoped_block for a new key
   ** doesn't support reallocing on an existing key, maybe in the future */
  void alloc(boost::scoped_ptr< variable_store::scoped_block > & p,
             const Key & key,
             size_t block_size)
  {
    boost::mutex::scoped_lock lock(m_mutex);
    // don't support reallocing for now
    typename hm_key_location::const_iterator it = m_key_location.find(key);

    if (it != m_key_location.end())
      throw std::invalid_argument("realloc not supported");

    p.reset( new variable_store::scoped_block(m_variable_store, block_size) );
    m_key_location[key] = location(p->block_size(), p->index());
  }

  /** frees a key */
  void free(const Key & key)
  {
    boost::mutex::scoped_lock lock(m_mutex);
    m_key_location.erase(key);
  }

public:

  // TODO: maybe an auto_scoped_block that uses a stringstream instead, resizes automatically depending on what you do to it

  /** scoped_block secures a block from the map_store, and exposes functionality of its predecessors */
  class scoped_block
  {
  private:
    map_store<Key, HashFcn> & m_map_store;
    const Key m_key;
    boost::scoped_ptr< variable_store::scoped_block > m_pscoped_block;
    bool m_free;
  public:
    scoped_block(map_store<Key, HashFcn> & map_store_,
                 const Key & key)
    : m_map_store(map_store_),
      m_key(key),
      m_free(false)
    {
      map_store_.get(m_pscoped_block, key);
    }
    scoped_block(map_store<Key, HashFcn> & map_store_,
                 const Key & key,
                 size_t block_size)
    : m_map_store(map_store_),
      m_key(key),
      m_free(false)
    {
      map_store_.alloc(m_pscoped_block, key, block_size);
    }
    ~scoped_block()
    {
      if (m_free)
        m_map_store.free(m_key);
    }
    void free()
    {
      m_pscoped_block->free();
      m_free = true;
    }
    std::fstream & operator * () { return m_pscoped_block->operator * (); }
    std::fstream * operator ->() { return m_pscoped_block->operator ->(); }
    size_t index()               { return m_pscoped_block->index(); }
    size_t block_size()          { return m_pscoped_block->block_size(); }
    operator bool () const       { return m_pscoped_block; }
    bool operator! () const      { return m_pscoped_block.operator !(); }
  };

  /** @brief Constructs a map_store.
   */
  map_store(const std::string & base_path,
            size_t min_block_size = 64,
            size_t max_block_size = 16777216,
            size_t streams_per_block_size = 8)
  : m_key_location_path((boost::filesystem::path(base_path) / boost::filesystem::path("key_locations")).string()),
    m_variable_store(base_path, min_block_size, max_block_size, streams_per_block_size)
  {
    std::ifstream in(m_key_location_path.c_str(), std::ios::binary);

    if (!in.is_open())
      return;

    size_t size = 0;
    boost::archive::binary_iarchive ia(in, boost::archive::no_header);
    ia >> size;
    location loc;
    while (size-- != 0)
    {
      Key key; // avoid weirdness with boost::serialization and strings... need to reinit each time?  scary
      ia >> key;
      ia >> loc.block_size;
      ia >> loc.index;
      m_key_location[key] = loc;
    }
  }

  /** @brief Destroys a map_store.
   */
  ~map_store()
  {
     size_t size = m_key_location.size();
     if ( size <= 0 )
        return;

     try
     {
        std::ofstream out(m_key_location_path.c_str(), std::ios::binary);
        if ( !out.is_open() )
           throw std::runtime_error("Cannot open file <" + m_key_location_path + "> for output!");
        boost::archive::binary_oarchive oa(out, boost::archive::no_header);
        oa << size;
        for (typename hm_key_location::const_iterator it = m_key_location.begin(); it != m_key_location.end(); ++it)
        {
           oa << it->first;
           oa << it->second.block_size;
           oa << it->second.index;
        }
     }
     catch (const std::exception& e)
     {
        std::cerr << "ERROR: " << e.what() << std::endl;
     }
  }

  /** If you're going to delete keys, you must specify a deleted key that will
   * never be used as a real key.
   */
  void set_deleted_key(const Key & key)
  {
    m_key_location.set_deleted_key(key);
  }
};

}} // moost::io

#endif // MOOST_IO_MAP_STORE_HPP__
