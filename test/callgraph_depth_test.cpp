// callgraph/callgraph_depth_test.cpp
// License: BSD-2-Clause
/// \brief Check the result of the depth function.

#include "test.hpp"
#include <callgraph/graph.hpp>

CALLGRAPH_TEST(empty_callgraph_depth) {
  callgraph::graph empty;
  CALLGRAPH_EQUAL(empty.depth(), 1);
}

CALLGRAPH_TEST(callgraph_depth_increase) {
  callgraph::graph pipe;
  auto a = []{};
  auto b = []{};
  auto c = []{};

  pipe.connect(a);
  pipe.connect(a, b);
  pipe.connect(a, c);

  CALLGRAPH_EQUAL(pipe.depth(), 2);

  auto d = []{};
  pipe.connect(a, d);
  CALLGRAPH_EQUAL(pipe.depth(), 3);
}

CALLGRAPH_TEST(callgraph_depth_decrease) {
    callgraph::graph pipe;
    auto a = []{};
    auto b = []{};
    auto c = []{};
    auto d = []{};
    auto e = []{};

    // a -> (b,c,d)
    // b -> (c, d)
    // c -> (d, e)
    // d -> (e)
    pipe.connect(a);
    pipe.connect(a, b);
    pipe.connect(a, c);
    pipe.connect(a, d);
    pipe.connect(b, c);
    pipe.connect(b, d);
    pipe.connect(c, d);
    pipe.connect(c, e);
    pipe.connect(d, e);

    CALLGRAPH_EQUAL(pipe.depth(), 6);

    pipe.reduce();
    // a -> b -> c -> d -> e
    CALLGRAPH_EQUAL(pipe.depth(), 1);
}
