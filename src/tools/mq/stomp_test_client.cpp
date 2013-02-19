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

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>

#include "../../../include/moost/shell.hpp"
#include "../../../include/moost/logging/global.hpp"
#include "../../../include/moost/mq/stomp_client.h"
#include "../../../include/moost/version.h"

namespace po = boost::program_options;

namespace {

   template <class I>
   void parse_arguments(const std::string& str, I iter)
   {
      boost::tokenizer< boost::escaped_list_separator<char> >
         tok(str, boost::escaped_list_separator<char>("\\", " \t", "'\""));
      std::copy(tok.begin(), tok.end(), iter);
   }

}

class stomp_test_client
{
public:
   stomp_test_client()
      : m_consumer_pool_size(1)
      , m_keepalive_interval(30.0)
      , m_reconnect_interval(1.0)
      , m_default_log_level("info")
   {
   }

   int run(int argc, char * argv[])
   {
      if (init(argc, argv))
      {
         process();
      }

      return 0;
   }

   std::string show_help() const
   {
      return
         "- connect           connect to queue\r\n"
         "- disconnect        disconnect from queue\r\n"
         "- subscribe         subscribe to topic\r\n"
         "- unsubscribe       unsubscribe from topic\r\n"
         "- status            connection status\r\n"
         "- num_processed     get number of processed messages\r\n"
         "- num_pending       get number of pending messages\r\n"
         "- send              send a message\r\n"
         "- reset             reset client object\r\n"
         "-------------------------------------------------------\r\n"
         " <cmd> --help       will show help for each command\r\n"
      ;
   }

   std::string get_prompt() const
   {
      std::ostringstream oss;
      oss << "stomp> ";
      return oss.str();
   }

   bool handle_command(std::string& rv, const std::string& cmd, const std::string& args)
   {
      cmd_method_t meth;

      if      (cmd == "connect")       { meth = &stomp_test_client::connect; }
      else if (cmd == "disconnect")    { meth = &stomp_test_client::disconnect; }
      else if (cmd == "subscribe")     { meth = &stomp_test_client::subscribe; }
      else if (cmd == "unsubscribe")   { meth = &stomp_test_client::unsubscribe; }
      else if (cmd == "status")        { meth = &stomp_test_client::status; }
      else if (cmd == "num_processed") { meth = &stomp_test_client::num_processed; }
      else if (cmd == "num_pending")   { meth = &stomp_test_client::num_pending; }
      else if (cmd == "send")          { meth = &stomp_test_client::send; }
      else if (cmd == "reset")         { meth = &stomp_test_client::reset; }
      else
      {
         return false;
      }

      std::vector<std::string> av;
      av.push_back(cmd);

      try
      {
         parse_arguments(args, std::back_inserter(av));
      }
      catch (const std::exception& e)
      {
         rv = std::string("parse error: ") + e.what() + "\n";
         return true;
      }

      std::ostringstream os;

      if (run_command(meth, rv, av, os))
      {
         rv = os.str();
      }

      return true;
   }

private:
   typedef void (stomp_test_client::*cmd_method_t)(const std::vector<std::string>&, std::ostream&);

   size_t m_consumer_pool_size;
   float m_keepalive_interval;
   float m_reconnect_interval;
   std::string m_default_log_level;
   std::string m_logging_config;

   boost::shared_ptr<moost::mq::stomp_client> m_client;

   void show_help(po::options_description& opt) const
   {
      std::cout
         << "mq-stomp-test-client (" << LIBMOOST_REVISION_STR << ")\n"
         << "Build: " << __DATE__ << " (" << __TIME__ << ") " << LIBMOOST_COPYRIGHT_STR << "\n\n"
         << opt << std::endl;

      exit(0);
   }

