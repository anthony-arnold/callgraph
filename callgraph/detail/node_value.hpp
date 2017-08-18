// callgraph/detail/node_value.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_VALUE_HPP
#define CALLGRAPH_DETAIL_NODE_VALUE_HPP

#include <future>
#include <utility>

#ifndef NO_DOC
namespace callgraph {
   namespace detail {

      template <typename T>
      struct node_value_base {
            node_value_base()
               : future_(promise_.get_future().share())
            {
            }

            void wait() {
               future_.wait();
            }

            void reset() {
               promise_ = std::move(std::promise<T>());
               future_ = promise_.get_future().share();
            }

            std::promise<T> promise_;
            std::shared_future<T> future_;
      };

      template <typename T>
      struct node_value : node_value_base<T> {
            const T& get() const {
               return node_value_base<T>::future_.get();
            }

            void set(T&& t) {
               node_value_base<T>::promise_.set_value(std::forward<T>(t));
            }
      };

      template <>
      struct node_value<void> : node_value_base<void> {
            void set() {
               node_value_base<void>::promise_.set_value();
            }
      };

      template <typename T, size_t N>
      struct ref_traits
         : std::tuple_element<N, T>
      {
      };

      template <typename T>
      struct ref_traits<T, -1> {
         using type = T;
      };

      template <typename T>
      struct node_value_ref_base {
            using type = T;
            virtual type get() const = 0;
      };

      template <typename T, typename U, size_t N>
      struct node_value_ref :
            node_value_ref_base<T>
      {
            using type = T;

            node_value_ref(node_value<U>& ref)
               : ref_(ref)
            {
            }

            type get() const override {
               using std::get;
               return static_cast<type>(get<N>(ref_.get()));
            }

            void wait() {
               ref_.wait();
            }

            node_value<U>& ref_;
      };

      template <typename T, typename U>
      struct node_value_ref<T, U, -1> : node_value_ref_base<T>
      {
         using type = T;

         node_value_ref(node_value<U>& ref)
            : ref_(ref)
         {
         }

         type get() const override {
            return static_cast<type>(ref_.get());
         }

         void wait() {
            ref_.wait();
         }

         node_value<U>& ref_;
      };

   }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_NODE_VALUE_HPP
