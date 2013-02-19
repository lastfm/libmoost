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

/*!
 * file_backed_data_source.hpp
 *
 * A generic automagically reloading file-backed data source.
 * The data type and the loading process must be specified in a DataPolicy class.
 * The DataPolicy should implement data_policy_base<T> where T is the type of the
 * data to be held in the source.  The filepath and other options for the source
 * are typically read from an xml file, though they can also be set directly on
 * a file_backed_data_source_config object.  In either case you can use a
 * file_backed_data_source_factory to create your source.
 *
 * The xml configuration format looks like this:
 *
 * <FileBackedDataSource>
 *    <Filepath>/path/to/data/file</Filepath>
 *    <MinSecsSinceLastLoad>30</MinSecsSinceLastLoad>
 *    <ThrowOnFirstLoadFail>true</ThrowOnFirstLoadFail>
 *    <MinProportionOfLastLoad>0.5</MinProportionOfLastLoad>
 * </FileBackedDataSource>
 *
 *
 * file_backed_data_source logs info and warnings related to data loading with
 * moost::logging. You must therefore initialiase moost::logging in your code
 * in the usual way before using a file_backed_data_source.
 *
 *
 * Synopsis:
 *
 *  // this policy loads a map<int,int> from file
 *  typedef std::map<int, int> int_map_t;
 *  class IntMapDataPolicy : public data_policy_base<int_map_t>
 *  {
 *  public:
 *    std::string getName() const { return "int map data"; }
 *    boost::shared_ptr<int_map_t> loadFromFile(const std::string& filepath) const
 *    {
 *      boost::shared_ptr<int_map_t> pData(new int_map_t);
 *      // read from filepath into pData ...
 *      return pData;
 *    }
 *    size_t size(shared_ptr<int_map_t> pData) const { return pData->size(); }
 *  };
 *
 *  typedef file_backed_data_source<IntMapDataPolicy> source_t;
 *
 *  file_backed_data_source_config conf;
 *  conf.filepath = "/path/to/data/file";
 *
 *  IntMapDataPolicy dataPolicy;
 *
 *  file_backed_data_source_factory sourceFactory;
 *  boost::shared_ptr<source_t> pSource = sourceFactory.createFromConfig(dataPolicy, conf);
 *
 *  pSource->load();
 *  // ... the data will now reload automagically when the file updates
 *
 *  {
 *    shared_ptr<int_map_t> pData = pSource->get_shared_ptr();
 *    // now you can use pData safely in this scope, even if a reload happens
 *  }
 *
 */

#ifndef MOOST_IO_FILE_BACKED_DATA_SOURCE_HPP__
#define MOOST_IO_FILE_BACKED_DATA_SOURCE_HPP__

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include "../safe_shared_ptr.hpp"
#include "file_watcher.hpp"
#include "../terminal_format.hpp"
#include "../xml/simple_parser.hpp"
#include "../logging/class_logger.hpp"

namespace moost { namespace io {

/*!
 * Implement this policy to define the data type to be held in your source
 * and how it is loaded from file.
 *
 * loadFromFile() can throw exceptions.  The enclosing source will rethrow your exception
 * if it is loading for the first time and has been configured with throwOnFirstLoadFail
 * set to true.  Otherwise the Source will simply log your exception and abandon the reload.
 */
template <typename T>
class data_policy_base
{
public:
   typedef T data_type;
   virtual ~data_policy_base() { }
   virtual std::string getName() const = 0;
   virtual boost::shared_ptr<T> loadFromFile(const std::string& filepath) const = 0;
   virtual size_t size(boost::shared_ptr<T> pData) const = 0;
};


/*!
 * Any class implementing this interface can be registered with a file_backed_data_source.
 * Registered loadables will be asked to load() whenever the source reloads.
 */
class loadable
{
public:
   virtual ~loadable() { }
   virtual void load() = 0;
};


/*!
 * Options to configure a file_backed_data_source.
 */
struct file_backed_data_source_config
{
public:
   std::string filepath;            // the filepath of the data file
   int minSecsSinceLastLoad;        // reject reload sooner than this
   bool throwOnFirstLoadFail;       // if true, throw an exception if the initial load fails
   double minProportionOfLastLoad;  //  reject reload smaller than this

