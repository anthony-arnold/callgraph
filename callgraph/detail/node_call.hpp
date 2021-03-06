// callgraph/detail/node_call.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_CALL_HPP
#define CALLGRAPH_DETAIL_NODE_CALL_HPP

#include <callgraph/detail/node_param_list.hpp>
#include <callgraph/detail/node_traits.hpp>
#include <callgraph/detail/node_value.hpp>

#ifndef NO_DOC
namespace callgraph {
    namespace detail {

        template <size_t...>
        struct node_call_sequence {};

        template <size_t M, size_t... N>
        struct generate_node_call_sequence
            : generate_node_call_sequence<M - 1, M - 1, N...>
        {
        };

        template <size_t... N>
        struct generate_node_call_sequence<0, N...>
        {
            using type = node_call_sequence<N...>;
        };

        template <typename T>
        struct node_call;

        template <typename R, typename... Args>
        struct node_call<R(Args...)> {
            template <typename T, typename U, typename V>
            static void apply(T& t, U& params, V& result) {
                using sequence_type =
                    typename generate_node_call_sequence<node_traits<T>::arity>::type;
                result.set(apply(t, params, sequence_type()));
            }

            template <typename T, typename U, size_t... N>
            static R apply(T& t, U& params, node_call_sequence<N...>) {
                return t(get_node_params<N>(params)...);
            }
        };

        template <typename... Args>
        struct node_call<void(Args...)> {
            template <typename T, typename U, typename V>
            static void apply(T& t, U& params, V& result) {
                using sequence_type =
                    typename generate_node_call_sequence<node_traits<T>::arity>::type;
                apply(t, params, sequence_type());
                result.set();
            }

            template <typename T, typename U, size_t... N>
            static void apply(T& t, U& params, node_call_sequence<N...>) {
                t(get_node_params<N>(params)...);
            }
        };

        template <typename R>
        struct node_call<R()> {
            template <typename T, typename U, typename V>
            static void apply(T& t, U& future, V& result) {
                if (future) {
                    future->wait();
                }
                result.set(t());
            }
        };

        template <>
        struct node_call<void()> {
            template <typename T, typename U, typename V>
            static void apply(T& t, U& future, V& result) {
                if (future) {
                    future->wait();
                }
                t();
                result.set();
            }
        };

    }
}

#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_NODE_CALL_HPP
