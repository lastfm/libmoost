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
 *
 * /file A policy driven scope exit framework
 *
 * A simple (Q&D) version of boost ScopeExit (because 1.35 doesn't have it!)
 * ---> Ironically, we are now using boost 1.42 but this is simpler to use :)
 *
 * http://www.boost.org/doc/libs/1_41_0/libs/scope_exit/doc/html/scope_exit/ref.html
 *
 */

#ifndef MOOST_UTILS_SCOPE_EXIT_HPP__
#define MOOST_UTILS_SCOPE_EXIT_HPP__

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>

namespace moost { namespace utils {

   // These are the default policies, use on or create your own.

   namespace scope_exit
   {
      namespace policy
      {
         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// The base for all policies. Models common concept interface
         template <typename scopedT>
         struct policy_base : private boost::noncopyable
         {
            typedef scopedT scoped_type;

            explicit policy_base(scoped_type const & scoped)
               : scoped_(scoped) { }

         protected:
            scoped_type scoped_;
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// The base for all policies that implement a getter
         template <typename scopedT>
         struct policy_base_with_get : policy_base<scopedT>
         {
            typedef scopedT scoped_type;

            explicit policy_base_with_get(scoped_type const & scoped)
               : policy_base<scopedT>(scoped) { }

            scoped_type const & get() const
            {
               return this->scoped_;
            }
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// This policy just sets scoped to the default constructed value.
         template <typename scopedT>
         struct set_default_value : policy_base<boost::reference_wrapper<scopedT> >
         {
            typedef scopedT scoped_type;
            typedef boost::reference_wrapper<scopedT> policy_type;

            explicit set_default_value(scoped_type & scoped) :
               policy_base<policy_type>(policy_type(scoped)) { }

            void operator()() const
            {
               // Set back to default constructed value
               this->scoped_.get() = typename boost::remove_cv<scoped_type>::type();
            }
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// This policy just sets scoped to a specific value.
         template <typename scopedT>
         struct set_specific_value : policy_base<boost::reference_wrapper<scopedT> >
         {
            typedef scopedT scoped_type;
            typedef boost::reference_wrapper<scopedT> policy_type;

            set_specific_value(scoped_type & scoped, scoped_type const & val) :
               policy_base<policy_type>(policy_type(scoped)), val_(val) { }

            void operator()() const
            {
               // Set back to default constructed value
               this->scoped_.get() = val_;
            }

         private:
            scopedT val_;
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// This policy just resets scoped back to the original value
         template <typename scopedT>
         struct restore_original_value : policy_base<boost::reference_wrapper<scopedT> >
         {
            typedef scopedT scoped_type;
            typedef boost::reference_wrapper<scopedT> policy_type;

            explicit restore_original_value(scoped_type & scoped) :
               policy_base<policy_type>(policy_type(scoped)), orig_scoped_(scoped)
            {
               // This policy takes a copy of the original scoped type to restore later
            }

            void operator()() const
            {
               // Set back to original value
               this->scoped_.get() = orig_scoped_;
            }

         private:
            scoped_type const orig_scoped_;
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// Calls a member function of the scoped object. If no member function
         /// is specified the void operator()(void) function operator is called
         template <typename scopedT>
         struct call_member_function : policy_base<boost::reference_wrapper<scopedT> >
         {
            typedef scopedT scoped_type;
            typedef boost::reference_wrapper<scopedT> policy_type;
            typedef void (scoped_type::*member_func_t)();

            call_member_function(scoped_type & scoped, member_func_t mf)
               : policy_base<policy_type>(policy_type(scoped)), mf_(mf)
            {
            }

            explicit call_member_function(scoped_type & scoped)
               : policy_base<policy_type>(policy_type(scoped)), mf_(&scoped_type::operator())
            {
            }

            void operator()() const
            {
               if (mf_)
               {
                  // call member function on scoped object
                  (this->scoped_.get().*mf_)();
               }
            }

         private:
            member_func_t mf_;
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// specialisation of call_member_function for "this" pointers
         template <typename scopedT>
         struct call_member_function<scopedT *> : policy_base<scopedT *>
         {
            typedef scopedT * scoped_type;
            typedef scoped_type policy_type;
            typedef void (scopedT::*member_func_t)();

            call_member_function(scoped_type scoped, member_func_t mf)
               : policy_base<policy_type>(scoped), mf_(mf)
            {
            }

            explicit call_member_function(scoped_type scoped)
               : policy_base<policy_type>(scoped), mf_(&scopedT::operator())
            {
            }

            void operator()() const
            {
               if (mf_)
               {
                  // call member function on scoped object
                  (this->scoped_->*mf_)();
               }
            }

         private:
            member_func_t mf_;
         };
         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// Calls a free function or functor, passing it scoped by *value*.
         template <typename scopedT>
         struct call_free_function_with_val : policy_base_with_get<scopedT>
         {
            typedef scopedT scoped_type;
            typedef scopedT policy_type;
            typedef  boost::function<void(scoped_type const &)> func_t;

            call_free_function_with_val(scoped_type const & scoped, func_t f_)
               : policy_base_with_get<policy_type>(scoped), f_(f_)
            {
            }

            void operator()() const
            {
               if (f_)
               {
                  // call free function with scoped object
                  f_(this->scoped_);
               }
            }

         private:
            func_t f_;
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// Calls a free function or functor, passing it scoped by *reference*.
         template <typename scopedT>
         struct call_free_function_with_ref : policy_base_with_get<boost::reference_wrapper<scopedT> >
         {
            typedef scopedT scoped_type;
            typedef boost::reference_wrapper<scopedT> policy_type;
            typedef boost::function<void(scoped_type &)> func_t;

