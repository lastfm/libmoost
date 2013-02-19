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

/**
 * @file guarded_ptr.hpp
 *
 * @brief scoped pointer paired with a mutex
 *
 * If objects of type 'foo' are not inherently thread-safe to use, a guarded_ptr
 * can be used to add locking around the foo object. A guarded_ptr cannot make
 * any arbitrary object automagically thread-safe, but it pairs the pointer to
 * the object with a mutex and only gives you access to the object while you
 * are holding a lock. The developer still needs to know what locks (shared or
 * exclusive) are necessary to carry out operations on the object. Shared locks
 * will only grant you access to a const-reference of the object.
 *
 * Also, just like with other smart pointers, you need to follow some rules in
 * order to benefit from the smartness. There are certainly ways to obtain a
 * pointer to the original object through some methods. Saving such a pointer and
 * using it in other places will undermine the benefits. (As it would if you did
 * the same for e.g. a shared_ptr.)
 *
 * @code
 * moost::guarded_ptr<foo> bar(new foo);
 * @endcode
 *
 * The guarded_ptr object has no methods to call. Upon destruction it will take of
 * destruction of the foo object.
 *
 * To use the object referenced by bar, a lock must be acquired.
 *
 * @code
 * moost::guarded_ptr<foo>::shared_access barlock(bar);
 * barlock->some_const_method_of_foo();
 * @endcode
 *
 * The lock is kept until barlock runs out of scope. The shared_access object keeps
 * are shared lock of the mutex in the guarded_ptr object. shared_access implements
 * operator* and operator->, which both return a const-pointer to the underlying foo
 * object. If you need to call non-const methods, you will have to acquire an
 * exclusive lock, for which a class exclusive_lock exists.
 *
 * @code
 * moost::guarded_ptr<foo>::exclusive_access barlock(bar);
 * barlock->some_method_of_foo();
 * @endcode
 *
 * The exclusive_access class works much like shared_access, but it acquires a full,
 * exclusive lock of the mutex within guarded_ptr. It allows to call const and non-const
 * methods of the underlying object.
 *
 * There is also an upgradable_access class.
 *
 * @code
 * moost::guarded_ptr<foo>::upgradable_access barlock(bar);
 * barlock->some_const_method_of_foo();
 * moost::guarded_ptr<foo>::upgradable_access::upgrade barlock2(barlock);
 * barlock2->some_method_of_foo();
 * @endcode
 *
 * The upgradable_access constructor acquires an upgradable lock. It secures shared
 * access to the object which can later be upgraded to exclusive access. The
 * upgradable_access object itself only yields const-pointers to the underlying
 * object, but an upgrade object can be constructed from it, which then also offers
 * non-const pointers. Upon construction of the (first) upgrade object from an
 * upgradable_access object, the lock held by upgradable_access is promoted to an
 * exclusive lock.
 *
 * @note
 * Objects of types shared_access, exclusive_access, upgradable_access and upgrade
 * must never outlive the objects they were created from.
 */

#ifndef MOOST_GUARDED_PTR_HPP__
#define MOOST_GUARDED_PTR_HPP__

#include <cassert>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

namespace moost {

/**
 * @brief Wrapper around shared_ptr that guards the referenced object with a mutex
 *
 * Objects of this template class are constructed from a shared pointer. A guarded_ptr
 * keeps a copy of the shared_ptr and allows construction of scoped shared_access,
 * exclusive_access and upgradable_access objects that acquire the corresponding type
 * of lock for the guarded_ptr's internal mutex and implement operator* and operator->
 * to reference the object the original shared_ptr refers to.
 */
template<typename T>
class guarded_ptr : boost::noncopyable
{
public:
   typedef T element_type;
   typedef T value_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;

   /**
    * @brief Objects of this class represent a shared lock of the guarded_ptr's mutex
    *
    * shared_access objects can only be constructed from guarded_ptr objects. They
    * yield const-pointers to the object referenced by the guarded_ptr.
    *
    * The shared_access object must never outlive the underlying guarded_ptr object.
    */
   class shared_access : boost::noncopyable
   {
   public:
      shared_access(guarded_ptr const & guard)
         : m_gptr(guard)
      {
         m_gptr.m_mutex.lock_shared();
      }