   // some possible default values, you still need to set the filepath!
   file_backed_data_source_config() :
   minSecsSinceLastLoad(30), throwOnFirstLoadFail(true), minProportionOfLastLoad(0.5)
   {
   }

   file_backed_data_source_config(const std::string& path, int minSecs, bool throwOnFirst,
      double minProportion) :
   filepath(path), minSecsSinceLastLoad(minSecs), throwOnFirstLoadFail(throwOnFirst),
   minProportionOfLastLoad(minProportion)
   {
   }
};

class file_backed_data_source_config_factory
{
public:
   file_backed_data_source_config createFromXml(const std::string& xmlFilepath)
   {
      tag_map_t tagMap;
      xmlToMap(tagMap, xmlFilepath);
      file_backed_data_source_config conf;
      setAndRemove(conf.filepath, "Filepath", tagMap);
      setAndRemove(conf.minSecsSinceLastLoad, "MinSecsSinceLastLoad", tagMap);
      setAndRemove(conf.throwOnFirstLoadFail, "ThrowOnFirstLoadFail", tagMap);
      setAndRemove(conf.minProportionOfLastLoad, "MinProportionOfLastLoad", tagMap);
      if (!tagMap.empty())
         throw std::runtime_error("Unexpected xml tag: " + tagMap.begin()->first);
      return conf;
   }

private:
   typedef std::map<std::string, std::string> tag_map_t;

   void xmlToMap(std::map<std::string, std::string>& tagMap, const std::string& xmlFilepath)
   {
      moost::xml::simple_parser xmlParser;
      xmlParser.load(xmlFilepath);
      const moost::xml::simple_parser::tree_branch_t& root = xmlParser.get_root();
      if (!root.front())
         throw std::runtime_error("No xml root tag");
      std::string rootTag(root.front()->header);
      if (rootTag != "FileBackedDataSource")
         throw std::runtime_error("Unexpected xml root tag: " + rootTag);
      root.front()->leaves2map(tagMap);
   }

   template <typename T>
   void setAndRemove(T& value, const std::string& name, tag_map_t& tagMap)
   {
      tag_map_t::iterator it = tagMap.find(name);
      if (it == tagMap.end())
         return;
      value = boost::lexical_cast<T>(it->second);
      tagMap.erase(it);
   }

   void setAndRemove(bool& value, const std::string& name, tag_map_t& tagMap)
   {
      tag_map_t::iterator it = tagMap.find(name);
      if (it == tagMap.end())
         return;
      std::string lowerVal = boost::to_lower_copy(it->second);
      if (lowerVal == "true")
         value = true;
      else if (lowerVal == "false")
         value = false;
      else
         throw std::runtime_error("Unexpected value for boolean tag: " + lowerVal);
      tagMap.erase(it);
   }
};

template <typename DataPolicy>
class file_backed_data_source : public loadable
{
public:

   typedef typename DataPolicy::data_type data_type;

   file_backed_data_source(const DataPolicy& dataPolicy) : m_dataPolicy(dataPolicy),
      m_firstLoad(true), m_lastLoadTime(-1) { }

   void configure(file_backed_data_source_config conf)
   {
      m_conf = conf;
   }

   const file_backed_data_source_config& getConfig() const
   {
      return m_conf;
   }

   size_t size() const
   {
      boost::shared_ptr<data_type> pData = m_pData.get_shared();
      return m_dataPolicy.size(pData);
   }

   int getLastLoadTime() const
   {
      return m_lastLoadTime;
   }

   void registerLoadable(boost::shared_ptr<loadable> pOther, bool loadOtherFirst = true)
   {
      if (loadOtherFirst)
         m_preRegistered.push_back(pOther);
      else
         m_postRegistered.push_back(pOther);
   }