            call_free_function_with_ref(scoped_type & scoped, func_t f_)
               : policy_base_with_get<policy_type>(policy_type(scoped)), f_(f_)
            {
            }

            void operator()() const
            {
               if (f_)
               {
                  // call free function with scoped object
                  f_(this->scoped_.get());
               }
            }

         private:
            func_t f_;
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// Calls free on pointer to release a malloc/realloc allocation
         template <typename scopedT>
         struct free_malloc : call_free_function_with_val<scopedT *>
         {
            typedef scopedT * scoped_type;
            typedef scoped_type policy_type;
            typedef call_free_function_with_val<scopedT *> base_policy;

            explicit free_malloc(scoped_type const & scoped)
               : call_free_function_with_val<scoped_type>(scoped, ::free)
            {
               // [6/4/2011 ricky] Yes, I'm afraid you *will* have to cast the direct output from malloc
               //                  to that of the scoped type. This is C++ not C so get over it. Besides,
               //                  what are you playing at using malloc anyway in C++ You'd better have a
               //                  darn good reason or the code police will be on your case. Hope you know
               //                  exactly what you're getting yourself into here - good luck :)
               //
               //                  PS. I did consider adding a void * constructor but that would have
               //                      lead to a situation where you could declare a scoped pointer to
               //                      one type and then initialise things with a pointer to another.
               //                      This would have been a recipe for disaster do I didn't =/
            }
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         /// Calls a boost::function0<void> functor
         struct call_functor : policy_base<boost::function0<void> >
         {
            typedef boost::function0<void> scoped_type;
            typedef scoped_type policy_type;

            explicit call_functor(scoped_type const & scoped)
               : policy_base<policy_type>(policy_type(scoped))
            {
            }

            void operator()() const
            {
               if (this->scoped_)
               {
                  this->scoped_();
               }
            }
         };

         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
         // *** OR CREATE YOUR OWN POLICY AS REQUIRED
         // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
      }

      /// This is the basic scope exit class, used when supplying custom policies

      template <
         typename policyT
      >
      struct basic_scope_exit
      {
         typedef policyT policy_t;
         typedef typename policy_t::policy_type policy_type;
         typedef typename policy_t::scoped_type scoped_type;

         template<typename scoped_typeT>
         explicit basic_scope_exit(scoped_typeT & scoped) :
            exit_policy_(scoped), cancel_(false)
         {
         }

         template<typename scoped_typeT>
         explicit basic_scope_exit(scoped_typeT const & scoped) :
         exit_policy_(scoped), cancel_(false)
         {
         }

         template<typename scoped_typeT, typename policy_dataT>
         basic_scope_exit(scoped_typeT & scoped, policy_dataT & policy_data) :
            exit_policy_(scoped, policy_data), cancel_(false)
         {
         }

         template<typename scoped_typeT, typename policy_dataT>
         basic_scope_exit(scoped_typeT const & scoped, policy_dataT & policy_data) :
            exit_policy_(scoped, policy_data), cancel_(false)
         {
         }

         template<typename scoped_typeT, typename policy_dataT>
         basic_scope_exit(scoped_typeT & scoped, policy_dataT const & policy_data) :
         exit_policy_(scoped, policy_data), cancel_(false)
         {
         }

         template<typename scoped_typeT, typename policy_dataT>
         basic_scope_exit(scoped_typeT const & scoped, policy_dataT const & policy_data) :
         exit_policy_(scoped, policy_data), cancel_(false)
         {
         }

         basic_scope_exit & operator = (policy_t const & exit_policy)
         {
            exit_policy_ = exit_policy;
         }

         void cancel(bool cancel = true)
         {
            cancel_ = cancel;
         }

         bool is_cancelled() const
         {
            return cancel_;
         }

         policy_t & operator *()
         {
            return exit_policy_;
         }

         policy_t const & operator *() const
         {
            return exit_policy_;
         }

         policy_t * operator ->()
         {
            return &exit_policy_;
         }

         policy_t const * operator ->() const
         {
            return &exit_policy_;
         }

         ~basic_scope_exit()
         {
            try
            {
               if(!cancel_) exit_policy_();
            }
            catch(...) {}
         }

      private:
         policy_t exit_policy_;
         bool cancel_;
      };


      /// when scoped exits call this boost::function0<void> functor
      typedef basic_scope_exit<policy::call_functor> call_functor;

      /// This is the general scope exit class, with the various policies ready for use
      template <typename scopedT>
      struct type
      {
         /// when scoped exits set scoped member to a default constructed value
         typedef basic_scope_exit<policy::set_default_value<scopedT> > set_default_value;

         /// when scoped exits set scoped member to a specific value
         typedef basic_scope_exit<policy::set_specific_value<scopedT> > set_specific_value;

         /// when scoped exits set scoped member to its original value
         typedef basic_scope_exit<policy::restore_original_value<scopedT> > restore_original_value;

         /// when scoped exits call a member function and pass it scoped member
         typedef basic_scope_exit<policy::call_member_function<scopedT> > call_member_function;

         /// when scoped exits call a free function and pass it scoped member by value
         typedef basic_scope_exit<policy::call_free_function_with_val<scopedT> > call_free_function_with_val;

         /// when scoped exits call a free function and pass it scoped member by reference
         typedef basic_scope_exit<policy::call_free_function_with_ref<scopedT> > call_free_function_with_ref;

         /// when scoped exits call free on the scoped member (which must be a pointer pointing to malloc/realloc'd memory)
         typedef basic_scope_exit<policy::free_malloc<scopedT> > free_malloc;

         private: type();
      };
   }
}}

#endif // MOOST_UTILS_SCOPE_EXIT_HPP__
