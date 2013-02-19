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

#ifndef MOOST_IO_DETAIL_ASYNC_STREAM_FORWARDER_POSIX_HPP__
#define MOOST_IO_DETAIL_ASYNC_STREAM_FORWARDER_POSIX_HPP__

#include <unistd.h>
#include <poll.h>

namespace moost { namespace io { namespace detail {

class forwarding_loop
{
public:
   forwarding_loop()
     : m_signal_in(-1)
   {
   }

   void stop() const
   {
      if (m_signal_in != -1)
      {
         char input = 'S';
         if (::write(m_signal_in, &input, 1) != 1)
         {
            // there's just nothing we can do...
         }
      }
   }

   void run(int from, int to) const
   {
      int signal_out;
      moost::io::helper::create_pipe(signal_out, m_signal_in);

      char input;
      struct pollfd pfd[2];
      pfd[0].fd = from;
      pfd[0].events = POLLIN;
      pfd[1].fd = signal_out;
      pfd[1].events = POLLIN;

      for (;;)
      {
         pfd[0].revents = 0;
         pfd[1].revents = 0;

         int rv = ::poll(&pfd[0], 2, -1);

         if (rv <= 0)
         {
            break;
         }

         if (pfd[1].revents & POLLIN)
         {
            break;
         }
         else if (pfd[0].revents & POLLIN)
         {
            if (::read(from, &input, 1) != 1 ||
                ::write(to, &input, 1) != 1)
            {
               break;
            }
         }
      }
   }

private:
   mutable int m_signal_in;
};

} } }

#endif
