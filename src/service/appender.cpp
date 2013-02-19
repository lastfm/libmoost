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

#include "../../include/moost/terminal_format.hpp"
#include "../../include/moost/service/appender.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/logger.h>
#include <log4cxx/helpers/transcoder.h>

using namespace moost::service;

stream_writer_iface::~stream_writer_iface()
{
}

appender_iface::~appender_iface()
{
}

appender_factory_iface::~appender_factory_iface()
{
}

//---------------------------------------------------------------------

class null_appender : public appender_iface
{
public:
   null_appender();

   virtual bool handle_command(std::string&, const std::string&, const std::string&);
   virtual std::string show_help() const;
   virtual bool attach();
   virtual bool detach();
};

null_appender::null_appender()
{
}

bool null_appender::handle_command(std::string&, const std::string&, const std::string&)
{
   return false;
}

std::string null_appender::show_help() const
{
   return "";
}

bool null_appender::attach()
{
   return true;
}

bool null_appender::detach()
{
   return true;
}

//---------------------------------------------------------------------

class log4cxx_appender : public moost::service::appender_iface
{
private:
   class impl : public log4cxx::AppenderSkeleton
   {
   public:
      impl(stream_writer_ptr sw)
        : m_writer(sw)
      {
      }

   protected:
      virtual void append(const log4cxx::spi::LoggingEventPtr &event, log4cxx::helpers::Pool &pool)
      {
         log4cxx::LogString ls;
         getLayout()->format(ls, event, pool);
         std::string s;
         log4cxx::helpers::Transcoder::encode(ls, s);
         m_writer->write(s.c_str(), s.length());
      }

      virtual void close()
      {
      }

      virtual bool requiresLayout() const
      {
         return true;
      }

   private:
      stream_writer_ptr m_writer;
   };

public:
   log4cxx_appender(stream_writer_ptr sw, log4cxx::LevelPtr& level)
      : m_app(new impl(sw))
      , m_layout(new log4cxx::PatternLayout)
   {
      set_conversion_pattern("[%d{yyyy-MMM-dd HH:mm:ss}|" + moost::terminal_format::color("%c", moost::C_CYAN) + "](%p) %m%n");
      m_app->setLayout(m_layout);
      m_app->setThreshold(level);
   }

   virtual bool handle_command(std::string& rv, const std::string& cmd, const std::string& args)
   {
      if (cmd == "level")
      {
         log_level(rv, args);
      }
      else if (cmd == "pattern")
      {
         log_pattern(rv, args);
      }
      else
      {
         return false;
      }

      return true;
   }

   virtual std::string show_help() const
   {
      std::string level;

      m_app->getThreshold()->toString(level);

      return "- level             show all logging levels\r\n"
             "- level off|fatal|error|warn|info|debug|trace|all\r\n"
             "                    set console log level <" + level + ">\r\n"
             "- level root|<appender name>\r\n"
             "        [off|fatal|error|warn|info|debug|trace|all]\r\n"
             "                    show/set root or appender log level\r\n"
             "- pattern [<fmt>]   show/set log pattern\r\n";
   }

   virtual bool attach()
   {
      log4cxx::Logger::getRootLogger()->addAppender(m_app);
      return true;
   }

   virtual bool detach()
   {
      log4cxx::Logger::getRootLogger()->removeAppender(m_app);
      return true;
   }

private:
   bool string_to_level(const std::string& level, log4cxx::LevelPtr& new_level) const
   {
      log4cxx::LevelPtr invalid = new log4cxx::Level(-1, LOG4CXX_STR("INVALID"), 7);
      new_level = log4cxx::Level::toLevel(level, invalid);
      return new_level != invalid;
   }

   std::string level_to_string(const log4cxx::LevelPtr& level) const
   {
      if (level)
      {
         std::string str;
         level->toString(str);
         return str;
      }

      return "<unknown>";
   }

   // getThreshold() is non-const so we require a non-const skeleton pointer
   std::string get_appender_level(log4cxx::AppenderSkeleton* skel) const
   {
      return level_to_string(skel->getThreshold());
   }

   std::string get_logger_level(const log4cxx::Logger* logger) const
   {
      return level_to_string(logger->getLevel());
   }

