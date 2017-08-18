// callgraph/callgraph_functional_test.cpp
// License: BSD-2-Clause
/// \brief Check for valid connections over standard library functors.

#include "test.hpp"
#include <callgraph/graph.hpp>
#include <callgraph/graph_runner.hpp>
#include <cassert>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

CALLGRAPH_TEST(callgraph_std_less_logical_not) {
   callgraph::graph pipe;

   std::queue<std::pair<int, int> > source;
   source.emplace(1,2);
   source.emplace(3,2);
   source.emplace(8,1);
   source.emplace(8,8);
   source.emplace(9,3);
   source.emplace(1,7);
   source.emplace(4,5);

   auto root(pipe.connect(
                [&source]() {
                   const auto p(source.front());
                   source.pop();
                   return p;
                }));
   std::less<int> compare;
   pipe.connect<0,0>(root, compare);
   pipe.connect<1,1>(root, compare);

   std::logical_not<bool> lnot;
   pipe.connect<0>(compare, lnot);

   std::vector<bool> sink;
   pipe.connect<0>(lnot, [&sink](bool b) { sink.push_back(b); });

   pipe.reduce();
   callgraph::graph_runner runner(pipe);
   while (!source.empty()) {
      auto future = runner.execute();
      future.wait_for(std::chrono::seconds(1));
   }

   std::vector<bool> expect = { false, true, true, true, true, false, false };
   CALLGRAPH_EQUAL(expect, sink);
}

CALLGRAPH_TEST(callgraph_equal) {
   struct seed {
         int i_;
         seed()
            : i_(0)
         {
         }

         int operator()() {
            return i_++;
         }
   };

   seed s1, s2, s3;

   callgraph::graph pipe;

   pipe.connect(s1);
   pipe.connect(s2);
   pipe.connect(s3);

   std::multiplies<int> mul;
   pipe.connect<0>(s1, mul);
   pipe.connect<1>(s2, mul);

   std::multiplies<int> sqr;
   pipe.connect<0>(s3, sqr);
   pipe.connect<1>(s3, sqr);

   std::equal_to<int> cmp;
   pipe.connect<0>(sqr, cmp);
   pipe.connect<1>(mul, cmp);

   auto check = [](bool b) { assert(b); };
   pipe.connect<0>(cmp, check);

   callgraph::graph_runner runner(pipe);
   auto future = runner.execute();
   future.wait_for(std::chrono::seconds(1));
}
