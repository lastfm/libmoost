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

#ifndef MOOST_PROCESS_DAEMON_HPP__
#define MOOST_PROCESS_DAEMON_HPP__

#include <boost/function.hpp>
#include <boost/thread.hpp>

#include "detail/daemon_linux.hpp"

namespace moost { namespace process {

/*!
 * \brief Provides everything necessary to convert a process into a daemon.
 *
 * You can use this class to fork your process as a daemon. It encapsulates everything
 * that will be required for the OS being targetted via a nice and simple to use interface.
 *
 */

class daemon
{
private:
   struct default_child_init_func
   {
      bool operator()() { return false; }
   };

public:
   /*!
    * \brief Forks a child process.
    *
    * Forks a child process and takes care of everything necessary to make it a daemon.
    * Allows caller to prevent termination of parent and to perform child initialisation.
    *
    * @param[in]  exit_parent  If 'true' the parent will exit without returning
    * @param[in]  child_init_func  Called after spawning the child but before performing
    *                              daemonisation stuff to turn child into a real daemon.
    *                              Should return a bool, with true indicating all daemonising
    *                              tasks have been completed or are not necessary so the daemon
    *                              class will not do these additional daemonising steps.
    *
    * \exception If there is an error during the forking a standard runtime_error exception will be thrown.
    */
   daemon(bool exit_parent, boost::function0<bool> child_init_func)
   {
      init(exit_parent, child_init_func);
   }

   /*!
    * \brief Forks a child process.
    *
    * Forks a child process and takes care of everything necessary to make it a daemon.
    * Allows caller to prevent termination of parent.
    *
    * @param[in]  exit_parent  If 'true' the parent will exit without returning
    *
    * \exception If there is an error during the forking a standard runtime_error exception will be thrown.
    */
   daemon(bool exit_parent)
   {
      init(exit_parent, default_child_init_func());
   }

   /*!
    * \brief Forks a child process.
    *
    * Forks a child process and takes care of everything necessary to make it a daemon.
    * Allows caller to perform child initialisation.
    *
    * @param[in]  child_init_func  Called after spawning the child but before performing
    *                              daemonisation stuff to turn child into a real daemon.
    *                              Should return a bool, with true indicating all daemonising
    *                              tasks have been completed or are not necessary so the daemon
    *                              class will not do these additional daemonising steps.
    *
    * \exception If there is an error during the forking a standard runtime_error exception will be thrown.
    */
   daemon(boost::function0<bool> child_init_func)
   {
      init(true, child_init_func);
   }

   /*!
    * \brief Get the child process ID
    *
    * \return If we're the parent we get the childs pid, if we're the child we get zero.
    */

   pid_t get_pid() const { return daemon_impl_.get_pid_(); }

   /*!
    * \brief Is this process the child?
    *
    * \return If child return true else return false
    */

   bool is_child() const { return 0 == get_pid(); }

   /*!
    * \brief Is this process the parent
    *
    * \return If child return false else return true
    */

   bool is_parent() const { return !is_child(); }

   /*!
    * \brief Once called, never returns - puts this thread to sleep forever
    *
    * A help function that can be called from the 'main' thread of a process
    * to put it to sleep forever so that the "worker" threads can do their thing.
    * Once this is called the only way to terminate the process is to register
    * a quit handler to stop the worker threads and then explicitly call exit().
    */
   static void sleep_forever()
   {
      for(;;)
      {
         // Never again shall this thread awaken from a deep slumber :)
         boost::this_thread::sleep(
            boost::posix_time::time_duration(boost::posix_time::pos_infin)
            );
      }
   }

private:
   void init(bool exit_parent, boost::function0<bool> child_init_func)
   {
      daemon_impl_.fork_(child_init_func);

      if(exit_parent && is_parent())
      {
         exit(EXIT_SUCCESS);
      }
   }

private:
   daemon_impl daemon_impl_;
};

}}


#endif // MOOST_PROCESS_DAEMON_HPP__
