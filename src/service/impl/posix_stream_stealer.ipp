#include <unistd.h>

detail::posix_stream_stealer::posix_stream_stealer(bool restore, bool close_pipe)
  : m_handle(0)
  , m_pipe_fd(-1)
  , m_backup_fd(-1)
  , m_restore(restore)
  , m_close_pipe(close_pipe)
{
}

detail::posix_stream_stealer::~posix_stream_stealer()
{
   if (m_restore)
   {
      restore(m_close_pipe);
   }
}

bool detail::posix_stream_stealer::steal(FILE *handle)
{
   int fd = fileno(handle);

   if (fd < 0)
   {
      return false;
   }

   int backup = dup(fd);

   if (backup < 0)
   {
      return false;
   }

   int pfd[2];

   if (pipe(pfd) != 0)
   {
      goto fail1;
   }

   if (dup2(pfd[1], fd) < 0)
   {
      goto fail2;
   }

   m_handle = handle;
   m_backup_fd = backup;
   m_pipe_fd = pfd[0];

   return true;

fail2:
   (void) close(pfd[0]);
   (void) close(pfd[1]);

fail1:
   (void) close(backup);
   return false;
}

bool detail::posix_stream_stealer::restore(bool close_pipe)
{
   if (m_handle == 0)
   {
      return true;
   }

   int fd = fileno(m_handle);

   m_handle = 0;

   if (fd < 0)
   {
      return false;
   }

   if (dup2(m_backup_fd, fd) < 0)
   {
      return false;
   }

   if (close(m_backup_fd) < 0)
   {
      return false;
   }

   if (close_pipe)
   {
      if (close(m_pipe_fd) < 0)
      {
         return false;
      }
   }

   return true;
}
