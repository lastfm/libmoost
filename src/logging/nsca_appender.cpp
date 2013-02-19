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

#include <string>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/asio/ip/host_name.hpp>

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/bufferedwriter.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/synchronized.h>

#include "../../include/moost/nagios/nsca_client.hpp"

#include "../../include/moost/logging/global.hpp"
#include "../../include/moost/logging/nsca_appender.hpp"
#include "../../include/moost/logging/pseudo_ostream.hpp"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace moost;
using namespace moost::nagios;

IMPLEMENT_LOG4CXX_OBJECT(NscaAppender)

namespace log4cxx
{
   NscaAppender::NscaAppender()
      : AppenderSkeleton(LayoutPtr(new PatternLayout(LOG4CXX_STR("%m"))))
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         std::string const & this_host,
         nsca_config const & cfg
         )
      : AppenderSkeleton(LayoutPtr(new PatternLayout(LOG4CXX_STR("%m"))))
        , this_host_(this_host)
        , this_host_desc_(this_host)
        , nsca_config_(cfg)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         std::string const & this_host,
         std::string const & nsca_svr_host
         )
      : AppenderSkeleton(LayoutPtr(new PatternLayout(LOG4CXX_STR("%m"))))
        , this_host_(this_host)
        , this_host_desc_(this_host)
        , nsca_config_(nsca_svr_host)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         std::string const & this_host,
         std::string const & this_host_desc,
         nsca_config const & cfg
         )
      : AppenderSkeleton(LayoutPtr(new PatternLayout(LOG4CXX_STR("%m"))))
        , this_host_(this_host)
        , this_host_desc_(this_host_desc)
        , nsca_config_(cfg)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         std::string const & this_host,
         std::string const & this_host_desc,
         std::string const & nsca_svr_host
         )
      : AppenderSkeleton(LayoutPtr(new PatternLayout(LOG4CXX_STR("%m"))))
        , this_host_(this_host)
        , this_host_desc_(this_host_desc)
        , nsca_config_(nsca_svr_host)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
   }

   NscaAppender::NscaAppender(LayoutPtr & layout)
      : log4cxx::AppenderSkeleton(layout)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         nsca_config const & cfg
         )
      : AppenderSkeleton(layout)
        , this_host_(this_host)
        , this_host_desc_(this_host)
        , nsca_config_(cfg)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         std::string const & nsca_svr_host
         )
      : AppenderSkeleton(layout)
        , this_host_(this_host)
        , this_host_desc_(this_host)
        , nsca_config_(nsca_svr_host)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         std::string const & this_host_desc,
         nsca_config const & cfg
         )
      : AppenderSkeleton(layout)
        , this_host_(this_host)
        , this_host_desc_(this_host_desc)
        , nsca_config_(cfg)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   NscaAppender::NscaAppender(
         LayoutPtr & layout,
         std::string const & this_host,
         std::string const & this_host_desc,
         std::string const & nsca_svr_host
         )
      : AppenderSkeleton(layout)
        , this_host_(this_host)
        , this_host_desc_(this_host_desc)
        , nsca_config_(nsca_svr_host)
        , out_(moost::logging::global_singleton::instance().get_ostream())
   {
      activateOptions();
   }

   void NscaAppender::setNscaHost(std::string const & host)
   {
      nsca_config_.nsca_svr_host = host;
      out_ << "NscaHost: " << host << std::endl;
   }

   void NscaAppender::setNscaPort(std::string const & port)
   {
      nsca_config_.nsca_svr_port = port;
      out_ << "NscaPort: " << port << std::endl;
   }

   void NscaAppender::setRecvTimeoutMs(boost::uint16_t timeout)
   {
      nsca_config_.recv_timeout = timeout;
      out_ << "RecvTimeout: " << timeout << std::endl;
   }

   void NscaAppender::setSendTimeoutMs(boost::uint16_t timeout)
   {
      nsca_config_.send_timeout = timeout;
      out_ << "SendTimeout: " << timeout << std::endl;
   }

   void NscaAppender::setEncType(std::string const & enctype)
   {
      nsca_config_.enctype = enctype;
      out_ << "EncType: " << enctype << std::endl;
   }

   void NscaAppender::setEncPass(std::string const & encpass)
   {
      nsca_config_.encpass = encpass;
      out_ << "EncPass: " << encpass << std::endl;
   }

   void NscaAppender::setThisHost(std::string const & host)
   {
      this_host_ = host;
      out_ << "ThisHost: " << host << std::endl;
   }

   void NscaAppender::setThisHostDesc(std::string const & desc)
   {
      this_host_desc_ = desc;
      out_ << "ThisHostDesc: " << desc << std::endl;
   }

   void NscaAppender::setOption(const LogString& option, const LogString& value)
   {
      if (StringHelper::equalsIgnoreCase(option,
               LOG4CXX_STR("NSCAHOST"), LOG4CXX_STR("nscahost")))
      {
         setNscaHost(value);
      }
      else
         if (StringHelper::equalsIgnoreCase(option,
                  LOG4CXX_STR("NSCAPORT"), LOG4CXX_STR("nscaport")))
         {
            setNscaPort(value);
         }
         else
            if (StringHelper::equalsIgnoreCase(option,
                     LOG4CXX_STR("RECVTIMEOUTMS"), LOG4CXX_STR("recvtimeoutms")))
            {
               setRecvTimeoutMs(OptionConverter::toInt(value,
                        nsca_const::DEFAULT_RECV_TIMEOUT_MS));
            }
            else
               if (StringHelper::equalsIgnoreCase(option,
                        LOG4CXX_STR("SENDTIMEOUTMS"), LOG4CXX_STR("sendtimeoutms")))
               {
                  setSendTimeoutMs(OptionConverter::toInt(value,
                           nsca_const::DEFAULT_SEND_TIMEOUT_MS));
               }
               else
                  if (StringHelper::equalsIgnoreCase(option,
                           LOG4CXX_STR("ENCTYPE"), LOG4CXX_STR("enctype")))
                  {
                     setEncType(value);
                  }
                  else
                     if (StringHelper::equalsIgnoreCase(option,
                              LOG4CXX_STR("ENCPASS"), LOG4CXX_STR("encpass")))
                     {
                        setEncPass(value);
                     }
                     else
                        if (StringHelper::equalsIgnoreCase(option,
                                 LOG4CXX_STR("THISHOST"), LOG4CXX_STR("thishost")))
                        {
                           setThisHost(value);
                        }
                        else
                           if (StringHelper::equalsIgnoreCase(option,
                                    LOG4CXX_STR("THISHOSTDESC"), LOG4CXX_STR("thishostdesc")))
                           {
                              setThisHostDesc(value);
                           }
                           else
                           {
                              AppenderSkeleton::setOption(option, value);
                           }
   }

   void NscaAppender::activateOptions()
   {
      Pool p;
      activateOptions(p);
   }

   void NscaAppender::activateOptions(Pool& p )
   {
      nsca_client_.reset(new nsca_client(nsca_config_));
      out_ << "Nagios client activated" << std::endl;

      // Ensure any base class options get activated
      AppenderSkeleton::activateOptions(p);
   }

   void NscaAppender::close()
   {
      nsca_client_.reset();
      out_ << "Nagios client terminated" << std::endl;
   }

   bool NscaAppender::requiresLayout() const
   {
      return false;
   }

   void NscaAppender::append (
         spi::LoggingEventPtr const & event,
         helpers::Pool & p
         )
   {
      try
      {
         if(!nsca_client_)
         {
            // fail silently, logging error should not take down application!
            out_
               << "ERROR: append failed (nsca_client not activated)"
               << std::endl;
            return;
         }

         std::string const & thost =
            this_host_.empty() ?
            boost::asio::ip::host_name():
            this_host_;

         std::string const & tdesc =
            this_host_desc_.empty() ?
            "unknown": // open to suggestions on how we can improve on this!
            this_host_desc_;

         out_
            << "Sending alert to Nagios" << std::endl
            << "- host: " << thost << std::endl
            << "- desc: " << tdesc << std::endl
            << "- type: ";

         int service_state = 0;

         switch(get_level(event->getLevel()->toInt()))
         {
            // it doesn't make sense why these would be logged to nagios so map to unknown
            case Level::ALL_INT:
            case Level::DEBUG_INT:
            case Level::TRACE_INT:
               out_ << "unknown" << std::endl;
               service_state = nsca_client::service_state::UNKNOWN;
               break;

               // map a logger info state to a nagios ok state
            case Level::INFO_INT:
               out_ << "ok" << std::endl;
               service_state = nsca_client::service_state::OK;
               break;

               // map a logger warning state to a nagios warning state
            case Level::WARN_INT:
               out_ << "warning" << std::endl;
               service_state = nsca_client::service_state::WARNING;
               break;

               // map a logger error or fatal state to a nagios critical state
            case Level::ERROR_INT:
            case Level::FATAL_INT:
               out_ << "critical" << std::endl;
               service_state = nsca_client::service_state::CRITICAL;
               break;

               // the logger is turned off -- do nothing!
            case Level::OFF_INT:
            default: // no action
               out_ << "none" << std::endl;
               return;
         }

         // format and send the message to the nsca server
         LogString msg;
         layout->format(msg, event, p);
         {
            helpers::synchronized sync(mutex);

            // this is a bit of a fudge to convert from wide to narrow but
            // since we're generally only using ASCII it's good enough.
            std::string narrow(msg.begin(), msg.end());

            out_
               << "- mesg: "
               << narrow
               << std::endl;

            nsca_client_->send(
                  thost,
                  tdesc,
                  service_state,
                  narrow);
         }
      }
      catch(std::exception const & e)
      {
         out_
            << "append failed: " << e.what() << std::endl;
      }
      catch(...)
      {
         out_
            << "append failed." << std::endl;
      }
   }

   int NscaAppender::get_level(int const level) // static
   {
      int const level_range[] = {
         Level::OFF_INT,
         Level::FATAL_INT,
         Level::ERROR_INT,
         Level::WARN_INT,
         Level::INFO_INT,
         Level::DEBUG_INT,
         Level::TRACE_INT,
         Level::ALL_INT,
      };

      int const * end = level_range + sizeof(level_range) / sizeof(level_range[0]);
      int const * plevel = std::lower_bound(level_range, end, level, std::greater<int>());

      if(plevel == end) throw std::invalid_argument("invalid level"); // this should not be possible

      return *plevel;
   }
}
