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

#ifndef MOOST_IO_DETAIL_HELPER_POSIX_HPP__
#define MOOST_IO_DETAIL_HELPER_POSIX_HPP__

#include <boost/asio.hpp>
#include <unistd.h>

namespace moost { namespace io { namespace detail {

class helper
{
public:
   typedef int native_io_t;
   typedef boost::asio::posix::basic_stream_descriptor<> async_stream_t;
   typedef int error_t;

   static native_io_t duplicate(native_io_t in)
   {
      native_io_t duped = ::dup(in);

      if (duped == -1)
      {
         throw std::runtime_error("failed to duplicate handle");
      }

      return duped;
   }

   static bool close(native_io_t in)
   {
      return ::close(in) != -1;
   }

   static void create_pipe(native_io_t& read_end, native_io_t& write_end)
   {
      int pfd[2];

      if (pipe(pfd) != 0)
      {
         throw std::runtime_error("failed to create pipe");
      }

      read_end = pfd[0];
      write_end = pfd[1];
   }

   static bool write(native_io_t io, const void *data, size_t length, size_t *written)
   {
      ssize_t rv = ::write(io, data, length);

      if (written)
      {
         *written = rv >= 0 ? static_cast<size_t>(rv) : 0;
      }

      return static_cast<size_t>(rv) == length;
   }

   static error_t error()
   {
      return errno;
   }
};

} } }

#endif
