// callgraph/detail/base_connector.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_BASE_CONNECTOR_HPP
#define CALLGRAPH_DETAIL_BASE_CONNECTOR_HPP
#ifndef NO_DOC

namespace callgraph {
    namespace detail {
        template <typename T>
        class base_connector {
        public:
            constexpr base_connector(T&& v) : vtx_(v) {}

            constexpr const T& vtx() const { return vtx_; }

        private:
            const T& vtx_;
        };
    }
}

#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_BASE_CONNECTOR_HPP
