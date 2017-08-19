// callgraph/callgraph_thread_test.cpp
// License: BSD-2-Clause
/// \brief Check for valid thread behaviour in callgraphs.

#include "test.hpp"
#include <callgraph/graph.hpp>
#include <callgraph/graph_runner.hpp>
#include <cassert>
#include <thread>

CALLGRAPH_TEST(callgraph_separate_threads) {
   callgraph::graph pipe;

   std::promise<void> p1;
   std::promise<void> p2;
   auto a = [&p1, &p2]() {
       p2.set_value();
       p1.get_future().wait();
       return std::this_thread::get_id();
   };
   auto b = [&p1, &p2]() {
       p2.get_future().wait();
       p1.set_value();
       return std::this_thread::get_id();
   };

   auto an(pipe.connect(a));
   auto bn(pipe.connect(b));

   std::equal_to<std::thread::id> compare;
   pipe.connect<0>(an, compare);
   pipe.connect<1>(bn, compare);

   std::logical_not<bool> lnot;
   pipe.connect<0>(compare, lnot);

   auto check = [](bool b) { assert(b); };
   pipe.connect<0>(lnot, check);

   callgraph::graph_runner runner(pipe);
   auto future = runner.execute();
   future.wait_for(std::chrono::seconds(1));
}

CALLGRAPH_TEST(callgraph_same_thread) {
   callgraph::graph pipe;

   std::thread::id id1, id2, id3;
   auto a = [&id1]() { id1 = std::this_thread::get_id(); };
   auto b = [&id2]() { id2 = std::this_thread::get_id(); };
   auto c = [&id3]() { id3 = std::this_thread::get_id(); };

   pipe.connect(a);
   pipe.connect(a, b);
   pipe.connect(a, c);
   pipe.connect(b, c);

   // reduces a->b, a->c, b->c (depth 2)
   // to a->b->c (depth 1)
   pipe.reduce();

   // Only one worker thread will be created.
   callgraph::graph_runner runner(pipe);
   auto future = runner.execute();
   future.wait_for(std::chrono::seconds(1));

   CALLGRAPH_EQUAL(id1, id2);
   CALLGRAPH_EQUAL(id2, id3);
}
