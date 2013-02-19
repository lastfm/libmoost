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

#ifndef MOOST_UTILS_QUIT_HANDLER_HPP__
#define MOOST_UTILS_QUIT_HANDLER_HPP__

#include <csignal>
#include <cstdlib>

#include <boost/function.hpp>

#include "hup_handler.hpp"

namespace moost { namespace process {

   /// This class represent and very simple way to trap and handle termination
   /// signals. It is not and never was intended to be a fully fledged signal
   /// handling framework. It's only job in life is to allow you to register
   /// a handler (which can be a simple free standing function or a functor),
   /// which will be called if any of the standard (and trapable) signals are
   /// raised. The idea is that you use your custom handler to shutdown your
   /// process gracefully; allowing you to persist data and stop services.
   ///
   /// To use it just call it from an approppriate place in your main thread
   /// before you start any services or actions that might need clearing up
   /// on termination. It is up to you to ensure your custom handler shuts
   /// down your process. The normal action of termination will be disabled.
   ///
   /// By default SIGABRT is also handled but you can disable this as required.
   ///
   /// By default SIGHUP is not handled. You can add this if required but the
   /// preferred method is to use the "hup_handler" and reload the config.
   //
   /// Note, SIGKILL and SIGSTOP are not handled (this is a Posix restriction).

   class quit_handler
   {
   private:
      // A Meyers Singleton (http://bit.ly/EsS4p)
      static quit_handler & get_single_instance()
      {
         static quit_handler quit_handler;
         return quit_handler;
      }

   public:
      typedef boost::function<void ()> handler_t;

      // Set the customer handler.
      static void set(handler_t const & handler, bool trap_abort = true, bool trap_hup = false)
      {
         quit_handler & quit_handler = quit_handler::get_single_instance();
         quit_handler.handler_ = handler;
         quit_handler.register_handler(trap_abort);

         if(trap_hup)
         {
            // Register with hup_handler, as this knows all about SIGHUP
            hup_handler::set(handler);
         }
      }

   private:
      handler_t handler_;

      // This is the proxy handler, which calls the custom handler
      static void sighandler(int)
      {
         quit_handler & quit_handler = quit_handler::get_single_instance();

         if(quit_handler.handler_)
         {
            quit_handler.handler_();
         }
      }

      // Register the proxy handler
      struct sigaction sa_;
      void register_handler(bool trap_abort)
      {
         memset(&sa_, 0, sizeof(sa_));
         sa_.sa_handler = quit_handler::sighandler;

         sigemptyset(&sa_.sa_mask);
         sa_.sa_flags = SIG_BLOCK;

         // These are the signals we'll block whilst the handlers being called
         sigaddset(&sa_.sa_mask, SIGINT);
         sigaddset(&sa_.sa_mask, SIGTERM);
         sigaddset(&sa_.sa_mask, SIGQUIT);

         if (trap_abort) { sigaddset(&sa_.sa_mask, SIGABRT); }

         // These are the signals we'll trap and handle
         sigaction(SIGINT, &sa_, NULL);
         sigaction(SIGTERM, &sa_, NULL);
         sigaction(SIGQUIT, &sa_, NULL);

         if (trap_abort) { sigaction(SIGABRT, &sa_, NULL); }
      }
   };

}}

#endif // MOOST_UTILS_QUIT_HANDLER_HPP__