   bool init(int argc, char * argv[])
   {
      po::options_description cmdline_options("Command line options");
      cmdline_options.add_options()
         ("consumer-pool-size", po::value<size_t>(&m_consumer_pool_size)->default_value(1), "consumer pool size")
         ("keepalive-interval", po::value<float>(&m_keepalive_interval)->default_value(30.0), "keepalive interval")
         ("reconnect-interval", po::value<float>(&m_reconnect_interval)->default_value(1.0), "reconnect interval")
         ("log-level,l", po::value<std::string>(&m_default_log_level)->default_value("info"), "default log level")
         ("logging-config", po::value<std::string>(&m_logging_config), "logging configuration file")
         ("help,h", "output help message and exit")
         ;

      po::variables_map vm;

      po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
      po::notify(vm);

      if (vm.count("help"))
      {
         show_help(cmdline_options);
         return false;
      }

      return true;
   }

   void parse_options(const std::vector<std::string>& args, const po::options_description& od, const po::positional_options_description& pd, po::variables_map& vm)
   {
      std::vector<char *> argv;
      for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
      {
         argv.push_back(const_cast<char *>(it->c_str()));
      }
      // po::store(po::parse_command_line(argv.size(), &argv[0], od), vm);
      po::store(po::command_line_parser(argv.size(), &argv[0]).options(od).positional(pd).run(), vm);
      po::notify(vm);
   }

   void error_callback(const boost::system::error_code& ec, const std::string& str)
   {
      std::cerr << "queue error: " << ec.message() << " (" << str << ")" << std::endl;
   }

   void message_callback(const std::string& topic, const std::string& msg)
   {
      std::cout << "[" << topic << "] " << msg << std::endl;
   }

   bool simple_command_help(const std::string& name, const std::vector<std::string>& args, std::ostream& os)
   {
      po::options_description opt(name + " options");
      opt.add_options()
         ("help,h", "output help message")
         ;

      po::positional_options_description pos;

      po::variables_map vm;

      parse_options(args, opt, pos, vm);

      if (vm.count("help"))
      {
         os << opt << std::endl;
         return false;
      }

      return true;
   }

   template <typename Func>
   void simple_command(const std::string& name, Func func, const std::vector<std::string>& args, std::ostream& os)
   {
      if (simple_command_help(name, args, os))
      {
         func();
      }
   }

   void connect(const std::vector<std::string>& args, std::ostream& os)
   {
      std::string server;
      int port;

      po::options_description opt("connect options");
      opt.add_options()
         ("server,s", po::value<std::string>(&server), "server name")
         ("port,p", po::value<int>(&port)->default_value(61613), "port")
         ("help,h", "output help message")
         ;

      po::positional_options_description pos;
      pos.add("server", 1).add("port", 1);

      po::variables_map vm;

      parse_options(args, opt, pos, vm);

      if (vm.count("help") || !vm.count("server") || !vm.count("port"))
      {
         os << opt << std::endl;
         return;
      }

      m_client->connect(server, port, boost::bind(&stomp_test_client::error_callback, this, _1, _2));
   }

   void subscribe(const std::vector<std::string>& args, std::ostream& os)
   {
      std::string topic;
      bool ack = false;
      float max_msg_interval = -1.0f;

      po::options_description opt("subscribe options");
      opt.add_options()
         ("topic,t", po::value<std::string>(&topic), "topic")
         ("ack,a", po::value<bool>(&ack)->zero_tokens(), "acknowledge")
         ("max-msg-interval,m", po::value<float>(&max_msg_interval), "max message interval")
         ("help,h", "output help message")
         ;

      po::positional_options_description pos;
      pos.add("topic", 1);

      po::variables_map vm;

      parse_options(args, opt, pos, vm);

      if (vm.count("help") || !vm.count("topic"))
      {
         os << opt << std::endl;
         return;
      }

      m_client->subscribe(topic, boost::bind(&stomp_test_client::message_callback, this, topic, _1),
                          ack ? moost::mq::stomp_client::ack::automatic : moost::mq::stomp_client::ack::client,
                          max_msg_interval < 0.0f ? boost::posix_time::time_duration(boost::posix_time::pos_infin)
                                                  : boost::posix_time::milliseconds(1000*max_msg_interval));
   }

