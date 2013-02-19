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

#include <cstdio>
#include <set>
#include <queue>
#include <string>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "../../include/moost/terminal_format.hpp"
#include "../../include/moost/utils/foreach.hpp"
#include "../../include/moost/io/helper.hpp"
#include "../../include/moost/io/async_stream_forwarder.hpp"
#include "../../include/moost/service/remote_shell.h"
#include "../../include/moost/service/posix_stream_stealer.h"

#if defined(_POSIX_SOURCE) || defined(__CYGWIN__)
// needed for ::dup(), ::write(), ::fileno()
# include <unistd.h>
#elif defined(_WIN32)
# include <windows.h>
#else
# error "apparently no local shell support has been added for this platform"
#endif

/*
 *  TODO: cleanup stdstream/dup/fileno/read/write/pipe stuff
 */

using boost::asio::ip::tcp;
using namespace boost;
using namespace moost;
using namespace moost::service;

class session_base;
typedef shared_ptr<session_base> session_ptr;
typedef shared_ptr<tcp::socket> socket_ptr;
typedef void (session_base::*session_meth)(const char *buffer, size_t count);

namespace moost { namespace service {

class remote_shell_server_impl : public noncopyable
{
private:
   struct noop_pre_shutdown_func
   {
      void operator()()
      {
      }
   };

   struct command
   {
      session_ptr session;
      std::string cmd;
      std::string args;

      command()
      {}

      command(const session_ptr& s, const std::string& c, const std::string& a)
         : session(s), cmd(c), args(a)
      {}
   };

#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
   typedef asio::posix::basic_stream_descriptor<> stdstream;
#endif

   void handle_accept(socket_ptr socket, remote_shell_iface *rsi, const system::error_code& error);
   void handle_stop(const std::string& msg);
   void accept_session(remote_shell_iface *rsi);
   void pre_shutdown();

#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
   bool snoop_stdio(posix_stream_stealer& stealer, FILE *stream, stdstream *bsd, session_meth cb);
   void on_stdio_read(stdstream *bsd, session_meth cb, const system::error_code& error, char *buffer, size_t count);
   void stdio_read_more(stdstream *bsd, session_meth cb, char *buffer);
#endif

   bool setup_stdio_snoopers();
   void teardown_stdio_snoopers();

   bool create_console_session(remote_shell_iface *rsi, session_ptr& new_session, stream_writer_ptr& writer);
   bool setup_console_session(remote_shell_iface *rsi);

   void command_thread(remote_shell_iface *rsi);

   shared_ptr<asio::io_service> m_ios;
   shared_ptr<tcp::acceptor> m_acceptor;
   std::set<session_ptr> m_sessions;
   appender_factory_ptr m_app_factory;
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
   stdstream m_stdout;
   stdstream m_stderr;
   posix_stream_stealer m_stdout_stealer;
   posix_stream_stealer m_stderr_stealer;
#endif
   std::string m_welcome;
   bool m_default_stdout_state;
   bool m_default_stderr_state;
   bool m_enable_local;
   bool m_can_snoop_stdio;
   unsigned short m_listen_port;
   shared_ptr<thread> m_cmd_runner;
   std::queue< command > m_cmd_queue;
   mutex m_cmd_queue_mutex;
   condition_variable m_cmd_queue_cond;
   boost::function0<void> m_pre_shutdown_func;

public:
   remote_shell_server_impl(shared_ptr<asio::io_service> ios);
   ~remote_shell_server_impl();
   void run(remote_shell_iface *rsi);
   void stop(const std::string& msg);
   void remove_session(session_ptr p);
   void set_appender_factory(appender_factory_ptr app_factory);
   void set_default_stdout_state(bool enabled);
   void set_default_stderr_state(bool enabled);
   void set_listen_port(unsigned short port);
   void set_pre_shutdown_function(boost::function0<void>& func);
   void enable_local_shell(bool enabled);
   void get_sessions_list(std::string& rv);
   void process_command(session_ptr session, const std::string& cmd, const std::string& args);

