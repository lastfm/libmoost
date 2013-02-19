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
* @file uri_elements.hpp
* @brief Represents the elements of a uri encoded according to RFC-2396
* @author Ricky Cormier
* @version 0.0.0.1
* @date 2011-10-17
*
*/

#ifndef FM_LAST_MOOST_MURCL_URI_ELEMENTS_HPP__
#define FM_LAST_MOOST_MURCL_URI_ELEMENTS_HPP__

#include <string>
#include <sstream>
#include <map>

#include <curl/curl.h>

#include <boost/lexical_cast.hpp>

#include "uri_params.hpp"
#include "uri_scheme.hpp"
#include "uri_encoder.hpp"

namespace moost { namespace murcl {

/**
* @brief This class represents the basic elements that make up a standard uri
*/
class uri_elements
{
public:
   /**
   * @brief construct a uri elements object
   *
   * @param scheme
   *     the uri scheme
   * @param host
   *     the uri host
   */
   uri_elements(
      uri_scheme const & scheme,
      std::string const & host
    )
      : scheme_(scheme.get())
      , host_(host)
      , port_(boost::lexical_cast<std::string>(scheme.get_port()))
   {
      encode();
   }

   /**
   * @brief construct a uri elements object
   *
   * @param scheme
   *     the uri scheme
   * @param host
   *     the uri host
   * @param path
   *     the uri path
   */
  uri_elements(
      uri_scheme const & scheme,
      std::string const & host,
      std::string const & path
    )
      : scheme_(scheme.get())
      , host_(host)
      , port_(boost::lexical_cast<std::string>(scheme.get_port()))
      , path_(path)
   {
      encode();
   }

   /**
   * @brief construct a uri elements object
   *
   * @param scheme
   *     the uri scheme
   * @param host
   *     the uri host
   * @param path
   *     the uri path
   * @param params
   *     the uri parameters
   */
   uri_elements(
      uri_scheme const & scheme,
      std::string const & host,
      std::string const & path,
      uri_params const & params
    )
      : scheme_(scheme.get())
      , host_(host)
      , port_(boost::lexical_cast<std::string>(scheme.get_port()))
      , path_(path)
      , params_(params)
   {
      encode();
   }

   /**
   * @brief construct a uri elements object
   *
   * @param scheme
   *     the uri scheme
   * @param host
   *     the uri host
   * @param path
   *     the uri path
   * @param map
   *     a map of key/value strings to be converted to url parameters
   */
   uri_elements(
      uri_scheme const & scheme,
      std::string const & host,
      std::string const & path,
      uri_params::map_t const & map
    )
      : scheme_(scheme.get())
      , host_(host)
      , port_(boost::lexical_cast<std::string>(scheme.get_port()))
      , path_(path)
      , params_(uri_params(map))
   {
      encode();
   }

   /**
   * @brief Get the scheme associated with this uri
   *
   * @return
   *     a string representing the scheme
   */
   std::string const & get_scheme() const
   {
      return scheme_;
   }

   /**
   * @brief Get the host associated with this uri
   *
   * @return
   *     a string representing the host uri, encoded according to RFC-2396
   */
   std::string const & get_host() const
   {
      return host_;
   }

   /**
   * @brief Get the port associated with this uri
   *
   * @return
   *     a string representing the path
   */
   std::string const & get_port() const
   {
      return port_;
   }

   /**
   * @brief Get the path associated with this uri
   *
   * @return
   *     a string representing the path uri, encoded according to RFC-2396
   */
   std::string const & get_path() const
   {
      return path_;
   }

   /**
   * @brief Get the params associated with this uri
   *
   * @return
   *     a string representing the uri params, encoded according to RFC-2396
   */
   std::string const & get_params() const
   {
      return params_;
   }

private:
   // uri encode stuff
   void encode()
   {
      // scheme should not need to be encoded
      uri_encoder::encode(host_);
      uri_encoder::encode(path_);
      // params don't need encoding as the uri_params class takes care of that
   }

   std::string const scheme_;
   std::string const host_;
   std::string const port_;
   std::string const path_;
   std::string const params_;
 };

}}


#endif // FM_LAST_MOOST_MURCL_URI_ELEMENTS_HPP__
