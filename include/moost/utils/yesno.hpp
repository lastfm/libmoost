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

#ifndef MOOST_UTILS_YESNO_HPP__
#define MOOST_UTILS_YESNO_HPP__

#include <stdexcept>
#include <string>
#include <iostream>

namespace moost { namespace utils {

class YesNo
{
public:
   YesNo(std::string const & sMsg, bool bImplicitNo = false) : bYes(false)
   {
      std::string s;

      for(;;)
      {
         std::cout << sMsg << " <yes|no> " << std::flush;
         if(!std::getline(std::cin, s))
         {
            throw std::runtime_error("Invalid input stream");
         }

         if(s == "yes")
         {
            bYes = true;
            break;
         }
         else
         if(bImplicitNo || s == "no")
         {
            break;
         }
      }
   }

   bool IsYes() const { return bYes; }
   bool IsNo() const { return !bYes; }

private:
   bool bYes;
};

}}

#endif // MOOST_UTILS_YESNO_HPP__