   const std::string& welcome() const
   {
      return m_welcome;
   }
};

} }

class session_io_socket
{
public:
   session_io_socket(socket_ptr socket)
     : m_socket(socket)
   {
   }

   void set_nodelay()
   {
      m_socket->set_option(tcp::no_delay(true));
   }

   std::string get_peer_string() const
   {
      std::ostringstream oss;
      oss << m_socket->remote_endpoint();
      return oss.str();
   }

   void close()
   {
      mutex::scoped_lock lock(m_mutex);
      m_socket->close();
   }

   template <typename HandlerT>
   void write_stdout(const std::string& data, HandlerT handler)
   {
      mutex::scoped_lock lock(m_mutex);
      asio::async_write(*m_socket, asio::buffer(data.c_str(), data.length()), handler);
   }

   template <typename HandlerT>
   void write_stderr(const std::string& data, HandlerT handler)
   {
      mutex::scoped_lock lock(m_mutex);
      asio::async_write(*m_socket, asio::buffer(data.c_str(), data.length()), handler);
   }

   template <typename HandlerT>
   void read_some(void *data, size_t size, HandlerT handler)
   {
      mutex::scoped_lock lock(m_mutex);
      m_socket->async_read_some(asio::buffer(data, size), handler);
   }

private:
   socket_ptr m_socket;
   mutex m_mutex;
};

class session_io_console
{
public:
   typedef moost::io::helper::native_io_t native_io_t;

   session_io_console(shared_ptr<asio::io_service> ios, native_io_t in_fd, native_io_t out_fd, native_io_t err_fd, const std::string& name)
     : m_ios(ios)
     , m_in_fwd(ios, in_fd)
     , m_out(out_fd)
     , m_err(err_fd)
     , m_name(name)
   {
   }

   void set_nodelay()
   {
      // nothing to do here
   }

   const std::string& get_peer_string() const
   {
      return m_name;
   }

   void close()
   {
      // this is required to make sure the async read finishes
      m_in_fwd.close();
   }

   template <typename HandlerT>
   void write_stdout(const std::string& data, HandlerT handler)
   {
      write_console(m_out, data, handler);
   }

   template <typename HandlerT>
   void write_stderr(const std::string& data, HandlerT handler)
   {
      write_console(m_err, data, handler);
   }

   template <typename HandlerT>
   void read_some(void *data, size_t size, HandlerT handler)
   {
      m_in_fwd.read_async(data, size, handler);
   }

private:
   template <typename HandlerT>
   void write_console(native_io_t fd, const std::string& data, HandlerT handler)
   {
      // We can't use boost::asio for this, as we could be writing
      // to a regular file, for which the asio reactor (e.g. epoll
      // or poll) might not have support.

      // So we just use the native, synchronous file i/o, but fake
      // it to behave as similar to boost::asio as possible.

      boost::system::error_code ec;
      size_t written = 0;

      {
         mutex::scoped_lock lock(m_mutex);

         if (!moost::io::helper::write(fd, data.c_str(), data.length(), &written))
         {
            ec.assign(moost::io::helper::error(), boost::asio::error::get_system_category());
            written = 0;
         }
      }

      // Make sure the handler is called synchronously, modelling
      // the behaviour of boost::asio.

      m_ios->post(bind(handler, ec, static_cast<std::size_t>(written)));
   }

   mutex m_mutex;
   shared_ptr<asio::io_service> m_ios;
   moost::io::async_stream_forwarder m_in_fwd;
   native_io_t m_out;
   native_io_t m_err;
   const std::string m_name;
};

template <class SessionIoT>
class session_writer : public stream_writer_iface
{
public:
   session_writer(shared_ptr<SessionIoT> io)
     : m_io(io)
   {
   }

   virtual void write(const char *data, size_t len)
   {
      std::string *s = new std::string(data, len);
      m_io->write_stdout(*s, bind(&session_writer<SessionIoT>::on_write_done, s));
   }

private:
   static void on_write_done(std::string *str)
   {
      delete str;
   }

