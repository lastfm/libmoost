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

/**
 * @file global_init.hpp
 * @brief Initialise the global logging API
 * @author Ricky Cormier
 * @date 2012-02-09
 */

#ifndef MOOST_LOGGING_GLOBAL_HPP__
#define MOOST_LOGGING_GLOBAL_HPP__

#include <iostream>
#include <boost/filesystem.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/propertyconfigurator.h>

#include "../utils/singleton.hpp"
#include "pseudo_ostream.hpp"
#include "../compiler/attributes.hpp"

/**
 * @brief Allow quick and diry initialisation of logging
 * @note This is provided for backwards compatibility with the
 *       now deprecated init_logging.hpp initialisation process.
 */
#define MLOG_INIT() \
   moost::logging::global_singleton::instance().enable(std::string(*argv))

/**
 * @brief Allow quick and dirty checking on intialisation state
 * @note This is provided for backwards compatibility with the
 *       now deprecated init_logging.hpp initialisation process.
 */
#define MLOG_IS_INITIALISED() \
   moost::logging::global_singleton::instance().is_configured() && \
   moost::logging::global_singleton::instance().is_enabled()

namespace moost { namespace logging {

   /**
    * @brief Initialise the global logging API
    *
    * This class is designed to be a singleton and represents a single point
    * of entry into the initialisation API of the logging framework. Before
    * you can make use of the logging framework you must 'enable' it using
    * one of the enable methods. Once the framework has been enabled there is
    * no additional need to access the global singleton unless you wish to
    * either disable it again or change some of the logging options.
    *
    * An ostream reference can also be assigned to the global singleton,
    * which will be used to output any status/error information.
    *
    */

   class global
   {
   template <typename T> friend
      class moost::utils::singleton_default<T>::friend_type;

   private:
      global()
         : enabled_(false)
      {
         // Initially, logging shall be disabled
         disable();
      }

   public:

      /**
       * @brief Check if the logging framework is enabled
       *
       * This differs from is_configured in so far as the framework may very
       * well be configured but not actually enabled.
       *
       * @return True/False
       */
      bool is_enabled() const
      {
         return enabled_;
      }

      /**
       * @brief Check if the logging framework has been configured yet
       *
       * This differs from is_enabled in so far as the framework may very
       * well be configured but not actually enabled.
       *
       * @return True/False
       */
      bool is_configured() const
      {
         return log4cxx::LogManager::getLoggerRepository()->isConfigured();
      }

      /**
       * @brief Use default basic configuration
       *
       * @return True if logging was successfully enabled
       *
       */
      bool enable()
      {
         out_
            << "Enable logging using default basic config"
            << std::endl;

         log4cxx::BasicConfigurator::configure();
         log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
         return set_enabled(is_configured());
      }

      /**
       * @brief Use default basic configuration
       *
       * Will use the program name and search got logging config, if that can't
       * be found it will either use the default basic configuration or leave
       * configuration in a disabled state.
       *
       * @param filepath to a config file
       * @param use_default if config cannot be found
       *
       * @return True if logging was successfully enabled
       *
       */
      bool enable(boost::filesystem::path const & filepath,
            bool use_default = false)
      {
         out_
            << "Enable logging using "
            << filepath
            << std::endl;

         log4cxx::LogManager::shutdown();

         if(boost::filesystem::exists(filepath))
         {
            log4cxx::PropertyConfigurator::configure(filepath.string());
         }
         else
         if(use_default)
         {
            return enable();
         }

         return set_enabled(is_configured());
      }

      /**
       * @brief Use process name
       *
       * Will use the program name and search got logging config, if that can't
       * be found it will either use the default basic configuration or leave
       * configuration in a disabled state.
       *
       * @note It is permissiable to pass the full path to the program name,
       *       which is normally the format provided by argv[0]. In this case
       *       the program name will be extracted and used.
       *
       * @param filepath to the current program
       * @param use_default if config cannot be found
       *
       * @return True if logging was successfully enabled
       *
       */
      bool enable(std::string const & filepath,
            bool use_default = false)
      {
         // whether name is a filename or a path this returns the leaf
         std::string s = boost::filesystem::path(filepath).leaf();

         s.append(".lcf");
         char const * search_path[] = {
            //-------------------------------------------------
            // you get these paths for free when building debug
#ifndef NDEBUG
            "./",
            "./etc/",
            "./etc/mir/",
            "~/",
            "~/.mir/",
#endif
            //-------------------------------------------------
            "/etc/",
            "/etc/mir/",
         };

         size_t const search_path_len =
            sizeof(search_path)/sizeof(search_path[0]);
         std::string cfgfile;
         bool found = false;

         out_
            << "Searching for logging config file..."
            << std::endl;

         for(size_t i = 0; i < search_path_len && !found; i++)
         {
            cfgfile = (boost::filesystem::path(search_path[i]) /= s).string();

            out_
               << "-  trying: "
               << cfgfile
               << std::endl;

            found = boost::filesystem::exists(cfgfile);
         }

         out_
            << "Logging config "
            << (found?"":"not ")
            << "found"
            << std::endl;

         if(found)
         {
            return enable(boost::filesystem::path(cfgfile), use_default);
         }

         return enable();
      }

      /**
       * @brief  Disables the logging framework
       *
       * @note   Once this has been called the logging framework is then
       *         considered configured but not enabled.
       */
      void disable()
      {
         log4cxx::LogManager::getLoggerRepository()->setConfigured(true);
         log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
         enabled_ = false;

         out_
            << "All logging has been disabled"
            << std::endl;
      }

      /**
       * @brief Attach an ostream
       *
       * If this is set logging debug information will be sent to this stream.
       *
       * @note Output to the stream will be protected by a mutex so there is
       *       no need to take any additional steps to prevent race conditions
       *       on the stream.
       *
       *       Once the ostream is assigned it MUST remain available for the
       *       lifetime of the logger or until 'detatch_ostream' is called.
       *       It shall be the responsibility of the consuming application to
       *       ensure this is the case.
       *
       * @param out std::ostream reference
       */
      void attach_ostream(std::ostream & out)
      {
         out_.attach(out);
      }

      /**
       * @brief Detach the ostream
       *
       * Once called the status/debug ostream associated with the logging
       * framework will be detached and no longer used. After this call it is
       * safe to destroy the ostream.
       */
      void detach_ostream()
      {
         out_.detach();
      }

      /**
       * @brief Get a reference to the global pseudo ostream
       *
       * @return pseudo ostream reference
       */
      pseudo_ostream & get_ostream()
      {
         return out_;
      }

   private:
      bool set_enabled(bool enabled)
      {
         enabled_ = enabled;

          out_
             << "logging was "
             << (enabled_?"":"not ")
             << "successfully enabled"
             << std::endl;

          return enabled_;
      }


   private:
      bool enabled_;
      pseudo_ostream out_;
   };

   /**
    * @brief Singleton instance of global
    *
    * To enable logging:
    *
    *    global_singleton::instance().enable(...);
    */
   typedef moost::utils::singleton_default<global> global_singleton;

   /**
    * @brief Ensure logging is initialised and disabled
    *
    */
   constructor__(init_global_singleton)
   {
      global_singleton::instance().disable();
   }
}}

#endif
