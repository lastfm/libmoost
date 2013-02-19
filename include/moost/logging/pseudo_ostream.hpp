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
 * @file pseudo_ostream.hpp
 * @brief A pseudo ostream for getting logging status/debug
 * @author Ricky Cormier
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-02-14
 */
#ifndef MOOST_LOGGING_PSEUDO_OSTREAM_HPP__
#define MOOST_LOGGING_PSEUDO_OSTREAM_HPP__

#include <boost/thread.hpp>

namespace moost { namespace logging {

/**
 * @brief A pseudo stream acts as an abstraction to a normal ostream
 *
 * Since a logger can't log its own failures (at least it's probably not the
 * best thing to try) the logging framework can be assigned an ostream that
 * will be used to log any status/error messages. This ostream will be wrapped
 * by the pseudo ostream class, which will provide thread synchronisation when
 * writing. Also, if no ostream is provided it will silently ignore writes.
 *
*/
class pseudo_ostream
{
   public:
      pseudo_ostream() : pout_(0) {}

      /**
       * @brief Assign a (new) ostream
       *
       * @param out : a reference to an output stream
       */
      void attach(std::ostream & out)
      {
         boost::lock_guard<boost::mutex> lock(mtx_);
         pout_ = &out;
      }

      /**
       * @brief Detach current ostream
       */
      void detach()
      {
         pout_ = 0;
      }

      /**
       * @brief Allow streaming of any type
       *
       * @tparam T : the type to be streamed
       * @param t  : the object to be streamed
       *
       * @return A reference to *this
       */
      template <typename T>
      pseudo_ostream & operator << (T const & t)
      {
         boost::lock_guard<boost::mutex> lock(mtx_);
         if(pout_)
         {
            *pout_ << t;
         }

         return *this;
      }

      /**
       * @brief IO manipulators need special handling when streaming
       *
       * @param iomanip : An IO manipulator
       *
       * @return A reference to *this
       */
      typedef std::ostream & (*iomanip_t)(std::ostream &);
      pseudo_ostream & operator << (iomanip_t const & iomanip)
      {
         boost::lock_guard<boost::mutex> lock(mtx_);
         if(pout_)
         {
            iomanip(*pout_);
         }

         return *this;
      }

      /**
       * @brief A quick way to check stream is still ok
       *
       * @return null if bad/fail else !null
       */
      operator void *() const
      {
         boost::lock_guard<boost::mutex> lock(mtx_);
         return pout_ ? *pout_ : static_cast<void *>(0);
      }

   private:
      std::ostream * pout_;
      mutable boost::mutex mtx_;

};

}}

#endif
