// callgraph/graph.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_GRAPH_HPP
#define CALLGRAPH_GRAPH_HPP

#include <callgraph/detail/graph_node.hpp>
#include <callgraph/detail/node.hpp>
#include <callgraph/detail/node_key.hpp>
#include <callgraph/detail/opaque_node.hpp>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <vector>

/// \brief The main Callgraph namespace.
namespace callgraph {
   class graph_runner;

/// \brief An error thrown if connecting a node would cause a cycle.
   class cycle_error : public std::runtime_error
   {
      public:
         cycle_error()
            : runtime_error("Connecting f->g forms a cycle.")
         {
         }
   };

/// \brief An error thrown if the source node in a connection
/// request is not found.
   class source_node_not_found : public std::runtime_error
   {
      public:
         source_node_not_found()
            : runtime_error("Source node not found in graph.")
         {
         }
   };

   /// \brief An opaque type which is returned from the connection
   /// functions of the \ref graph type.
   ///
   /// Nodes can be used as placeholders for future connections to
   /// avoid having to share the actual function objects used to
   /// form graph nodes. Instead, the node may be shared with
   /// users of the graph who may connect their own objects.
   template <typename T>
   using node = detail::opaque_node<T>;

/// \brief A graph is a container of asynchronous executable nodes
/// joined into a directed acyclic graph.
///
/// The purpose of the graph
/// is to describe an asyncronous work pattern and information flow,
/// allowing the graph object to manage synchronisation and resources.
   class graph {
      public:

         /// \brief Construct an empty graph, consisting only of a no-op root node.
         graph()
            : root_(&graph::dummy),
              root_node_(ensure_node(root_)->second)
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
         node<typename detail::unwrap_opaque_node<T>::type> connect(T&& t) {
            return connect(std::forward<void(*)()>(root_), std::forward<T>(t));
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
         node<typename detail::unwrap_opaque_node<G>::type> connect(F&& f, G&& g) {
            using f_type = typename detail::unwrap_opaque_node<F>::type;
            using g_type = typename detail::unwrap_opaque_node<G>::type;

            auto fnode = get_node(std::forward<F>(f));
            if (fnode == nodes_.end()) {
               throw source_node_not_found();
            }
            throw_if_cycle(fnode->second, std::forward<G>(g));

            auto gnode = ensure_node(std::forward<G>(g));
            to_node<g_type>(gnode)->connect(*to_node<f_type>(fnode));
            fnode->second.add_child(&gnode->second);
            return node<g_type>(g);
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
         node<typename detail::unwrap_opaque_node<G>::type>
         connect(F&& f, G&& g) {
            using f_type = typename detail::unwrap_opaque_node<F>::type;
            using g_type = typename detail::unwrap_opaque_node<G>::type;

            auto fnode = get_node(std::forward<F>(f));
            if (fnode == nodes_.end()) {
               throw source_node_not_found();
            }
            throw_if_cycle(fnode->second, std::forward<G>(g));

            auto gnode = ensure_node(std::forward<G>(g));
            to_node<g_type>(gnode)->connect<To>(*to_node<f_type>(fnode));
            fnode->second.add_child(&gnode->second);
            return node<g_type>(g);
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
         node<typename detail::unwrap_opaque_node<G>::type>
         connect(F&& f, G&& g) {
            using f_type = typename detail::unwrap_opaque_node<F>::type;
            using g_type = typename detail::unwrap_opaque_node<G>::type;

            auto fnode = get_node(std::forward<F>(f));
            if (fnode == nodes_.end()) {
               throw source_node_not_found();
            }
            throw_if_cycle(fnode->second, std::forward<G>(g));

            auto gnode = ensure_node(std::forward<G>(g));
            to_node<g_type>(gnode)->connect<From, To>(*to_node<f_type>(fnode));
            fnode->second.add_child(&gnode->second);
            return node<g_type>(g);
         }

         /// \brief Check that each node in the graph with a non-empty
         /// parameter list has each parameter bound.
         bool valid() const {
            return std::all_of(std::begin(nodes_),
                               std::end(nodes_),
                               [this](const auto& pair)
                               {
                                  return pair.first == fn_key(root_) ||
                                     pair.second.valid();
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
            for (auto& pair : nodes_) {
               if (pair.second.children_.size() == 0) {
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
            using pair_type =
                std::pair<graph_node_type*, const graph_node_type*>;
            std::vector<pair_type> remove;
            for (auto& kpair : nodes_) {
               graph_node_type& knode(kpair.second);
               for (const graph_node_type* jnode : knode.children_) {
                  if (longest_path(&knode, jnode) > 1) {
                     remove.emplace_back(&knode, jnode);
                  }
               }
            }
            for (auto &rpair : remove) {
               rpair.first->children_.erase(rpair.second);
            }
         }

      private:
         template <typename T>
         using node_type = callgraph::detail::node<T>;

         using graph_node_type = callgraph::detail::graph_node;
         friend class graph_runner;

         using fn_key = detail::node_key;
         using map_type = std::unordered_map<fn_key, graph_node_type>;

         template <typename T, typename It>
         static node_type<T>* to_node(It it) {
            return it->second.to_node<T>();
         }

         template <typename G>
         void throw_if_cycle(const graph_node_type& f, G&& g) {
            auto gnode = get_node(std::forward<G>(g));
            if (gnode != nodes_.end()) {
               if (f.makes_cycle(&gnode->second)) {
                  throw cycle_error();
               }
            }
         }

         template <typename T>
         static fn_key to_key(T&& t) {
            return detail::to_node_key<T>::apply(std::forward<T>(t));
         };

         template <typename T>
         map_type::iterator ensure_node(T&& t) {
            fn_key key = to_key(t);
            auto found = nodes_.find(key);
            if (found == nodes_.end()) {
               found = nodes_.emplace(
                  key, detail::make_graph_node(std::forward<T>(t))).first;
            }
            return found;
         }

         template <typename T>
         map_type::iterator get_node(T&& t) {
            fn_key key = to_key(t);
            return nodes_.find(key);
         }

         template <typename T>
         map_type::const_iterator get_node(T&& t) const {
            fn_key key = to_key(t);
            return nodes_.find(key);
         }

         static void dummy() {}

         map_type nodes_;
         void (*root_)();
         graph_node_type& root_node_;
   };

}

#endif // CALLGRAPH_GRAPH_HPP
