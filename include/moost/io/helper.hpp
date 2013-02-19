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

#ifndef MOOST_IO_HELPER_HPP__
#define MOOST_IO_HELPER_HPP__

/**
 * \file helper.hpp
 *
 * The moost::io::helper class provides a collection of static methods that implement
 * certain i/o related functionality in a platform independent way. Examples are
 * duplicating or closing a file handle or creating pipes.
 */

#include <boost/asio.hpp>

#if defined(BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE) && defined(_WIN32)
# include "detail/helper_win32.hpp"
#elif defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR) && (defined(_POSIX_SOURCE) || defined(__CYGWIN__))
# include "detail/helper_posix.hpp"
#else
# error "apparently no i/o helper support has been added for this platform"
#endif

namespace moost { namespace io {

typedef detail::helper helper;

} }

#endif
