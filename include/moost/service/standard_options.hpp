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

#ifndef FM_LAST_MOOST_SERVICE_STANDARD_OPTIONS_H_
#define FM_LAST_MOOST_SERVICE_STANDARD_OPTIONS_H_

/**
 * \file standard_options.hpp
 */

#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>

#include "option_validator.hpp"

namespace moost { namespace service {

class standard_options : public boost::noncopyable
{
private:
   class option
   {
   private:
      const char *m_spec;
      const char *m_desc;

   public:
      option(const char *spec, const char *desc)
        : m_spec(spec)
        , m_desc(desc)
      {
      }

      const char *spec() const
      {
         return m_spec;
      }

      const char *desc() const
      {
         return m_desc;
      }

      static std::string longname(const char *spec)
      {
         std::vector<std::string> parts;
         boost::split(parts, spec, boost::is_any_of(","));
         return parts[0];
      }

      std::string longname() const
      {
         return longname(m_spec);
      }
   };

   option opt_config() const
   {
      return option("config,c", "config file name");
   }

   boost::program_options::options_description& m_od;
   option_validator& m_val;

   template <typename T>
   standard_options& add_option(const option& opt, boost::program_options::typed_value<T> *value)
   {
      m_od.add_options() (opt.spec(), value, opt.desc());
      return *this;
   }

   template <typename T>
   standard_options& add_option(const option& opt, boost::program_options::typed_value<T> *value, boost::shared_ptr<validator::base> validator, bool mandatory = false)
   {
      validator->set_option(opt.longname(), opt.desc());
      validator->mandatory(mandatory);
      m_val.add(validator);
      return add_option(opt, value);
   }

public:
   struct options
   {
      std::string config_file;
      int port;
      int pool;
      bool verbose;
   };

   standard_options(boost::program_options::options_description& od, option_validator& val)
     : m_od(od)
     , m_val(val)
   {
   }

   standard_options(boost::program_options::options_description& od, option_validator& val,
                    options& target, int default_port, int default_pool = 8)
     : m_od(od)
     , m_val(val)
   {
      config(target.config_file);
      port(target.port, default_port);
      pool(target.pool, default_pool);
      verbose(target.verbose);
   }

   standard_options& config(std::string& storage, bool mandatory = true)
   {
      return add_option(opt_config(),
                        boost::program_options::value<std::string>(&storage),
                        validator::file(storage),
                        mandatory);
   }

   standard_options& config(std::string& storage, const std::string& defval)
   {
      return add_option(opt_config(),
                        boost::program_options::value<std::string>(&storage)->default_value(defval),
                        validator::file(storage));
   }

   template <typename T>
   standard_options& port(T& storage, const T& defval)
   {
      return add_option(option("port,p", "listening port of service"),
                        boost::program_options::value<T>(&storage)->default_value(defval),
                        validator::number<T>(storage, 1024, 65535));
   }

   template <typename T>
   standard_options& port(const char *spec, const char *desc, T& storage, bool mandatory = false)
   {
      return add_option(option(spec, desc),
                        boost::program_options::value<T>(&storage),
                        validator::number<T>(storage, 1024, 65535),
                        mandatory);
   }

   template <typename T>
   standard_options& port(const char *spec, const char *desc, T& storage, const T& defval)
   {
      return add_option(option(spec, desc),
                        boost::program_options::value<T>(&storage)->default_value(defval),
                        validator::number<T>(storage, 1024, 65535));
   }

   template <typename T>
   standard_options& pool(T& storage, const T& defval = 8)
   {
      return add_option(option("pool", "pool size"),
                        boost::program_options::value<T>(&storage)->default_value(defval),
                        validator::number<T>(storage, 1, std::numeric_limits<T>::max()));
   }

   standard_options& verbose(bool& storage)
   {
      return add_option(option("verbose,v", "start with verbose set to 'everything'"),
                        boost::program_options::value<bool>(&storage)->zero_tokens()->default_value(false));
   }

   standard_options& operator()(const char *spec, const char *desc)
   {
      m_od.add_options() (spec, desc);
      return *this;
   }

   template <class T>
   standard_options& operator()(const char *spec, boost::program_options::typed_value<T> *value, const char *desc)
   {
      return add_option(option(spec, desc), value);
   }

   template <class T>
   standard_options& operator()(const char *spec, boost::program_options::typed_value<T> *value, const char *desc,
                                boost::shared_ptr<validator::base> validator)
   {
      return add_option(option(spec, desc), value, validator);
   }
};

} }

#endif
