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

#ifndef MOOST_TEST_DIRECTORY_CREATOR_H_
#define MOOST_TEST_DIRECTORY_CREATOR_H_

#include <string>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace moost { namespace testing {

   // Creates a directory (if it already exists it removes it then re-creates it
   // to ensure a steril environment) for testing purposes and removes it after.

   class test_directory_creator
   {
   public:
      test_directory_creator(std::string const & path = "Test_Directory_GUID_2E222A01_3D94_4360_968D_8957DD89417D") : path_(path)
      {
         remove_dir(); // just in case it already exists
         boost::filesystem::create_directory(path_);
      }

      virtual ~test_directory_creator()
      {
         remove_dir();
      }

      virtual std::string const & GetPath() const { return path_; }

      virtual std::string GetFilePath(std::string const & sFilename) const
      {
         return (boost::filesystem::path(path_) /= sFilename).string();
      }

      void remove_dir(bool throw_on_error = false) const
      {
         try
         {
            boost::filesystem::remove_all(path_);
         }
         catch(std::exception const & e)
         {
            // if it fails it'll (hopefully) be 'cos it never existed in the first place
            std::cerr << "Failed to remove path " << path_ << ": " << e.what() << "\n";

            if (throw_on_error) throw;
         }
      }

      void create_dir(bool throw_on_error = false) const
      {
         try
         {
            boost::filesystem::create_directory(path_);
         }
         catch(std::exception const & e)
         {
            std::cerr << "Failed to create path " << path_ << ": " << e.what() << "\n";
            if (throw_on_error) throw;
         }
      }

   private:
      std::string path_;
   };

}}

#endif // MOOST_TEST_DIRECTORY_CREATOR_H_
