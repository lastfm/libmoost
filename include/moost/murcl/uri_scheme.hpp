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
* @file uri_scheme.hpp
* @brief Represents a Uri scheme
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*
*/

#ifndef FM_LAST_MOOST_MURCL_URI_SCHEME_HPP__
#define FM_LAST_MOOST_MURCL_URI_SCHEME_HPP__

#include <string>

namespace moost { namespace murcl {

/**
* @brief This class is a base class for all supported uri schemes
*/
class uri_scheme
{
public:
   /**
   * @brief A enum of supported schemes and their associated default ports
   */
   struct port
   {
      enum {
         HTTP = 80
         // add more as required
      };
   };

   /**
   * @brief Gets a string representation of a valid uri scheme
   *
   * @return
   *     string representation of the scheme
   */
   std::string const & get() const
   {
      return scheme_;
   }

   /**
   * @brief Get the port number associated with this scheme
   *
   * @return
   *     The port as an integer value
   */
   int get_port() const
   {
      return port_;
   }

protected:
   uri_scheme(
      std::string const & scheme,
      int port
      )
      : scheme_(scheme)
      , port_(port)
   {
   }

private:
   std::string scheme_;
   int port_;
};

struct http_scheme : uri_scheme
{
   /**
   * @brief Construct a scheme representing HTTP
   *
   * @param port
   *     The port associated with this scheme, defaults to scheme standard
   */
   http_scheme(int port = uri_scheme::port::HTTP)
      : uri_scheme("http://", port)
   {
   }
};

}}


#endif // FM_LAST_MOOST_MURCL_URI_HPP__