   shared_ptr<SessionIoT> m_io;
};

class session_base : public enable_shared_from_this<session_base>, public noncopyable
{
protected:
   enum state {
      SESSION_CREATED,
      SESSION_ATTACHED,
      SESSION_STOPPING,
      SESSION_STOPPED
   };

public:
   session_base(remote_shell_server_impl& srv, remote_shell_iface *rsi, bool allow_quit, bool enable_cout_cerr, bool enable_cls)
      : m_srv(srv)
      , m_state(SESSION_CREATED)
      , m_rsi(rsi)
      , m_cout_on(true)
      , m_cerr_on(true)
      , m_processing_input(true)
      , m_allow_quit(allow_quit)
      , m_enable_cout_cerr(enable_cout_cerr)
      , m_enable_cls(enable_cls)
      , m_t_connect(boost::posix_time::second_clock::universal_time())
   {
   }

   virtual ~session_base()
   {
   }

   void start(appender_ptr appender)
   {
      set_nodelay();
      m_app = appender;
      m_app->attach();
      m_state = SESSION_ATTACHED;
      m_peer = get_peer_string();
      continue_session("accepted client connection from " + m_peer + "\r\n" + m_srv.welcome());
      read_more();
   }

   void stop(const std::string& msg)
   {
      if (is_attached())
      {
         write("\r\n" + msg, SESSION_STOPPING);
      }
   }

   void set_stdout_state(bool on)
   {
      m_cout_on = on;
   }

   void set_stderr_state(bool on)
   {
      m_cerr_on = on;
   }

   std::string get_info() const
   {
      boost::posix_time::time_duration duration = boost::posix_time::second_clock::universal_time() - m_t_connect;
      std::ostringstream oss;

      oss << m_peer << " [" << duration << "]";

      return oss.str();
   }

   void add_stdout(const char *buffer, size_t count);
   void add_stderr(const char *buffer, size_t count);
   void command_result(bool handled, const std::string& cmd, const std::string& rv);

protected:
   virtual void set_nodelay() = 0;
   virtual std::string get_peer_string() const = 0;
   virtual void close() = 0;
   virtual void read_more(char *data, size_t max) = 0;
   virtual void write_stdout(std::string *s, state st) = 0;
   virtual void write_stderr(std::string *s, state st) = 0;

   void on_write_done(std::string *s, state st, const system::error_code& error);
   void on_read_done(const system::error_code& error, size_t bytes_transferred);

private:
   void read_more()
   {
      read_more(m_data, max_length);
   }

   void write(const std::string& str)
   {
      write(str, m_state);
   }

   void write(const std::string& str, state st)
   {
      std::string *s = new std::string(str);
      write_stdout(s, st);
   }

   bool is_attached() const
   {
      return m_state >= SESSION_ATTACHED && m_state < SESSION_STOPPED;
   }

   bool is_stopped() const
   {
      return m_state >= SESSION_STOPPED;
   }

   template <typename T>
   void get_more_help(std::ostream& os, const T& obj) const;

   void get_help(std::string& rv) const;
   void continue_session(const std::string& str = "");
   void handle_stop();
   bool handle_command(std::string& rv, const std::string& cmd, const std::string& args);
   bool read_next_command(std::string& cmd, std::string& args);
   bool log_level(std::string& rv, const std::string& args);
   void process_input();

   remote_shell_server_impl& m_srv;
   std::string m_peer;
   enum { max_length = 1024 };
   char m_data[max_length];
   appender_ptr m_app;
   enum state m_state;
   remote_shell_iface * const m_rsi;
   std::string m_inbuf;
   bool m_cout_on;
   bool m_cerr_on;
   bool m_processing_input;
   const bool m_allow_quit;
   const bool m_enable_cout_cerr;
   const bool m_enable_cls;
   const boost::posix_time::ptime m_t_connect;
};

