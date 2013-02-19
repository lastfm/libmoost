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

#ifndef MOOST_CONTIANER_GEO_MAP_HPP
#define MOOST_CONTIANER_GEO_MAP_HPP

#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace moost { namespace container {

/** @brief geo_map is a container that associates locations with objects of type @c Data.
 *
 * It is a pair associative container, meaning that its value type is @c pair<const Location, Data>.  Elements in the
 * container may have duplicate locations or values.
 *
 * Elements in the container may be iterated, or searched for given a geographic bounding box, or a point and
 * radius.
 */
template<class Data>
class geo_map
{
public:

   /** @brief Key/lookup type of geo_map.
    *
    * Coordinates are interpreted as decimal degrees.
    */
   struct location
   {
      location() {}
      location(float latitude_, float longitude_) : latitude(latitude_), longitude(longitude_) {}
      float latitude;
      float longitude;
   };

   /// The type of object, @c pair<const location, Data>, stored in the geo_map.
   typedef std::pair<location, Data> value_type;

   /// Const iterator used to iterate through a geo_map.
   typedef typename std::vector<value_type>::const_iterator const_iterator;

private:

   // TODO: until i figure out a better way to ensure constness of just the location, all public iterators are const
   typedef typename std::vector<value_type>::iterator iterator;

   /// The math constant pi.
   static double pi()
   { return 3.141592653589793238462643383279502884197; }

   /// Earth's mean radius, in kilometers.
   static double R()
   { return 6371.0; }

   static bool cmp_value_type(const value_type & value1, const value_type & value2)
   {
      return value1.first.longitude < value2.first.longitude;
   }

   /// convert a locations latitude and longitude from degrees to radians
   void degrees2radians(location& loc)
   {
      loc.latitude *= static_cast<float>(pi() / 180.0);
      loc.longitude *= static_cast<float>(pi() / 180.0);
   }

   /// construct a longitude delta for a query location in radians and a given radius
   float radius2deltalon(location& loc, float radius)
   {
      // construct a delta with a bearing due east (pi / 2)
      float dLon = static_cast<float>(
                      std::atan2(std::sin(pi() / 2.0) * std::sin(radius / R()) * std::cos(loc.latitude),
                                 std::cos(radius / R()) - std::sin(loc.latitude) * std::sin(loc.latitude))
                                 );

      // if our delta is negative, because our radius is very big or we are at an extreme latitude so the radius wraps around,
      // just search everything
      if (dLon < 0.0F)
        dLon = static_cast<float>(pi());

      return dLon;
   }

   /// distance between two locations using the Haversine formula
   float haversine_dist(location& x, location& y)
   {
      double a =  std::sin((x.latitude - y.latitude) / 2.0)
                   * std::sin((x.latitude - y.latitude) / 2.0)
                   + std::cos(y.latitude) * std::cos(x.latitude)
                   * std::sin((x.longitude - y.longitude) / 2.0)
                   * std::sin((x.longitude - y.longitude) / 2.0);
      return static_cast<float>( R() * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a)) );
   }

   // kinda funky that we don't actually store a vector of value_type
   // the const requirement of the location doesn't play well with vectors
   // requiring the assignment operator for moving elements during inserts/erases
   std::vector< value_type > m_values;

