// callgraph/operators.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_OPERATORS_HPP
#define CALLGRAPH_OPERATORS_HPP

#include <callgraph/graph.hpp>
#include <callgraph/detail/connect_vertex.hpp>
#include <callgraph/detail/from_to_connector.hpp>
#include <callgraph/detail/to_connector.hpp>

namespace callgraph {
    /// \brief Connect the result of node `t`
    /// to the parameter in the To index of the node wrapped by `con`.
    /// \tparam T The type of the node on the LHS.
    /// \tparam U The type of the node on the RHS.
    /// \tparam To The index of the parameter to connect.
    /// \param t The node on the LHS.
    /// \param con The wrapped node on the RHS.
    /// \return undefined
    template <typename T, typename U, size_t To>
    constexpr auto
    operator>>(vertex<T> t,
               detail::to_connector<U, To> con) -> decltype(auto) {
        return detail::connect_vertex<To>(t, con.vtx());
    }

    /// \brief Connect the result component in the From index of node `t`
    /// to the parameter in the To index of the node wrapped by `con`.
    /// \tparam T The type of the node on the LHS.
    /// \tparam U The type of the node on the RHS.
    /// \tparam From The index of the result component to connect.
    /// \tparam To The index of the parameter to connect.
    /// \param t The node on the LHS.
    /// \param con The wrapped node on the RHS.
    /// \return undefined
    template <typename T, typename U, size_t From, size_t To>
    constexpr auto
    operator>>(vertex<T> t,
               detail::from_to_connector<U, From, To> con) -> decltype(auto) {
        return detail::connect_vertex<From, To>(t, con.vtx());
    }

    /// \brief Connect function object `t` to the root node of graph `g`.
    /// \tparam T A Callable type which takes no parameters, or a
    /// node wrapper which wraps such a type.
    /// \return A node wrapper which can be used as a handle to the
    /// node represented by `t`. Future calls may use either `t` or
    /// the returned object to make connections to 't'.
    template <typename T>
    constexpr auto operator>>(graph& g, T&& t) -> decltype(auto) {
        return g.connect(t);
    }

    /// \brief Connect node `t` to function object `u`
    /// such that `t` becomes a dependency of `u`.
    /// \tparam T A Callable type.
    /// \tparam U A Callable type which takes zero or one parameters
    /// which corresponds to the return type of `T`.
    /// \return A node wrapper which can be used as a handle to the
    /// node represented by `u`. Future calls may use either `u` or
    /// the returned object to make connections to `u`. This operator
    /// may be chained in such a way.
    template <typename T, typename U>
    constexpr auto operator>>(vertex<T> t, U&& u) -> decltype(auto) {
        return detail::connect_vertex(
            std::forward<vertex<T>>(t), std::forward<U>(u));
    }
}

#endif // CALLGRAPH_OPERATORS_HPP
