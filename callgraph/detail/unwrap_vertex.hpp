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

            static constexpr type& apply(T& t) {
                return t;
            }
        };

        template <typename T, typename U>
        struct unwrap_vertex_impl<T, vertex<U> > {
            using type = U;

            static constexpr type& apply(T& t) {
                return t.impl();
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
