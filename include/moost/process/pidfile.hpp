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

#ifndef MOOST_PROCESS_PIDFILE_HPP__
#define MOOST_PROCESS_PIDFILE_HPP__

#include <boost/filesystem.hpp>

#include <string>
#include <fstream>

namespace moost { namespace process {

/*!
 * \brief Create a process pid file
 *
 * Will create the pid on construction and remove on destruction.
 *
 */

class pidfile
{
public:
   /*!
    * \brief Construct a pidfile object
    *
    * Construct a pidfile object with the pid set
    *
    * \exception if it fails to create the pid file a runtime_error will be emitted.
    *
    * @param the pid that will be associated with this instance
    * @param process_name The name of the process to associate with this pidfile
    * @param rundir The run directory where the pid file will be created You should use
                    get_default_rundir() unless you wish to specify a non-standard location
    *
    * \warning This class is NOT thread safe
    *
    */
   pidfile(
      pid_t const pid,
      std::string const & process_name,
      boost::filesystem::path const & rundir
      )
      : pid_(pid), filepath_((rundir.empty() ? get_default_rundir() : rundir) / (process_name + ".pid"))
   {
      if(!create())
      {
         throw std::runtime_error("failed to create pid file");
      }
   }

   /*!
    * \brief Construct a pidfile object
    *
    * Construct a pidfile object with the pid unset at this time (-1)
    *
    * @param process_name The name of the process to associate with this pidfile
    * @param rundir The run directory where the pid file will be created You should use
    *               get_default_rundir() unless you wish to specify a non-standard location
    *
    */
   explicit pidfile(
      std::string const & process_name,
      boost::filesystem::path const & rundir
      )
      : pid_(-1), filepath_((rundir.empty() ? get_default_rundir() : rundir) / (process_name + ".pid"))
   {
   }

   /*!
    * \brief Construct a pidfile object
    *
    * Construct a pidfile object with the pid set
    *
    * \exception if it fails to create the pid file a runtime_error will be emitted.
    *
    * @param the pid that will be associated with this instance
    * @param filepath the file name and path of the pidfile
    *
    * \warning This class is NOT thread safe
    *
    */
   pidfile(
      pid_t const pid,
      boost::filesystem::path const & filepath
      )
      : pid_(pid), filepath_(filepath)
   {
      if(!create())
      {
         throw std::runtime_error("failed to create pid file");
      }
   }

   /*!
    * \brief Construct a pidfile object
    *
    * Construct a pidfile object with the pid unset at this time (-1)
    *
    * @param filepath the file name and path of the pidfile
    *
    */
   explicit pidfile(
      boost::filesystem::path const & filepath
      )
      : pid_(-1), filepath_(filepath)
   {
   }

   /*!
    * \brief Destruct a pidfile object
    *
    * Destruct a pidfile object, removing the pid file in the process
    *
    */
   ~pidfile()
   {
      try
      {
         remove();
      }
      catch(...) {}
   }

   /*!
    * \brief Create a new pid file
    *
    * Using the last assigned pid, a new pid file will be created (any old one is removed first)
    *
    * \exception if the pid has not been set a runtime_error will be emitted.
    *
    */

   bool create() const
   {
      if(pid_ == -1)
      {
         throw std::runtime_error("Cannot create a pid file until the pid is set.");
      }

      remove(); // just in case there is a rogue file

      std::ofstream out(filepath_.string().c_str());
      out << pid_;
      return out.good();
   }

   /*!
   * \brief  Create a new pid file
   *
   * Set a new pid for this instance and use it to create a new pid file
   *
   * @param the pid that will be associated with this instance
   *
   */
   bool create(pid_t const pid)
   {
      pid_ = pid;
      return create();
   }

   /*!
    * \brief Remove pid file
    *
    * Remove the pid file associated with this instance
    *
    */
   bool remove() const
   {
      return boost::filesystem::remove(filepath_);
   }


   /*!
    * \brief Get the default run directory
    *
    * Get the default run directory where the pidfile should be created for this platform. You can use
    * any directory you like but it is generally best to stick to using the default for the platform.
    *
    */
   static boost::filesystem::path get_default_rundir()
   {
      return "/var/run";
   }


private:
   pid_t pid_;
   boost::filesystem::path filepath_;
};

}}


#endif // MOOST_PROCESS_PIDFILE_HPP__
