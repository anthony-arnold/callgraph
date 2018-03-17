// callgraph/from.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_FROM_HPP
#define CALLGRAPH_FROM_HPP

#include <callgraph/detail/from_to_connector.hpp>

namespace callgraph {

    /// \brief A type that designated the "from" index in a connection
    /// operator (`>>`) chain when connecting a component from one node's
    /// return value to an indexed parameter of another node.
    ///
    /// To specify the "to" index (the parameter index of the receiving node),
    /// use the static `to` function in this class.
    /// \tparam From The index of the previous function's return value component
    /// to connect to the next function.
    template <size_t From>
    struct from {
        /// \brief Specify the destination of the result component `From` of the
        /// previous function in the stream in terms of the parameter index.
        ///
        /// This function is used during a chain of connection operators
        /// (right-shift operators). It indicates that `From` component of
        /// the result of the previous function is to connect to a parameter
        /// in the next function, where the next function takes multiple
        /// parameters and the previous function returns a tuple-like value.
        /// The usual rules for  connections apply.
        /// \tparam To The parameter index (0-based) that the value is to be
        /// connected to.
        /// \tparam T The type of the function that will take the value.
        /// \param t The function that is being connected to.
        /// \return undefined
        template <size_t To, typename T>
        static constexpr auto to(T&& t) -> decltype(auto) {
            return detail::from_to_connector<T, From, To>(std::forward<T>(t));
        }
    };
}

#include <callgraph/operators.hpp>

#endif // CALLGRAPH_FROM_HPP
