// callgraph/to.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_TO_HPP
#define CALLGRAPH_TO_HPP

#include <callgraph/detail/to_connector.hpp>

namespace callgraph {
/// \brief Specify the destination of the result of the
    /// previous function in the stream in terms of the parameter index.
    ///
    /// This function is used during a chain of connection operators
    /// (right-shift operators). It indicates that the result of the
    /// previous function is to connect to a parameter in the next function,
    /// where the next function takes multiple parameters. The usual rules for
    /// connections apply.
    /// \tparam To The parameter index (0-based) that the value is to be
    /// connected to.
    /// \tparam T The type of the function that will take the value.
    /// \param t The function that is being connected to.
    /// \return undefined
    template <size_t To, typename T>
    constexpr auto to(T&& t) -> decltype(auto) {
        return detail::to_connector<T, To>(std::forward<T>(t));
    }
}

#include <callgraph/operators.hpp>

#endif // CALLGRAPH_TO_HPP
