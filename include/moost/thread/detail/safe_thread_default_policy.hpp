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

#ifndef MOOST_THREAD_SAFE_THREAD_GROUP_DEFAULT_POLICY
#define MOOST_THREAD_SAFE_THREAD_GROUP_DEFAULT_POLICY

#include <stdexcept>

#include "../../logging.hpp"

namespace moost { namespace thread {

   struct default_safe_thread_policy
   {
      template<typename F>
      void operator()(F & f) const
      {
         try
         {
            f();
         }
         catch(boost::thread_interrupted const &)
         {
            throw; // rethrow this as it means the thread was intentionally interupted
         }
         catch(std::exception const & e)
         {
            // Log and ignore (the thread will be terminated but the process lives on!)
            MLOG_NAMED_ERROR("Unhandled Thread Exception", e.what());
         }
         catch(...)
         {
            // Log and ignore (the thread will be terminated but the process lives on!)
            MLOG_NAMED_ERROR("Unhandled Thread Exception", "Unknown Error");
         }
      }
   };
}}

#endif