   void unsubscribe(const std::vector<std::string>& args, std::ostream& os)
   {
      std::string topic;

      po::options_description opt("unsubscribe options");
      opt.add_options()
         ("topic,t", po::value<std::string>(&topic), "topic")
         ("help,h", "output help message")
         ;

      po::positional_options_description pos;
      pos.add("topic", 1);

      po::variables_map vm;

      parse_options(args, opt, pos, vm);

      if (vm.count("help") || !vm.count("topic"))
      {
         os << opt << std::endl;
         return;
      }

      m_client->unsubscribe(topic);
   }

   void send(const std::vector<std::string>& args, std::ostream& os)
   {
      std::string topic;
      std::string message;

      po::options_description opt("send options");
      opt.add_options()
         ("topic,t", po::value<std::string>(&topic), "topic")
         ("message,m", po::value<std::string>(&message), "message")
         ("help,h", "output help message")
         ;

      po::positional_options_description pos;
      pos.add("topic", 1);
      pos.add("message", 2);

      po::variables_map vm;

      parse_options(args, opt, pos, vm);

      if (vm.count("help") || !vm.count("topic") || !vm.count("message"))
      {
         os << opt << std::endl;
         return;
      }

      m_client->send(topic, message);
   }

   void disconnect(const std::vector<std::string>& args, std::ostream& os)
   {
      simple_command("disconnect", boost::bind(&moost::mq::stomp_client::disconnect, m_client), args, os);
   }

   void reset(const std::vector<std::string>& args, std::ostream& os)
   {
      simple_command("reset", boost::bind(&stomp_test_client::reset_client, this), args, os);
   }

   void status(const std::vector<std::string>& args, std::ostream& os)
   {
      if (simple_command_help("status", args, os))
      {
         os << (m_client->is_connected() ? "connected" : "disconnected") << ", "
            << (m_client->is_online() ? "online" : "offline") << "\n";
      }
   }

   void num_processed(const std::vector<std::string>& args, std::ostream& os)
   {
      if (simple_command_help("num_processed", args, os))
      {
         os << "processed messages: " << m_client->get_num_processed() << "\n";
      }
   }

   void num_pending(const std::vector<std::string>& args, std::ostream& os)
   {
      if (simple_command_help("num_pending", args, os))
      {
         os << "pending messages: " << m_client->get_num_pending() << "\n";
      }
   }

   bool run_command(cmd_method_t meth, std::string& rv, const std::vector<std::string>& av, std::ostream& os)
   {
      try
      {
         (this->*meth)(av, os);
         return true;
      }
      catch (const std::exception& e)
      {
         rv = std::string("error: ") + e.what() + "\n";
      }
      catch (...)
      {
         rv = "unknown error during command execution\n";
      }

      return false;
   }

   void process()
   {
      moost::logging::global_singleton::instance().enable(boost::filesystem::path(m_logging_config), true);
      moost::shell<stomp_test_client> shell(*this, m_default_log_level);
      reset_client();
      shell.run();
      m_client.reset();
   }

   void reset_client()
   {
      m_client.reset(new moost::mq::stomp_client(m_consumer_pool_size,
                                                 boost::posix_time::milliseconds(1000*m_keepalive_interval),
                                                 boost::posix_time::milliseconds(1000*m_reconnect_interval)));
   }
};

int main(int argc, char **argv)
{
   int retval = -1;

   try
   {
      retval = stomp_test_client().run(argc, argv);
   }
   catch(std::exception const & e)
   {
      std::cerr << "ERROR: " << e.what() << std::endl;
   }
   catch(...)
   {
      std::cerr << "ERROR: unknown error" << std::endl;
   }

   return retval;
}
