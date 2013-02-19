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

#ifndef MOOST_IO_DETAIL_HELPER_WIN32_HPP__
#define MOOST_IO_DETAIL_HELPER_WIN32_HPP__

#include <boost/asio.hpp>
#include <windows.h>
#include <strsafe.h>

namespace moost { namespace io { namespace detail {

class helper
{
public:
   typedef HANDLE native_io_t;
   typedef boost::asio::windows::basic_stream_handle<> async_stream_t;
   typedef DWORD error_t;

   static native_io_t duplicate(native_io_t in)
   {
      native_io_t duped;

      if (!DuplicateHandle(GetCurrentProcess(), in, GetCurrentProcess(), &duped, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
         throw std::runtime_error("failed to duplicate handle");
      }

      return duped;
   }

   static bool close(native_io_t in)
   {
      return CloseHandle(in) == TRUE;
   }

   static void create_pipe(native_io_t& read_end, native_io_t& write_end)
   {
      /*
       *  For some strange reason, anonymous pipes, even though they're using named pipes under
       *  the hood according to the documentation, do not support asynchronous i/o.
       *  So I'm resorting to creating a named pipe with a unique name here.
       */

      unsigned int instance = 0;
      const unsigned int max_instances = 100;
      std::basic_string<TCHAR> pipename;
      native_io_t pipe_read, pipe_write;

      while (instance < max_instances)
      {
         std::basic_stringstream<TCHAR> ss;
         ss << TEXT("\\\\.\\pipe\\") << GetCurrentProcessId() << TEXT("\\iohelper") << instance;
         pipename = ss.str();

         pipe_write = CreateNamedPipe(pipename.c_str(), PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
                                        PIPE_TYPE_BYTE | PIPE_WAIT, 1, 64, 64, 0, NULL);

         if (pipe_write != INVALID_HANDLE_VALUE)
         {
            break;
         }

         ++instance;
      }

      if (pipe_write == INVALID_HANDLE_VALUE)
      {
         throw std::runtime_error("failed to create named pipe");
      }

      try
      {
         pipe_read = CreateFile(pipename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);

         if (pipe_read == INVALID_HANDLE_VALUE)
         {
            throw std::runtime_error("failed to open named pipe");
         }
      }
      catch (...)
      {
         CloseHandle(pipe_write);

         throw;
      }

      read_end = pipe_read;
      write_end = pipe_write;
   }

   static bool write(native_io_t io, const void *data, size_t length, size_t *written)
   {
      DWORD wr;

      bool rv = ::WriteFile(io, data, length, &wr, NULL) == TRUE;

      if (written)
      {
         *written = rv ? static_cast<size_t>(wr) : 0;
      }

      return rv;
   }

   static error_t error()
   {
      return GetLastError();
   }
};

} } }

#endif
