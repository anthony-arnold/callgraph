// callgraph/detail/connect_vertex.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_CONNECT_VERTEX_HPP
#define CALLGRAPH_DETAIL_CONNECT_VERTEX_HPP

#include <callgraph/graph.hpp>
#include <callgraph/vertex.hpp>
#include <callgraph/detail/node_traits.hpp>
#include <callgraph/detail/unwrap_vertex.hpp>

#ifndef NO_DOC
namespace callgraph {
    namespace detail {

        template <typename T, typename U, typename>
        struct connect_vertex_t {
            static constexpr auto apply(vertex<T>&& t, U&& u) -> decltype(auto) {
                constexpr auto c = &graph::connect<0, vertex<T>, U>;
                return (t.owner().*c)(std::forward<vertex<T>>(t),
                                    std::forward<U>(u));
            }
        };
        template <typename T, typename U>
        struct connect_vertex_t<T, U, void> {
            static constexpr auto apply(vertex<T>&& t, U&& u) -> decltype(auto) {
                constexpr auto c = &graph::connect<vertex<T>, U>;
                return (t.owner().*c)(std::forward<vertex<T>>(t),
                                    std::forward<U>(u));
            }
        };

        template <typename T, typename U>
        constexpr auto connect_vertex(vertex<T>&& t, U&& u) -> decltype(auto) {
            using t_type = typename std::decay<T>::type;
            using r_type = typename node_traits<t_type>::result_type;
            return connect_vertex_t<T, U, r_type>::apply(
                std::forward<vertex<T>>(t), std::forward<U>(u));
        }
    }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_CONNECT_VERTEX_HPP
