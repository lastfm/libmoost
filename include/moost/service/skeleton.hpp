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

#ifndef FM_LAST_MOOST_SERVICE_SKELETON_H_
#define FM_LAST_MOOST_SERVICE_SKELETON_H_

/**
 * \file skeleton.hpp
 *
 * The moost::service::skeleton template encapsulates common patterns used to run
 * services at Last.fm. It makes writing new services (and adapting old services)
 * very easy.
 *
 * It covers the following areas:
 *
 *   - exception handling: it ensures all exceptions will be handled
 *
 *   - command line option handling (both common and service-specific)
 *
 *   - command line option validation
 *
 *   - optional process ownership handling (i.e. changing process ownership
 *     from root to a less privileged user)
 *
 *   - optional logging framework initialisation
 *
 *   - setup and configuration of moost::process::service
 *
 * All customisation is done either template policies or by subclassing the
 * moost::service::skeleton template:

\code
// MyService is a moost::process::service compatible class

class MyServiceSkeleton : public moost::service::skeleton<MyService, NoProcessOwnershipPolicy>
{
private:
   int m_port;

public:
   virtual std::string name() const        { return "MyService"; }
   virtual std::string description() const { return "Bla bla."; }
   virtual std::string copyright() const   { return "Last.fm 2011"; }

   virtual void add_options(boost::program_options::options_description& od)
   {
      od.add_options()
         ("port,p", boost::program_options::value<int>(&m_port)->default_value(4711), "server port")
         ;
   }

   shared_ptr<MyService> create_service_instance()
   {
      return shared_ptr<MyService>(new MyService(m_port, name()));
   }
};
\endcode

 * With the above class, your main() function becomes as simple as:

\code
int main(int argc, char **argv)
{
   return moost::service::main<MyServiceSkeleton>(argc, argv);
}
\endcode

 * Please do not put any other code in main(). Put any other setup code you may need
 * either in the constructor of your service skeleton or in an appropriate overridable
 * method provided by \c moost::service::skeleton.
 *
 * The advantage of using moost::service::skeleton for your service is that all
 * functionality shared by the various services will look alike and work exactly
 * the same as in every other service. This greatly simplifies service maintenance
 * and other tasks like writing init scripts. It also ensures that once a new common
 * feature is added, all services will support that feature as soon as they're
 * recompiled.
 *
 * Of course, not all services are the same, and so you can configure moost::service::skeleton
 * to your liking. This is done either by template policies or by overriding methods.
 * Currently, the following policies are supported:
 *
 * - The \c ProcessOwnershipPolicy controls whether the service process supports changing
 *   ownership at runtime. The \c UidGidProcessOwnershipPolicy will provide the \c --uid
 *   and \c --gid command line options and enforce them if the process is run as root.
 *   If you don't require this, use the \c NoProcessOwnershipPolicy instead.
 *
 * - The \c LoggingPolicy controls whether or not the service process supports logging.
 *   By default, the \c MoostLoggingPolicy enables logging using the standard logging
 *   framework. This adds the \c --log-level command line option to set a default log
 *   level for moost::logging::standard_console and/or the remote shell. If your service
 *   doesn't support logging, you can use the \c NoLoggingPolicy instead.
 *
 * - The \c ConsoleLoggerPolicy is used to configure the moost::process::service instance
 *   and is taken from the \c LoggingPolicy by default.
 *
 * Here's an overview of what happens inside moost::service::main:
 *
 * - First of all, a try-catch-handler is set up. This handler is used purely for exceptions
 *   happening during construction of the skeleton instance. Once the skeleton instance is
 *   created, its main() method is called.
 *
 * - The skeleton's main() method is used only to set up a custom try-catch-handler. If
 *   you want to override the try-catch-handler, override main(). The try block only calls
 *   safe_main().
 *
 * - Now, command line options are gathered from various sources (policies, add_options())
 *   and all command line arguments are parsed.
 *
 * - If no command line arguments were found or the \c --help option was present, the
 *   skeleton writes a help message to the console and returns.
 *
 * - Next, command line options are validated.
 *
 * - Now the process ownership is changed according to the chosen policy.
 *
 * - Finally, the moost::process::service object is created and initialised using
 *   the instance returned by create_service_instance(). The moost::process::service
 *   object is configured based on the command line options provided and then its
 *   run() method is called.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

#include "../logging.hpp"
#include "../process/service.hpp"
#include "../process/ownership.hpp"
#include "standard_options.hpp"
#include "../utils/assert.hpp"

namespace moost { namespace service {

/**
 * \brief This policy implements no process ownership handling
 */
