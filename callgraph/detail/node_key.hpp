// callgraph/detail/node_key.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_KEY_HPP
#define CALLGRAPH_DETAIL_NODE_KEY_HPP

#include <callgraph/detail/opaque_node.hpp>

#ifndef NO_DOC
namespace callgraph {
   namespace detail {
      using node_key = void*;

      template <typename T>
      struct to_node_key_impl
      {
            static node_key apply(T& t) {
               return &t;
            }
      };

      template <typename T>
      struct to_node_key_impl<T*>
      {
            static node_key apply(T* t) {
               return *reinterpret_cast<node_key*>(&t);
            }
      };

      template <typename R, typename... Args>
      struct to_node_key_impl<R (Args...)>
      {
            template <typename T>
            static node_key apply(T&& t) {
                return &t;
            }
      };

      template <typename T>
      struct to_node_key_impl<opaque_node<T> > {
            static node_key apply(opaque_node<T>& node) {
               return to_node_key_impl<T>::apply(node.impl_);
            }
      };

      template <typename T>
      struct to_node_key :
            to_node_key_impl<typename std::decay<T>::type>
      {
      };
   }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_NODE_KEY_HPP
