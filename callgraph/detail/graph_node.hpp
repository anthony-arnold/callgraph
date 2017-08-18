// callgraph/detail/graph_node.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_GRAPH_NODE_HPP
#define CALLGRAPH_DETAIL_GRAPH_NODE_HPP

#include <callgraph/detail/invoke_once.hpp>
#include <callgraph/detail/node.hpp>
#include <callgraph/detail/opaque_node.hpp>

#include <functional>
#include <stack>
#include <unordered_set>

#ifndef NO_DOC
namespace callgraph {
   class graph;

   namespace detail {
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
                  node_executor(void* ptr)
                     : ptr_(ptr)
                  {
                  }

                  void operator()() {
                     (*static_cast<node<T>*>(ptr_))();
                  }

                  void* ptr_;
            };

            template <typename T>
            graph_node(T&& t)
               : node_(new node<T>(std::forward<T>(t)), node_deleter<T>()),
                 exec_fn_(node_executor<T>(node_.get())),
                 validator_fn_(node_validator<T>()),
                 resetter_fn_(node_resetter<T>())
            {
            }

            template <typename T>
            node<T>* to_node() const {
               return static_cast<node<T>*>(node_.get());
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

            void reset(){
               exec_fn_.reset();
               resetter_fn_(node_.get());
            }

            void add_child(const graph_node* child)  {
               children_.insert(child);
            }

            size_t depth() const {
               size_t d(0);
               for (const graph_node* child : children_) {
                  d += child->depth();
               }
               return d > 0 ? d : 1;
            }

            bool makes_cycle(const graph_node* candidate_child) const  {
               bool cycle(false);
               if (this == candidate_child) {
                  cycle = true;
               }
               else if (path_exists(candidate_child, this)) {
                  cycle = true;
               }
               return cycle;
            }

            friend bool has_child(const graph_node* a, const graph_node* b) {
               return a->children_.find(b) != a->children_.end();
            }

            friend bool path_exists(const graph_node* a, const graph_node* b)  {
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
            std::shared_ptr<void> node_;
            invoke_once exec_fn_;
            std::function<bool(void*)> validator_fn_;
            std::function<void(void*)> resetter_fn_;
      };

      template <typename T, typename>
      struct make_graph_node_impl
      {
            template <typename U>
            static graph_node apply(U&& t) {
               return graph_node(std::forward<U>(t));
            }
      };

      template <typename T, typename U>
      struct make_graph_node_impl<T, opaque_node<U> >
      {
            static graph_node apply(const opaque_node<U>& n) {
               return graph_node(static_cast<U>(n.impl_));
            }
      };

      template <typename T>
      graph_node make_graph_node(T&& t) {
         return make_graph_node_impl<T, typename std::decay<T>::type>::apply(t);
      }
   }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_GRAPH_NODE_HPP
