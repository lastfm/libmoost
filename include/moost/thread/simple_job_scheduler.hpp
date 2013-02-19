/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       simple_job_scheduler.hpp
 * \author     Marcus Holland-Moritz (marcus@last.fm)
 * \copyright  Copyright © 2008-2013 Last.fm Limited
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

#ifndef MOOST_THREAD_SIMPLE_JOB_SCHEDULER_HPP
#define MOOST_THREAD_SIMPLE_JOB_SCHEDULER_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#include "job_batch.hpp"

namespace moost { namespace thread {

class simple_job_batch : public job_batch
                       , public boost::noncopyable
{
public:
   simple_job_batch()
      : m_count(0)
   {
   }

   virtual void add(job_t job)
   {
      try
      {
         job();  // JFDI! :)
      }
      catch (const std::exception& e)
      {
         m_errors.push_back(e.what());
      }
      catch (...)
      {
         m_errors.push_back("unknown exception caught");
      }

      ++m_count;
   }

   void run()
   {
   }

   size_t count() const
   {
      return m_count;
   }

   const std::vector<std::string>& errors() const
   {
      return m_errors;
   }

private:
   size_t m_count;
   std::vector<std::string> m_errors;
};

class simple_job_scheduler : public boost::noncopyable
{
public:
   typedef simple_job_batch job_batch_type;

   void dispatch(boost::shared_ptr<job_batch_type>)
   {
   }
};

}}

#endif
