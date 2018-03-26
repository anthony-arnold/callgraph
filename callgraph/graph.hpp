// callgraph/graph.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_GRAPH_HPP
#define CALLGRAPH_GRAPH_HPP

#include <callgraph/error.hpp>
#include <callgraph/detail/graph_node.hpp>
#include <callgraph/detail/node.hpp>
#include <callgraph/detail/node_key.hpp>
#include <callgraph/detail/unwrap_vertex.hpp>
#include <callgraph/vertex.hpp>

#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>
#include <iostream>
/// \brief The main Callgraph namespace.
namespace callgraph {
    class graph_runner;

/// \brief A graph is a container of asynchronous executable nodes
/// joined into a directed acyclic graph.
///
/// The purpose of the graph
/// is to describe an asyncronous work pattern and information flow,
/// allowing the graph object to manage synchronisation and resources.
    class graph {
    public:
        /// \brief Construct an empty graph,
        /// consisting only of a no-op root node.
        graph()
            : root_(&graph::dummy),
              root_node_(make_root_node())
            {
            }

        graph(const graph&) = delete;
        /// \brief Move-construct a graph.
        graph(graph&& src) = default;

        graph& operator=(const graph&) = delete;
        /// \brief Move-assign a graph.
        graph& operator=(graph&&) = default;

        /// \brief Connect function object `t` to the root node.
        /// \tparam T A Callable type which takes no parameters, or a
        /// node wrapper which wraps such a type.
        /// \return A node wrapper which can be used as a handle to the
        /// node represented by `t`. Future calls may use either `t` or
        /// the returned object to make connections to 't'.
        template <typename T>
        constexpr auto connect(T&& t) -> decltype(auto) {
            return connect(root_, std::forward<T>(t));
        }

        /// \brief Connect functions `f` and `g`.
        /// \tparam F A Callable type which returns void, or a
        /// node wrapper which wraps such a type.
        /// \tparam G A Callable type which takes no parameters, or a
        /// node wrapper which wraps such a type.
        /// \throws cycle_error if connecting `f` to `g` forms a cycle.
        /// \throws source_node_not_found if `f` is not already connected
        /// to the graph.
        /// \return A node wrapper which can be used as a handle to the
        /// node represented by `g`. Future calls may use either `g` or
        /// the returned object to make connections to 'g'.
        template <typename F, typename G>
        constexpr auto connect(F&& f, G&& g) -> decltype(auto) {
            using f_type = typename detail::unwrap_vertex<F>::type;
            using g_type = typename detail::unwrap_vertex<G>::type;

            auto fit = prepare_connect(std::forward<F>(f), std::forward<G>(g));

            // Find or make the destination node.
            auto git = ensure_node(std::forward<G>(g));

            // Connect
            to_node<g_type>(*git)->connect(*to_node<f_type>(*fit));

            // Add dest node as a child.
            add_child(fit, *git);

            return make_vertex(std::forward<G>(g));
        }

        /// \brief Connect functions `f` and `g`.
        /// \tparam F A Callable type which returns non-void, or a
        /// node wrapper which wraps such a type.
        /// \tparam G A Callable type for which the parameter at
        /// index `To` is assignable from the type returned by `f(...)`, or a
        /// node wrapper which wraps such a type.
        /// \tparam To The index of the parameter of `g` to bind
        /// the result of `f`.
        /// \throws cycle_error if connecting `f` to `g` forms a cycle.
        /// \throws source_node_not_found if `f` is not already connected
        /// to the graph.
        /// \return A node wrapper which can be used as a handle to the
        /// node represented by `g`. Future calls may use either `g` or
        /// the returned object to make connections to 'g'.
        template <size_t To, typename F, typename G>
        constexpr auto connect(F&& f, G&& g) -> decltype(auto) {
            using f_type = typename detail::unwrap_vertex<F>::type;
            using g_type = typename detail::unwrap_vertex<G>::type;

            auto fit = prepare_connect(std::forward<F>(f), std::forward<G>(g));

            // Find or make the destination node.
            auto git = ensure_node(std::forward<G>(g));

            // Connect
            to_node<g_type>(*git)->template connect<To>(*to_node<f_type>(*fit));

            // Add dest node as a child.
            add_child(fit, *git);

            return make_vertex(std::forward<G>(g));
        }

        /// \brief Connect functions `f` and `g`.
        /// \tparam F A Callable type which returns a type for which
        /// `std::get<From>(f(...))` is defined, or a
        /// node wrapper which wraps such a type.
        /// \tparam G A Callable type for which the parameter at
        /// index `To` is assignable from the type returned by
        /// `std::get<From>(f(...))`, or a
        /// node wrapper which wraps such a type.
        /// \tparam To The index of the parameter of `g` to bind
        /// the result of `f`.
        /// \tparam From The index of component of the result of `f` to
        /// bind to `g`.
        /// \throws cycle_error if connecting `f` to `g` forms a cycle.
        /// \throws source_node_not_found if `f` is not already connected
        /// to the graph.
        /// \return A node wrapper which can be used as a handle to the
        /// node represented by `g`. Future calls may use either `g` or
        /// the returned object to make connections to 'g'.
        template <size_t From, size_t To, typename F, typename G>
        constexpr auto connect(F&& f, G&& g) -> decltype(auto) {
            using f_type = typename detail::unwrap_vertex<F>::type;
            using g_type = typename detail::unwrap_vertex<G>::type;

            auto fit = prepare_connect(std::forward<F>(f), std::forward<G>(g));

            // Find or make the destination node.
            auto git = ensure_node(std::forward<G>(g));

            // Connect
            to_node<g_type>(*git)->template connect<From, To>(
                *to_node<f_type>(*fit));

            // Add dest node as a child.
            add_child(fit, *git);

            return make_vertex(std::forward<G>(g));
        }

