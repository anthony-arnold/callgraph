// callgraph/vertex.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_VERTEX_HPP
#define CALLGRAPH_VERTEX_HPP

namespace callgraph {
    class graph;

    /// A vertex wraps a callable already present in a graph.
    /// It is useful because it can be supplied by an interface
    /// without having to supply a declaration of the callable it
    /// wraps. It can then be passed to the same graph to forge
    /// further connections.
    ///
    /// This type is returned from calls to graph::connect.
    template <typename T>
    struct vertex {
        constexpr vertex(T& impl, graph& g)
            : impl_(impl), g_(g)
            {
            }

        constexpr vertex(const vertex<T>&) = default;
        constexpr vertex<T>& operator=(const vertex<T>&) = default;

        constexpr vertex(vertex<T>&&) = default;
        constexpr vertex<T>& operator=(vertex<T>&&) = default;


        /// Get the underlying implementation.
        constexpr T& impl() const {
            return impl_;
        }

        /// Get the graph this vertex is connected to.
        constexpr graph& owner() const {
            return g_;
        }

    private:
        T& impl_;
        graph& g_;
    };
}
#endif // CALLGRAPH_VERTEX_HPP
