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

#ifndef FM_LAST_MOOST_SERVICE_APPENDER
#define FM_LAST_MOOST_SERVICE_APPENDER

/**
 * \file appender.h
 *
 * Log appender interface
 */

#include <string>
#include <boost/shared_ptr.hpp>
#include <log4cxx/level.h>

namespace moost { namespace service {

class stream_writer_iface
{
public:
   virtual ~stream_writer_iface();
   virtual void write(const char *data, size_t len) = 0;
};

typedef boost::shared_ptr<stream_writer_iface> stream_writer_ptr;

/**
 * \brief Interface for log appenders.
 */
class appender_iface
{
public:
   virtual ~appender_iface();
   virtual bool handle_command(std::string& _return, const std::string& cmd, const std::string& args) = 0;
   virtual std::string show_help() const = 0;
   virtual bool attach() = 0;
   virtual bool detach() = 0;
};

typedef boost::shared_ptr<appender_iface> appender_ptr;

/**
 * \brief Interface for socket appender factories.
 */
class appender_factory_iface
{
public:
   virtual ~appender_factory_iface();

   /**
    * \brief Create a new appender.
    *
    * This method is typically called by remote_shell_server_impl when a
    * new client session is opened.
    */
   virtual appender_ptr create(stream_writer_ptr) = 0;
};

typedef boost::shared_ptr<appender_factory_iface> appender_factory_ptr;

/**
 * \brief A factory for log4cxx appenders.
 */
class log4cxx_appender_factory : public appender_factory_iface
{
private:
   log4cxx::LevelPtr m_default_level;

public:
   log4cxx_appender_factory(const std::string& = "warn");
   virtual appender_ptr create(stream_writer_ptr);
};

/**
 * \brief A factory for appenders that do nothing.
 */
class null_appender_factory : public appender_factory_iface
{
public:
   null_appender_factory();
   virtual appender_ptr create(stream_writer_ptr);
};

} }

#endif

