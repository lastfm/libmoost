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

#ifndef MOOST_THREAD_TOKEN_MUTEX_H__
#define MOOST_THREAD_TOKEN_MUTEX_H__

#include <set>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

namespace moost { namespace thread {

/** \brief token_mutex allows you to enter a critical section of code if you have a token that no one else does
 *
 * This allows super-granular access to thread-safe regions
 */
template<typename T>
class token_mutex
{
private:

   boost::mutex m_mutex;
   boost::condition m_cond;
   std::set<T> m_tokens;
   bool m_full_lock;

public:
   class scoped_promote_lock;

  /**
   * \brief locks a token mutex with a given token
   */
   class scoped_lock
   {
   private:
      token_mutex<T>& m_token_mutex;
      T m_token;

      friend class scoped_promote_lock;
   public:
      scoped_lock( token_mutex<T> & token_mutex,
                   const T & token )
         : m_token_mutex(token_mutex),
         m_token(token)
      {
         m_token_mutex.lock(m_token);
      }
      ~scoped_lock()
      {
         m_token_mutex.unlock(m_token);
      }
   };

  /**
   * \brief locks a token mutex with a given token
   */
  class scoped_try_lock
  {
  private:
     token_mutex<T>& m_token_mutex;
     T m_token;
     bool m_got_lock;
  public:
     scoped_try_lock( token_mutex<T> & token_mutex,
                      const T & token )
        : m_token_mutex(token_mutex),
        m_token(token)
     {
        m_got_lock = m_token_mutex.trylock(m_token);
     }
     operator bool () const { return m_got_lock; }
     bool operator ! () const { return !m_got_lock; }
     ~scoped_try_lock()
     {
        if (m_got_lock)
           m_token_mutex.unlock(m_token);
     }
  };

  /**
  * \brief locks everybody but first wait that beding are done
  */
  class scoped_full_lock
  {
  private:
     token_mutex<T>& m_token_mutex;
  public:
     scoped_full_lock(token_mutex<T> & token_mutex)
        : m_token_mutex(token_mutex)
     {
        m_token_mutex.full_lock();
     }
     ~scoped_full_lock()
     {
        m_token_mutex.full_unlock();
     }
  };

    /**
  * \brief locks everybody but first wait that beding are done
  */
  class scoped_promote_lock
  {
  private:
     token_mutex<T>& m_token_mutex;
  public:
     scoped_promote_lock(scoped_lock& lock)
        : m_token_mutex(lock.m_token_mutex)
     {
        m_token_mutex.unlock(lock.m_token);
        m_token_mutex.full_lock();
     }

     ~scoped_promote_lock()
     {
        m_token_mutex.full_unlock();
     }
  };

  /**
  * \brief constructor, also make sure m_full_lock is set to false
  */
  token_mutex() : m_full_lock(false) {}

  /**
  * \brief lock on a given token
  */
  void lock(const T& token)
  {
     boost::mutex::scoped_lock lock(m_mutex);
     while (m_tokens.find(token) != m_tokens.end() || m_full_lock)
        m_cond.wait(lock);
     m_tokens.insert(token);
  }

  /**
  * \brief unlock on a given token
  */
  void unlock(const T& token)
  {
     boost::mutex::scoped_lock lock(m_mutex);
     m_tokens.erase(token);
     m_cond.notify_all();
  }

  /**
  * \brief lock for everybody (but first wait until pending are done)
  */
  void full_lock()
  {
     boost::mutex::scoped_lock lock(m_mutex);
     m_full_lock = true;
     while (!m_tokens.empty())
        m_cond.wait(lock);
  }

  /**
  * \brief unlock the full_lock
  */
  void full_unlock()
  {
     boost::mutex::scoped_lock lock(m_mutex);
     m_full_lock = false;
     m_cond.notify_all();
  }

  /**
  * \brief try to lock on a given token
  *
  * returns true if the lock was successful,
  *         immediately returns false if the lock is already held for that token
  */
  bool trylock(const T& token)
  {
     boost::mutex::scoped_lock lock(m_mutex);
     if (m_tokens.find(token) == m_tokens.end() && !m_full_lock)
     {
        m_tokens.insert(token);
        return true;
     }
     return false;
  }
};

}} // moost::thread

#endif // MOOST_THREAD_TOKEN_MUTEX_H__
