/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * \file       job_batch.hpp
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

#ifndef MOOST_THREAD_JOB_BATCH_HPP
#define MOOST_THREAD_JOB_BATCH_HPP

#include <boost/function.hpp>

namespace moost { namespace thread {

/**
 * Interface for classes implementing a job batch
 *
 * The abstract interface of a job batch is fairly simple
 * as it's sole purpose is to allow new jobs to be added
 * to the batch.
 */
class job_batch
{
public:
   virtual ~job_batch() {}

   /**
    * Type representing a job to be run
    *
    * A job is just a simple function object. It gets run in
    * in a way determined by the concrete implementation of
    * job_batch.
    */
   typedef boost::function0<void> job_t;

   /**
    * Add a new job to the batch
    *
    * This method adds a job to the concrete implementation.
    * When this job gets run is determined by the implementation.
    * However, new jobs may be added even while jobs from the
    * current batch are being run as long as the batch doesn't
    * run out of jobs. In particular, this means that it's always
    * safe to add new jobs from within jobs of the current batch.
    *
    * \param job             The job to be added.
    */
   virtual void add(job_t job) = 0;
};

}}

#endif
