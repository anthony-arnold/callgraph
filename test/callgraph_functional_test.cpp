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
#include <cmath>

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

   auto sourcefn = [&source]() {
       const auto p(source.front());
       source.pop();
       return p;
   };

   auto root(pipe.connect(sourcefn));
   std::less<int> compare;
   pipe.connect<0,0>(root, compare);
   pipe.connect<1,1>(root, compare);

   std::logical_not<bool> lnot;
   pipe.connect<0>(compare, lnot);

   std::vector<bool> sink;
   auto sinkfn = [&sink](bool b) { sink.push_back(b); };
   pipe.connect<0>(lnot, sinkfn);

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

#include <fstream>
CALLGRAPH_TEST(callgraph_complexity) {
   using namespace std::chrono;
   using namespace std::chrono_literals;
   using namespace std::placeholders;

   struct ticker {
      int i;
      int operator()() {
         return i++;
      }
   };

   struct tick_to_rad {
      double operator()(int tick) {
         auto ts = high_resolution_clock::now();
         rate = 0s;
         if (prev.time_since_epoch() != 0s) {
            rate = ts - prev;
         }
         prev = ts;
         return tick / 60.0;
      }

      void throttle() {
         using target_rate = duration<double, std::ratio<1, 60>>;
         auto r = target_rate(rate);
         if (r.count() < 1.0) {
            std::this_thread::sleep_for(target_rate(1.0 - r.count()));
         }
      }

      duration<double> rate;
      time_point<high_resolution_clock> prev;
   };

   struct mem {
      void remember(double r, double m) {
         rads.push_back(r);
         muls.push_back(m);
      }

      std::vector<double> rads;
      std::vector<double> muls;
   };

   auto trig = [](double r) {
      return std::make_tuple(std::sin(r), std::cos(r));
   };

   ticker tick {};
   auto t = [&tick]() {
      return tick();
   };

   tick_to_rad to_rad;
   auto r = [&to_rad](int i) {
      return to_rad(i);
   };

   std::multiplies<double> mul;

   mem memory;
   std::function<void(double, double)> m = std::bind(&mem::remember, &memory, _1, _2);
   std::function<void()> throttle = std::bind(&tick_to_rad::throttle, &to_rad);

   callgraph::graph g;
   g.connect(t);
   g.connect<0>(t, r);
   g.connect<0>(r, trig);
   g.connect<0, 0>(trig, mul);
   g.connect<1, 1>(trig, mul);
   g.connect<0>(r, m);
   g.connect<1>(mul, m);
   g.connect(m, throttle);

   callgraph::graph_runner runner(g);
   auto start = high_resolution_clock::now();
   while(high_resolution_clock::now() - start < 3s) {
      runner.execute().wait();
   }

   assert(memory.rads.size() > 0);
   assert(memory.muls.size() == memory.rads.size());
   assert(memory.muls.size() == tick.i);

   for (size_t i = 0; i < tick.i; i++) {
      double rad = memory.rads[i];
      double prod = memory.muls[i];
      double comp = std::sin(2 * rad) * 0.5;
      double diff = std::abs(comp - prod);

      assert(diff < 1.0e-10);
   }
}
