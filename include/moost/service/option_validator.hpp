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

#ifndef FM_LAST_MOOST_SERVICE_OPTION_VALIDATOR_H_
#define FM_LAST_MOOST_SERVICE_OPTION_VALIDATOR_H_

/**
 * \file option_validator.hpp
 */

#include <string>

#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

#include "../utils/foreach.hpp"
#include "../utils/assert.hpp"

namespace moost { namespace service {

namespace validator {

typedef std::map<std::string, std::string> constraints_map_t;

class base
{
private:
   std::string m_longopt;
   std::string m_desc;
   bool m_mandatory;

public:
   base()
     : m_mandatory(false)
   {
   }

   virtual ~base()
   {
   }

   virtual void validate(const boost::program_options::variables_map& vm, const constraints_map_t& constraints) const
   {
      if (m_mandatory && vm.count(longopt()) == 0)
      {
         throw std::runtime_error("please specify the " + desc() + " with --" + longopt());
      }

      operator()(vm, constraints);
   }

   virtual void operator()(const boost::program_options::variables_map& vm, const constraints_map_t& constraints) const = 0;

   base *mandatory(bool mandatory = true)
   {
      m_mandatory = mandatory;
      return this;
   }

   void set_option(const std::string& longopt, const std::string& desc)
   {
      m_longopt = longopt;
      m_desc = desc;
   }

   const std::string& longopt() const
   {
      return m_longopt;
   }

   const std::string& desc() const
   {
      return m_desc;
   }
};

template <typename T>
class typed_base : public base
{
private:
   const T& m_data;

public:
   typed_base(const T& data)
     : m_data(data)
   {
   }

   const T& data() const
   {
      return m_data;
   }
};

class cfile : public typed_base<std::string>
{
private:
   const bool m_must_exist;

public:
   cfile(const std::string& file, bool must_exist)
     : typed_base<std::string>(file)
     , m_must_exist(must_exist)
   {
   }

   void operator()(const boost::program_options::variables_map& vm, const constraints_map_t& constraints) const
   {
      if (vm.count(longopt()) > 0)
      {
         if (m_must_exist && !boost::filesystem::exists(boost::filesystem::path(data())))
         {
            throw std::runtime_error("the " + desc() + " specified with --" + longopt() + " does not exist: " + data());
         }

         constraints_map_t::const_iterator it = constraints.find("absolute_filenames");

         if (it != constraints.end() && boost::lexical_cast<bool>(it->second))
         {
            moost::utils::assert_absolute_path(data(), desc());
         }
      }
   }
};

inline boost::shared_ptr<base>
file(const std::string& file, bool must_exist = true)
{
   return boost::shared_ptr<base>(new cfile(file, must_exist));
}

template <typename T>
class cnumber : public typed_base<T>
{
private:
   const T m_min;
   const T m_max;

public:
   cnumber(const T& num, const T& min, const T& max)
     : typed_base<T>(num)
     , m_min(min)
     , m_max(max)
   {
   }

   void operator()(const boost::program_options::variables_map& vm, const constraints_map_t&) const
   {
      if (vm.count(this->longopt()) > 0)
      {
         if (this->data() < m_min || this->data() > m_max)
         {
            throw std::runtime_error("the " + this->desc() + " specified with --" + this->longopt() + " is out of range");
         }
      }
   }
};

template <typename T>
inline boost::shared_ptr<base>
number(const T& num, const T& min = std::numeric_limits<T>::min(), const T& max = std::numeric_limits<T>::max())
{
   return boost::shared_ptr<base>(new cnumber<T>(num, min, max));
}

class cregex : public typed_base<std::string>
{
private:
   const std::string m_regex;
   boost::smatch *m_results;
   boost::regex_constants::match_flag_type m_match_flags;

public:
   cregex(const std::string& text, const std::string& regex, boost::smatch *results,
         boost::regex_constants::match_flag_type match_flags)
     : typed_base<std::string>(text)
     , m_regex(regex)
     , m_results(results)
     , m_match_flags(match_flags)
   {
   }

   void operator()(const boost::program_options::variables_map& vm, const constraints_map_t&) const
   {
      if (vm.count(longopt()) > 0)
      {
         boost::regex rx(m_regex);

         if (!(m_results ? boost::regex_match(data(), *m_results, rx, m_match_flags) : boost::regex_match(data(), rx)))
         {
            throw std::runtime_error("the " + desc() + " specified with --" + longopt() + " doesn't match its specification: " + data());
         }
      }
   }
};

inline boost::shared_ptr<base>
regex(const std::string& text, const std::string& regex, boost::smatch *results = 0,
      boost::regex_constants::match_flag_type match_flags = boost::regex_constants::match_default)
{
   return boost::shared_ptr<base>(new cregex(text, regex, results, match_flags));
}

}

class option_validator : public boost::noncopyable
{
private:
   std::vector< boost::shared_ptr<validator::base const> > m_validators;

public:
   void add(boost::shared_ptr<validator::base const> validator)
   {
      m_validators.push_back(validator);
   }

   void operator()(const boost::program_options::variables_map& vm, const validator::constraints_map_t& constraints) const
   {
      foreach(boost::shared_ptr<validator::base const> validator, m_validators)
      {
         validator->validate(vm, constraints);
      }
   }
};

} }

#endif
