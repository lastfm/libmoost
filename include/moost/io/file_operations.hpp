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

#ifndef MOOST_IO_FILE_OPERATIONS_HPP__
#define MOOST_IO_FILE_OPERATIONS_HPP__

#include <errno.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>

#if defined(__GNUC__)
#include <unistd.h>
#elif defined(_MSC_VER)
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <share.h>
#endif

/**
 * \namespace moost::io
 * \brief IO-related routines used commonly everywhere
 */
namespace moost { namespace io {

/**
 * \brief Collection of file operations not easily doable using stdlib functions.
 */
class file_operations
{
private:

  /**
   * \brief Get an error string from an error code.
   */
  static std::string get_error_string(int err)
  {
    char buf[512];
#if defined(__GNUC__)
    strerror_r(err, buf, sizeof(buf));
#elif defined(_MSC_VER)
    strerror_s(buf, sizeof(buf), err);
#endif
    return std::string(buf);
  }

public:

  /**
   * \brief Change the file of a size.  Smaller sizes will truncate the file, larger sizes will pad
   *        the file with zeroes.
   */
  static void change_size(const std::string & path, boost::int64_t length)
  {
#if defined(__GNUC__)
    if (truncate(path.c_str(), length) != 0)
      throw std::runtime_error("could not change size: " + get_error_string(errno));
#elif defined(_MSC_VER)
    int fh;

    if( _sopen_s( &fh, path.c_str(), _O_RDWR, _SH_DENYNO, _S_IREAD | _S_IWRITE ) == 0 )
    {
      int result = _chsize_s( fh, length );
      if (result != 0)
      {
        _close( fh );
        throw std::runtime_error( std::string("could not change size: ") + get_error_string(errno) );
      }
      _close( fh );
    }
    else
      throw std::runtime_error("could not open file: " + get_error_string(errno));
#endif
  }
};

}} // moost::io

#endif // MOOST_IO_FILE_OPERATIONS_HPP__
