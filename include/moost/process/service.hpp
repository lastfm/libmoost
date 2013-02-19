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

#ifndef FM_LAST_MOOST_PROCESS_SERVICE_H_
#define FM_LAST_MOOST_PROCESS_SERVICE_H_

/**
 * \file service.hpp
 *
 * The moost::process::service class hides most of the process of setting up a system
 * service within an easy to use, yet customisable template. It takes care of:
 *
 *   - daemonisation
 *   - pid file creation
 *   - setting up signal handlers
 *   - setting up a local and/or remote command shell
 *   - setting up a moost::standard_console
 *   - starting the service
 *   - shutting down the service
 *
 * For this to work, a (usually very small) service class must be provided that implements
 * the following methods:

\code
class MyService
{
public:
   typedef MyServiceHandler HandlerType;   // a typedef for the service handler

   const std::string& name() const;        // returns the name of the service

   boost::shared_ptr<HandlerType> handler();
                                 // returns a shared pointer to the service handler object,
                                 //    which must implement the necessary methods to be used
                                 //    with moost::service::remote_shell
                                 // guaranteed not to be called before start() or after stop()

   void start();                 // performs all operations necessary to create and start the
                                 //    service handler

   void stop();                  // performs all operations necessary to stop the service
                                 //    handler
};
\endcode

 * The service handler defined by \c HandlerType must be compatible with
 * moost::service::remote_shell, i.e. it must implement the get_prompt(), show_help()
 * and handler_command() methods. It can be, but doesn't neccessarily have to be derived
 * from moost::fm303_cmd_shell.
 *
 * With the above class, a simple service can be implemented in a couple of lines of code:

\code
int main()
{
   MLOG_INIT();

   moost::process::service<MyService> service(new MyService);

   service.set_pidfile(pidfile);       // optional: override pid file name
   service.set_shell_port(shell_port); // optional: to enable the remote shell, set the shell port

   service.run(in_daemon_mode);        // run the service in daemon/non-daemon mode

   return 0;
}
\endcode

 * Nothing important happens inside the \c service object until its run() method is
 * called. The run method will do the following:
 *
 * - When \c in_daemon_mode is \c true, it will first fork a child process using
 *   moost::process::daemon.
 *
 * - The parent process will create a pid file and exit.
 *
 * - If \c in_daemon_mode is \c true or a remote shell port has been set (i.e.
 *   if there is no local shell), it will set up a signal handler using
 *   moost::process::quit_handler.
 *
 * - It will then call the start() method of service class defined above.
 *   The start() method must return with the created service handler running
 *   in its own thread.
 *
 * - If a shell port has been set or if \c in_daemon_mode is \c false, it will
 *   now set up a shell using moost::service::remote_shell. Otherwise, it will
 *   just set up an interruptible sleeper using moost::process::sleeper.
 *
 * - Whenever either shell exits or a terminating signal is received, the
 *   stop() method of the service class will be called.
 *
 * - Finally, the run() method returns.
 *
 * - The pid file will be deleted by the moost::process::service destructor.
 *
 * The service class must ensure that its handler() method returns a valid
 * handler object after the start() method has been called until the stop()
 * method is called.
 *
 * Here are some examples for more advanced usage:
 *
 * - You can use the set_child_init_func() method to set a boost::function
 *   object that will be passed on to moost::process::daemon and that will
 *   be called just after the child process has been forked. If the service
 *   is not daemonised, the function object will be called just after the
 *   run() method has been entered. The return value is ignored in that case.
 *   Note that the child init function is called just in time to allow
 *   further configuration of the moost::process::service instance, i.e.
 *   you can still configure all options relevant for the child process
 *   from within the child init function.
 *
 * - You can also override the exit behaviour of the parent process using
 *   set_parent_exit_func(). The function object is called with the child's
 *   process id as its only argument. The default implementation just calls
 *   exit(0). If you don't call exit() from the function object, run() will
 *   just return in the parent process and as a side effect, the pid file
 *   will be deleted as soon as the parent's instance of moost::process::service
 *   is destroyed.
 *
 * - You can override the creation of a console logger by passing a console
 *   logger policy as a template argument to moost::process::service. The default
 *   \c MoostStandardConsoleLoggerPolicy will create a moost::logging::standard_console
 *   instance. Use NoConsoleLoggerPolicy if you don't want a console logger.
 *   Note that the remote shell doesn't require a console logger to support logging.
 *
 * If you think that your service code is too long even with moost::process::service,
 * have a look at moost::service::skeleton.
 */

#include <stdexcept>
#include <cassert>

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include "daemon.hpp"
#include "sleeper.hpp"
#include "pidfile.hpp"
#include "quit_handler.hpp"
#include "../logging/class_logger.hpp"
#include "../logging/standard_console.hpp"
#include "../service/remote_shell.h"
#include "../service/appender.h"

// XXX: This is just a workaround until hopefully some day show_help() and get_prompt() are const...
// #define MPS_FM303_SHELL_CONST const
#define MPS_FM303_SHELL_CONST

namespace moost { namespace process {

class NoConsoleLoggerPolicy
{
public:
   void create()
   {
   }

