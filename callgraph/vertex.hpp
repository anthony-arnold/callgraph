// callgraph/vertex.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_VERTEX_HPP
#define CALLGRAPH_VERTEX_HPP

namespace callgraph {
    class graph;

    /// \brief A vertex wraps a callable already present in a graph.
    /// It is useful because it can be supplied by an interface
    /// without having to supply a declaration of the callable it
    /// wraps. It can then be passed to the same graph to forge
    /// further connections.
    ///
    /// This type is returned from calls to graph::connect.
    template <typename T>
    struct vertex {
        /// \brief Construct a new vertex.
        /// \param impl The function the vertex wraps.
        /// \param g The graph the function is connected to.
        constexpr vertex(T& impl, graph& g)
            : impl_(impl), g_(g)
            {
            }

        /// \brief Copy-construct a vertex.
        constexpr vertex(const vertex<T>&) = default;

        /// \brief Copy-assign a vertex.
        constexpr vertex<T>& operator=(const vertex<T>&) = default;

        /// \brief Move-construct a vertex.
        constexpr vertex(vertex<T>&&) = default;

        /// \brief Move-assign a vertex.
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
