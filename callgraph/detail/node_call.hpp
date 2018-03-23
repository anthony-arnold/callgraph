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

        template <int...>
        struct call_seq {};

        template <int M, int... N>
        struct gen_call_seq
            : gen_call_seq<M - 1, M - 1, N...>
        {
        };

        template <int... N>
        struct gen_call_seq<0, N...>
        {
            using type = call_seq<N...>;
        };

        template <typename T>
        struct node_call;

        template <typename R, typename... Args>
        struct node_call<R(Args...)> {
            template <typename T, typename U, typename V>
            static void apply(T&& t, U&& params, V&& result) {
                using t_type = typename std::decay<T>::type;
                static const auto arity = node_traits<t_type>::arity;
                using sequence_type = typename gen_call_seq<arity>::type;
                result.set(apply(std::forward<T>(t),
                                 std::forward<U>(params),
                                 sequence_type()));
            }

            template <typename T, typename U, int... N>
            static R apply(T&& t, U&& params, call_seq<N...>) {
                return t(get_node_params<N>(std::forward<U>(params))...);
            }
        };

        template <typename... Args>
        struct node_call<void(Args...)> {
            template <typename T, typename U, typename V>
            static void apply(T&& t, U&& params, V&& result) {
                using t_type = typename std::decay<T>::type;
                static const auto arity = node_traits<t_type>::arity;
                using sequence_type = typename gen_call_seq<arity>::type;
                apply(std::forward<T>(t),
                      std::forward<U>(params),
                      sequence_type());
                result.set();
            }

            template <typename T, typename U, int... N>
            static void apply(T&& t, U&& params, call_seq<N...>) {
                t(get_node_params<N>(std::forward<U>(params))...);
            }
        };

        template <typename R>
        struct node_call<R()> {
            template <typename T, typename U, typename V>
            static void apply(T&& t, U&& future, V&& result) {
                if (future) {
                    future->wait();
                }
                result.set(t());
            }
        };

        template <>
        struct node_call<void()> {
            template <typename T, typename U, typename V>
            static void apply(T&& t, U&& future, V&& result) {
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
