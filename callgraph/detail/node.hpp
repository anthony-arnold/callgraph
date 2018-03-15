// callgraph/detail/node.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_HPP
#define CALLGRAPH_DETAIL_NODE_HPP

#include <callgraph/detail/node_call.hpp>
#include <callgraph/detail/node_param_list.hpp>
#include <callgraph/detail/node_traits.hpp>
#include <callgraph/detail/node_value.hpp>

#ifndef NO_DOC
namespace callgraph { namespace detail {

        template <typename T>
        struct node_base;

        template <typename R, typename... Params>
        struct node_base<R (Params...)> {
            node_base()
                {
                }

            template <size_t To, typename T>
            void connect(node_base<T>& source) {
                params_.connect<To>(source.result_);
            }

            template <size_t From, size_t To, typename T>
            void connect(node_base<T>& source) {
                params_.connect<From, To>(source.result_);
            }

            template <typename T>
            void call(T& t) {
                using signature = typename node_traits<T>::signature;
                node_call<signature>::apply(t, params_, result_);
            }

            bool valid() const {
                return params_.valid();
            }

            void reset() {
                result_.reset();
            }

            node_value<R> result_;
            node_param_list<Params...> params_;
        };

        template <typename R>
        struct node_base<R ()> {
            node_base()
                : input_(nullptr)
                {
                }

            template <typename T>
            void connect(node_base<T>& source) {
                input_ = &source.result_;
            }

            template <typename T>
            void call(T& t) {
                using signature = typename node_traits<T>::signature;
                node_call<signature>::apply(t, input_, result_);
            }

            void reset() {
                result_.reset();
            }

            bool valid() const {
                return input_ != nullptr;
            }

            node_value<void>* input_;
            node_value<R> result_;
        };

        template <typename T>
        struct node
            : node_base<typename node_traits<
                            typename std::decay<T>::type>::signature> {
        public:
            using type = typename std::decay<T>::type;
            using traits_type = node_traits<type>;
            using signature = typename traits_type::signature;
            using base_type = node_base<signature>;


            node(T&& t)
                : fn_(std::forward<T>(t))
                {
                }

            void operator()() {
                base_type::call(fn_);
            }

            void reset() {
                base_type::reset();
            }

            bool valid() const {
                return base_type::valid();
            }

        private:
            type fn_;
        };

    }
}
#endif
#endif // CALLGRAPH_DETAIL_NODE_HPP
