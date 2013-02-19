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

#ifndef MOOST_UTILS_HUP_HANDLER_HPP__
#define MOOST_UTILS_HUP_HANDLER_HPP__

#include <csignal>
#include <cstdlib>

#include <boost/function.hpp>

namespace moost { namespace process {

   /// This class represent and very simple way to trap and handle hang-up
   /// signals. It is not and never was intended to be a fully fledged signal
   /// handling framework. It's only job in life is to allow you to register
   /// a handler (which can be a simple free standing function or a functor),
   /// which will be called if the posix standard SIGHUP signal is raised.
   ///
   /// The default behaviour that a process should implement is to reload
   /// the configuration file and then use any changes found.
   ///
   /// To use it just call it from an appropirate place in your main thread
   /// before you start any services or actions that might need clearing up
   /// on termination. It is up to you to ensure your custom handler shuts
   /// down your process. The normal action of termination will be disabled.

   class hup_handler
   {
   private:
      // A Meyers Singleton (http://bit.ly/EsS4p)
      static hup_handler & get_single_instance()
      {
         static hup_handler hup_handler;
         return hup_handler;
      }

   public:
      typedef boost::function<void ()> handler_t;

      // Set the customer handler.
      static void set(handler_t const & handler)
      {
         hup_handler & hup_handler = hup_handler::get_single_instance();
         hup_handler.handler_ = handler;
         hup_handler.register_handler();
      }

   private:
      handler_t handler_;

      // This is the proxy handler, which calls the custom handler
      static void sighandler(int)
      {
         hup_handler & hup_handler = hup_handler::get_single_instance();

         if(hup_handler.handler_)
         {
            hup_handler.handler_();
         }
      }

      // Register the proxy handler
      struct sigaction sa_;
      void register_handler()
      {
         memset(&sa_, 0, sizeof(sa_));
         sa_.sa_handler = hup_handler::sighandler;

         // These are the signals we'll block whilst the handlers being called
         sigemptyset(&sa_.sa_mask);
         sa_.sa_flags = SIG_BLOCK;
         sigaddset(&sa_.sa_mask, SIGHUP);

         // These are the signals we'll trap and handle
         sigaction(SIGHUP, &sa_, NULL);
      }
   };

}}

#endif // MOOST_UTILS_HUP_HANDLER_HPP__
