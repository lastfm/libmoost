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

#ifndef MOOST_SHELL_H__
#define MOOST_SHELL_H__

#include "service/remote_shell.h"
#include "service/appender.h"

namespace moost {

// A thin, backwards-compatible wrapper to use the "local" part
// of moost::service::remote_shell in an easy way.

template <class HandlerType>
class shell : public service::remote_shell<HandlerType>
{
public:
   shell(HandlerType& handler)
      : service::remote_shell<HandlerType>(handler)
   {
      this->enable_local_shell();
   }

   shell(HandlerType& handler, const std::string& log_level)
      : service::remote_shell<HandlerType>(handler)
   {
      this->enable_local_shell();
      this->set_appender_factory(moost::service::appender_factory_ptr(
         new service::log4cxx_appender_factory(log_level)
      ));
   }
};

}

#endif /* MOOST_SHELL_H */
