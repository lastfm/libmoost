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

#ifndef MOOST_IO_DETAIL_ASYNC_STREAM_FORWARDER_WIN32_HPP__
#define MOOST_IO_DETAIL_ASYNC_STREAM_FORWARDER_WIN32_HPP__

#include <windows.h>

namespace moost { namespace io { namespace detail {

class forwarding_loop
{
public:
   void stop() const
   {
      // nothing to do
   }

   void run(HANDLE from, HANDLE to) const
   {
      char input;
      LPVOID inbuf = reinterpret_cast<LPVOID>(&input);
      DWORD dummy;   // we don't care; we know it's one byte if it succeeded

      while (ReadFile(from, inbuf, 1, &dummy, NULL) &&
             WriteFile(to, inbuf, 1, &dummy, NULL))
      {
      }
   }
};

} } }

#endif