   bool log_level(std::string& rv, const std::string& args)
   {
      std::ostringstream oss;

      if (args.empty())
      {
         log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();

         oss << "console log level is [" << get_appender_level(m_app) << "]\r\n";
         oss << "root logger level is [" << get_logger_level(root) << "]\r\n";

         log4cxx::AppenderList appenders = root->getAllAppenders();

         for (log4cxx::AppenderList::iterator it = appenders.begin(); it != appenders.end(); ++it)
         {
            log4cxx::AppenderSkeleton* skel = dynamic_cast<log4cxx::AppenderSkeleton *>(it->operator->());

            if (skel && skel != m_app && !skel->getName().empty())
            {
               std::string name;
               log4cxx::helpers::Transcoder::encode(skel->getName(), name);
               oss << name << " appender level is [" << get_appender_level(skel) << "]\r\n";
            }
         }
      }
      else
      {
         std::vector<std::string> argv;
         boost::algorithm::split(argv, args, boost::algorithm::is_any_of(" \t"));
         log4cxx::LevelPtr new_level;
         bool write = false;

         switch (argv.size())
         {
            case 1:   // write console log level OR read root logger / named appender log level
               if (string_to_level(argv[0], new_level))
               {
                  // write console log level

                  m_app->setThreshold(new_level);

                  rv = "console log level set to [" + get_appender_level(m_app) + "]\r\n";
                  return true;
               }
               break;

            case 2:   // write named logger log level
               if (!string_to_level(argv[1], new_level))
               {
                  rv = "invalid log level: " + argv[1] + "\r\n";
                  return true;
               }

               write = true;

               break;

            default:
               rv = "invalid number of arguments\r\n";
               return true;
         }

         std::string action(write ? "set to" : "is");

         if (argv[0] == "root")
         {
            log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();

            if (write)
            {
               root->setLevel(new_level);
            }

            oss << "root logger level " << action << " [" + get_logger_level(root) << "]\r\n";
         }
         else
         {
            log4cxx::LogString name;
            log4cxx::helpers::Transcoder::decode(argv[0], name);
            log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();
            log4cxx::AppenderPtr app = root->getAppender(name);
            log4cxx::AppenderSkeleton* skel = dynamic_cast<log4cxx::AppenderSkeleton *>(app.operator->());

            if (!skel)
            {
               rv = "no such appender: " + argv[0] + "\r\n";
               return true;
            }

            if (write)
            {
               skel->setThreshold(new_level);
            }

            oss << name << " appender level " << action << " [" << get_appender_level(skel) << "]\r\n";
         }
      }

      rv = oss.str();

      return true;
   }

   bool log_pattern(std::string& rv, const std::string& args)
   {
      if (!args.empty())
      {
         set_conversion_pattern(args);
      }

      rv = "conversion pattern set to [" + get_conversion_pattern() + "]\r\n";

      return true;
   }

   void set_conversion_pattern(const std::string& pattern)
   {
      log4cxx::LogString ls_pattern;
      log4cxx::helpers::Transcoder::decode(pattern, ls_pattern);
      m_layout->setConversionPattern(ls_pattern);
   }

   std::string get_conversion_pattern() const
   {
      std::string pattern;
      log4cxx::helpers::Transcoder::encode(m_layout->getConversionPattern(), pattern);
      return pattern;
   }

   log4cxx::helpers::ObjectPtrT<impl> m_app;
   log4cxx::helpers::ObjectPtrT<log4cxx::PatternLayout> m_layout;
};

//---------------------------------------------------------------------

log4cxx_appender_factory::log4cxx_appender_factory(const std::string& level)
   : m_default_level(log4cxx::Level::toLevel(level, log4cxx::Level::getWarn()))
{
}

appender_ptr log4cxx_appender_factory::create(stream_writer_ptr sw)
{
   return boost::shared_ptr<appender_iface>(new log4cxx_appender(sw, m_default_level));
}

//---------------------------------------------------------------------

null_appender_factory::null_appender_factory()
{
}

appender_ptr null_appender_factory::create(stream_writer_ptr)
{
   return boost::shared_ptr<appender_iface>(new null_appender);
}
