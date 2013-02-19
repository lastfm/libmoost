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

#ifndef MOOST_IO_ASYNC_STREAM_FORWARDER_HPP__
#define MOOST_IO_ASYNC_STREAM_FORWARDER_HPP__

/**
 * \file async_stream_forwarder.hpp
 *
 * The moost::io::async_stream_forwarder class allows to connect file handles
 * that cannot be read in non-blocking mode to be used with boost::asio.
 * It does so by creating a forwarding thread that pushes the data read from
 * the file handle into a pipe that can be read in non-blocking mode.
 */

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "helper.hpp"

#if defined(BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE) && defined(_WIN32)
# include "detail/async_stream_forwarder_win32.hpp"
#elif defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR) && (defined(_POSIX_SOURCE) || defined(__CYGWIN__))
# include "detail/async_stream_forwarder_posix.hpp"
#else
# error "apparently no stream forwarder support has been added for this platform"
#endif

namespace moost { namespace io {

/*
 *  Unfortunately, asynchronous reads for standard file streams (e.g. stdin) are a bad idea,
 *  so in order to make them work with boost::asio, we need a separate thread that forwards the
 *  stream data via an object that supports asynchronous i/o.
 */

class async_stream_forwarder
{
public:
   typedef helper::native_io_t native_io_t;
   typedef helper::async_stream_t async_stream_t;

   async_stream_forwarder(boost::shared_ptr<boost::asio::io_service> ios)
     : m_ios(ios)
     , m_input(*ios)
   {
   }

   async_stream_forwarder(boost::shared_ptr<boost::asio::io_service> ios, native_io_t input, bool dup_input = true)
     : m_ios(ios)
     , m_input(*ios)
   {
      assign(input, dup_input);
   }

   ~async_stream_forwarder()
   {
      // just to be on the safe side in case someone forgets to call close() explicitly
      try
      {
         close();
      }
      catch (...)
      {
         // close() shouldn't throw at all, but let's make sure we don't throw during destruction
      }
   }

   void assign(native_io_t input, bool dup_input = true)
   {
      if (m_input_thread)
      {
         throw std::runtime_error("attempt to assign handle twice");
      }

      m_in = dup_input ? helper::duplicate(input) : input;

      try
      {
         native_io_t pipe_read;

         helper::create_pipe(pipe_read, m_pipe_write);

         try
         {
            m_input.assign(pipe_read);

            m_input_thread.reset(new boost::thread(boost::bind(&async_stream_forwarder::input_thread, this)));
         }
         catch (...)
         {
            if (m_input.is_open())
            {
               m_input.close();
            }
            else
            {
               helper::close(pipe_read);
            }

            throw;
         }
      }
      catch (...)
      {
         if (dup_input)
         {
            helper::close(m_in);
         }

         throw;
      }
   }

   template <typename HandlerT>
   void read_async(void *data, size_t size, HandlerT handler)
   {
      m_input.async_read_some(boost::asio::buffer(data, size), handler);
   }

   void close()
   {
      if (m_input_thread)
      {
         m_input.close();
         helper::close(m_in);
         helper::close(m_pipe_write);
         m_loop.stop();
         m_input_thread->join();
         m_input_thread.reset();
      }
   }

private:
   void input_thread() const
   {
      // Continue forwarding from m_in to m_pipe_write as long as both reading
      // and writing don't fail. If either call fails, it's most probably due
      // to the associated file descriptor having been closed by the call to
      // the close() method.
      m_loop.run(m_in, m_pipe_write);
   }

   boost::shared_ptr<boost::asio::io_service> m_ios;
   boost::shared_ptr<boost::thread> m_input_thread;

   detail::forwarding_loop m_loop;

   native_io_t m_in;
   native_io_t m_pipe_write;
   async_stream_t m_input;
};

} }

#endif
