// callgraph/detail/unwrap_vertex.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_UNWRAP_VERTEX_HPP
#define CALLGRAPH_DETAIL_UNWRAP_VERTEX_HPP

#ifndef NO_DOC
namespace callgraph {
    template <typename T>
    struct vertex;

    namespace detail {

        template <typename T, typename>
        struct unwrap_vertex_impl {
            using type = T;

            template <typename U>
            static constexpr auto apply(U&& u) -> decltype(auto) {
                return std::forward<U>(u);
            }
        };

        template <typename T, typename U>
        struct unwrap_vertex_impl<T, vertex<U> > {
            using type = U;

            template <typename V>
            static constexpr auto apply(V&& v) -> decltype(auto) {
                return v.impl();
            }
        };

        template <typename T>
        struct unwrap_vertex
            : unwrap_vertex_impl<T, typename std::decay<T>::type> {
        };
    }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_UNWRAP_VERTEX_HPP
