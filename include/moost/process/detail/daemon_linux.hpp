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

#ifndef MOOST_PROCESS_DETAIL_DAEMON_LINUX_HPP__
#define MOOST_PROCESS_DETAIL_DAEMON_LINUX_HPP__

#include <cstdio>
#include <cassert>
#include <stdexcept>
#include <sys/stat.h>

#include <boost/function.hpp>

namespace moost { namespace process {

class daemon_impl
{
private:
   friend class daemon;

   daemon_impl() : pid_(-1) {}

   /*!
    * \brief A generic daemoniser for Linux.
    * See http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
    */

   void fork_(boost::function0<bool> child_init_func)
   {
       // Our process and Session IDs
        pid_t sid;

        // Fork off the parent process
        pid_ = fork();
        if (pid_ < 0) {
           throw std::runtime_error("Unable to fork process");
        }

        // The child process must make itself daemon friendly
        if (0 == pid_)
        {
           // Perform child initialisation, if false finalisation of daemon is required.
           if(!child_init_func())
           {
              // Create a new SID for the child process
              sid = setsid();
              if (sid < 0) {
                   throw std::runtime_error("Unable to setsid for child process");
              }

              // Change the current working directory
              if ((chdir("/")) < 0) {
                 throw std::runtime_error("Unable to set current working directory to '/' for child process");
              }

              // Redirect standard file descriptors
              if (!freopen("/dev/null", "w", stdout))
              {
                 // this should never fail, but it's not a critical failure
                 assert(false);
              }

              if (!freopen("/dev/null", "w", stderr))
              {
                 // this should never fail, but it's not a critical failure
                 assert(false);
              }

              if (!freopen("/dev/null", "r", stdin))
              {
                 // this should never fail, but it's not a critical failure
                 assert(false);
              }
           }
        }
  }

   pid_t get_pid_() const { return pid_; }

private:
   pid_t pid_;
};

}}

#endif // MOOST_PROCESS_DAEMON_LINUX_HPP__
