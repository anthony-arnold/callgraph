// callgraph/callgraph_shift_connect_test.cpp
// License: BSD-2-Clause
/// \brief Check for valid connections using shift operator shortcuts.

#include "test.hpp"
#include <callgraph/graph.hpp>
#include <callgraph/graph_runner.hpp>
#include <array>
#include <chrono>
#include <functional>

CALLGRAPH_TEST(callgraph_shift_connect_to_root) {
    callgraph::graph pipe;

    bool run(false);
    auto a = [&run] { run = true; };
    pipe >> a;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_CHECK(run);
}

CALLGRAPH_TEST(callgraph_shift_connect_void_void) {
    callgraph::graph pipe;

    bool runa(false), runb(false);
    auto a = [&runa] { runa = true; };
    auto b = [&runb] { runb = true; };

    pipe >> a >> b;
    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_CHECK(runa);
    CALLGRAPH_CHECK(runb);
}

CALLGRAPH_TEST(callgraph_shift_connect_int_int) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef);

    int val(0);
    auto a = [] ()->int { return 0xdeadbeef; };
    auto b = [&val](int v) { val = v; };

    pipe >> a >> b;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_shift_connect_functor) {
    struct functor {
        int operator()(int a) {
            return a + 1;
        }
    };
    static const int expect(0xdeadbeef + 1);

    int val(0);
    auto a = [] { return 0xdeadbeef; };
    auto b = [&val] (int i) { val = i; };
    functor f;

    callgraph::graph pipe;
    pipe >> a >> f >> b;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_shift_connect_member_fn) {
    using namespace std::placeholders;
    static const int expect(0xdeadbeef + 0xbadf00d);

    struct functor {
        void run(int i, int j) {
            val = i + j;
        };

        int val;
    };

    auto a = [] { return 0xdeadbeef; };
    auto b = [] { return 0xbadf00d; };
    functor f;
    std::function<void(int,int)> func(std::bind(&functor::run, &f, _1, _2));

    callgraph::graph pipe;
    pipe >> a >> func;
    auto node = pipe >> b;
    pipe.connect<1>(node, func);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(f.val, expect);
}


