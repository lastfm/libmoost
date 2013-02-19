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

#ifndef MOOST_NAGIOS_NSCA_CRC32_HPP__
#define MOOST_NAGIOS_NSCA_CRC32_HPP__

#include <boost/cstdint.hpp>

#include <vector>

namespace moost { namespace nagios {

   //---> NASTY CODE ALERT
   // [ricky 7/14/2011] hoiked (and cleaned up a tad) from nsca_send
   class nsca_crc32
   {
   public:

      nsca_crc32()
         : crc32_table_(crc32_table_size)
      {
         generate_crc32_table();
      }

      boost::uint32_t calculate(nsca_data_packet const & send_packet) const
      {
         char const * buffer = reinterpret_cast<char const *>(&send_packet);
         size_t const buffer_size = sizeof(send_packet);

         boost::uint32_t crc = 0xFFFFFFFF;
         boost::uint32_t  this_char;

         crc=0xFFFFFFFF;

         for(size_t current_index = 0 ; current_index < buffer_size ; current_index++)
         {
            this_char = buffer[current_index];
            crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32_table_[(crc ^ this_char) & 0xFF];
         }

         return (crc ^ 0xFFFFFFFF);
      }

   private:
      void generate_crc32_table()
      {
         boost::uint32_t crc;
         boost::uint32_t const poly = 0xEDB88320L;

         for(boost::uint32_t i=0 ; i < crc32_table_size ; i++)
         {
            crc = i;
            for(size_t j = 8 ; j > 0 ; j--)
            {
               if(crc & 1)
               {
                  crc = (crc >> 1) ^ poly;
               }
               else
               {
                  crc >>= 1;
               }
            }

            crc32_table_[i] = crc;
         }

         return;
      }


   private:
      std::vector<boost::uint32_t> crc32_table_;
      static size_t const crc32_table_size = 256;
   };
   // ---<

}}

#endif