class NoProcessOwnershipPolicy
{
public:
   void add_options(boost::program_options::options_description&, option_validator&)
   {
   }

   void validate_options(const boost::program_options::variables_map&) const
   {
   }

   void change_ownership(const boost::program_options::variables_map&)
   {
   }
};

/**
 * \brief This policy implements process ownership handling based on user/groud IDs
 */
class UidGidProcessOwnershipPolicy
{
private:
   moost::process::ownership own;
   std::string m_uid;
   std::string m_gid;

public:
   void add_options(boost::program_options::options_description& od, option_validator&)
   {
      od.add_options()
         ("uid", boost::program_options::value<std::string>(&m_uid), "run process as this user")
         ("gid", boost::program_options::value<std::string>(&m_gid), "run process as this group")
         ;
   }

   void validate_options(const boost::program_options::variables_map& vm) const
   {
      if (own.is_superuser() && vm.count("uid") == 0)
      {
         throw std::runtime_error("please use --uid option when running as root");
      }

      if (vm.count("uid") == 0 && vm.count("gid") != 0)
      {
         throw std::runtime_error("cannot use --gid without --uid option");
      }
   }

   void change_ownership(const boost::program_options::variables_map& vm)
   {
      if (vm.count("uid"))
      {
         own.drop_privileges(m_uid, m_gid);
      }
   }
};

/**
 * \brief This policy implements no logging
 */
class NoLoggingPolicy
{
public:
   typedef moost::process::NoConsoleLoggerPolicy DefaultConsoleLoggerPolicy;

   void set_init_verbose(bool)
   {
   }

   void add_options(boost::program_options::options_description&, option_validator&)
   {
   }

   void validate_options(const boost::program_options::variables_map&) const
   {
   }

   bool initialise(const std::string&, const boost::program_options::variables_map&)
   {
      return false;
   }

   template <class T>
   void configure_service(T&) const
   {
   }
};

/**
 * \brief This policy implements logging using moost::logging
 */
class MoostLoggingPolicy
{
private:
   std::string m_log_level;
   std::string m_log_config;
   bool m_verbose;

public:
   typedef moost::process::MoostStandardConsoleLoggerPolicy DefaultConsoleLoggerPolicy;

   MoostLoggingPolicy()
      : m_verbose(false)
   {
   }

   void set_init_verbose(bool verbose)
   {
      m_verbose = verbose;
   }

   void add_options(boost::program_options::options_description& od, option_validator& opt_validator)
   {
      standard_options(od, opt_validator)
         ("log-level,l", boost::program_options::value<std::string>(&m_log_level)->default_value("info"), "default shell log level")
         ("logging-config", boost::program_options::value<std::string>(&m_log_config), "logging configuration file", validator::file(m_log_config))
         ;
   }

   void validate_options(const boost::program_options::variables_map&) const
   {
      log4cxx::LevelPtr invalid_level(new log4cxx::Level(-1, LOG4CXX_STR("INVALID"), 7 ));
      log4cxx::LevelPtr level = log4cxx::Level::toLevel(m_log_level, invalid_level);

      if (level == invalid_level)
      {
         throw std::runtime_error("invalid log level");
      }

      if (!m_log_config.empty() && !boost::filesystem::exists(m_log_config))
      {
         throw std::runtime_error("logging configuration file does not exist: " + m_log_config);
      }
   }

   bool initialise(const std::string& program_name, const boost::program_options::variables_map&)
   {
      moost::logging::global & glog = moost::logging::global_singleton::instance();

      glog.attach_ostream(std::cout);

      if (!(m_log_config.empty() ? glog.enable(program_name)
                                 : glog.enable(boost::filesystem::path(m_log_config))))
      {
         // the root (default) logger was not configured from a config file so we'll do our own thing with it

#if defined(NDEBUG) && !defined(SERVICE_SKELETON_NOCFG_LOG_ALL)
         MLOG_SET_DEFAULT_LEVEL_INFO();
#else
         MLOG_SET_DEFAULT_LEVEL_ALL();
#endif
      }

      return true;
   }

   template <class T>
   void configure_service(T& service) const
   {
      service.set_log_level(m_log_level);
      service.set_appender_factory(moost::service::appender_factory_ptr(new moost::service::log4cxx_appender_factory(m_log_level)));
   }
};

