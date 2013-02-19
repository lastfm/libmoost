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
 * @file nsca_appender.hpp
 * @brief
 * @author Ricky Cormier
 * @date 2012-02-14
 */

#ifndef MOOST_LOGGING_NSCA_APPENDER_HPP__
#define MOOST_LOGGING_NSCA_APPENDER_HPP__

#include <string>

#include <boost/shared_ptr.hpp>

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/patternlayout.h>

#include "../nagios/detail/nsca_config.hpp"

namespace moost {
   namespace nagios {
      class nsca_client;
   }

   namespace logging {
      class pseudo_ostream;
   }

}

/**
 * @brief The appender is defined in the log4cxx namespace
 *
 * We have to define the appender in the log4cxx namespace because the
 * magic macros that add the special sauce to make this a fully qualified
 * appender assume this is the case.
 *
 */
namespace log4cxx
{
   namespace helpers {
      class Pool;
   }

   /**
    * @brief An appender that sends alerts to nagios via an nsca client
    */
   class NscaAppender : public AppenderSkeleton
   {
   public:
      DECLARE_LOG4CXX_OBJECT(NscaAppender)
      BEGIN_LOG4CXX_CAST_MAP()
         LOG4CXX_CAST_ENTRY(NscaAppender)
         LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
      END_LOG4CXX_CAST_MAP()

      NscaAppender();

      /**
       * @brief Construct an appender
       *
       * @param this_host : host name of the client
       * @param cfg : configuration structure
       */
      NscaAppender(
         std::string const & this_host,
         moost::nagios::nsca_config const & cfg
         );

      /**
       * @brief Construct an appender
       *
       * @param this_host : host name of the client
       * @param nsca_svr_host : host name of the nsca server
       */
      NscaAppender(
         std::string const & this_host,
         std::string const & nsca_svr_host
         );

       /**
       * @brief Construct an appender
       *
       * @param this_host : host name of the client
       * @param this_host_desc : short descript of the client
       * @param cfg : configuration structure
       */
      NscaAppender(
         std::string const & this_host,
         std::string const & this_host_desc,
         moost::nagios::nsca_config const & cfg
         );

       /**
       * @brief Construct an appender
       *
       * @param this_host : host name of the client
       * @param this_host_desc : short descript of the client
       * @param nsca_svr_host : host name of the nsca server
       */
      NscaAppender(
         std::string const & this_host,
         std::string const & this_host_desc,
         std::string const & nsca_svr_host
         );

       /**
       * @brief Construct an appender
       *
       * @param layout : A pointer to a layout pattern
       */
      NscaAppender(LayoutPtr & layout);

       /**
       * @brief Construct an appender
       *
       * @param layout : A pointer to a layout pattern
       * @param this_host : host name of the client
       * @param cfg : configuration structure
       */
      NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         moost::nagios::nsca_config const & cfg
         );

       /**
       * @brief Construct an appender
       *
       * @param layout : A pointer to a layout pattern
       * @param this_host : host name of the client
       * @param nsca_svr_host : host name of the nsca server
       */
      NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         std::string const & nsca_svr_host
         );

       /**
       * @brief Construct an appender
       *
       * @param layout : A pointer to a layout pattern
       * @param this_host : host name of the client
       * @param this_host_desc : short descript of the client
       * @param cfg : configuration structure
       */
      NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         std::string const & this_host_desc,
         moost::nagios::nsca_config const & cfg
         );

       /**
       * @brief Construct an appender
       *
       * @param layout : A pointer to a layout pattern
       * @param this_host : host name of the client
       * @param this_host_desc : short descript of the client
       * @param nsca_svr_host : host name of the nsca server
       */
      NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         std::string const & this_host_desc,
         std::string const & nsca_svr_host
         );

      /**
       * @brief Set the nsca server host name
       *
       * @param host : the hostname to use
       */
      void setNscaHost(std::string const & host);

      /**
       * @brief Set the nsca server port
       *
       * @param port : the port to use
       */
      void setNscaPort(std::string const & port);

      /**
       * @brief Set the socket receive timeout in ms
       *
       * @param timeout : the timeout to use
       */
      void setRecvTimeoutMs(boost::uint16_t timeout);

      /**
       * @brief Set the socket send timeout in ms
       *
       * @param timeout : the timeout to use
       */
      void setSendTimeoutMs(boost::uint16_t timeout);

      /**
       * @brief Set the encryption type
       *
       * @param enctype : the encryption type to use
       */
      void setEncType(std::string const & enctype);

      /**
       * @brief Set the encryption password if needed
       *
       * @param encpass : the password to use
       */
      void setEncPass(std::string const & encpass);

      /**
       * @brief Set the host name of the client
       *
       * @param host : the hostname to use
       */
      void setThisHost(std::string const & host);

      /**
       * @brief Set a description for the client
       *
       * @param desc : the clients description
       */
      void setThisHostDesc(std::string const & desc);

      /**
       * @brief Set an option value by name
       *
       * This is used by the log4cxx framework to allow the dynamic setting
       * of options from a logging configuration file.
       *
       * @param option : the option name
       * @param value  : the value to set the option to
       */
      void setOption(const LogString& option, const LogString& value);

      /**
       * @brief Returns a bool to indicate if a layout is required
       *
       * The nsca appender will use the standard basic layout if none is
       * provided. This should be good enough for nearly all occasions.
       *
       * @return : always false since a layout is not actually required
       */
      bool requiresLayout() const;

      /**
       * @brief Activates options once they have been set
       *
       * Once options have been configured, calling this activates them.
       *
       * @note This method MUST be called after changing any options to
       *       activate them. The reason for this is because it would be
       *       too expensive to re-activate them on the fly as they are
       *       modified.
       */
      void activateOptions();

      /**
       * @brief Activates options set via the logging config file
       *
       * This method is automatically called by the log4cxx framework when
       * assigning settings from a logging configuration file.
       *
       * @param p : this is an internal parameter to log4cxx
       */
      void activateOptions(log4cxx::helpers::Pool& p);

      /**
       * @brief Closes down the appender.
       *
       * Once this is called the nsca client will be disconnected. To
       * re-establish the connection you should call activateOptions().
       */
      void close();

      /**
       * @brief Called by doAppend() to perform the actual appending
       *
       * The doAppend() method in AppenderSkeleton will have already
       * determined if this appender is of an appropriate threshold to
       * perform any appending so there is no specific logic applied
       * here other than to send the event to the nagios server.
       *
       * @param event : the event being logged
       * @param p     : an internal parameter to log4cxx
       */
      void append (
         spi::LoggingEventPtr const & event,
         helpers::Pool & p
         );

      static int get_level(int const level);

   private:
      std::string this_host_;
      std::string this_host_desc_;
      moost::nagios::nsca_config nsca_config_;
      boost::shared_ptr<moost::nagios::nsca_client> nsca_client_;
      moost::logging::pseudo_ostream & out_;
   };

   LOG4CXX_PTR_DEF(NscaAppender);
}

#endif
