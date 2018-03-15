// callgraph/detail/node_traits.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_TRAITS_HPP
#define CALLGRAPH_DETAIL_NODE_TRAITS_HPP

#ifndef NO_DOC

namespace callgraph {
    namespace detail {

        template <typename R, typename... Args>
        struct node_traits_base {
            using result_type = R;
            using signature = R(Args...);
            enum {
                arity = sizeof...(Args)
            };
        };

        template <typename T>
        struct node_traits : node_traits<decltype(&T::operator())>
        {};

        template <typename R, typename... Args>
        struct node_traits<R (*)(Args...)> : node_traits_base<R, Args...>
        {};

        template <typename R, typename C, typename... Args>
        struct node_traits<R (C::*)(Args...)> : node_traits_base<R, Args...>
        {};

        template <typename R, typename C, typename... Args>
        struct node_traits<R (C::*)(Args...) const> : node_traits_base<R, Args...>
        {};
    }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_NODE_TRAITS_HPP
