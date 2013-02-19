/* vim:set ts=2 sw=2 sts=2 et: */
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

#ifndef MOOST_CONTAINER_RESOURCE_STACK_HPP__
#define MOOST_CONTAINER_RESOURCE_STACK_HPP__

#include <exception>
#include <stack>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>
#include "../thread/xtime_util.hpp"
#include "../compiler/attributes/deprecated.hpp"

namespace moost { namespace container {

class no_resource_available : public std::runtime_error
{
public:
   no_resource_available(const std::string resource_name = "") throw()
     : std::runtime_error("no resource available for " + resource_name) {}
};

/** @brief a resource_stack is a thread-safe collection of resources.
 * Threads can get resources from the stack by using a scoped_resource.
 */
template<typename T>
class resource_stack
{
private:

  std::stack< boost::shared_ptr<T> > m_resources;
  boost::mutex                       m_mutex;
  boost::condition                   m_cond;
  size_t                             m_total_size;
  std::string                        m_resource_name;

public:

   typedef T resource_type;

public:

  /// Use a scoped_resource to get a resource from the resource_stack
  class scoped_resource
  {
  private:
    boost::shared_ptr<T> m_resource;
    resource_stack &     m_resource_stack;
  public:
    scoped_resource(resource_stack & resource_stack_, bool wait_on_empty = true);
    scoped_resource(resource_stack & resource_stack_, int timeout_ms, bool wait_on_empty = true);
    ~scoped_resource();
    T & operator* ();
    T * operator->();
  };

  /// Constructs a resource.
  resource_stack();
  resource_stack(const std::string& resource_name);

  /// Sets the name of the resource.
  void set_resource_name(const std::string& resource_name)
  { m_resource_name = resource_name; }

  /// Adds a resource to the stack.
  /// IMPORTANT: this will take the ownership of the pointer!!
  deprecated__ void add_resource(T * presource);

  void add_resource(boost::shared_ptr<T> spresource);

  /// Gets the size of the resource stack
  /// Note that this returns the number of available resources, not the total number of resources ever added.
  size_t size();

  /// Gets the total size of the resource stack (the number of resources added via add_resource)
  size_t total_size();
};

// implementation:

template<typename T>
resource_stack<T>::resource_stack()
   : m_total_size(0)
   , m_resource_name(
#if (defined(__GNUC__) && !defined(__GXX_RTTI))
        "undef"
#else
        typeid(T).name() // using RTTI
#endif
     )
{
}

template<typename T>
resource_stack<T>::resource_stack(const std::string& resource_name)
   : m_total_size(0)
   , m_resource_name(resource_name)
{
}

// NOTE: this is really dangerous - what if T* wasn't created on the heap? For this reason this method is now
//  deprecated. Please use void add_resource(boost::shared_ptr<T> spresource) instead.
template<typename T>
void resource_stack<T>::add_resource(T * presource)
{
  boost::shared_ptr<T> spresource(presource);
  boost::mutex::scoped_lock lock(m_mutex);
  m_resources.push(spresource);
  ++m_total_size;
  m_cond.notify_one();
}

template<typename T>
void resource_stack<T>::add_resource(boost::shared_ptr<T> spresource)
{
   boost::mutex::scoped_lock lock(m_mutex);
   m_resources.push(spresource);
   ++m_total_size;
   m_cond.notify_one();
}

template<typename T>
size_t resource_stack<T>::size()
{
  boost::mutex::scoped_lock lock(m_mutex);
  return m_resources.size();
}

template<typename T>
size_t resource_stack<T>::total_size()
{
  boost::mutex::scoped_lock lock(m_mutex);
  return m_total_size;
}

// scoped_resource:

template<typename T>
resource_stack<T>::scoped_resource::scoped_resource(resource_stack<T> & resource_stack_, bool wait_on_empty)
: m_resource_stack(resource_stack_)
{
  boost::mutex::scoped_lock lock(m_resource_stack.m_mutex);
  if ( !wait_on_empty && m_resource_stack.m_resources.empty() )
     throw no_resource_available(m_resource_stack.m_resource_name);

  while (m_resource_stack.m_resources.empty())
    m_resource_stack.m_cond.wait(lock);
  m_resource = m_resource_stack.m_resources.top();
  m_resource_stack.m_resources.pop();
}

template<typename T>
resource_stack<T>::scoped_resource::scoped_resource(resource_stack<T> & resource_stack_, int timeout_ms, bool wait_on_empty)
: m_resource_stack(resource_stack_)
{
  boost::mutex::scoped_lock lock(m_resource_stack.m_mutex);
  if ( !wait_on_empty && m_resource_stack.m_resources.empty() )
     throw no_resource_available(m_resource_stack.m_resource_name);

  while (m_resource_stack.m_resources.empty())
  {
    boost::xtime deadline =
       moost::thread::xtime_util::add_ms(moost::thread::xtime_util::now(), timeout_ms);
    if (!m_resource_stack.m_cond.timed_wait(lock, deadline))
      throw no_resource_available(m_resource_stack.m_resource_name);
  }
  m_resource = m_resource_stack.m_resources.top();
  m_resource_stack.m_resources.pop();
}

template<typename T>
resource_stack<T>::scoped_resource::~scoped_resource()
{
  boost::mutex::scoped_lock lock(m_resource_stack.m_mutex);
  m_resource_stack.m_resources.push(m_resource);
  m_resource_stack.m_cond.notify_one();
}

template<typename T>
T & resource_stack<T>::scoped_resource::operator* ()
{
  return *m_resource;
}

template<typename T>
T * resource_stack<T>::scoped_resource::operator-> ()
{
  return m_resource.get();
}

}} // moost::container

#endif // MOOST_CONTAINER_RESOURCE_STACK_HPP__