/**
 * \brief Configurable template implementing common functionality to run a service
 */
template <class ServiceT,
          class ProcessOwnershipPolicy = UidGidProcessOwnershipPolicy,
          class LoggingPolicy = MoostLoggingPolicy,
          class ConsoleLoggerPolicy = typename LoggingPolicy::DefaultConsoleLoggerPolicy>
class skeleton : public boost::noncopyable
{
private:
   bool m_logging_enabled;
   bool m_options_valid;
   std::string m_pidfile;
   unsigned short m_shell_port;
   boost::program_options::variables_map m_opt_varmap;
   ProcessOwnershipPolicy m_ownership;
   LoggingPolicy m_logging;
   std::string m_program_name;

   typedef moost::process::service<ServiceT, ConsoleLoggerPolicy> ProcessServiceType;

   class child_init_func
   {
   public:
      bool operator()()
      {
         m_log_enabled = m_log.initialise(m_program_name, m_varmap);
         m_log.configure_service(m_svc);
         return false;
      }

      child_init_func(ProcessServiceType& svc, LoggingPolicy& log, bool& log_enabled, const std::string& program_name, const boost::program_options::variables_map& varmap)
        : m_svc(svc)
        , m_log(log)
        , m_log_enabled(log_enabled)
        , m_program_name(program_name)
        , m_varmap(varmap)
      {
      }

   private:
      ProcessServiceType& m_svc;
      LoggingPolicy& m_log;
      bool& m_log_enabled;
      const std::string m_program_name;
      const boost::program_options::variables_map& m_varmap;
   };

protected:
   typedef ServiceT ServiceType;

   /**
    * \brief Add custom options for your service
    *
    * Override this if you want to define any extra command line options for your service.
    */
   virtual void add_options(boost::program_options::options_description&)
   {
   }

   /**
    * \brief Add custom options for your service, with validator support
    *
    * Override this if you want to define any extra command line options for your service.
    */
   virtual void add_options(boost::program_options::options_description&, option_validator&)
   {
   }

   /**
    * \brief Add options validation code
    *
    * Override this if you want to supply code to check command line options consistency.
    * You can access the raw option map using \c options().
    */
   virtual void validate_options() const
   {
   }

   /**
    * \brief Return the service name.
    */
   virtual std::string name() const = 0;

   /**
    * \brief Return the service version.
    */
   virtual std::string version() const = 0;

   /**
    * \brief Return short description of the service
    */
   virtual std::string description() const = 0;

   /**
    * \brief Return copyright string
    */
   virtual std::string copyright() const = 0;

   /**
    * \brief Create the service instance
    *
    * This method creates the instance of the service class after all checks have been
    * carried out, logging is set up and the process is running as a non-privileged user.
    */
   virtual boost::shared_ptr<ServiceType> create_service_instance() = 0;

   /**
    * \brief Return header for help message
    *
    * Override this if you really need full control over the service header that
    * is written to the console along with the command line options description.
    */
   virtual std::string help_header() const
   {
      std::ostringstream oss;
      oss << "\r\n" << name() << ": " << description() << "\r\n\r\n";
      oss << "Version: " << version() << " :: Build: " << __DATE__ << " (" << __TIME__ << ") :: (c) "
          << copyright() << "\r\n\r\n";
      return oss.str();
   }

   /**
    * \brief Check whether or not the logging framework is already set up.
    *
    * This is primarily useful for implementing custom catch() blocks in main().
    */
   bool logging_enabled() const
   {
      return m_logging_enabled;
   }

   /**
    * \brief Read-only access to command line options variable map.
    *
    * Call this method to validate/access command line options.
    */
   const boost::program_options::variables_map& options() const
   {
      if (!m_options_valid)
      {
         throw std::runtime_error("(BUG) attempt to access options map before initialisation");
      }

      return m_opt_varmap;
   }

   /**
    * \brief Check whether the service is about to be started in daemon mode.
    */
   bool running_as_daemon() const
   {
      return options().count("daemon") != 0;
   }

