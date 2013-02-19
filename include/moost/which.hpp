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

#ifndef MOOST_WHICH_H__
#define MOOST_WHICH_H__

#include <utility>    // for pair
#include <functional> // for less
#include <iterator>   // for iterator_traits

namespace moost {

/** which can be used to simplify access to pairs
 *
 * i.e. to sort a pair<int, float> with the second entry just
 *
 * sort( v.begin(), v.end(), which<2>::comparer<greater>() ); // descending
 * sort( v.begin(), v.end(), which<2>::comparer<less>() );    // ascending
 * sort( v.begin(), v.end(), which<2>::comparer<>() );        // ascending
 */
template<int N>
struct which;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

template <typename Cont, int N>
class which_back_inserter_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
   Cont& container_;
   which<N> which_;
public:
   which_back_inserter_iterator(Cont& container) : container_(container) {}
   which_back_inserter_iterator& operator*()     { return *this; }
   which_back_inserter_iterator& operator++()    { return *this; }
   which_back_inserter_iterator& operator++(int) { return *this; }

   template <typename T1, typename T2>
   which_back_inserter_iterator & operator=(const std::pair<T1, T2>& value)
   {
      container_.push_back( which_(value) );
      return *this;
   }
};


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

template <typename Cont, int N>
class which_inserter_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
   Cont& container_;
   typename Cont::iterator where_;
   which<N> which_;

public:
   which_inserter_iterator(Cont& container, typename Cont::iterator where) : container_(container), where_(where) {}
   which_inserter_iterator& operator*()     { return *this; }
   which_inserter_iterator& operator++()    { return *this; }
   which_inserter_iterator& operator++(int) { return *this; }
   template <typename T1, typename T2>
   which_inserter_iterator & operator=(const std::pair<T1, T2>& value)
   {
      where_ = container_.insert( where_, which_(value) );
      ++where_;
      return *this;
   }
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace detail {

template<class Pair, int N>
struct which_helper;

template<class Pair>
struct which_helper<Pair, 1>
{
    typedef typename Pair::first_type type;
    static type Pair::*member()
    {
        return &Pair::first;
    }
};

template<class Pair>
struct which_helper<Pair, 2>
{
    typedef typename Pair::second_type type;
    static type Pair::*member()
    {
        return &Pair::second;
    }
};
}

template<int N>
struct which
{
  typedef which<3-N> other_type;

  /// get the element
  template<typename Pair>
  typename detail::which_helper<Pair, N>::type & operator()(Pair & el) const
  {
    return el.*detail::which_helper<Pair, N>::member();
  }

  /// get a const
  template<typename Pair>
  typename detail::which_helper<Pair, N>::type const & operator()(Pair const & el) const
  {
    return el.*detail::which_helper<Pair, N>::member();
  }

  struct iterator_accessor
  {
    /// if it is inside an iterator
    template<typename IT>
    typename detail::which_helper<typename std::iterator_traits<IT>::value_type, N>::type &
    operator()(IT & el) const
    {
      return el->*detail::which_helper<typename std::iterator_traits<IT>::value_type, N>::member();
    }

    /// if it is inside an iterator (const version)
    template<typename IT>
    typename detail::which_helper<typename std::iterator_traits<IT>::value_type, N>::type const &
    operator()(IT const & el) const
    {
      return el->*detail::which_helper<typename std::iterator_traits<IT>::value_type, N>::member();
    }
  };

  /// the comparer
  template< template <class> class Pred = std::less>
  struct comparer
  {
    template <typename Pair>
    bool operator()(Pair const & lhs, Pair const & rhs ) const
    {
      return Pred<typename detail::which_helper<Pair, N>::type>()(
          lhs.*detail::which_helper<Pair, N>::member(),
          rhs.*detail::which_helper<Pair, N>::member());
    }
  };

  /// the value_comparer
  template <typename T, template <class> class Pred = std::equal_to>
  class value_comparer
  {
  public:
    value_comparer(const T& value)
      : m_value(value)
    {}

    template <typename Pair>
    bool operator()(Pair const & val) const
    {
      return Pred<T>()(val.*detail::which_helper<Pair, N>::member(), m_value);
    }

  private:
    const T& m_value;
  };

  template<typename Cont>
  static which_back_inserter_iterator<Cont, N> back_inserter(Cont& container)
  {  // return a back_insert_iterator
     return (which_back_inserter_iterator<Cont, N>(container));
  }

  template<typename Cont, typename Iterator>
  static which_inserter_iterator<Cont, N> inserter(Cont& container, Iterator where)
  {  // return a back_insert_iterator
     return (which_inserter_iterator<Cont, N>(container, where));
  }

};

} // moost

#endif // MOOST_WHICH_H__
