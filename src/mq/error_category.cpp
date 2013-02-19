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

#include <boost/lexical_cast.hpp>

#include "error_category.h"

namespace moost { namespace mq {

const stomp_client_error_category g_stomp_queue_error_category;

// required default ctor, as we're declaring a const global object
stomp_client_error_category::stomp_client_error_category()
{
}

const char *stomp_client_error_category::name() const
{
   return "moost.mq.error";
}

boost::system::error_condition stomp_client_error_category::default_error_condition(int ev) const
{
   return boost::system::error_condition(ev, g_stomp_queue_error_category);
}

std::string stomp_client_error_category::message(int ev) const
{
   switch (ev)
   {
      case error::success:
         return "success";

      case error::queue_error:
         return "queue error";

      case error::subscribe_failed:
         return "subscribe failed";

      case error::connect_cmd_failed:
         return "connect command failed";

      case error::connection_lost:
         return "connection lost";

      default:
         return "unknown error: " + boost::lexical_cast<std::string>(ev);
   }
}

const boost::system::error_category& mq_error_category()
{
   return g_stomp_queue_error_category;
}

}}
