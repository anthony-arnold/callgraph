// callgraph/detail/opaque_node.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_OPAQUE_NODE_HPP
#define CALLGRAPH_DETAIL_OPAQUE_NODE_HPP

#ifndef NO_DOC
namespace callgraph {
   namespace detail {

      template <typename T>
      struct opaque_node {
            opaque_node(T& impl)
               : impl_(impl)
            {
            }

            T& impl_;
      };

      template <typename T, typename>
      struct unwrap_opaque_node_impl
      {
            using type = T;
      };

      template <typename T, typename U>
      struct unwrap_opaque_node_impl<T, opaque_node<U> >
      {
            using type = U;
      };

      template <typename T>
      struct unwrap_opaque_node
         : unwrap_opaque_node_impl<T, typename std::decay<T>::type>
      {
      };
   }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_OPAQUE_NODE_HPP
