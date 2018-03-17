// callgraph/detail/from_to_connector.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_FROM_TO_CONNECTOR_HPP
#define CALLGRAPH_DETAIL_FROM_TO_CONNECTOR_HPP

#include <callgraph/detail/base_connector.hpp>

#ifndef NO_DOC
namespace callgraph {
    namespace detail {
        template <typename T, size_t From, size_t To>
        class from_to_connector : public base_connector<T> {
        public:
            constexpr from_to_connector(T&& v)
                : base_connector<T>(std::forward<T>(v)) {}
        };
    }
}

#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_FROM_TO_CONNECTOR_HPP
