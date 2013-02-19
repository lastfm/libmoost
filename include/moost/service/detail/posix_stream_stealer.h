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

#ifndef FM_LAST_MOOST_SERVICE_DETAIL_POSIX_STREAM_STEALER
#define FM_LAST_MOOST_SERVICE_DETAIL_POSIX_STREAM_STEALER

namespace moost { namespace service { namespace detail {

class posix_stream_stealer
{
public:
   posix_stream_stealer(bool restore, bool close_pipe);
   ~posix_stream_stealer();
   bool steal(FILE *);
   bool restore(bool);

   int get_pipe_fd() const
   {
      return m_pipe_fd;
   }

   int get_backup_fd() const
   {
      return m_backup_fd;
   }

private:
   FILE *m_handle;
   int m_pipe_fd;
   int m_backup_fd;
   bool m_restore;
   bool m_close_pipe;
};

} } }

#endif
