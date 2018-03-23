// callgraph/detail/node_param_list.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_PARAM_LIST_HPP
#define CALLGRAPH_DETAIL_NODE_PARAM_LIST_HPP

#include <callgraph/error.hpp>
#include <callgraph/detail/node_value.hpp>

#include <tuple>
#include <memory>

#ifndef NO_DOC
namespace callgraph {
    namespace detail {

        template <typename T, size_t N>
        struct node_param_list_valid_t {
            static bool apply(const T& t) {
                return static_cast<bool>(std::get<N>(t)) &&
                    node_param_list_valid_t<T, N-1>::apply(t);
            }
        };

        template <typename T>
        struct node_param_list_valid_t<T, 0> {
            static bool apply(const T& t) {
                return static_cast<bool>(std::get<0>(t));
            }
        };

        template <typename T>
        bool node_param_list_valid(const T& t) {
            enum {
                size = std::tuple_size<T>::value
            };
            return node_param_list_valid_t<T, size-1>::apply(t);
        }

        template <size_t N, typename Param, typename... Params>
        struct node_param_type : node_param_type<N - 1, Params...>
        {
        };

        template <typename Param, typename... Params>
        struct node_param_type<0, Param, Params...> {
            using type = Param;
        };

        template <typename... Params>
        struct node_param_list {
            template <size_t To, typename T>
            void connect(node_value<T>& source) {
                using dst_t = typename node_param_type<To, Params...>::type;
                using src_t = T;
                using type = node_value_ref<dst_t, src_t, -1>;
                using std::get;
                get<To>(params_).reset(new type(source));
            }

            template <size_t From, size_t To, typename T>
            void connect(node_value<T>& source) {
                using dst_t = typename node_param_type<To, Params...>::type;
                using src_t = T;
                using type = node_value_ref<dst_t, src_t, From>;
                using std::get;
                get<To>(params_).reset(new type(source));
            }

            template <size_t N>
            typename node_param_type<N, Params...>::type get() const {
                using std::get;

                const auto& p(get<N>(this->params_));
                if (!p) {
                    CALLGRAPH_THROW(node_parameter_missing);
                }
                return p->get();
            }

            bool valid() const {
                return node_param_list_valid(params_);
            }

            std::tuple<std::shared_ptr<node_value_ref_base<Params>>...> params_;
        };

        template <size_t N, typename... Params>
        typename node_param_type<N, Params...>::type
        get_node_params(const node_param_list<Params...>& list) {
            return list.template get<N>();
        }

    }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_NODE_PARAM_LIST_HPP
