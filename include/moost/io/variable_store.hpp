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

#ifndef MOOST_IO_VARIABLE_STORE_HPP__
#define MOOST_IO_VARIABLE_STORE_HPP__

#include <cmath>
#include <string>
#include <fstream>
#include <stdexcept>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "block_store.hpp"

namespace moost { namespace io {

/** @brief variable_store provides variable length storage by presenting a thread-safe scoped_stream given a requested size
 */
class variable_store
{
private:

  size_t m_min_block_size;
  size_t m_max_block_size;
  boost::scoped_array< boost::scoped_ptr< block_store > > m_block_stores;

  size_t index_from_size(size_t block_size)
  {
    if (block_size < m_min_block_size)
      block_size = m_min_block_size;
    else if ( block_size > m_max_block_size )
      throw std::invalid_argument("cannot exceed max block size");
    return static_cast<size_t>( log( static_cast<double>(( block_size * 2 - 1) / m_min_block_size) ) / log(2.0) );
  }

public:

  /** scoped_block secures a block from the variable_store, and exposes functionality of its predecessors */
  class scoped_block
  {
  private:
    block_store::scoped_block m_scoped_block;
  public:
    scoped_block(variable_store & variable_store_,
                 size_t block_size)
    : m_scoped_block( * variable_store_.m_block_stores[variable_store_.index_from_size(block_size)])
    {
    }
    scoped_block(variable_store & variable_store_,
                 size_t block_size,
                 size_t index)
    : m_scoped_block( * variable_store_.m_block_stores[variable_store_.index_from_size(block_size)], index)
    {
    }
    void free() { m_scoped_block.free(); }
    std::fstream & operator * () { return m_scoped_block.operator* (); }
    std::fstream * operator ->() { return m_scoped_block.operator->(); }
    size_t index()               { return m_scoped_block.index(); }
    size_t block_size()          { return m_scoped_block.block_size(); }
  };

  /** @brief Constructs a block_store.
   */
  variable_store(const std::string & base_path,
                 size_t min_block_size = 64,
                 size_t max_block_size = 16777216,
                 size_t streams_per_block_size = 8)
  : m_min_block_size(min_block_size),
    m_max_block_size(max_block_size),
    m_block_stores( new boost::scoped_ptr< block_store >[1 + index_from_size(m_max_block_size) ] )
  {
    for (int i = 0; min_block_size <= max_block_size; min_block_size *= 2, ++i)
    {
      boost::filesystem::path p = boost::filesystem::path(base_path) / boost::filesystem::path(boost::lexical_cast<std::string>(min_block_size));
      m_block_stores[i].reset(new block_store(p.string(), min_block_size, streams_per_block_size));
    }
  }
};

}} // moost::io

#endif // MOOST_IO_VARIABLE_STORE_HPP__
