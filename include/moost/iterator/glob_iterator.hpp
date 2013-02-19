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
 * @brief Provide a filtered view of a boost directory_iterator
 *
 * The Boost directory_iterator provides no mechanism for filtering entries. The
 * glob_iterator provides a simple solution for this by presenting facade of a
 * filtered entries using the Boost filter_iterator and a suitable predicate
 * designed to allow filtering either using a regex or a glob pattern.
 *
 * The glob_iterator requires a directory_iterator and a glob_predicate, both
 * of which must have been previously contructed using suitable parameters. The
 * result is an iterator that will skip entries that do not meet the filtering
 * requirements.
 *
 * The name 'glob' has been used due to the fact that most of the time it is
 * expected that the filtering will be done using posix glob pattern matching;
 * however, internally these are converted into regular expessions so to provide
 * a more powerful/flexible alternative to glob patterns the glob_predicate can
 * also be constructed using a boost::regex object.
 *
 * When the glob_predicate is constructed using a glob pattern the whole glob
 * must match the filespec for it to be considered a match. When using regular
 * expressions a search is performed rather than a match so as to allow the
 * regex to define bounds using start and end anchors, as required.
 *
 * NB. If your filtering doesn't seem to work make sure you don't have any
 *     leading relative path for cwd (eg.  './foo' instead of 'foo') as the
 *     directory_iterator only considers "." and not "" as the cwd! :(
 *
 */

#ifndef MOOST_ITERATOR_GLOB_ITERATOR_HPP__
#define MOOST_ITERATOR_GLOB_ITERATOR_HPP__

#include <string>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/regex.hpp>

/**
 * @brief Iterator namespace, for all things to do with custom iterators
 */
namespace moost { namespace iterator {

/**
* @brief Predicate used for filtering by glob_iterator
*
* Construct with either a glob pattern or a regex designed to filter
* entries according to your requirements. A default constructed predicate
* matches nothing but is required by glob_predicate to allow an end iterator
* to be constructed.
*
*/
class glob_predicate
{
public:
   /**
    * @brief Default constructor
    *
    * Matches nothing, needed to allow glob_iterator to be default
    * constructed so it can be used as the 'end' iterator.
    */

   glob_predicate()
      : glob_(false)
   {
   }

   /**
    * @brief Construct a predicate using a regex based filter
    *
    * @param re
    *    regular expression object used to filter entries
    */

   glob_predicate(boost::regex const & re)
      : filter_(re)
      , glob_(false)
   {
   }

   /**
    * @brief Construct a predicate using a glob based filter
    *
    * @param glob
    *    glob pattern used to filter entries
    */

   glob_predicate(std::string const & glob)
      : filter_(glob2re(glob))
      , glob_(true)
   {
   }

   /**
    * @brief Predicate operator
    *
    * @param p
    *    The path being filtered
    *
    * @return true if filter matches, else false
    */
   bool operator()(boost::filesystem::path p) const
   {
      return glob_ ?
         // when we're globbing we want to force an exact match
         boost::regex_match(p.string(), filter_) :
         // when we're pattern matching use search for flexibility
         boost::regex_search(p.string(), filter_);
   }

private:
   boost::regex glob2re(std::string const & glob)
   {
      std::ostringstream oss;
      bool lesc = false;

      // iterate glob and convert to regex
      for(std::string::const_iterator itr = glob.begin() ;
          itr != glob.end(); ++itr)
      {
         bool oesc = false;

         switch(*itr)
         {
            // slash is an esc-char unless it's the 2nd one else its a literal
            case '\\':
               if(lesc) { oss << "\\\\"; }
               oesc = !lesc;
               break;
            // match 0 or more chars
            case '*':
               oss << (lesc ? "\\*" : ".*");
               break;
            // match 1 char
            case '?':
               oss << (lesc ? "\\?" : ".");
               break;
            // literal dot
            case '.':
               oss << "\\.";
               break;
            // everything else
            default:
               oss << *itr;
               break;
         };

         lesc = oesc;
      }

      return boost::regex(oss.str());
   }

   boost::regex filter_;
   bool glob_;
};

/**
 * @brief Glob iterator typedef
 *
 * The actual filtering mechanics is handled by a specialisation of
 * filter_iterator, called glob_iterator. Usage is as follows:
 *
 * @code
 *
 * boost::directory_iterator ditr("/some/path");
 * glob_predicate pred("*.txt")
 * glob_iterator beg(pred, ditr);
 * glob_iterator end;
 * // now use beg and end to iterate the filtered entries
 *
 */
typedef boost::filter_iterator<
   glob_predicate,
   boost::filesystem::directory_iterator
   > glob_iterator;

}}


#endif // MOOST_ITERATOR_GLOB_ITERATOR_HPP__