      ~shared_access()
      {
         m_gptr.m_mutex.unlock_shared();
      }

      const_reference operator*() const
      {
         return *(m_gptr.m_ptr);
      }
      const_pointer operator->() const
      {
         return m_gptr.m_ptr.get();
      }

   private:
      guarded_ptr const & m_gptr;
   };

   /**
    * @brief Objects of this class represent an exclusive lock of the guarded_ptr's mutex
    *
    * exclusive_access objects can only be constructed from guarded_ptr objects. They
    * yield pointers to the object referenced by the guarded_ptr.
    *
    * The exclusive_access object must never outlive the underlying guarded_ptr object.
    */
   class exclusive_access : boost::noncopyable
   {
   public:
      exclusive_access(guarded_ptr &guard)
         : m_gptr(guard)
      {
         m_gptr.m_mutex.lock();
      }

      ~exclusive_access()
      {
         m_gptr.m_mutex.unlock();
      }

      reference operator*()
      {
         return *(m_gptr.m_ptr);
      }
      const_reference operator*() const
      {
         return *(m_gptr.m_ptr);
      }
      pointer operator->()
      {
         return m_gptr.m_ptr.get();
      }
      const_pointer operator->() const
      {
         return m_gptr.m_ptr.get();
      }

   private:
      guarded_ptr & m_gptr;
   };

   /**
    * @brief Objects of this class represent an upgradable lock of the guarded_ptr's mutex
    *
    * upgradable_access objects can only be constructed from guarded_ptr objects. They
    * yield const-pointers to the object referenced by the guarded_ptr. An upgrade object
    * can be constructed from the upgradable_access object, which leads to upgrading the
    * upgradable lock to an exclusive lock. The upgrade object can deliver a non-const
    * pointer to the referenced object.
    *
    * The upgradable_access object must never outlive the underlying guarded_ptr object.
    */
   class upgradable_access : boost::noncopyable
   {
   public:
      upgradable_access(guarded_ptr &guard)
         : m_gptr(guard)
         , m_upgraded(false)
      {
         m_gptr.m_mutex.lock_upgrade();
      }
      ~upgradable_access()
      {
         if (m_upgraded)
         {
            m_gptr.m_mutex.unlock();
         }
         else
         {
            m_gptr.m_mutex.unlock_upgrade();
         }
      }

      const_reference operator*() const
      {
         return *(m_gptr.m_ptr);
      }
      const_pointer operator->() const
      {
         return m_gptr.m_ptr.get();
      }

      /**
       * @brief Objects of this class represent an upgrade of an upgradable lock
       *
       * upgrade objects can only be constructed from upgradable_access objects. The
       * lock held by the upgradable_access object is promoted to an exclusive lock upon
       * construction of the first upgrade object. The exclusive lock is never downgraded
       * to a shared lock again.
       *
       * The upgrade object must never outlive the underlying upgradable_access object.
       */
      class upgrade
      {
      public:
         upgrade(upgradable_access & parent)
            : m_parent(parent)
         {
            parent.m_gptr.m_mutex.unlock_upgrade_and_lock();
            parent.m_upgraded = true;
         }

         reference operator*()
         {
            return *(m_parent.m_gptr.m_ptr);
         }
         const_reference operator*() const
         {
            return *(m_parent.m_gptr.m_ptr);
         }
         pointer operator->()
         {
            return m_parent.m_gptr.m_ptr.get();
         }
         const_pointer operator->() const
         {
            return m_parent.m_gptr.m_ptr.get();
         }

      private:
         upgradable_access & m_parent;
      };

   private:
      guarded_ptr & m_gptr;
      bool m_upgraded;
   };

   guarded_ptr(pointer ptr)
      : m_ptr(assert_not_null(ptr)) // do the assert early during construction
   {
   }

private:
   pointer assert_not_null(pointer ptr)
   {
      assert(ptr);
      return ptr;
   }

   boost::scoped_ptr<element_type> const m_ptr;
   boost::shared_mutex mutable m_mutex;
};

} // namespace moost

#endif // ifndef MOOST_GUARDED_PTR_HPP__
