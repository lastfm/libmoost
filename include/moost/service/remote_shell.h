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

#ifndef FM_LAST_MOOST_SERVICE_REMOTE_SHELL
#define FM_LAST_MOOST_SERVICE_REMOTE_SHELL

/**
 * \file remote_shell.h
 *
 * Remote command line shell and drop-in replacement for the moost::shell template. The
 * handler needs to implement the handle_command(), get_prompt() and show_help() methods
 * as defined by remote_shell_iface, but there's no need for it to inherit from that
 * interface. remote_shell_iface is only used internally and for documentation purposes.
 * You should not use it as a base class for your handler classes.

\code

class my_handler
{
public:
   void start();
   void stop();

   bool handle_command(string& rv, const string& cmd, const string& args)
   {
      // handle command with args, rv is a container for a message to send back to the user
      return false;   // return true if command was handled, false otherwise
   }

   std::string get_prompt() const
   {
      return "my_handler> ";
   }

   std::string show_help() const
   {
      return "This is my_handler's help\n";
   }
};

int main(void)
{
   my_handler hdl;
   moost::service::remote_shell<my_handler> shell(hdl);

   // use the following to integrate the remote shell with log4cxx
   shell.set_appender_factory(moost::service::appender_factory_ptr(
      new moost::service::log4cxx_appender_factory(default_log_level)
   ));

   // set the port the shell is listening on for remote connections
   shell.set_listen_port(42001);

   // disable stderr forwarding by default
   shell.set_default_stderr_state(false);

   hdl.start();
   shell.run();
   hdl.stop();

   return 0;
}
\endcode

 * To connect to the remote shell, use something like this:

\code
rlwrap nc myhost 42001
\endcode

 * Multiple concurrent connections are explicitly supported. Each connection will
 * have its own session with separate log appenders. A connection can be closed
 * using the \c exit, \c quit, or \c bye commands. This will \b not leave the run()
 * method. To leave the run() method in order to shut down the application, use the
 * \c shutdown command.
 */

#include <iostream>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "appender.h"

namespace moost { namespace service {

/**
 * \brief Interface for remote command shell.
 */
class remote_shell_iface
{
public:
   /**
    * \brief Application-specific command handler.
    *
    * \param _return    Write any message to be sent back to the user to this variable.
    * \param cmd        The command entered by the user
    * \param args       Any arguments entered after the command
    *
    * \returns          Boolean indicating whether or not the command was known, \b not
    *                   whether the command could be handled successfully. Return \c false
    *                   if you don't implement any commands.
    */
   virtual bool handle_command(std::string& _return, const std::string& cmd, const std::string& args) = 0;

   /**
    * \brief Return application prompt.
    *
    * \returns          String containing the application's prompt.
    */
   virtual std::string get_prompt() const = 0;

   /**
    * \brief Return command help.
    *
    * \returns          String containing usage information for the commands provided by
    *                   the application's command handler. Return an empty string if you
    *                   don't implement any commands.
    */
   virtual std::string show_help() const = 0;
};

class remote_shell_server_impl;

/**
 * \brief Compiler firewall for remote_shell_server_impl.
 */
class remote_shell_server : public boost::noncopyable
{
private:
   remote_shell_server_impl *m_impl;

public:
   remote_shell_server(boost::shared_ptr<boost::asio::io_service> ios);
   ~remote_shell_server();
   void run(remote_shell_iface *);
   void stop(const std::string&);
   void set_appender_factory(appender_factory_ptr);
   void set_default_stdout_state(bool);
   void set_default_stderr_state(bool);
   void set_listen_port(unsigned short);
   void set_pre_shutdown_function(boost::function0<void>&);
   void enable_local_shell(bool);
};

/**
 * \brief A remote command shell used for interacting with the user
 *
 * \tparam HandlerType   Class handling application-specific commands.
 *                       This class must implement the handle_command(), get_prompt()
 *                       and show_help() methods as defined by remote_shell_iface.
 */
template <class HandlerType>
class remote_shell : private remote_shell_iface
{
private:
   HandlerType& m_hdl;
   remote_shell_server m_srv;

public:
   remote_shell(HandlerType& hdl)
      : m_hdl(hdl), m_srv(boost::shared_ptr<boost::asio::io_service>(new boost::asio::io_service))
   {
   }

   remote_shell(HandlerType& hdl, boost::shared_ptr<boost::asio::io_service> ios)
      : m_hdl(hdl), m_srv(ios)
   {
   }

   virtual std::string get_prompt() const
   {
      return m_hdl.get_prompt();
   }

   virtual std::string show_help() const
   {
      return m_hdl.show_help();
   }

   virtual bool handle_command(std::string& _return, const std::string& cmd, const std::string& args)
   {
      return m_hdl.handle_command(_return, cmd, args);
   }

   void run()
   {
      m_srv.run(this);
   }

   void set_appender_factory(appender_factory_ptr app_factory)
   {
      m_srv.set_appender_factory(app_factory);
   }

   void set_default_stdout_state(bool enabled)
   {
      m_srv.set_default_stdout_state(enabled);
   }

   void set_default_stderr_state(bool enabled)
   {
      m_srv.set_default_stderr_state(enabled);
   }

   void set_listen_port(unsigned short port)
   {
      m_srv.set_listen_port(port);
   }

   void set_pre_shutdown_function(boost::function0<void> func)
   {
      m_srv.set_pre_shutdown_function(func);
   }

   void enable_local_shell(bool enabled = true)
   {
      m_srv.enable_local_shell(enabled);
   }

   void stop(const std::string& msg)
   {
      m_srv.stop(msg);
   }
};

} }

#endif

