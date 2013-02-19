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

#ifndef MOOST_SAFE_SHARED_PTR_HPP__
#define MOOST_SAFE_SHARED_PTR_HPP__

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>

#include "compiler/attributes/deprecated.hpp"

namespace moost {

/** safe_shared_ptr is a thread-safe shared ptr
 */
template <class T>
class safe_shared_ptr
{
private:

  template<class Y> friend class safe_shared_ptr;
  template<class Y, class Z> friend bool operator==(safe_shared_ptr<Y> const &, safe_shared_ptr<Z> const &);

  boost::shared_ptr<T> p_;
  mutable boost::mutex mutex_;

  /// Using strategy from "Modern C++ design", page 178
  class Tester
  {
     void operator delete(void*);
  };

public:

  typedef T element_type;

  safe_shared_ptr() // never throws
  {
  }

  template<class Y>
  explicit safe_shared_ptr(Y * p)
    : p_(p)
  {
  }

  template<class Y, class D>
  explicit safe_shared_ptr(Y * p, D d)
    : p_(p, d)
  {
  }

  template<class Y, class D, class A>
  explicit safe_shared_ptr(Y * p, D d, A a)
    : p_(p, d, a)
  {
  }

  safe_shared_ptr(safe_shared_ptr const & r)
    : p_(r.get_shared())
  {
  }

  template<class Y>
  safe_shared_ptr(safe_shared_ptr<Y> const & r) // never throws
    : p_(r.get_shared())
  {
  }

  template<class Y>
  safe_shared_ptr(boost::shared_ptr<Y> const & r) // never throws
    : p_(r)
  {
  }

  template<class Y>
  safe_shared_ptr(boost::shared_ptr<Y> const & r, T * p) // never throws
    : p_(r, p)
  {
  }

  template<class Y>
  explicit safe_shared_ptr(boost::weak_ptr<Y> const & r)
    : p_(r)
  {
  }

  template<class Y>
  explicit safe_shared_ptr(std::auto_ptr<Y> & r)
    : p_(r)
  {
  }

  safe_shared_ptr<T> & operator=(safe_shared_ptr<T> const & r) // never throws
  {
    if (this != &r)
    {
      boost::shared_ptr<T> tmp(r.get_shared());
      swap(tmp);
    }
    return *this;
  }

  safe_shared_ptr<T> & operator=(boost::shared_ptr<T> const & r) // never throws
  {
    boost::shared_ptr<T> tmp(r);
    swap(tmp);
    return *this;
  }

  void reset() // never throws
  {
    boost::shared_ptr<T> tmp;
    swap(tmp);
  }

  template<class Y>
  void reset(Y * p)
  {
    boost::shared_ptr<T> tmp(p);
    swap(tmp);
  }

  template<class Y, class D>
  void reset(Y * p, D d)
  {
    boost::shared_ptr<T> tmp(p, d);
    swap(tmp);
  }

  template<class Y, class D, class A>
  void reset(Y * p, D d, A a)
  {
    boost::shared_ptr<T> tmp(p, d, a);
    swap(tmp);
  }

  /*
   * The standard guarantees that you can safely call a method
   * or access a member on the temporary object returned by the
   * operator->(). As 12.2 [class.temporary] states, "temporary
   * objects are destroyed as the last step in evaluating the
   * full-expression that (lexically) contains the point where
   * they were created." This means that
   *
   *   a) the boost::shared_ptr<> will live for the duration of
   *      the method call (and possibly even longer if evaluation
   *      of the expression isn't finished when the method call
   *      returns) and
   *
   *   b) as a result, even if the safe_shared_ptr<> is either
   *      reset or destroyed, the temporary boost::shared_ptr<>
   *      will still keep the referenced object alive for the
   *      duration of expression evaluation.
   */
  boost::shared_ptr<T> operator->() const // never throws
  {
     return get_shared();
  }

  boost::shared_ptr<T> get_shared() const // never throws
  {
    boost::mutex::scoped_lock lock(mutex_);
    return p_;
  }

  bool unique() const // never throws
  {
    boost::mutex::scoped_lock lock(mutex_);
    return p_.unique();
  }

  long use_count() const // never throws
  {
    boost::mutex::scoped_lock lock(mutex_);
    return p_.use_count();
  }

  // operator! is redundant, but some compilers need it
  bool operator! () const // never throws
  {
    boost::mutex::scoped_lock lock(mutex_);
    return p_.operator !();
  }

  operator Tester*() const
  {
     {
       boost::mutex::scoped_lock lock(mutex_);
       if ( p_.get() == 0 )
         return 0;
     }

     static Tester test;
     return & test;
  }

  //operator bool() const // never throws
  //{
  //  boost::mutex::scoped_lock lock(mutex_);
  //  return p_.get() != 0;
  //}

  void swap(safe_shared_ptr<T> & b) // never throws
  {
    if (this != &b)
    {
      boost::mutex::scoped_lock lock1(&mutex_ < &b.mutex_ ? mutex_ : b.mutex_);
      boost::mutex::scoped_lock lock2(&mutex_ > &b.mutex_ ? mutex_ : b.mutex_);
      p_.swap(b.p_);
    }
  }

  void swap(boost::shared_ptr<T> & b)
  {
    boost::mutex::scoped_lock lock(mutex_);
    p_.swap(b);
  }

  // use safe_shared_ptr::scoped_lock if you want to deref multiple times but only lock once
  class scoped_lock : public boost::noncopyable
  {
  private:

    boost::mutex::scoped_lock lock_;
    boost::shared_ptr<T> & p_;

  public:

    scoped_lock(safe_shared_ptr<T> & p) : lock_(p.mutex_), p_(p.p_) {}

    void swap( boost::shared_ptr<T>& p ) { p_.swap(p); }

    T & operator*() const  { return p_.operator*(); } // never throws
    T * operator->() const { return p_.operator->(); } // never throws
    T * get() const        { return p_.get(); } // never throws
    bool unique() const    { return p_.unique(); } // never throws
    long use_count() const { return p_.use_count(); } // never throws
    operator bool() const  { return p_.operator bool(); } // never throws
  };

  class const_scoped_lock : public boost::noncopyable
  {
  private:

     boost::mutex::scoped_lock lock_;
     const boost::shared_ptr<T> & p_;

  public:

     const_scoped_lock(const safe_shared_ptr<T> & p) : lock_(p.mutex_), p_(p.p_) {}

     T & operator*() const  { return p_.operator*(); } // never throws
     T * operator->() const { return p_.operator->(); } // never throws
     T * get() const        { return p_.get(); } // never throws
     bool unique() const    { return p_.unique(); } // never throws
     long use_count() const { return p_.use_count(); } // never throws
     operator bool() const  { return p_.operator bool(); } // never throws
  };

};

template<class Y, class Z>
inline bool operator==(safe_shared_ptr<Y> const & a, safe_shared_ptr<Z> const & b)
{
  if (static_cast<const void *>(&a) == static_cast<const void *>(&b))
  {
    return true;
  }
  else
  {
    boost::mutex::scoped_lock lock1(&a.mutex_ < &b.mutex_ ? a.mutex_ : b.mutex_);
    boost::mutex::scoped_lock lock2(&a.mutex_ > &b.mutex_ ? a.mutex_ : b.mutex_);
    return a.p_ == b.p_;
  }
}

} // moost

#endif // MOOST_SAFE_SHARED_PTR_HPP__
