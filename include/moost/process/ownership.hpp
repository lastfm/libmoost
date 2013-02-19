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

#ifndef FM_LAST_MOOST_PROCESS_OWNERSHIP_H_
#define FM_LAST_MOOST_PROCESS_OWNERSHIP_H_

#include "detail/ownership_posix.hpp"

namespace moost { namespace process {

/**
 * \brief Process ownership information and manipulation
 *
 * This class allows a process to retrieve information about or manipulate its ownership.
 * For example, it allows a daemon process to drop privileges when being run by the superuser.
 *
 * Platform specific abstractions are provided for Win32 and POSIX systems, although the
 * Win32 abstraction is just a dummy at the moment.
 */

class ownership
{
public:
   typedef detail::ownership::uid_type uid_type;
   typedef detail::ownership::gid_type gid_type;

   /**
    * \brief Check for superuser privileges
    *
    * \returns Boolean indicating whether the process is currently running with
    *          superuser privileges.
    */
   bool is_superuser() const
   {
      return m_impl.is_superuser();
   }

   /**
    * \brief Lookup user name by user id
    *
    * \param name Reference to a string to store the user name in.
    *
    * \param uid  User id in the platform's native format.
    *
    * \returns Boolean indicating whether the user name was found.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   bool lookup_user(std::string& name, const uid_type& uid) const
   {
      return m_impl.lookup_user(name, uid);
   }

   /**
    * \brief Lookup user id by user name
    *
    * \param uid  Reference to native type to store the user id in.
    *
    * \param name User name.
    *
    * \returns Boolean indicating whether the user id was found.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   bool lookup_uid(uid_type& uid, const std::string& name) const
   {
      return m_impl.lookup_uid(uid, name);
   }

   /**
    * \brief Lookup user id and group id by user name
    *
    * \param uid  Reference to native type to store the user id in.
    *
    * \param uid  Reference to native type to store the group id in.
    *
    * \param name User name.
    *
    * \returns Boolean indicating whether the user id was found.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   bool lookup_uid(uid_type& uid, gid_type& gid, const std::string& name) const
   {
      return m_impl.lookup_uid(uid, gid, name);
   }

   /**
    * \brief Set new user id
    *
    * \param uid User id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   void set_uid(uid_type uid)
   {
      m_impl.set_uid(uid);
   }

   /**
    * \brief Set new effective user id
    *
    * \param uid User id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   void set_effective_uid(uid_type uid)
   {
      m_impl.set_effective_uid(uid);
   }

   /**
    * \brief Get current user id
    *
    * \returns User id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   uid_type get_uid() const
   {
      return m_impl.get_uid();
   }

   /**
    * \brief Get current effective user id
    *
    * \returns User id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   uid_type get_effective_uid() const
   {
      return m_impl.get_effective_uid();
   }

   /**
    * \brief Lookup group name by group id
    *
    * \param name Reference to a string to store the group name in.
    *
    * \param gid  Group id in the platform's native format.
    *
    * \returns Boolean indicating whether the group name was found.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   bool lookup_group(std::string& name, const gid_type& gid) const
   {
      return m_impl.lookup_group(name, gid);
   }

   /**
    * \brief Lookup group id by group name
    *
    * \param gid  Reference to native type to store the group id in.
    *
    * \param name Group name.
    *
    * \returns Boolean indicating whether the group id was found.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   bool lookup_gid(gid_type& gid, const std::string& name) const
   {
      return m_impl.lookup_gid(gid, name);
   }

   /**
    * \brief Set new group id
    *
    * \param uid Group id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   void set_gid(gid_type gid)
   {
      m_impl.set_gid(gid);
   }

   /**
    * \brief Set new effective group id
    *
    * \param uid Group id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   void set_effective_gid(gid_type gid)
   {
      m_impl.set_effective_gid(gid);
   }

   /**
    * \brief Get current group id
    *
    * \returns Group id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   gid_type get_gid() const
   {
      return m_impl.get_gid();
   }

   /**
    * \brief Get current effective group id
    *
    * \returns Group id in the platform's native format.
    *
    * \exception Throws an instance of boost::system::system_error on failure.
    */
   gid_type get_effective_gid() const
   {
      return m_impl.get_effective_gid();
   }

