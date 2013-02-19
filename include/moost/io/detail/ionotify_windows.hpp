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

#ifndef MOOST_IO_NOTIFY_WINDOWS_HPP__
#define MOOST_IO_NOTIFY_WINDOWS_HPP__

#include "../file_watcher.hpp"

namespace moost { namespace io {

   // For now this is implimeneted using file_watcher as a windows version is a bit more work :(
   class ionotify
   {
   public:
      ionotify(bool ignore_dup_events = false, bool stop_immediately = false) : fw_(50)
      {
      }

      ~ionotify()
      {
      }

      typedef file_watcher::file_action file_action;
      typedef file_watcher::callback_t callback_t;
      file_action static const CHANGED = file_watcher::CHANGED;
      file_action static const CREATED = file_watcher::CREATED;
      file_action static const DELETED = file_watcher::DELETED;

   public:
      void insert(std::string const & path, callback_t const & callback, bool call_now = false)
      {
         fw_.insert(path, callback, call_now);
      }

      void erase(std::string const & path)
      {
         fw_.erase(path);
      }

      void start()
      {
         fw_.start();
      }

      void stop()
      {
         fw_.stop();
      }

   private:
      file_watcher fw_;
   };


}} // moost::io

#endif // MOOST_IO_NOTIFY_WINDOWS_HPP__