   bool set_log_level(const std::string&)
   {
      return false;
   }

   std::string show_help() const
   {
      return "";
   }

   bool handle_command(std::string&, const std::string&, const std::string&)
   {
      return false;
   }

   void enable()
   {
   }

   void disable()
   {
   }

   void log(const std::string&)
   {
   }
};

class MoostStandardConsoleLoggerPolicy
{
public:
   MoostStandardConsoleLoggerPolicy()
     : m_disabled(false)
     , m_invalid_log_level(new log4cxx::Level(-1, LOG4CXX_STR("INVALID"), 7 ))
   {
   }

   void create()
   {
      if (!m_console)
      {
         m_console.reset(new moost::logging::standard_console);
         set_log_level(m_log_level);
      }
   }

   bool set_log_level(const std::string& level)
   {
      if (m_console)
      {
         if (level.empty())
         {
            // default to 'info' level
            return m_console->setThreshold(log4cxx::Level::getInfo());
         }

         return m_console->setThreshold(level);
      }

      // this shouldn't be here; but we're safe as long as
      // this code and the logging framework are both in moost

      log4cxx::LevelPtr new_level = log4cxx::Level::toLevel(level, m_invalid_log_level);

      if (new_level == m_invalid_log_level)
      {
         return false;
      }

      m_log_level = level;

      return true;
   }

   std::string show_help() const
   {
      if (m_console && !m_disabled)
      {
         return "- level [off|fatal|error|warn|info|debug|trace|all]\n"
                "      set console log level [default: info]\n";
      }

      return "";
   }

   bool handle_command(std::string& rv, const std::string& cmd, const std::string& args)
   {
      if (m_console && !m_disabled)
      {
         if (cmd == "level")
         {
            if (set_log_level(args))
            {
               std::string level;
               m_console->getThreshold(level);
               rv = "log level set to [" + level + "]\n";
            }
            else
            {
               rv = "unknown log level " + args + "\n";
            }

            return true;
         }
      }

      return false;
   }

   void enable()
   {
      if (m_console && m_disabled)
      {
         m_console->enable();
         m_disabled = false;
      }
   }

   void disable()
   {
      if (m_console && !m_disabled)
      {
         m_console->disable();
         m_disabled = true;
      }
   }

   void log(const std::string& msg)
   {
      MLOG_NAMED_INFO("moost::process::service", msg);
   }

private:
   bool m_disabled;
   boost::scoped_ptr<moost::logging::standard_console> m_console;
   std::string m_log_level;
   log4cxx::LevelPtr m_invalid_log_level;
};

template<class ServiceT, class ConsoleLoggerPolicy = MoostStandardConsoleLoggerPolicy>
class service : public boost::noncopyable
{
private:
   struct noop_child_init_func
   {
      bool operator()()
      {
         return false;
      }
   };

   struct default_parent_exit_func
   {
      void operator()(pid_t)
      {
         exit(0);
      }
   };

   class service_wrapper : public boost::noncopyable
   {
   private:
      boost::shared_ptr<typename ServiceT::HandlerType> checked_handler()
      {
         boost::shared_ptr<typename ServiceT::HandlerType> hdl(m_service->handler());

         if (!hdl)
         {
            throw std::runtime_error("got null pointer for service handler");
         }

         return hdl;
      }

      boost::shared_ptr<const typename ServiceT::HandlerType> checked_handler() const
      {
         boost::shared_ptr<const typename ServiceT::HandlerType> hdl(m_service->handler());

         if (!hdl)
         {
            throw std::runtime_error("got null pointer for service handler");
         }

         return hdl;
      }

   public:
      service_wrapper(boost::shared_ptr<ServiceT> service)
        : m_service(service)
      {
      }

      bool set_log_level(const std::string& level)
      {
         return m_logger.set_log_level(level);
      }

      std::string get_prompt() MPS_FM303_SHELL_CONST
      {
         return checked_handler()->get_prompt();
      }

      std::string show_help() MPS_FM303_SHELL_CONST
      {
         std::string help;

         help += m_logger.show_help();
         help += checked_handler()->show_help();

         return help;
      }

      bool handle_command(std::string& rv, const std::string& cmd, const std::string& args)
      {
         if (m_logger.handle_command(rv, cmd, args))
         {
            return true;
         }

         return checked_handler()->handle_command(rv, cmd, args);
      }

      std::string name() const
      {
         return m_service->name();
      }

      void start()
      {
         m_logger.create();

         m_service->start();
      }

      void disable_logger()
      {
         m_logger.disable();
      }

      void enable_logger()
      {
         m_logger.enable();
      }

      void stop()
      {
         m_service->stop();
      }

      boost::shared_ptr<ServiceT> get_service()
      {
         return m_service;
      }

      void log(const std::string& msg)
      {
         m_logger.log(msg);
      }

   private:
      boost::shared_ptr<ServiceT> m_service;
      ConsoleLoggerPolicy m_logger;
   };

   struct enable_logger_func
   {
   public:
      enable_logger_func(service_wrapper& svc)
        : m_svc(svc)
      {
      }

