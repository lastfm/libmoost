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

#include <stdexcept>
#include <pthread.h>
#include "../../include/moost/signal/signal_handler.h"

using namespace moost::signal;

class signal_handler::impl
{
private:

  sigset_t       m_signal_mask;     // signals to block
  pthread_t      m_thread;          // our pthread
  pthread_attr_t m_thread_attr;     // thread attributes

  boost::function<void(int)> m_callback;  // our callback

  void wait_loop();

  static void * t_entry(void * p)
  {
    signal_handler::impl * psig_handler = static_cast<signal_handler::impl *>(p);
    psig_handler->wait_loop();
    return NULL;
  }

public:

  impl(const boost::function<void(int)> & callback);

  ~impl();
};

signal_handler::signal_handler(const boost::function<void(int)> & callback)
: m_pimpl(new impl(callback))
{
}

signal_handler::impl::impl(const boost::function<void(int)> & callback)
: m_callback(callback)
{
  sigemptyset(&m_signal_mask);
  sigaddset(&m_signal_mask, SIGINT);
  sigaddset(&m_signal_mask, SIGHUP);
  sigaddset(&m_signal_mask, SIGTERM);
  // we use SIGUSR1 for our own devices:
  // sighandler thread wakes up when receiving it and knows to exit
  sigaddset(&m_signal_mask, SIGUSR1);

  // inform pthreads that this thread (and any thread spawning from it)
  // should intercept the signals masked into m_signal_mask
  if (pthread_sigmask(SIG_BLOCK, &m_signal_mask, NULL) != 0)
    throw std::runtime_error("failed to set sigmask");

  if (pthread_attr_init(&m_thread_attr) != 0)
    throw std::runtime_error("failed to set thread attr");

  if (pthread_create(&m_thread, &m_thread_attr, t_entry, this) != 0)
    throw std::runtime_error("failed to create thread");
}

signal_handler::impl::~impl()
{
  // wake thread up
  pthread_kill(m_thread, SIGUSR1);
  pthread_join(m_thread, NULL);
  // revert to not handling the aforementioned signals
  pthread_sigmask(SIG_UNBLOCK, &m_signal_mask, NULL);
}

void signal_handler::impl::wait_loop()
{
  int      sig_caught;   // caught signal

  for (;;)
  {
    if (sigwait(&m_signal_mask, &sig_caught) != 0)
      m_callback(-1); // error catching signal
    else if (sig_caught == SIGUSR1)
    {
      return;
    }
    else
      m_callback(sig_caught);
  }
}