   boost::shared_ptr<data_type> get_shared_ptr()
   {
      return m_pData.get_shared();
   }

   void load()
   {
      if (m_firstLoad)
      {
         m_fileWatcher.start();
         m_fileWatcher.insert(m_conf.filepath, boost::bind(&file_backed_data_source<DataPolicy>::reload, this, _1, _2));
      }
      reload(file_watcher::CHANGED, m_conf.filepath);
      if (m_firstLoad)
      {
         m_firstLoad = false;
      }
   }

private:
   DataPolicy m_dataPolicy;

   file_backed_data_source_config m_conf;
   bool m_firstLoad;
   int m_lastLoadTime;

   file_watcher m_fileWatcher;

   moost::safe_shared_ptr<data_type> m_pData;

   typedef std::vector<boost::shared_ptr<loadable> > registered_t;
   registered_t m_preRegistered;
   registered_t m_postRegistered;

   void reload(file_watcher::file_action action, const std::string& filepath)
   {
      if (action != file_watcher::CHANGED)
         return;

      // abandon reload if it's too soon after previous one
      int timeNow = static_cast<int>(time(NULL));
      if (!m_firstLoad && timeNow < m_lastLoadTime + m_conf.minSecsSinceLastLoad)
         return;

      // force pre-registered sources to reload first
      for (registered_t::iterator it = m_preRegistered.begin(); it != m_preRegistered.end(); ++it)
         (*it)->load();

      MLOG_CLASS_INFO("Updating " << m_dataPolicy.getName() << "..");

      boost::shared_ptr<data_type> pData;
      loadWithErrorHandling(pData, filepath);

      // abandon new dataset if it looks too small
      size_t newSize = m_dataPolicy.size(pData);
      if (!m_firstLoad && newSize < size() * m_conf.minProportionOfLastLoad)
         return;

      MLOG_CLASS_INFO(moost::terminal_format::getOkay() << ": Loaded " << newSize);

      m_pData = pData;
      m_lastLoadTime = static_cast<int>(time(NULL));

      // force post-registered sources to reload afterwards
      for (registered_t::iterator it = m_postRegistered.begin(); it != m_postRegistered.end(); ++it)
         (*it)->load();

   }

   void loadWithErrorHandling(boost::shared_ptr<data_type>& pData, const std::string& filepath)
   {
      try
      {
         pData = m_dataPolicy.loadFromFile(filepath);
      }
      catch (std::runtime_error& ex)
      {
         MLOG_CLASS_WARN(ex.what() << " loading " << m_dataPolicy.getName()
            << " from " << filepath);
         if (m_firstLoad && m_conf.throwOnFirstLoadFail)
         {
            throw;
         }
      }
      catch (...)
      {
         MLOG_CLASS_WARN("exception loading " << m_dataPolicy.getName()
            << " from " << filepath);
         if (m_firstLoad && m_conf.throwOnFirstLoadFail)
         {
            throw;
         }
      }
   }
};

class file_backed_data_source_factory
{
public:

   template <typename DataPolicy>
   boost::shared_ptr<file_backed_data_source<DataPolicy> > createFromConfig(const DataPolicy& dataPolicy, file_backed_data_source_config conf)
   {
      boost::shared_ptr<file_backed_data_source<DataPolicy> > pSource(new file_backed_data_source<DataPolicy>(dataPolicy));
      pSource->configure(conf);

      return pSource;
   }

   template <typename DataPolicy>
   boost::shared_ptr<file_backed_data_source<DataPolicy> > createFromXml(const DataPolicy& dataPolicy, const std::string& xmlFilepath)
   {
      file_backed_data_source_config_factory config_factory;
      file_backed_data_source_config conf = config_factory.createFromXml(xmlFilepath);
      return createFromConfig(dataPolicy, conf);
   }
};

}}

#endif
