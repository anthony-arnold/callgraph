// callgraph/detail/node_key.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_KEY_HPP
#define CALLGRAPH_DETAIL_NODE_KEY_HPP

#include <callgraph/vertex.hpp>

#ifndef NO_DOC
namespace callgraph {
    namespace detail {
        using node_key = const void*;

        template <typename>
        struct to_node_key_impl
        {
            template <typename T>
            static node_key apply(T&& t) {
               return reinterpret_cast<node_key>(&t);
            }
        };

        template <typename T>
        struct to_node_key_impl<T*>
        {
            static node_key apply(T* t) {
                return reinterpret_cast<node_key>(t);
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
        struct to_node_key_impl<vertex<T> > {
            template <typename V>
            static node_key apply(V&& node) {
                return to_node_key_impl<T>::apply(node.impl());
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
