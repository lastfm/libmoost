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

#ifndef MOOST_UTILS_DEMANGLER_HPP__
#define MOOST_UTILS_DEMANGLER_HPP__

#include <cstring>
#include <string>

/*!
*
* /file Demangle symbol names
*
* The gcc compiler doesn't produce very pretty output from type_info (Windows does an ok job)
* and since we often use the type_info class names when logging this is needed to pretty it up.
*
* http://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
*
*/

#if (!defined(NDEBUG) || defined(DEMANGLE_NAMES)) && defined(__GNUG__)
#define DEMANGLE_NAMES___
#endif

#ifdef DEMANGLE_NAMES___
#include <cxxabi.h>
#include "scope_exit.hpp"
#endif

namespace moost { namespace utils {

   inline
      std::string demangle_name(char const * name)
   {
#ifdef DEMANGLE_NAMES___
      int status = -1;
      moost::utils::scope_exit::type<char>::free_malloc res(
         abi::__cxa_demangle(name, NULL, NULL, &status)
         );
      return (status==0)?res->get():name;
#else
      return name;
#endif
   }
}}

#ifdef DEMANGLE_NAMES___

#define DEMANGLE_NAME(name) \
   moost::utils::demangle_name(name)

#else

#define DEMANGLE_NAME(name) name

#endif

/*!
*
* /file Convert full function names to short function names
*
* This function will convert the full function name (and all its adornments) into something a little less
* verbose. It will strip off all calling and template arguments and the return type and leave behind just the
* name of the function and, if it is a class member function, the class to which it belongs. Note the name of the
* class in the case of a class member function is not shortened, so if it contains a vast number of template
* parameters it may still be long.
*
* http://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
*
*/

namespace moost { namespace utils {
   inline
      std::string short_function_name(char const * name)
   {
      // [31/3/2011 ricky] I'm using nasty C string functions here to delay as much as I can the assignment to string
      //                   I could just assign to the string then use standard string::find and string::substr/resize
      //                   but this is a sub-optimal approach as it will involve more copying and memory usage. This
      //                   may not be pretty but it should be quicker.

      // Find the functions opening parameter
      char const * end = strchr(name, '(');
      char const * start = end;
      bool shorterned = false;

      // Find the start of the function name

      if(end)
      {
         size_t bcnt = 0;
         bool start_parse = false;
         bool end_tplate = false;

         // we now need to parse back from where we think the function name
         // starts to find where it actually starts and where it actually ends.
         while(!shorterned && --start!=name)
         {
            // first of all, there could be space that we should just skip
            if(!start_parse)
            {
               // so far we've only seen white space so we're gonna skip it
               start_parse = 0 == isspace(*start);
            }

            if(start_parse)
            {
               // all white space skipped, now let's try and find the start and end of this function name
               switch(*start)
               {
               case '>':
                  // we seem to be in a template so we'll skil until it's closed.
                  ++ bcnt;
                  break;
               case '<':
                  // when bcnt == 0 we're at the end of all the function template stuff
                  if(0 == --bcnt && !end_tplate)
                  {
                     end = start; // this is the real end of the function name
                     end_tplate = true; // any more templates will be part of the class name (if this is a member function)
                  }
                  break;
               default:
                  // have we skipped over any template params and white space we don't care about?
                  if(isspace(*start) != 0&& 0 == bcnt)
                  {
                     // We have, job done -- we have a candidate:)
                     shorterned = true;
                  }
                  break;
               }
            }
         }

         // just make sure we didn't get here and failed to find a start and end (if start == name we failed).
         shorterned = (start!=name);
      }

      // If we found the short name return it otherwise return the original
      return shorterned ? std::string(start+1, end - start - 1) : std::string(name); // hopefully NRVO will optimise this return by value
   }

}}

#endif // MOOST_UTILS_DEMANGLER_HPP__
