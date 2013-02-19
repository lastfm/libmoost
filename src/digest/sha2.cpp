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

#include <boost/cast.hpp>
#include <boost/cstdint.hpp>

#include "../../include/moost/digest/sha2.h"

#include "rfc6234/sha.h"

#define SHA2_CLASS_IMPL_(bits)                                        \
   class sha ## bits ## _impl                                         \
   {                                                                  \
   public:                                                            \
      SHA ## bits ## Context context;                                 \
   };                                                                 \
                                                                      \
   sha ## bits::sha ## bits()                                         \
      : m_impl(new sha ## bits ## _impl)                              \
   {                                                                  \
      reset();                                                        \
   }                                                                  \
                                                                      \
   void sha ## bits::reset()                                          \
   {                                                                  \
      SHA ## bits ## Reset(&m_impl->context);                         \
   }                                                                  \
                                                                      \
   void sha ## bits::add_raw(const void *data, size_t size)           \
   {                                                                  \
      SHA ## bits ## Input(&m_impl->context,                          \
                  reinterpret_cast<const boost::uint8_t *>(data),     \
                  boost::numeric_cast<unsigned int>(size));           \
   }                                                                  \
                                                                      \
   std::string sha ## bits::digest() const                            \
   {                                                                  \
      boost::uint8_t digest[SHA ## bits ## HashSize];                 \
      SHA ## bits ## Result(&m_impl->context, digest);                \
      return std::string(reinterpret_cast<const char *>(&digest[0]),  \
                         sizeof(digest));                             \
   }

namespace moost { namespace digest {

SHA2_CLASS_IMPL_(224)
SHA2_CLASS_IMPL_(256)
SHA2_CLASS_IMPL_(384)
SHA2_CLASS_IMPL_(512)

}}