   /**
    * \brief Set up and process command line options.
    */
   void process_cmdline_options(boost::program_options::options_description& cmdline_options, option_validator& opt_validator, int argc, char **argv)
   {
      add_options(cmdline_options);
      add_options(cmdline_options, opt_validator);

      standard_options(cmdline_options, opt_validator)
         .port("shell-port", "remote shell port when daemonised", m_shell_port)
         ("pidfile", boost::program_options::value<std::string>(&m_pidfile), "pidfile location")
         ("noerr,n", "shut up stderr (for thrift)")
         ("help,h", "output help message and exit")
         ("version", "output service version and exit")
         ("daemon", "daemonise (fork to background)")
         ;

      m_ownership.add_options(cmdline_options, opt_validator);
      m_logging.add_options(cmdline_options, opt_validator);

      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, cmdline_options), m_opt_varmap);
      boost::program_options::notify(m_opt_varmap);
      m_options_valid = true;
   }

   /**
    * \brief Safe main() implementation.
    */
   int safe_main(int argc, char **argv, bool noargs)
   {
      m_program_name = argv[0];

      // process command line options first

      boost::program_options::options_description cmdline_options;
      option_validator opt_validator;
      process_cmdline_options(cmdline_options, opt_validator, argc, argv);

      if (options().count("help") || (!noargs && argc == 1))
      {
         std::cout << help_header() << cmdline_options << std::endl;
         return 0; // nothing more to do here
      }

      if (options().count("version"))
      {
         std::cout << version() << std::endl;
         return 0; // nothing more to do here
      }

      // some basic command line options consistency checks

      if (running_as_daemon() && m_shell_port == 0)
      {
         throw std::runtime_error("please use --shell-port option when running with --daemon");
      }

      moost::utils::assert_absolute_path(m_pidfile, "pidfile");

      // service specific checks

      validator::constraints_map_t opt_constraints;

      if (running_as_daemon())
      {
         opt_constraints["absolute_filenames"] = "1";
      }

      opt_validator(options(), opt_constraints);
      m_ownership.validate_options(options());
      m_logging.validate_options(options());
      validate_options();

      // configure logging initialisation verbosity

      m_logging.set_init_verbose(!running_as_daemon());

      // change ownership according to policy

      m_ownership.change_ownership(options());

      run();

      return 0;  // apparently, we've managed to finish cleanly
   }

   /**
    * \brief Run the service
    *
    * Override this if you want to do anything else than just running the service
    * depending on the commandline options. To actually run the service, call
    * run_service() from within your override.
    */
   virtual void run()
   {
      run_service();
   }

   /**
    * \brief Run the service
    *
    * Creates, configures and runs a service instance. Make sure you call this from
    * run() at some point when overriding it.
    */
   void run_service()
   {
      ProcessServiceType service(create_service_instance());

      child_init_func child_init(service, m_logging, m_logging_enabled, m_program_name, options());

      service.set_child_init_func(boost::ref(child_init));

      if (options().count("noerr"))
      {
         service.set_default_stderr_state(false);
      }

      if (!m_pidfile.empty())
      {
         service.set_pidfile(m_pidfile);
      }

      if (m_shell_port)
      {
         service.set_shell_port(m_shell_port);
      }

      service.run(running_as_daemon());
   }

public:
   skeleton()
      : m_logging_enabled(false)
      , m_options_valid(false)
      , m_shell_port(0)
   {
   }

   virtual ~skeleton()
   {
   }

   /**
    * \brief Main entry point for skeleton.
    *
    * Override this if you want to catch exceptions other than std:::exception or you
    * want to supply custom exception handling code. Make sure you don't change the
    * try { } block in your implementation and only call safe_main().
    */
   virtual int main(int argc, char **argv, bool noargs)
   {
      try
      {
         return safe_main(argc, argv, noargs);
      }
      catch (const std::exception& e)
      {
         if (logging_enabled())
         {
            MLOG_DEFAULT_FATAL(e.what());
         }

         throw; // will be caught by main template below
      }
      catch (...)
      {
         if (logging_enabled())
         {
            MLOG_DEFAULT_FATAL("unknown exception caught");
         }

         throw; // will be caught by main template below
      }

      return 1;
   }
};

template <class SkeletonT>
int main(int argc, char **argv, bool noargs = false)
{
   // This try/catch catches exceptions during construction of the skeleton instance
   // (those should be really rare).

   // Any other exceptions will be caught by the skeleton's main() implementation.

   try
   {
      return SkeletonT().main(argc, argv, noargs);
   }
   catch (const std::exception& e)
   {
      std::cerr << "ERROR: " << e.what() << std::endl;
   }
   catch (...)
   {
      std::cerr << "ERROR: unknown exception caught" << std::endl;
   }

   return 1;
}

} }

#endif
