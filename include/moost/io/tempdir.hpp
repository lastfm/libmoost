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

#ifndef MOOST_IO_TEMPDIR_HPP
#define MOOST_IO_TEMPDIR_HPP

#include <cstdlib>
#include <string>
#include <vector>
#include <stdexcept>
#include <boost/filesystem.hpp>

/**
 * @brief Creates a unique temporary directory; removed on scope exit.
 *
 * @note
 *
 * Using this is safer than generating a temp filename and then creating
 * the file as this can lead to a race condition situation. By creating
 * a temporary directory (which the OS guarantees will be unique and not
 * used again for as long as it exists) and writing files to this new
 * location you can be sure you won't collide with another process.
 *
 */

namespace moost { namespace io {

   class tempdir : public boost::filesystem::path
   {
      private:
         static std::string mktempdir(std::string const & pattern)
         {
            std::vector<char> buf(pattern.begin(), pattern.end());
            buf.push_back('\0'); // null terminate - thanks sven :)

            char const * path = mkdtemp(&buf[0]);

            if (!path)
            {
               throw std::runtime_error("unable to create tempdir");
            }

            return std::string(path);

         }
      public:
         tempdir(std::string pattern = "lastfm_moost_io_tempdir_XXXXXX")
            : boost::filesystem::path(mktempdir(pattern))
         {
         }

         ~tempdir()
         {
            // wave goodbye to your temporary directory
            boost::filesystem::remove_all(this->string());
         }
   };
}}

#endif
