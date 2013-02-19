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

#include "../../include/moost/configurable/binder.h"

#include <sstream>
#include <stdexcept>
#include <set>

using namespace boost;
using namespace moost::configurable;

binder::~binder()
{
  for (std::vector<persistable *>::iterator it = m_bindings.begin(); it != m_bindings.end(); ++it)
    delete *it;
}

void binder::child(const std::string & key, persistable & value)
{
  m_routes[key] = &value;
}

/// set all key/value pairs
void binder::read(std::istream & source)
{
  // first, read {
  std::string token;

  source >> token;

  if (token != "{")
    throw std::runtime_error("bad token: '" + token + "', expecting '}'");

  // any route that doesn't read, we must set the default
  std::set< std::string > found_routes;

  for (;;)
  {
    source >> token;

    if (token == "}")
      break;

    if (source.eof())
      throw std::runtime_error("unexpected eof, expecting '}'");

    route_map::iterator it = m_routes.find(token);

    if (it == m_routes.end())
      throw std::runtime_error("no route for token: '" + token + "'");

    found_routes.insert(it->first);

    source.get(); // skip past the ' '
    try
    {
      it->second->read(source);
    }
    catch (const std::runtime_error & e)
    {
      throw std::runtime_error(it->first + ": " + e.what());
    }
  }

  // set defaults for any remaining routes
  for (route_map::iterator it = m_routes.begin(); it != m_routes.end(); ++it)
  {
    if (found_routes.find(it->first) == found_routes.end())
    {
      try
      {
        it->second->set_default();
      }
      catch (const std::runtime_error & e)
      {
        throw std::runtime_error(it->first + ": " + e.what());
      }
    }
  }
}

/// get all key/value pairs
void binder::write(std::ostream & dest, int indent /* = 0 */) const
{
  // new context always means new indent
  indent += configurable::DEFAULT_INDENT;

  // first, write {
  dest << '{' << std::endl;
  for (int i = 0; i != indent; ++i)
    dest << ' ';

  for (route_map::const_iterator it = m_routes.begin(); it != m_routes.end(); ++it)
  {
    dest << it->first << ' ';
    it->second->write(dest, indent);
    dest << std::endl;
    for (int i = 0; i != indent; ++i)
      dest << ' ';
  }

  dest.seekp(-configurable::DEFAULT_INDENT, std::ios::cur);
  dest << '}';
}

/// get one key/value pair
void binder::set(const std::string & key, const std::string & value)
{
  route_map::iterator it = m_routes.find(key);

  size_t n = key.size();
  for ( ; it == m_routes.end(); )
  {
    n = key.rfind('.', n - 1);
    if (n == 0 || n == std::string::npos)
      throw std::runtime_error("no route for key: '" + key + "'");
    it = m_routes.find(key.substr(0, n));
  }

  if (n != key.size())
  {
    configurable * p = dynamic_cast<configurable *>(it->second);
    if (!p)
      throw std::runtime_error("no route for key: '" + key + "'");
    p->set(key.substr(n + 1, key.size() - n - 1), value);
  }
  else
  {
    std::istringstream iss(value);
    it->second->read(iss);
  }
}

/// set one key/value pair
void binder::get(const std::string & key, std::string & value) const
{
  route_map::const_iterator it = m_routes.find(key);

  size_t n = key.size();
  for (; it == m_routes.end(); )
  {
    n = key.rfind('.', n - 1);
    if (n == 0 || n == std::string::npos)
      throw std::runtime_error("no route for key: '" + key + "'");
    it = m_routes.find(key.substr(0, n));
  }

  if (n != key.size())
  {
    configurable * p = dynamic_cast<configurable *>(it->second);
    if (!p)
      throw std::runtime_error("no route for key: '" + key + "'");
    p->get(key.substr(n + 1, key.size() - n - 1), value);
  }
  else
  {
    std::ostringstream oss;
    it->second->write(oss);
    value = oss.str();
  }
}

void binder::list(std::vector< std::pair< std::string, std::string > > & items)
{
  for (route_map::iterator it = m_routes.begin(); it != m_routes.end(); ++it)
  {
    configurable * p = dynamic_cast<configurable *>(it->second);
    if (p)
    {
      std::vector< std::pair< std::string, std::string > > sub_items;
      p->list(sub_items);
      for (std::vector< std::pair< std::string, std::string > >::const_iterator it_s = sub_items.begin(); it_s != sub_items.end(); ++it_s)
        items.push_back(std::make_pair(it->first + '.' + it_s->first, it_s->second));
    }
    else
    {
      std::ostringstream oss;
      it->second->write(oss);
      items.push_back(std::make_pair(it->first, oss.str()));
    }
  }
}

void binder::set_default()
{
  for (route_map::iterator it = m_routes.begin(); it != m_routes.end(); ++it)
  {
    try
    {
      it->second->set_default();
    }
    catch (const std::runtime_error &)
    {
      throw std::runtime_error(it->first + ": cannot set default");
    }
  }

}
