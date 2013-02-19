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

#ifndef FM_LAST_MOOST_PROCESS_DETAIL_OWNERSHIP_POSIX_H_
#define FM_LAST_MOOST_PROCESS_DETAIL_OWNERSHIP_POSIX_H_

#include <string>
#include <vector>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <boost/system/system_error.hpp>

namespace moost { namespace process { namespace detail {

class ownership
{
private:
   typedef std::vector<char> buffer_type;

public:
   typedef int uid_type;
   typedef int gid_type;

   bool is_superuser() const
   {
      return get_uid() == 0 && get_gid() == 0;
   }

   bool lookup_user(std::string& name, const uid_type& uid) const
   {
      struct passwd pwd;
      buffer_type buf;

      if (lookup_user(uid, buf, &pwd))
      {
         name = pwd.pw_name;
         return true;
      }

      return false;
   }

   bool lookup_group(std::string& name, const gid_type& gid) const
   {
      struct group grp;
      buffer_type buf;

      if (lookup_group(gid, buf, &grp))
      {
         name = grp.gr_name;
         return true;
      }

      return false;
   }

   bool lookup_uid(uid_type& uid, const std::string& name) const
   {
      struct passwd pwd;
      buffer_type buf;

      if (lookup_user(name, buf, &pwd))
      {
         uid = pwd.pw_uid;
         return true;
      }

      return false;
   }

   bool lookup_uid(uid_type& uid, gid_type& gid, const std::string& name) const
   {
      struct passwd pwd;
      buffer_type buf;

      if (lookup_user(name, buf, &pwd))
      {
         uid = pwd.pw_uid;
         gid = pwd.pw_gid;
         return true;
      }

      return false;
   }

   bool lookup_gid(gid_type& gid, const std::string& name) const
   {
      struct group grp;
      buffer_type buf;

      if (lookup_group(name, buf, &grp))
      {
         gid = grp.gr_gid;
         return true;
      }

      return false;
   }

   bool lookup_gid(gid_type& gid, const uid_type& uid) const
   {
      struct passwd pwd;
      buffer_type buf;

      if (lookup_user(uid, buf, &pwd))
      {
         gid = pwd.pw_gid;
         return true;
      }

      return false;
   }

   void set_uid(uid_type uid)
   {
      if (setuid(uid) == -1)
      {
         boost::system::error_code ec(errno, boost::system::get_system_category());
         boost::system::system_error e(ec, "setuid");
         boost::throw_exception(e);
      }
   }

   void set_effective_uid(uid_type uid)
   {
      if (seteuid(uid) == -1)
      {
         boost::system::error_code ec(errno, boost::system::get_system_category());
         boost::system::system_error e(ec, "seteuid");
         boost::throw_exception(e);
      }
   }

   uid_type get_uid() const
   {
      return getuid();
   }

   uid_type get_effective_uid() const
   {
      return geteuid();
   }

   void set_gid(gid_type gid)
   {
      if (setgid(gid) == -1)
      {
         boost::system::error_code ec(errno, boost::system::get_system_category());
         boost::system::system_error e(ec, "setgid");
         boost::throw_exception(e);
      }
   }

   void set_effective_gid(gid_type gid)
   {
      if (setegid(gid) == -1)
      {
         boost::system::error_code ec(errno, boost::system::get_system_category());
         boost::system::system_error e(ec, "setegid");
         boost::throw_exception(e);
      }
   }

   gid_type get_gid() const
   {
      return getgid();
   }

   gid_type get_effective_gid() const
   {
      return getegid();
   }

private:
   static void buffer_reserve(buffer_type& buf, int which)
   {
      long bufsize = sysconf(which);

      if (bufsize == -1)
      {
         bufsize = 4096;
      }

      buf.reserve(bufsize);
   }

   bool lookup_user(uid_type uid, buffer_type& buf, struct passwd* pwd) const
   {
      struct passwd *result;
      int rv;

      buffer_reserve(buf, _SC_GETPW_R_SIZE_MAX);

      rv = getpwuid_r(uid, pwd, &buf[0], buf.capacity(), &result);

      if (result == NULL)
      {
         if (rv == 0)
         {
            return false;
         }
         else
         {
            boost::system::error_code ec(rv, boost::system::get_system_category());
            boost::system::system_error e(ec, "getpwuid_r");
            boost::throw_exception(e);
         }
      }

      return true;
   }

   bool lookup_user(const std::string& name, buffer_type& buf, struct passwd* pwd) const
   {
      struct passwd *result;
      int rv;

      buffer_reserve(buf, _SC_GETPW_R_SIZE_MAX);

      rv = getpwnam_r(name.c_str(), pwd, &buf[0], buf.capacity(), &result);

      if (result == NULL)
      {
         if (rv == 0)
         {
            return false;
         }
         else
         {
            boost::system::error_code ec(rv, boost::system::get_system_category());
            boost::system::system_error e(ec, "getpwnam_r");
            boost::throw_exception(e);
         }
      }

      return true;
   }

   bool lookup_group(gid_type gid, buffer_type& buf, struct group* grp) const
   {
      struct group *result;
      int rv;

      buffer_reserve(buf, _SC_GETGR_R_SIZE_MAX);

      rv = getgrgid_r(gid, grp, &buf[0], buf.capacity(), &result);

      if (result == NULL)
      {
         if (rv == 0)
         {
            return false;
         }
         else
         {
            boost::system::error_code ec(rv, boost::system::get_system_category());
            boost::system::system_error e(ec, "getgrgid_r");
            boost::throw_exception(e);
         }
      }

      return true;
   }

   bool lookup_group(const std::string& name, buffer_type& buf, struct group* grp) const
   {
      struct group *result;
      int rv;

      buffer_reserve(buf, _SC_GETGR_R_SIZE_MAX);

      rv = getgrnam_r(name.c_str(), grp, &buf[0], buf.capacity(), &result);

      if (result == NULL)
      {
         if (rv == 0)
         {
            return false;
         }
         else
         {
            boost::system::error_code ec(rv, boost::system::get_system_category());
            boost::system::system_error e(ec, "getgrnam_r");
            boost::throw_exception(e);
         }
      }

      return true;
   }
};

} } }

#endif