template <class SessionIoT, bool AllowQuit = true, bool EnableCLS = true>
class session : public session_base
{
public:
   session(remote_shell_server_impl& srv, remote_shell_iface *rsi, shared_ptr<SessionIoT> io, bool enable_cout_cerr = true)
      : session_base(srv, rsi, AllowQuit, enable_cout_cerr, EnableCLS)
      , m_io(io)
   {
   }

protected:
   virtual void set_nodelay()
   {
      m_io->set_nodelay();
   }

   virtual std::string get_peer_string() const
   {
      return m_io->get_peer_string();
   }

   virtual void close()
   {
      return m_io->close();
   }

   virtual void read_more(char *data, size_t max)
   {
      m_io->read_some(data, max, bind(&session::on_read_done, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
   }

   virtual void write_stdout(std::string *s, state st)
   {
      m_io->write_stdout(*s, bind(&session::on_write_done, shared_from_this(), s, st, asio::placeholders::error));
   }

   virtual void write_stderr(std::string *s, state st)
   {
      m_io->write_stderr(*s, bind(&session::on_write_done, shared_from_this(), s, st, asio::placeholders::error));
   }

private:
   shared_ptr<SessionIoT> m_io;
};

/**********************************************************************/

void session_base::add_stdout(const char *buffer, size_t count)
{
   if (m_cout_on)
   {
      std::string *s = new std::string();
      s->append(terminal_format::color(C_GREEN));
      s->append(buffer, count);
      s->append(terminal_format::reset());
      write_stdout(s, m_state);
   }
}

void session_base::add_stderr(const char *buffer, size_t count)
{
   if (m_cerr_on)
   {
      std::string *s = new std::string();
      s->append(terminal_format::color(C_RED));
      s->append(terminal_format::bold());
      s->append(buffer, count);
      s->append(terminal_format::reset());
      write_stderr(s, m_state);
   }
}

template <typename T>
void session_base::get_more_help(std::ostream& os, const T& obj) const
{
   std::string help;

   try
   {
      help = obj.show_help();
   }
   catch (const std::exception& e)
   {
      help = std::string("exception while getting help: ") + e.what() + "\r\n";
   }
   catch (...)
   {
      help = "unknown exception while getting help\r\n";
   }

   if (!help.empty())
   {
      os << "-------------------------------------------------------\r\n"
         << help;
   }
}

void session_base::get_help(std::string& rv) const
{
   std::ostringstream oss;

   oss <<    "=======================================================\r\n"
             "- help              show this help\r\n";

   if (m_enable_cout_cerr)
   {
      oss << "- cout [on|off]     get [set] stdout " << (m_cout_on ? "<ON>/off" : "on/<OFF>") << "\r\n"
             "- cerr [on|off]     get [set] stderr " << (m_cerr_on ? "<ON>/off" : "on/<OFF>") << "\r\n";
   }

   oss <<    "- sessions          show shell sessions\r\n";

   if (m_enable_cls)
   {
      oss << "- clear|cls         clear screen\r\n";
   }

   if (m_allow_quit)
   {
      oss << "- quit|exit|bye     quit this connection\r\n";
   }

   oss <<    "- shutdown          shut down application\r\n";

   get_more_help(oss, *m_app);
   get_more_help(oss, *m_rsi);

   oss <<    "=======================================================\r\n";

   rv = oss.str();
}

void session_base::on_write_done(std::string *s, state st, const system::error_code& error)
{
   if (st > m_state)
   {
      m_state = st;
   }

   delete s;

   if (!error)
   {
      switch (m_state)
      {
         case SESSION_ATTACHED:
            break;

         case SESSION_STOPPING:
            handle_stop();
            break;

         default:
            handle_stop();
            break;
      }
   }
   else if (!is_stopped())
   {
      handle_stop();
   }
}

void session_base::on_read_done(const system::error_code& error, size_t bytes_transferred)
{
   if (error)
   {
      handle_stop();
      return;
   }

   m_inbuf.append(m_data, bytes_transferred);

   process_input();
}

void session_base::process_input()
{
   while (m_processing_input)
   {
      std::string cmd, args;

      if (!read_next_command(cmd, args))
      {
         read_more();
         return;
      }

      if (cmd.empty())
      {
         continue_session();
      }
      else
      {
         std::string rv;

         if (handle_command(rv, cmd, args))
         {
            continue_session(rv);
         }
         else
         {
            /*
             *  This is either a service command, or an unknown command.
             *  (Only the service can tell for sure, unfortunately.)
             *  In any case, it potentially accesses a global resource which
             *  may not be accessed concurrently from multiple shell sessions.
             *  Thus, we route the request through the server who will queue
             *  all service commands and process them sequentially in a
             *  separate thread in order to avoid blocking of the clients.
             */

            m_processing_input = false;

            m_srv.process_command(shared_from_this(), cmd, args);
         }
      }
   }
}

void session_base::command_result(bool handled, const std::string& cmd, const std::string& rv)
{
   if (handled)
   {
      continue_session(rv);
   }
   else
   {
      continue_session("unknown command: " + cmd);
   }

   m_processing_input = true;

   process_input();
}

void session_base::continue_session(const std::string& str)
{
   if (!is_stopped())
   {
      std::ostringstream oss;

      oss << str;

      if (!str.empty() && str[str.length() - 1] != '\n')
      {
         oss << "\r\n";
      }

      try
      {
         oss << m_rsi->get_prompt();
      }
      catch (const std::exception& e)
      {
         oss << "(exception while getting prompt: " << e.what() << ")> ";
      }
      catch (...)
      {
         oss << "(unknown exception while getting prompt)> ";
      }

      write(oss.str());
   }
}

void session_base::handle_stop()
{
   if (!is_stopped())
   {
      close();

      if (is_attached())
      {
         m_app->detach();
      }

      m_cerr_on = m_cout_on = false;

      m_state = SESSION_STOPPED;

      m_srv.remove_session(shared_from_this());
   }
}

bool session_base::handle_command(std::string& rv, const std::string& cmd, const std::string& args)
{
   if (m_allow_quit && (cmd == "exit" || cmd == "quit" || cmd == "bye"))
   {
      handle_stop();
   }
   else if (cmd == "shutdown")
   {
      m_srv.stop("*** shutdown initiated by " + m_peer + " ***\r\n");
   }
   else if (m_enable_cout_cerr && (cmd == "cerr" || cmd == "cout"))
   {
      bool *var = cmd == "cerr" ? &m_cerr_on : &m_cout_on;

      if (args == "on")
      {
         *var = true;
      }
      else if (args == "off")
      {
         *var = false;
      }
      else if (args != "")
      {
         rv = "invalid argument for " + cmd;
         return true;
      }

      rv = cmd + " set to [" + (*var ? "on" : "off") + "]";
   }
   else if (cmd == "help")
   {
      get_help(rv);
   }
   else if (m_enable_cls && (cmd == "clear" || cmd == "cls"))
   {
      rv = "\033[2J\033[H";
   }
   else if (cmd == "sessions")
   {
      m_srv.get_sessions_list(rv);
   }
   else
   {
      try
      {
         return m_app->handle_command(rv, cmd, args);
      }
      catch (const std::exception& e)
      {
         rv = "exception while running appender command " + cmd + ": " + e.what();
      }
      catch (...)
      {
         rv = "unknown exception while running appender command " + cmd;
      }
   }

   return true;
}

bool session_base::read_next_command(std::string& cmd, std::string& args)
{
   // do we have a full line?

   std::string::size_type pos = m_inbuf.find('\n');

   if (pos == m_inbuf.npos)
   {
      return false;
   }

   // strip comments

   std::string::size_type cpos = m_inbuf.find('#');

   if (cpos != m_inbuf.npos && cpos < pos)
   {
      m_inbuf.replace(cpos, pos - cpos, "", 0);
      pos = cpos;
   }

   // parse command and arguments

   std::istringstream iss(m_inbuf);

   m_inbuf.replace(0, pos + 1, "", 0);

   iss >> cmd;

   getline(iss, args);
   trim(args);

   return true;
}

/**********************************************************************/

remote_shell_server_impl::remote_shell_server_impl(shared_ptr<asio::io_service> ios)
   : m_ios(ios)
   , m_app_factory(new null_appender_factory)
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
   , m_stdout(*ios)
   , m_stderr(*ios)
#endif
   , m_default_stdout_state(true)
   , m_default_stderr_state(true)
   , m_enable_local(false)
   , m_can_snoop_stdio(false)
   , m_listen_port(0)
   , m_pre_shutdown_func(noop_pre_shutdown_func())
{
}

remote_shell_server_impl::~remote_shell_server_impl()
{
}

#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
bool remote_shell_server_impl::snoop_stdio(posix_stream_stealer& stealer, FILE *stream, stdstream *bsd, session_meth cb)
{
   if (!stealer.steal(stream))
   {
      return false;
   }

   bsd->assign(stealer.get_pipe_fd());

   stdio_read_more(bsd, cb, new char[BUFSIZ]);

   return true;
}

void remote_shell_server_impl::on_stdio_read(stdstream *bsd, session_meth cb, const system::error_code& error, char *buffer, size_t count)
{
   if (error)
   {
      delete[] buffer;
      return;
   }

   foreach(session_ptr s, m_sessions)
   {
      (s.get()->*cb)(buffer, count);
   }

   stdio_read_more(bsd, cb, buffer);
}

void remote_shell_server_impl::stdio_read_more(stdstream *bsd, session_meth cb, char *buffer)
{
   bsd->async_read_some(asio::buffer(buffer, BUFSIZ),
       bind(&remote_shell_server_impl::on_stdio_read, this, bsd, cb,
         asio::placeholders::error, buffer, asio::placeholders::bytes_transferred));
}
#endif

bool remote_shell_server_impl::setup_stdio_snoopers()
{
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
   if (!snoop_stdio(m_stdout_stealer, stdout, &m_stdout, &session_base::add_stdout) ||
       !snoop_stdio(m_stderr_stealer, stderr, &m_stderr, &session_base::add_stderr))
   {
      return false;
   }

   // make sure our stolen streams behave like "standard" stdout/stderr
   setvbuf(stdout, NULL, _IOLBF, 0);
   setvbuf(stderr, NULL, _IONBF, 0);

   return true;
#else
   return false;
#endif
}

void remote_shell_server_impl::teardown_stdio_snoopers()
{
#ifdef BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR
   m_stdout_stealer.restore();
   m_stdout.close();
   m_stderr_stealer.restore();
   m_stderr.close();
#endif
}

bool remote_shell_server_impl::create_console_session(remote_shell_iface *rsi, session_ptr& new_session, stream_writer_ptr& writer)
{
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR) && defined(_POSIX_SOURCE)

   int orig_in_fd = ::fileno(stdin);

   if (orig_in_fd == -1)
   {
      return false;
   }

   int input_fd = ::dup(orig_in_fd);

   if (input_fd == -1)
   {
      return false;
   }

   // set up console session
   shared_ptr<session_io_console> io(new session_io_console(m_ios, input_fd,
      m_stdout_stealer.get_backup_fd(), m_stderr_stealer.get_backup_fd(), "local console"));

   new_session.reset(new session<session_io_console, false, true>(*this, rsi, io, m_can_snoop_stdio));

   writer.reset(new session_writer<session_io_console>(io));

   return true;

#elif defined(BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE) && defined(_WIN32)

   HANDLE in_hd = ::GetStdHandle(STD_INPUT_HANDLE);
   HANDLE out_hd = ::GetStdHandle(STD_OUTPUT_HANDLE);
   HANDLE err_hd = ::GetStdHandle(STD_ERROR_HANDLE);

   if (in_hd == INVALID_HANDLE_VALUE ||
       out_hd == INVALID_HANDLE_VALUE ||
       err_hd == INVALID_HANDLE_VALUE)
   {
      return false;
   }

   // set up console session
   shared_ptr<session_io_console> io(new session_io_console(m_ios, in_hd, out_hd, err_hd, "local console"));

   new_session.reset(new session<session_io_console, false, false>(*this, rsi, io, m_can_snoop_stdio));

   writer.reset(new session_writer<session_io_console>(io));

   return true;

#else

   return false;

#endif
}

bool remote_shell_server_impl::setup_console_session(remote_shell_iface *rsi)
{
   session_ptr session;
   stream_writer_ptr writer;

   if (!create_console_session(rsi, session, writer))
   {
      return false;
   }

   session->set_stdout_state(m_default_stdout_state);
   session->set_stderr_state(m_default_stderr_state);

   m_sessions.insert(session);

   session->start(m_app_factory->create(writer));

   return true;
}

void remote_shell_server_impl::stop(const std::string& msg)
{
   m_ios->post(bind(&remote_shell_server_impl::handle_stop, this, msg));
}

void remote_shell_server_impl::handle_accept(socket_ptr socket, remote_shell_iface *rsi, const system::error_code& error)
{
   if (!error)
   {
      shared_ptr<session_io_socket> io(new session_io_socket(socket));
      session_ptr new_session(new session<session_io_socket>(*this, rsi, io, m_can_snoop_stdio));

      stream_writer_ptr writer(new session_writer<session_io_socket>(io));

      new_session->set_stdout_state(m_default_stdout_state);
      new_session->set_stderr_state(m_default_stderr_state);

      m_sessions.insert(new_session);

      new_session->start(m_app_factory->create(writer));

      accept_session(rsi);
   }
}

void remote_shell_server_impl::accept_session(remote_shell_iface *rsi)
{
   socket_ptr socket(new tcp::socket(*m_ios));

   m_acceptor->async_accept(*socket,
       bind(&remote_shell_server_impl::handle_accept,
         this, socket, rsi, asio::placeholders::error));
}

void remote_shell_server_impl::remove_session(session_ptr p)
{
   m_sessions.erase(p);
}

void remote_shell_server_impl::handle_stop(const std::string& msg)
{
   if (m_acceptor)
   {
      m_acceptor->close();
   }

   foreach(session_ptr s, m_sessions)
   {
      m_ios->post(bind(&session_base::stop, s, msg));
   }

   m_ios->post(bind(&remote_shell_server_impl::pre_shutdown, this));
}

void remote_shell_server_impl::pre_shutdown()
{
   teardown_stdio_snoopers();
   m_pre_shutdown_func();
}

void remote_shell_server_impl::get_sessions_list(std::string& rv)
{
   std::ostringstream oss;

   foreach(session_ptr s, m_sessions)
   {
      oss << s->get_info() << "\r\n";
   }

   rv = oss.str();
}

void remote_shell_server_impl::process_command(session_ptr session, const std::string& cmd, const std::string& args)
{
   {
      mutex::scoped_lock lock(m_cmd_queue_mutex);
      m_cmd_queue.push(command(session, cmd, args));
   }

   m_cmd_queue_cond.notify_one();
}

void remote_shell_server_impl::command_thread(remote_shell_iface *rsi)
{
   /*
    *  Commands may need some time to execute, thus they are moved to a separate
    *  thread (this one) in order to avoid:
    *
    *    - blocking of any stdout/stderr output
    *    - blocking of any other client sessions
    */

   while (true)
   {
      command cmd;

      {
         mutex::scoped_lock lock(m_cmd_queue_mutex);

         if (m_cmd_queue.empty())
         {
            m_cmd_queue_cond.wait(lock);
         }

         if (m_cmd_queue.empty())
         {
            continue;
         }

         cmd = m_cmd_queue.front();
         m_cmd_queue.pop();
      }

      if (!cmd.session)
      {
         break;
      }

      std::string rv;
      bool handled = true;

      try
      {
         handled = rsi->handle_command(rv, cmd.cmd, cmd.args);
      }
      catch (const std::exception& e)
      {
         std::ostringstream oss;
         oss << "exception while running " << cmd.cmd << " command: " << e.what();
         rv = oss.str();
      }
      catch (...)
      {
         std::ostringstream oss;
         oss << "unknown exception while running " << cmd.cmd << " command";
         rv = oss.str();
      }

      if (m_sessions.find(cmd.session) != m_sessions.end())
      {
         m_ios->post(bind(&session_base::command_result, cmd.session, handled, cmd.cmd, rv));
      }
   }
}

void remote_shell_server_impl::set_appender_factory(appender_factory_ptr app_factory)
{
   m_app_factory = app_factory;
}

void remote_shell_server_impl::set_default_stdout_state(bool enabled)
{
   m_default_stdout_state = enabled;
}

void remote_shell_server_impl::set_default_stderr_state(bool enabled)
{
   m_default_stderr_state = enabled;
}

void remote_shell_server_impl::set_listen_port(unsigned short port)
{
   m_listen_port = port;
}

void remote_shell_server_impl::set_pre_shutdown_function(boost::function0<void>& func)
{
   m_pre_shutdown_func = func;
}

void remote_shell_server_impl::enable_local_shell(bool enabled)
{
   m_enable_local = enabled;
}

void remote_shell_server_impl::run(remote_shell_iface *rsi)
{
   if (m_listen_port == 0 && !m_enable_local)
   {
      throw std::runtime_error("invalid configuration (no local and no remote port)");
   }

   std::ostringstream oss;

   m_can_snoop_stdio = setup_stdio_snoopers();

   if (!m_can_snoop_stdio)
   {
      oss << "stdio snooping not available\r\n";
   }

   m_welcome = oss.str();

   if (m_enable_local)
   {
      if (!setup_console_session(rsi))
      {
         throw std::runtime_error("failed to set up local shell session");
      }
   }

   if (m_listen_port != 0)
   {
      m_acceptor.reset(new tcp::acceptor(*m_ios, tcp::endpoint(tcp::v4(), m_listen_port)));
      accept_session(rsi);
   }

   m_cmd_runner.reset(new thread(boost::bind(&remote_shell_server_impl::command_thread, this, rsi)));

   m_ios->run();

   {
      mutex::scoped_lock lock(m_cmd_queue_mutex);
      m_cmd_queue.push(command()); // shutdown thread
   }

   m_cmd_queue_cond.notify_one();
   m_cmd_runner->join();
}

/**********************************************************************/

remote_shell_server::remote_shell_server(shared_ptr<asio::io_service> ios)
   : m_impl(new remote_shell_server_impl(ios))
{
}

remote_shell_server::~remote_shell_server()
{
   delete m_impl;
}

void remote_shell_server::run(remote_shell_iface *rsi)
{
   m_impl->run(rsi);
}

void remote_shell_server::stop(const std::string& msg)
{
   m_impl->stop(msg);
}

void remote_shell_server::set_appender_factory(appender_factory_ptr app_factory)
{
   m_impl->set_appender_factory(app_factory);
}

void remote_shell_server::set_default_stdout_state(bool enabled)
{
   m_impl->set_default_stdout_state(enabled);
}

void remote_shell_server::set_default_stderr_state(bool enabled)
{
   m_impl->set_default_stderr_state(enabled);
}

void remote_shell_server::set_listen_port(unsigned short port)
{
   m_impl->set_listen_port(port);
}

void remote_shell_server::set_pre_shutdown_function(boost::function0<void>& func)
{
   m_impl->set_pre_shutdown_function(func);
}

void remote_shell_server::enable_local_shell(bool enabled)
{
   m_impl->enable_local_shell(enabled);
}

