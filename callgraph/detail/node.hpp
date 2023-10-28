// callgraph/detail/node.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_NODE_HPP
#define CALLGRAPH_DETAIL_NODE_HPP

#include <callgraph/detail/node_call.hpp>
#include <callgraph/detail/node_param_list.hpp>
#include <callgraph/detail/node_traits.hpp>
#include <callgraph/detail/node_value.hpp>

#include <typeindex>
#include <typeinfo>

#ifndef NO_DOC
namespace callgraph { namespace detail {

        struct node_base {
            virtual ~node_base() = default;
            virtual const std::type_info& type_info() const = 0;
            virtual bool less(const node_base* other) const = 0;
            virtual bool less(const node_base& other) const = 0;
        };

        template <typename>
        struct node_impl;

        template <typename R, typename... Params>
        struct node_impl<R (Params...)> : node_base {
            node_impl()
                {
                }

            template <size_t To, typename T>
            void connect(node_impl<T>& source) {
                params_.template connect<To>(source.result_);
            }

            template <size_t From, size_t To, typename T>
            void connect(node_impl<T>& source) {
                params_.template connect<From, To>(source.result_);
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
        struct node_impl<R ()> : node_base {
            node_impl()
                : input_(nullptr)
                {
                }

            template <typename T>
            void connect(node_impl<T>& source) {
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
            : node_impl<typename node_traits<
                            typename std::decay<T>::type>::signature> {
        public:
            using type = typename std::decay<T>::type;
            using traits_type = node_traits<type>;
            using signature = typename traits_type::signature;
            using base_type = node_impl<signature>;


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

            const std::type_info& type_info() const override {
                return typeid(T);
            }

            bool less(const node_base& other) const override {
                const auto& lht = this->type_info();
                const auto& rht = other.type_info();

                if (lht == rht) {
                    const auto& f = fn();
                    const auto& g = static_cast<const node<T>*>(&other)->fn();
                    return &f < &g;
                }
                else {
                    return std::type_index(lht) < std::type_index(rht);
                }
            }

            bool less(const node_base* other) const override {
                if (other != nullptr) {
                    return less(*other);
                }
                return false;
            }

            const type& fn() const {
                return fn_;
            }
        private:
            type fn_;
        };

    }
}
#endif
#endif // CALLGRAPH_DETAIL_NODE_HPP
