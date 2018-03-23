// callgraph/callgraph_run_test.cpp
// License: BSD-2-Clause
/// \brief Check for valid run conditions.

#include "test.hpp"
#include <callgraph/graph.hpp>
#include <callgraph/graph_runner.hpp>
#include <chrono>

CALLGRAPH_TEST(empty_callgraph_runs) {
  callgraph::graph empty;
  callgraph::graph_runner runner(empty);
  auto future = runner();
  auto status = future.wait_for(std::chrono::seconds(1));
  CALLGRAPH_CHECK(status == std::future_status::ready);
}

CALLGRAPH_TEST(callgraph_runs_once) {
    int i(0);
    static const int expect(123);
    auto a = [&i] () { i = 123; };

    callgraph::graph pipe;
    pipe.connect(a);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    auto status = future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_CHECK(status == std::future_status::ready);
    CALLGRAPH_EQUAL(i, expect);
}

CALLGRAPH_TEST(callgraph_runs_multiple_times) {
    int k(0);
    static const int expect(10);
    auto a = [&k] () { k++; };

    callgraph::graph pipe;
    pipe.connect(a);

    callgraph::graph_runner runner(pipe);
    for (int i = 0; i < 10; i++) {
        auto future = runner();
        auto status = future.wait_for(std::chrono::seconds(1));
        CALLGRAPH_EQUAL(status, std::future_status::ready);
    }
    CALLGRAPH_EQUAL(k, expect);
}
