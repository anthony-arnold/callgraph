// callgraph/callgraph_connect_test.cpp
// License: BSD-2-Clause
/// \brief Check for valid connections.

#include "test.hpp"
#include <callgraph/graph.hpp>

CALLGRAPH_TEST(empty_callgraph_is_valid) {
    callgraph::graph empty;
    CALLGRAPH_CHECK(empty.valid());
}

CALLGRAPH_TEST(connected_callgraph_is_valid) {
    callgraph::graph pipe;

    auto a = []{};
    auto b = []{};

    pipe.connect(a);
    pipe.connect(a,b);

    CALLGRAPH_CHECK(pipe.valid());
}

CALLGRAPH_TEST(callgraph_multiple_params_is_valid) {
    callgraph::graph pipe;

    auto a = [] { return 1; };
    auto b = [] { return 2; };
    auto c = [] { return 3; };
    auto d = [] { return 4; };

    auto e = [] (int, int, int, int) {};

    pipe.connect(a);
    pipe.connect(b);
    pipe.connect(c);
    pipe.connect(d);

    pipe.connect<0>(a,e);
    CALLGRAPH_CHECK(!pipe.valid());

    pipe.connect<1>(b,e);
    CALLGRAPH_CHECK(!pipe.valid());

    pipe.connect<2>(c,e);
    CALLGRAPH_CHECK(!pipe.valid());

    pipe.connect<3>(d,e);
    CALLGRAPH_CHECK(pipe.valid());
}

CALLGRAPH_TEST(callgraph_valid_after_reduction) {
    callgraph::graph pipe;

    auto a = [] {};
    auto b = [] {};
    auto c = [] {};
    auto d = [] {};
    auto e = [] {};

    pipe.connect(a);
    pipe.connect(a, b);
    pipe.connect(b, e);
    pipe.connect(a, e);
    pipe.connect(b, c);
    pipe.connect(c, e);
    pipe.connect(b, d);
    pipe.connect(d, e);

    size_t d1(pipe.depth());
    CALLGRAPH_CHECK(pipe.valid());

    pipe.reduce();

    size_t d2(pipe.depth());
    CALLGRAPH_CHECK(pipe.valid());
    CALLGRAPH_CHECK(d2 < d1);
}