   /**
    * \brief Set new user id by user name
    *
    * \param name String holding the user name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the user could not be found.
    */
   bool set_user(const std::string& name)
   {
      uid_type uid;

      if (!lookup_uid(uid, name))
      {
         return false;
      }

      set_uid(uid);

      return true;
   }

   /**
    * \brief Set new effective user id by user name
    *
    * \param name String holding the user name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the user could not be found.
    */
   bool set_effective_user(const std::string& name)
   {
      uid_type uid;

      if (!lookup_uid(uid, name))
      {
         return false;
      }

      set_effective_uid(uid);

      return true;
   }

   /**
    * \brief Set new group id by group name
    *
    * \param name String holding the group name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the group could not be found.
    */
   bool set_group(const std::string& name)
   {
      gid_type gid;

      if (!lookup_gid(gid, name))
      {
         return false;
      }

      set_gid(gid);

      return true;
   }

   /**
    * \brief Set new effective group id by group name
    *
    * \param name String holding the group name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the group could not be found.
    */
   bool set_effective_group(const std::string& name)
   {
      gid_type gid;

      if (!lookup_gid(gid, name))
      {
         return false;
      }

      set_effective_gid(gid);

      return true;
   }

   /**
    * \brief Get current user name
    *
    * \returns String holding the user name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the user could not be found.
    */
   std::string get_user() const
   {
      std::string name;

      if (!lookup_user(name, get_uid()))
      {
         throw std::runtime_error("failed to lookup uid");
      }

      return name;
   }

   /**
    * \brief Get current effective user name
    *
    * \returns String holding the user name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the user could not be found.
    */
   std::string get_effective_user() const
   {
      std::string name;

      if (!lookup_user(name, get_effective_uid()))
      {
         throw std::runtime_error("failed to lookup uid");
      }

      return name;
   }

   /**
    * \brief Get current group name
    *
    * \returns String holding the group name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the group could not be found.
    */
   std::string get_group() const
   {
      std::string name;

      if (!lookup_group(name, get_gid()))
      {
         throw std::runtime_error("failed to lookup gid");
      }

      return name;
   }

   /**
    * \brief Get current effective group name
    *
    * \returns String holding the group name.
    *
    * \exception Throws an instance of boost::system::system_error on failure
    *            or a std::runtime_error when the group could not be found.
    */
   std::string get_effective_group() const
   {
      std::string name;

      if (!lookup_group(name, get_effective_gid()))
      {
         throw std::runtime_error("failed to lookup gid");
      }

      return name;
   }

   /**
    * \brief Drop process ownership privileges
    *
    * \param user  User name to continue to run this process as.
    *
    * \param group Optional group name to run this process as. Defaults
    *              to the primary group of the user.
    *
    * \exception Throws an instance of boost::system::system_error on failure or a
    *            std::runtime_error when either user or group could not be found.
    */
   void drop_privileges(const std::string& user, const std::string& group = "")
   {
      uid_type uid = 0;
      gid_type gid = 0;

      if (group.empty())
      {
         if (!lookup_uid(uid, gid, user))
         {
            throw std::runtime_error("failed to lookup uid");
         }
      }
      else
      {
         if (!lookup_uid(uid, user))
         {
            throw std::runtime_error("failed to lookup uid");
         }

         if (!lookup_gid(gid, group))
         {
            throw std::runtime_error("failed to lookup gid");
         }
      }

      set_gid(gid);
      set_uid(uid);
   }

private:
   detail::ownership m_impl;
};

} }

#endif