public:

   /// Constructs an empty geo_map.
   geo_map()
   {}

   /// reserve space
   void reserve(int num_entries)
   {
      m_values.reserve(num_entries);
   }

   /// Inserts @a value into the geo_map.
   const_iterator insert(const value_type & value, bool ordered = true)
   {
      if (   std::abs(value.first.latitude) >= 90.0
          || std::abs(value.first.longitude) >= 180.0)
          throw std::invalid_argument("bad location coordinates");
      // convert to radians
      value_type v( location( static_cast<float>(value.first.latitude * pi() / 180.0),
                              static_cast<float>(value.first.longitude * pi() / 180.0)),
                    value.second);
      if (ordered)
         return m_values.insert(std::lower_bound(m_values.begin(), m_values.end(), v, geo_map<Data>::cmp_value_type), v);
      else
         return m_values.insert(m_values.end(), v);
   }

   /// @brief Orders the geo_map.
   /// Only necessary if unordered insert was invoked.
   void order()
   {
      std::sort(m_values.begin(), m_values.end(), geo_map<Data>::cmp_value_type);
   }

   /** @brief Finds all values that lie within a distance @a radius of @a location.
    * @param query the query point
    * @param radius the bounding search distance from @a location
    * @param result an Output Iterator to which the range of matching values are copied
    */
   template <class OutputIterator>
   void find(location query, float radius, OutputIterator result)
   {
      if (   std::abs(query.latitude) >= 90.0
          || std::abs(query.longitude) >= 180.0)
          throw std::invalid_argument("bad location coordinates");

      // convert location to radians
      degrees2radians(query);

      // construct a minimum and maximum longitude to binary search our collection
      // longitude given a distance and bearing:
      // lat2 = asin(sin(lat1)*cos(d/R) + cos(lat1)*sin(d/R)*cos(θ))
      // lon2 = lon1 + atan2(sin(θ)*sin(d/R)*cos(lat1), cos(d/R)−sin(lat1)*sin(lat2))
      // d/R is the angular distance (in radians), where d is the distance traveled and R is the earth’s radius
      // also note that because we're going due east, lat2 == lat1

      float dLon = radius2deltalon(query, radius);

      iterator it = std::lower_bound(m_values.begin(), m_values.end(), value_type(location(0.0F, query.longitude - dLon), Data()), geo_map<Data>::cmp_value_type);
      iterator end = std::upper_bound(m_values.begin(), m_values.end(), value_type(location(0.0F, query.longitude + dLon), Data()), geo_map<Data>::cmp_value_type);

      for (; it != end; ++it)
      {
         float d = haversine_dist(it->first, query);
         if (d <= radius)
            *result++ = *it;
      }
   }

   /** @brief Finds all values, and their distances from @a location inside a distace of @a radius.
    * @param query the query point
    * @param radius the bounding search distance from @a location
    * @param result an Output Iterator to which the range of matching value,distance pairs are copied
    */
   template <class OutputIterator>
   void find_distances(location query, float radius, OutputIterator result)
   {
      if (   std::abs(query.latitude) >= 90.0
          || std::abs(query.longitude) >= 180.0)
          throw std::invalid_argument("bad location coordinates");

      degrees2radians(query);
      float dLon = radius2deltalon(query, radius);

      iterator it = std::lower_bound(m_values.begin(), m_values.end(), value_type(location(0.0F, query.longitude - dLon), Data()), geo_map<Data>::cmp_value_type);
      iterator end = std::upper_bound(m_values.begin(), m_values.end(), value_type(location(0.0F, query.longitude + dLon), Data()), geo_map<Data>::cmp_value_type);

      for (; it != end; ++it)
      {
         float d = haversine_dist(it->first, query);
         if (d <= radius)
            *result++ = std::make_pair(*it, d);
      }
   }

   /** @brief Finds all values that lie within a bounding box.
    * @param min the southwest corner of the bounding box.
    * @param max the northeast corner of the bounding box.
    * @param result an Output Iterator to which the range of matching values are copied
    */
   template <class OutputIterator>
   void find(location min, location max, OutputIterator result)
   {
      // convert locations to radians
      min.latitude *= static_cast<float>( pi() / 180.0 );
      min.longitude *= static_cast<float>( pi() / 180.0 );
      max.latitude *= static_cast<float>( pi() / 180.0 );
      max.longitude *= static_cast<float>( pi() / 180.0 );

      iterator it = std::lower_bound(m_values.begin(), m_values.end(), value_type(min, Data()), geo_map<Data>::cmp_value_type);
      iterator end = std::upper_bound(m_values.begin(), m_values.end(), value_type(max, Data()), geo_map<Data>::cmp_value_type);

      for (; it != end; ++it)
      {
         if (   it->first.latitude >= min.latitude
             && it->first.latitude <= max.latitude
             && it->first.longitude >= min.longitude
             && it->first.longitude <= max.longitude)
            *result++ = *it;
      }
   }

   /// Clears all values from the geo_map.
   void clear()
   {
      m_values.clear();
   }

   /** @brief Exchanges the contents of the geo_map with the contents of @a table.
    * @param table another geo_map of the same type as this whose content is swapped with that of this container.
    */
   void swap(geo_map<Data> & table)
   {
      m_values.swap(table.m_values);
   }

   /** @brief Returns the number of elements in the geo_map.
   */
   size_t size()
   {
      return m_values.size();
   }

   /// Returns a @c const_iterator pointing to the beginning of the geo_map.
   const_iterator begin() const { return m_values.begin(); }
   /// Returns a @c const_iterator pointing to the end of the geo_map.
   const_iterator end() const { return m_values.end(); }
};

}} // moost::container

#endif // MOOST_CONTIANER_GEO_MAP_HPP