        /// \brief Check that each node in the graph with a non-empty
        /// parameter list has each parameter bound.
        bool valid() const {
            return std::all_of(std::begin(nodes_),
                               std::end(nodes_),
                               [this](const auto& node)
                               {
                                   // root is invalid due to missing input but
                                   // it doesn't matter because we start with
                                   // the root.
                                   return node.holds(root_) || node.valid();
                               });
        }

        /// \brief Get the depth of the graph, which hints at the
        /// number of worker threads required.
        size_t depth() const {
            return root_node_.depth();
        }

        /// \brief Get the number of nodes which have no children.
        size_t leaves() const {
            size_t l(0);
            for (const auto& node : nodes_) {
                if (node.children_.size() == 0) {
                    l++;
                }
            }
            return l;
        }

        /// \brief Reduce the internal graph by performing a transitive
        /// reduction.
        ///
        /// This operation does not affect the callgraph invokation.
        /// It does however potentially reduce the number of concurrent
        /// threads required.
        void reduce()  {
            // For each pair of nodes, if there is a path
            // between them with a distance > 1, remove the
            // edge between them.
            for (auto it = nodes_.begin(); it != nodes_.end(); ++it) {
                std::vector<const graph_node_type*> remove;
                const graph_node_type& knode(*it);
                for (const graph_node_type* jnode : knode.children_) {
                    if (longest_path(&knode, jnode) > 1) {
                        remove.emplace_back(jnode);
                    }
                }
                if (!remove.empty()) {
                    graph_node_type copy(knode);
                    for (auto child : remove) {
                        copy.children_.erase(child);
                    }
                    auto hint = nodes_.erase(it);
                    nodes_.emplace(std::move(copy));
                }
            }
        }

    private:
        friend class graph_runner;

        using graph_node_type = callgraph::detail::graph_node;
        using set_type = std::set<graph_node_type, detail::graph_node_less>;

        template <typename T>
        constexpr auto make_vertex(T&& t) -> decltype(auto) {
            using type = typename detail::unwrap_vertex<T>::type;
            return vertex<type>(unwrap_vertex(std::forward<T>(t)), *this);
        }

        template <typename T>
        static constexpr auto unwrap_vertex(T&& t) -> decltype(auto) {
            return detail::unwrap_vertex<T>::apply(std::forward<T>(t));
        }

        template <typename F, typename G>
        constexpr auto prepare_connect(F&& f, G&& g) -> decltype(auto) const {

            // Find the source node.
            auto fit = find_node(std::forward<F>(f));
            if (fit == nodes_.end()) {
                CALLGRAPH_THROW(source_node_not_found);
            }
            throw_if_cycle(*fit, std::forward<G>(g));
            return fit;
        }

        template <typename T>
        static auto to_node(const graph_node_type& g) -> decltype(auto) {
            return detail::to_node<T>(g);
        }

        void add_child(set_type::iterator parent,
                       const graph_node_type& child) {
            // Do a constant-time replace.
            auto hint = parent;
            hint++;
            graph_node_type cpy(*parent);
            cpy.add_child(child);
            nodes_.erase(parent);
            nodes_.emplace_hint(hint, std::move(cpy));
        }
        template <typename G>
        void throw_if_cycle(const graph_node_type& f, G&& g) {
            auto git = find_node(std::forward<G>(g));
            if (git != nodes_.end()) {
                const auto& gnode = *git;
                if (f.makes_cycle(&gnode)) {
                    CALLGRAPH_THROW(cycle_error);
                }
            }
        }
        template <typename T>
        constexpr auto ensure_node(T&& t) -> decltype(auto) {
            auto found = nodes_.find(std::forward<T>(t));
            if (found == nodes_.end()) {
                found = nodes_.emplace(
                    detail::make_graph_node(std::forward<T>(t))).first;
            }
            return found;
        }

        template <typename T>
        constexpr auto find_node(T&& t) -> decltype(auto) {
            return nodes_.find(std::forward<T>(t));
        }
        template <typename T>
        constexpr auto find_node(T&& t) const -> decltype(auto)  {
            return nodes_.find(std::forward<T>(t));
        }

        static void dummy() {}

        const graph_node_type& make_root_node() {
            return *ensure_node(root_);
        }

        set_type nodes_;
        void (*root_)();
        const graph_node_type& root_node_;
    };
}

#endif // CALLGRAPH_GRAPH_HPP
