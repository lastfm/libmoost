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

#ifndef MOOST_TRANSACTION_QUEUE_HPP__
#define MOOST_TRANSACTION_QUEUE_HPP__

// A collection of queue types for use with moost/transaction/handler.hpp

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace moost { namespace transaction {


   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // Use this class as the base for your specialized serializer
   class SerializerBase
   {
   public:
      void Purge(std::string const & key) const
      {
         boost::filesystem::remove(key);
      }

   protected:
      // Only my sub-class can create and destroy me :)
      SerializerBase(){}
      ~SerializerBase(){}
   };
   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // This is a generic serialiser for POD types ONLY. If you data has
   // specific serialisation requirements you need to implement you own
   // serialization functor.

   template <
      typename dataT,
      typename istreamT = std::ifstream,
      typename ostreamT = std::ofstream
   >
   class Serializer : public SerializerBase
   {
   public:
      bool Serialise(std::string const & key, dataT const & data) const
      {
         ostreamT oStream(key.c_str(), std::ios::binary | std::ios::trunc);
         bool bOk = oStream.is_open();

         if(bOk)
         {
            oStream.write(reinterpret_cast<char const *>(&data), sizeof(data));
            bOk = oStream.good();
            oStream.close();
         }

         return bOk;
      }

      bool Deserialise(std::string const & key, dataT & data) const
      {
         istreamT iStream(key.c_str(), std::ios::binary);
         bool bOk = iStream.is_open();

         if(bOk)
         {
            iStream.read(reinterpret_cast<char *>(&data), sizeof(data));
            bOk = iStream.good();
            iStream.close();
         }

         return bOk;
      }
   };

   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // This is the generic interface for transaction queues

   template <
      typename dataT
   >
   class ITransactionQueue
   {
   public:
      typedef dataT value_type;

      // The interface is polymorphic so any sub-classes can be interchanged dynamically at runtime
      virtual ~ITransactionQueue(){}

      virtual size_t size() const = 0;
      virtual bool empty() const = 0;
      virtual value_type & front() = 0;
      virtual void push_back(value_type const & data) = 0;
      virtual void pop_front() = 0;
   };

   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // A non-persisted queue, that uses the ITransactionQueue interface. This
   // is provided to allow none, partial and full parsistance to be swapped
   // dynamically at runtime depending upon settings

   template <
      typename dataT,
      template<typename T, typename A = std::allocator<T> > class queueT = std::deque
   >
   class NonePersistedTQ : public ITransactionQueue<dataT>
   {
   public:
      typedef dataT value_type;

      size_t size() const { return m_queue.size(); }
      bool empty() const { return m_queue.empty(); }
      value_type & front() { return m_queue.front(); }
      void push_back(value_type const & data) { m_queue.push_back(data); };
      void pop_front() { m_queue.pop_front(); };
   private:
      queueT<dataT> m_queue;
   };

   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // This is the base class for the Fully and Partially persisted queues
   // When the queue is instantiated it will check to see if items exist
   // in the backing store and they do it will automatically add them to
   // the front of the queue.

   template <
      typename dataT,
      typename serializerT = Serializer<dataT>,
      template<typename T, typename A = std::allocator<T> > class queueT = std::deque
   >
   class BasePersistedTQ : public ITransactionQueue<dataT>
   {
   public:

      typedef dataT value_type;

      BasePersistedTQ(std::string const & rootDir, std::string const & queueId) :
         m_nextKey(0), m_rootDir(rootDir), m_guid("15934E61-04A5-47cf-86FF-3E02F08F5931"), m_queueId(queueId)
      {
         //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
         // Create a regex to compare the found files with what we actually want
         //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
         std::stringstream ss;
         ss                                // Eg. ^15934E61-04A5-47cf-86FF-3E02F08F5931-([\dABCDEFGabcdefg]{8})\.myqueue$
            << "^" << m_guid               // Starts with our guid
            <<"-([\\dABCDEFGabcdefg]{8})"  // followed by 8 digit hexidecimal
            << "\\."                       // file extension seperator
            << m_queueId << "$";           // ends with file extension, using the queue id
         //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                           //     [              guid                ] [ key  ] [ qid ]
         boost::regex re(ss.str());        // Eg. 15934E61-04A5-47cf-86FF-3E02F08F5931-0F0A0002.myqueue
         //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

         boost::filesystem::directory_iterator dirItr;

         try
         {
            // As far as I can tell, this doesn't support globbing :(
            dirItr = boost::filesystem::directory_iterator(rootDir);
         }
         catch(std::exception const & e)
         {
            std::stringstream ss;
            ss
               << "Unable to open file queue: "
               << rootDir
               <<
               "("
               << e.what()
               << ")";

            throw std::runtime_error(ss.str());
         }

         for( ; dirItr != boost::filesystem::directory_iterator() ; ++ dirItr)
         {
            boost::smatch m;

            // We need to bind the leaf to a const reference as it's a temporary
            // but we need to to exist beyond the lifetime of the call.
            std::string const & fname = dirItr->path().leaf();

            if(boost::filesystem::is_regular(dirItr->status()) && boost::regex_match(fname, m, re) && m.size() > 1)
            {
               // Decode string hex value
               std::istringstream ss(m[1]);
               uint32_t key = uint32_t();
               ss >> std::hex >> key;

               dataT data = dataT();

               if(!m_serializer.Deserialise(dirItr->string(), data))
               {
                  throw std::runtime_error(std::string("Error loading file queue: ") + dirItr->string());
               }

               push_back_(key, data);
               m_nextKey = std::max(m_nextKey, key + 1);
            }
         }
      }

      ~BasePersistedTQ(){}

      size_t size() const { return m_queue.size(); }

      bool empty() const { return m_queue.empty(); }

      value_type & front()
      {
         return m_queue.front().second;
      }

      void push_back(value_type const & data)
      {
         push_back_(data);
      }

      void pop_front()
      {
         // Remove persisted item
         m_serializer.Purge(GenerateSerialiseKey(m_queue.front().first));

         // Remove from the queue
         m_queue.pop_front();
      }

   protected:

      std::string GenerateSerialiseKey(uint32_t key) const
      {
         std::stringstream ss;

         // We put the guid first as this allows the regex that matches paths
         // to short circuit and terminate early. It can abort as soon as the
         // guid, which is a fixed string, is found not to match!
         ss
            << m_guid << "-"
            << std::hex << std::setw(8) << std::setfill('0') << key
            << "." << m_queueId;

         return (m_rootDir / ss.str()).string();
      }

      void push_back_(uint32_t key, value_type const & data)
      {
         m_queue.push_back(std::make_pair(key, data));
      }

      uint32_t push_back_(value_type const & data)
      {
         uint32_t key = m_nextKey++;
         push_back_(key, data);
         return key;
      }

   protected:
      queueT<std::pair<uint32_t, dataT> > m_queue;
      serializerT m_serializer;

   private:
      uint32_t m_nextKey;
      boost::filesystem::path m_rootDir;
      std::string m_guid;
      std::string m_queueId;
   };

   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // This queue will persist the item at the front when it is requested

   template <
      typename dataT,
      typename serializerT = Serializer<dataT>,
      template<typename T, typename A = std::allocator<T> > class queueT = std::deque
   >
   class PartiallyPersistedTQ : public BasePersistedTQ<dataT, serializerT, queueT>
   {
   public:
      PartiallyPersistedTQ(
         std::string const & rootDir,
         std::string const & queueId
         ) : BasePersistedTQ<dataT, serializerT, queueT>(rootDir, queueId) {}

      dataT & front()
      {
         // First we get the key and data item from the queue
         uint32_t key = this->m_queue.front().first;
         dataT & data = this->m_queue.front().second;

         // Now we serialise the data
         this->m_serializer.Serialise(this->GenerateSerialiseKey(key), data);

         // Now we return the data
         return data;
      }
   };

   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   // This queue will persist all items as they are added to the queue

   template <
      typename dataT,
      typename serializerT = Serializer<dataT>,
      template<typename T, typename A = std::allocator<T> > class queueT = std::deque
   >
   class FullyPersistedTQ : public BasePersistedTQ<dataT, serializerT, queueT >
   {
   public:
      FullyPersistedTQ(
         std::string const & rootDir,
         std::string const & queueId
         ) : BasePersistedTQ<dataT, serializerT, queueT>(rootDir, queueId) {}


      void push_back(dataT const & data)
      {
         // First we add it to the queue
         uint32_t key = this->push_back_(data);

         // And now it's in the queue we serialise
         this->m_serializer.Serialise(this->GenerateSerialiseKey(key), data);
      }
   };
   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

}}

#endif // MOOST_TRANSACTION_QUEUE_HPP__
