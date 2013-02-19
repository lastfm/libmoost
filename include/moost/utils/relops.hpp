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
 * @file relops.hpp
 * @brief Simplify implementing member relationship operators
 * @author Ricky Cormier
 * @version See version.h (N/A if it doesn't exist)
 * @date 2012-06-15
 */

#ifndef MOOST_UTILS_RELOPS_HPP__
#define MOOST_UTILS_RELOPS_HPP__

namespace moost { namespace utils {

   /**
    * @brief Sub-class this to implement relops in your class
    *
    * @tparam T The type being compared
    *
    * @note Each operator takes its rhs as a type U, meaning that as long
    *       as T has implemented a == and < for type U it will be able to
    *       handle all the others (!=, >, <= & >=) for free
    *
    *       This framework isn't meant as a replacement for the Boost operators
    *       library; rather, it serves the single job of making the addition
    *       of relationship operators to a class as trivial as possible. Your
    *       class just exposes '==' and '<' and then sub-classs relops<T>. It
    *       will then get support for all relationship operators, both ways.
    *
    *       ie. X cmp Y and Y cmp X (where cmp is a relationship operator)
    *
    */
   template<typename T>
      class relops
      {
         protected:
            relops() {}

         public:
            /**
             * @brief Equality operator
             *
             * @param rhs The right hand side value
             *
             * @return true if lhs == rhs
             *
             * @note You need to implement this
             */

            template <typename U>
            bool operator == (U const & rhs) const
            {
               // I am really a type T
               return static_cast<T const *>(this)->operator == (rhs);
            }

            template <typename U>
            friend bool operator == (U const & lhs, T const & rhs)
            {
               return rhs.operator == (lhs);
            }

            /**
             * @brief Less-than operator
             *
             * @param rhs The right hand side value
             *
             * @return true if lhs < rhs
             *
             * @note You need to implement this
             */

            template <typename U>
            bool operator < (U const & rhs) const
            {
               // I am really a type T
               return static_cast<T const *>(this)->operator < (rhs);
            }

            template <typename U>
            friend bool operator < (U const & lhs, T const & rhs)
            {
               return rhs.operator > (lhs);
            }

            /**
             * @brief In-equality operator
             *
             * @param rhs The right hand side value
             *
             * @return lhs != rhs
             *
             * @note You get this for free
             *
             */

            template <typename U>
            bool operator != (U const & rhs) const
            {
               return !(this->operator == (rhs));
            }

            template <typename U>
            friend bool operator != (U const & lhs, T const & rhs)
            {
               return rhs.operator != (lhs);
            }

            /**
             * @brief Greater-than operator
             *
             * @param rhs The right hand side value
             *
             * @return lhs > rhs
             *
             * @note You get this for free
             *
             */

            template <typename U>
            bool operator > (U const & rhs) const
            {
               return !(this->operator == (rhs) || this->operator < (rhs));
            }

            template <typename U>
            friend bool operator > (U const & lhs, T const & rhs)
            {
               return rhs.operator < (lhs);
            }

            /**
             * @brief Less-than or equal
             *
             * @param rhs The right hand side value
             *
             * @return lhs <= rhs
             *
             * @note You get this for free
             *
             */

            template <typename U>
            bool operator <= (U const & rhs) const
            {
               return !(this->operator > (rhs));
            }

            template <typename U>
            friend bool operator <= (U const & lhs, T const & rhs)
            {
               return rhs.operator >= (lhs);
            }

            /**
             * @brief Greater-than or equal operator
             *
             * @param rhs The right hand side value
             *
             * @return lhs >= rhs
             *
             * @note You get this for free
             *
             */

            template <typename U>
            bool operator >= (U const & rhs) const
            {
               return !(this->operator < (rhs));
            }

            template <typename U>
            friend bool operator >= (U const & lhs, T const & rhs)
            {
               return rhs.operator <= (lhs);
            }
      };

}}

#endif
