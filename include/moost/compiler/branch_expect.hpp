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

#ifndef MOOST_COMPILER_BRANCH_EXPECT_HPP
#define MOOST_COMPILER_BRANCH_EXPECT_HPP

/**
* Use this to set what an if branch is likely or not to be entered, i.e.
* if ( expect_likely__( someCondition ) )
* {
*    // .. This is likely to happen
* }
* else
* {
*    // .. This is not so likely to happen
* }
*/

#if defined(__GNUC__)

#define expect_likely__(expr)   __builtin_expect((expr),(1==1))
#define expect_unlikely__(expr) __builtin_expect((expr),(1==0))

#else

#define expect_likely__(expr)   (expr)
#define expect_unlikely__(expr) (expr)

#endif

#endif
