// callgraph/detail/graph_node.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_GRAPH_NODE_HPP
#define CALLGRAPH_DETAIL_GRAPH_NODE_HPP

#include <callgraph/detail/invoke_once.hpp>
#include <callgraph/detail/node.hpp>
#include <callgraph/vertex.hpp>
#include <callgraph/detail/unwrap_vertex.hpp>


#include <functional>
#include <stack>
#include <typeindex>
#include <unordered_set>

#ifndef NO_DOC
namespace callgraph {
    class graph;

    namespace detail {

        template <typename>
        struct function_less_t {
            template <typename T, typename U>
            static bool apply(T&& t, U&& u) {
                return &t < &u;
            }
        };


        template <typename V>
        struct function_less_t<V*> {
            template <typename T, typename U>
            static bool apply(T&& t, U&& u) {
                return t < u;
            }
        };

        template <typename T>
        struct function_less : function_less_t<T> {};

        struct graph_node {
            friend struct graph_worker;
            friend class callgraph::graph;

            template <typename T>
            struct node_deleter {
                void operator()(void* ptr) {
                    delete static_cast<node<T>*>(ptr);
                }
            };

            template <typename T>
            struct node_resetter {
                void operator()(void* ptr) {
                    static_cast<node<T>*>(ptr)->reset();
                }
            };

            template <typename T>
            struct node_validator {
                bool operator()(void* ptr) {
                    return static_cast<node<T>*>(ptr)->valid();
                }
            };

            template <typename T>
            struct node_executor {
                using type = node<T>*;

                node_executor(node_base* ptr) : ptr_(ptr) {
                }

                void operator()() {
                    (*static_cast<type>(ptr_))();
                }

                node_base* ptr_;
            };

            template <typename T, typename U>
            graph_node(T&& t, U)
                : node_(new node<T>(std::forward<T>(t)),
                        node_deleter<T>()),
                  exec_fn_(node_executor<T>(node_.get())),
                  validator_fn_(node_validator<T>()),
                  resetter_fn_(node_resetter<T>())
                {
                }


            graph_node(graph_node&& g) = default;
            graph_node& operator=(graph_node&& g) = default;
            graph_node(const graph_node& g) = default;
            graph_node& operator=(const graph_node& g) = default;
/*
            template <typename T>
            constexpr auto to_node() const -> decltype(auto) {
                return static_cast<node<T>*>(node_.get());
            }
*/
            node_base* to_node() const {
                return node_.get();
            }

            bool valid() const {
                return validator_fn_(node_.get());
            }

            template <typename R>
            bool run(R& runner) const{
                bool executed(exec_fn_());
                if (executed) {
                    for (const graph_node* child : children_) {
                        runner.enqueue_node(child);
                    }
                }
                return executed;
            }

            void reset() const {
                exec_fn_.reset();
                resetter_fn_(node_.get());
            }

            void add_child(const graph_node& child)  {
                children_.insert(&child);
            }

            size_t depth() const {
                size_t d(0);
                for (const graph_node* child : children_) {
                    d += child->depth();
                }
                return d > 0 ? d : 1;
            }

            bool makes_cycle(const graph_node* candidate_child) const {
                bool cycle(false);
                if (this == candidate_child) {
                    cycle = true;
                }
                else if (path_exists(candidate_child, this)) {
                    cycle = true;
                }
                return cycle;
            }

            template <typename T>
            bool holds(T&& t) const {
                if (node_) {
                    auto lhs = std::type_index(node_->type_info());
                    auto rhs = std::type_index(typeid(std::forward<T>(t)));

                    if (lhs == rhs) {
                        auto fn = static_cast<node<T>*>(node_.get())->fn();
                        return fn == t;
                    }
                }
                return false;
            }

            template <typename T>
            friend bool operator <(const graph_node& g, T&& t) {
                using dtype = typename std::decay<
                    typename unwrap_vertex<T>::type>::type;
                bool is_less(true);
                if (g.node_) {
                    auto lhs = std::type_index(g.node_->type_info());
                    auto rhs = std::type_index(typeid(dtype));

                    if (lhs == rhs) {
                        const auto& fn =
                            static_cast<node<dtype>*>(g.node_.get())->fn();
                        is_less = function_less<dtype>::apply(
                            fn, unwrap_vertex<T>::apply(std::forward<T>(t)));
                    }
                    else {
                        is_less = lhs < rhs;
                    }
                }
                return is_less;
            }

            friend bool operator <(const graph_node& l, const graph_node& r) {
                bool is_less(false);
                if (l.node_) {
                    is_less = l.node_->less(r.node_.get());
                }
                else if (r.node_) {
                    is_less = true;
                }
                return is_less;
            }

            friend bool has_child(const graph_node* a, const graph_node* b) {
                return a->children_.find(b) != a->children_.end();
            }

            friend bool path_exists(const graph_node* a, const graph_node* b) {
                return longest_path(a, b) > 0;
            }

            friend int longest_path(const graph_node* a, const graph_node* b) {
                int distance(0);
                if (has_child(a, b)) {
                    distance = 1;
                }
                for (const graph_node* child : a->children_) {
                    int dsub(longest_path(child, b));
                    if (dsub > 0) {
                        dsub++;
                    }
                    if (dsub > distance) {
                        distance = dsub;
                    }
                }
                return distance;
            }
        private:
            std::unordered_set<const graph_node*> children_;
            std::shared_ptr<node_base> node_;
            mutable invoke_once exec_fn_;
            std::function<bool(void*)> validator_fn_;
            mutable std::function<void(void*)> resetter_fn_;
        };

        template <typename T, typename U>
        struct graph_node_less_impl {
            static bool apply(const T& t, const U& u) {
                return node<T>(t).less(node<U>(u));
            }
        };

        template <typename T>
        struct graph_node_less_impl<T, graph_node> {
            static bool apply(const T& t, const graph_node& g) {
                return !(g < t);
            }
        };
        template <typename T>
        struct graph_node_less_impl<graph_node, T> {
            static bool apply(const graph_node& g, const T& t) {
                return g < t;
            }
        };
        template <>
        struct graph_node_less_impl<graph_node, graph_node> {
            static bool apply(const graph_node& l, const graph_node& r) {
                return l < r;
            }
        };
        struct graph_node_less {
            using is_transparent = std::true_type;

            template <typename L, typename R>
            bool operator()(L&& l, R&& r) const {
                return graph_node_less_impl<
                    typename std::decay<L>::type,
                    typename std::decay<R>::type>::apply(std::forward<L>(l),
                                                         std::forward<R>(r));
            }
        };

        template <typename T, typename>
        struct make_graph_node_impl
        {
            template <typename U>
            static graph_node apply(U&& t) {
                return graph_node(std::forward<U>(t), 0);
            }
        };

        template <typename T, typename U>
        struct make_graph_node_impl<T, vertex<U> >
        {
            static graph_node apply(const vertex<U>& n) {
                return graph_node(static_cast<U>(n.impl()), 0);
            }
        };

        template <typename T>
        graph_node make_graph_node(T&& t) {
            return make_graph_node_impl<T,
                                        typename std::decay<T>::type>::apply(t);
        }

        template <typename T>
        node<T>* to_node(const graph_node& gn) {
            return static_cast<node<T>*>(gn.to_node());
        }
    }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_GRAPH_NODE_HPP