CALLGRAPH_TEST(callgraph_shift_connect_static_fn) {
    struct functor {
        static int run(int i, int j) {
            return i + j;
        };
    };
    static const int expect(0xdeadbeef + 0xbadf00d);

    int val(0);
    auto a = [] { return 0xdeadbeef; };
    auto b = [] { return 0xbadf00d; };
    auto c = [&val] (int k) { val = k; };

    callgraph::graph pipe;
    pipe >> a >> &functor::run >> c;
    pipe >> b;
    pipe.connect<1>(b, &functor::run);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

static void callgraph_shift_connect_free_function(int in, int& out) {
    out = in;
}

CALLGRAPH_TEST(callgraph_shift_connect_free_function) {
    int val(0);
    static const int expect(0xdeadbeef);

    auto a = [] { return 0xdeadbeef; };
    auto b = [&val] () -> int& { return val; };

    callgraph::graph pipe;
    pipe >> a >> callgraph_shift_connect_free_function;
    pipe >> b;
    pipe.connect<1>(b, callgraph_shift_connect_free_function);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}


CALLGRAPH_TEST(callgraph_shift_connect_object_params) {
    struct type {
        int i,j,k;
    };

    type t = {};
    auto a = [] { return type { 1, 2, 3 }; };
    auto b = [&t] (const type& src) {
        t = src;
    };

    callgraph::graph pipe;
    pipe >> a >> b;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(t.i, 1);
    CALLGRAPH_EQUAL(t.j, 2);
    CALLGRAPH_EQUAL(t.k, 3);
}


CALLGRAPH_TEST(callgraph_shift_connect_object_ref) {
    struct type {
        int i,j,k;
    };

    type t = { 1, 2, 3 }, u = {};
    auto a = [&t]() -> type& { return t; };
    auto b = [&u] (type& src) {
        u = src;
        src = type { -1, -2, -3 };
    };

    callgraph::graph pipe;
    pipe >> a >> b;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(t.i, -1);
    CALLGRAPH_EQUAL(t.j, -2);
    CALLGRAPH_EQUAL(t.k, -3);

    CALLGRAPH_EQUAL(u.i, 1);
    CALLGRAPH_EQUAL(u.j, 2);
    CALLGRAPH_EQUAL(u.k, 3);
}

CALLGRAPH_TEST(callgraph_shift_connect_object_const_ref) {
    struct type {
        int i;
    };

    type t = { 1 };
    const type* p = nullptr;

    auto a = [&t] () -> const type& { return t; };
    auto b = [&p] (const type& r) { p = &r; };

    callgraph::graph pipe;
    pipe >> a >> b;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(p, &t);
}

namespace {
   struct vec3i {
         int x,y,z;
   };
   template <size_t I>
   struct vec3i_getter
   {
   };

   template<>
   struct vec3i_getter<0> {
         static int apply(const vec3i& v) {
            return v.x;
         }
   };

   template<>
   struct vec3i_getter<1> {
         static int apply(const vec3i& v) {
            return v.y;
         }
   };

   template<>
   struct vec3i_getter<2> {
         static int apply(const vec3i& v) {
            return v.z;
         }
   };

   template <size_t I>
   constexpr int get(const vec3i& v) {
      return vec3i_getter<I>::apply(v);
   }
}

CALLGRAPH_TEST(callgraph_shift_connect_pass_gettable_object) {
   vec3i v = { 0, 0, 0 };
   auto a = [] { return vec3i { 1, 2, 3 }; };
   auto b = [&v] (const vec3i& V) { v = V; };

   callgraph::graph pipe;
   pipe >> a >> b;

   callgraph::graph_runner runner(pipe);
   auto future = runner();
   future.wait_for(std::chrono::seconds(1));
   CALLGRAPH_EQUAL(v.x, 1);
   CALLGRAPH_EQUAL(v.y, 2);
   CALLGRAPH_EQUAL(v.z, 3);
}

CALLGRAPH_TEST(callgraph_shift_connect_cycle) {
    auto a = [] {};
    auto b = [] {};

    callgraph::graph pipe;
    pipe >> a >> b;
    CALLGRAPH_THROWS(pipe >> b >> a);
}

CALLGRAPH_TEST(callgraph_shift_connect_long_cycle) {
    auto a = [] { return 0; };
    auto b = [] { return 1; };
    auto c = [] (int i, int j) { return i + j; };
    auto d = [] (int k) {};

    callgraph::graph pipe;
    auto n = pipe >> a >> c >> d;
    pipe >> b;
    pipe.connect<1>(b, c);

    CALLGRAPH_THROWS(n >> a);
}

CALLGRAPH_TEST(callgraph_shift_connect_node_ref) {
   callgraph::graph pipe;

   bool runa(false), runb(false);
   auto a = [&runa] { runa = true; };
   auto b = [&runb] { runb = true; };

   auto n = pipe >> a;
   n >> b;

   callgraph::graph_runner runner(pipe);
   auto future = runner();
   future.wait_for(std::chrono::seconds(1));
   CALLGRAPH_CHECK(runa);
   CALLGRAPH_CHECK(runb);
}

CALLGRAPH_TEST(callgraph_shift_connect_node_ref_param) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef);

    int val(0);
    auto a = [] ()->int { return 0xdeadbeef; };
    auto b = [&val](int v) { val = v; };

    auto n = pipe >> a;
    n >> b;

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_shift_connect_node_ref_function_pointer) {
    struct functor {
        static int run(int i, int j) {
            return i + j;
        };
    };
    static const int expect(0xdeadbeef + 0xbadf00d);

    int val(0);
    auto a = [] { return 0xdeadbeef; };
    auto b = [] { return 0xbadf00d; };
    auto c = [&val] (int k) { val = k; };

    callgraph::graph pipe;
    auto n = pipe >> a >> &functor::run;
    n >> c;
    pipe >> b;
    pipe.connect<1>(b, n);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}
