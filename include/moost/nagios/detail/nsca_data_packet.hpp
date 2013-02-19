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

// [ricky 7/13/2011 ] Mostly ripped from the nsca_send common.h file

#ifndef MOOST_NAGIOS_NSCA_CLIENT_NSCA_DATA_PACKET_HPP__
#define MOOST_NAGIOS_NSCA_CLIENT_NSCA_DATA_PACKET_HPP__

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "nsca_common.hpp"

namespace moost { namespace nagios {

   struct nsca_data_packet // MUST be a POD
   {
      // for some reason the client must randomise the struct (including padding)
      // I can only assume this is so identical packets differ when encrypted.
      // Seems like overkill to me but, meh -- let's just do it for now.
      static void randomize(nsca_data_packet & packet)
      {
         // [ricky 7/14/2011] Just emulating what the real client does!
         srand((int)time(NULL));
         for(size_t idx = 0 ; idx < sizeof(packet) ; idx++)
         {
            ((char *)&packet)[idx]=(int)'0'+(int)(72.0*rand()/(RAND_MAX+1.0));
         }
      }

      static void zeroize(nsca_data_packet & packet)
      {
          memset(&packet, 0, sizeof(packet));
      }

      boost::int16_t packet_version;
      boost::uint32_t crc32_value;
      boost::uint32_t timestamp;
      boost::int16_t return_code;
      char host_name[nsca_const::MAX_HOSTNAME_LENGTH];
      char svc_description[nsca_const::MAX_DESCRIPTION_LENGTH];
      char plugin_output[nsca_const::MAX_PLUGINOUTPUT_LENGTH];
   };

   // Allow us to get a packet from a stream
   inline
   std::istream & operator >> (std::istream & os, nsca_data_packet & packet)
   {
      std::vector<std::string> lines;
      std::string line;
      while(getline(os, line) && lines.size() < 4)
      {
         lines.push_back(line);
      }

      std::string errmsg;
      for(int check = 0 ; check >= 0 && !os.bad();)
      {
         switch(check++)
         {
         case 0:
            if(lines.size() != 4)
            {
               errmsg = "Payload string is malformed";
            }
            break;
         case 1:
            if(lines[0].size() > nsca_const::MAX_HOSTNAME_LENGTH-1)
            {
               errmsg = "Hostname is too long for NSCA to handle";
            }
            break;
         case 2:
            if(lines[1].size() > nsca_const::MAX_DESCRIPTION_LENGTH-1)
            {
               errmsg = "Service description is too long for NSCA to handle";
            }
            break;
         case 3:
            if(lines[3].size() > nsca_const::MAX_PLUGINOUTPUT_LENGTH-1)
            {
               errmsg = "Servcice description is too long for NSCA to handle";
            }
            break;
         default:
            check = -1; // no more checks.
            break;
         }

         if(!errmsg.empty())
         {
            // Oops, something's wrong
            os.setstate(std::ios::failbit);

            // If the caller is expecting exceptions for this
            // stream give them what they asked for.
            if((os.exceptions() & std::ios::failbit) == std::ios::failbit)
            {
               throw std::invalid_argument(errmsg);
            }
         }
      }

      // All good?
      if(!os.bad())
      {
         strcpy(packet.host_name, lines[0].c_str());
         strcpy(packet.svc_description, lines[1].c_str());
         strcpy(packet.plugin_output, lines[3].c_str());
         packet.return_code = boost::lexical_cast<boost::int16_t>(lines[2]);
      }

      return os;
   }

}}

#endif