      void operator()()
      {
         m_svc.enable_logger();
      }

   private:
      service_wrapper& m_svc;
   };

public:
   service(boost::shared_ptr<ServiceT> service)
     : m_svc(service)
     , m_started(false)
     , m_daemonised(false)
     , m_shell_port(0)
     , m_default_stdout_state(true)
     , m_default_stderr_state(true)
     , m_child_init_func(noop_child_init_func())
     , m_parent_exit_func(default_parent_exit_func())
   {
   }

   ~service()
   {
      if (m_pidfile)
      {
         m_pidfile->remove();
         m_pidfile.reset();
      }
   }

   bool set_log_level(const std::string& log_level)
   {
      return m_svc.set_log_level(log_level);
   }

   std::string name() const
   {
      return m_svc.name();
   }

   void set_shell_port(unsigned short shell_port)
   {
      m_shell_port = shell_port;
   }

   void set_appender_factory(moost::service::appender_factory_ptr factory)
   {
      m_app_factory = factory;
   }

   void set_default_stdout_state(bool enabled)
   {
      m_default_stdout_state = enabled;
   }

   void set_default_stderr_state(bool enabled)
   {
      m_default_stderr_state = enabled;
   }

   void set_child_init_func(boost::function0<bool> child_init_func)
   {
      m_child_init_func = child_init_func;
   }

   void set_parent_exit_func(boost::function1<void, pid_t> parent_exit_func)
   {
      m_parent_exit_func = parent_exit_func;
   }

   void run(bool daemonise = false)
   {
      if (daemonise)
      {
         moost::process::daemon d(false, m_child_init_func);

         m_pidfile.reset(m_pidfile_name.empty() ? new moost::process::pidfile(m_svc.name(), moost::process::pidfile::get_default_rundir().string())
                                                : new moost::process::pidfile(m_pidfile_name));

         if (d.is_parent())
         {
            pid_t child_pid = d.get_pid();

            if (!m_pidfile->create(child_pid))
            {
               throw std::runtime_error("failed to create pid file");
            }

            m_parent_exit_func(child_pid);

            return;
         }

         m_daemonised = true;
      }
      else
      {
         m_child_init_func();
      }

      moost::process::quit_handler::set(boost::bind(&moost::process::service<ServiceT, ConsoleLoggerPolicy>::quit_handler, this));

      m_svc.start();

      m_started = true;

      if (daemonise && !m_shell_port)
      {
         m_svc.disable_logger();
         m_sleeper.sleep();
      }
      else
      {
         // Create our io_service object and preload it with a function
         // object that will disable the standard_console logger. This
         // will be executed as soon as the io_service starts its main
         // event loop, i.e. when the local shell has taken over control.

         boost::shared_ptr<boost::asio::io_service> ios(new boost::asio::io_service);

         ios->post(boost::bind(&service_wrapper::disable_logger, &m_svc));

         m_remote_shell.reset(new remote_shell_t(m_svc, ios));

         if (m_app_factory)
         {
            m_remote_shell->set_appender_factory(m_app_factory);
         }

         m_remote_shell->set_default_stdout_state(m_default_stdout_state);
         m_remote_shell->set_default_stderr_state(m_default_stderr_state);

         if (m_shell_port)
         {
            m_remote_shell->set_listen_port(m_shell_port);
         }

         if (!daemonise)
         {
            // Enable a local shell if we're not daemonising. The pre shutdown
            // function object ensures that the standard_console has taken over
            // logging from the local shell before the local shell quits.

            m_remote_shell->enable_local_shell();
            m_remote_shell->set_pre_shutdown_function(enable_logger_func(m_svc));
         }

         m_remote_shell->run();
      }

      m_started = false;

      m_svc.stop();
   }

   void quit(const std::string& msg = "")
   {
      if (m_started)
      {
         if (m_remote_shell)
         {
            m_remote_shell->stop(msg);
         }

         m_sleeper.awaken();

         m_started = false;
      }
   }

   void set_pidfile(const std::string& pidfile)
   {
      boost::filesystem::path p(pidfile);

      if (!p.has_root_directory())
      {
         throw std::runtime_error("require absolute path for pid file");
      }

      m_pidfile_name = pidfile;
   }

   boost::shared_ptr<ServiceT> get_service()
   {
      return m_svc.get_service();
   }

private:
   void quit_handler()
   {
      m_svc.log("Received quit signal, shutting down.");
      quit("quit signal received, shutting down");
   }

   typedef moost::service::remote_shell<service_wrapper> remote_shell_t;

   service_wrapper m_svc;
   std::string m_pidfile_name;
   bool m_started;
   bool m_daemonised;
   boost::scoped_ptr<moost::process::pidfile> m_pidfile;
   unsigned short m_shell_port;
   boost::scoped_ptr<remote_shell_t> m_remote_shell;
   moost::service::appender_factory_ptr m_app_factory;
   moost::process::sleeper m_sleeper;
   bool m_default_stdout_state;
   bool m_default_stderr_state;
   boost::function0<bool> m_child_init_func;
   boost::function1<void, pid_t> m_parent_exit_func;
};

} }

#endif
