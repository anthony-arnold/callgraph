// callgraph/callgraph_connect_test.cpp
// License: BSD-2-Clause
/// \brief Check for valid connections.

#include "test.hpp"
#include <callgraph/graph.hpp>
#include <callgraph/graph_runner.hpp>
#include <array>
#include <chrono>
#include <functional>

CALLGRAPH_TEST(callgraph_connect_to_root) {
    callgraph::graph pipe;

    bool run(false);
    auto a = [&run] { run = true; };
    pipe.connect(a);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_CHECK(run);
}

CALLGRAPH_TEST(callgraph_connect_void_void) {
    callgraph::graph pipe;

    bool runa(false), runb(false);
    auto a = [&runa] { runa = true; };
    auto b = [&runb] { runb = true; };

    pipe.connect(a);
    pipe.connect(a, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_CHECK(runa);
    CALLGRAPH_CHECK(runb);
}

CALLGRAPH_TEST(callgraph_connect_int_int) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef);

    int val(0);
    auto a = [] ()->int { return 0xdeadbeef; };
    auto b = [&val](int v) { val = v; };

    pipe.connect(a);
    pipe.connect<0>(a, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_ulong_int) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef);

    int val(0);
    auto a = [] ()-> unsigned long { return 0xdeadbeef; };
    auto b = [&val](int v) { val = v; };

    pipe.connect(a);
    pipe.connect<0>(a, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_pointer) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef);

    int val1(0), val2(0);
    auto a = [] { return 0xdeadbeef; };
    auto b = [&val1] (int i) { val1 = i; return &val1; };
    auto c = [&val2] (int* p) { val2 = *p; };

    pipe.connect(a);
    pipe.connect<0>(a,b);
    pipe.connect<0>(b,c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val2, expect);
}

CALLGRAPH_TEST(callgraph_connect_two_to_one) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef + 0xbadf00d);

    int val(0);
    auto a = [] { return 0xdeadbeef; };
    auto b = [] { return 0x0badf00d; };
    auto c = [&val] (int i, int j) { val = i + j; };

    pipe.connect(a);
    pipe.connect(b);
    pipe.connect<0>(a,c);
    pipe.connect<1>(b,c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_tuple_explode) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef + 0xbadf00d);

    int val(0);
    auto a = [] { return std::make_tuple(0xdeadbeef, 0xbadf00d); };
    auto b = [&val](int i, int j) { val = i + j; };

    pipe.connect(a);
    pipe.connect<0,0>(a, b);
    pipe.connect<1,1>(a, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_tuple_explode2) {
    callgraph::graph pipe;
    static const float A(0.5f);
    static const double PI(3.14);
    static const double L(3e6);
    static const double expect(A * L * PI);

    double val(0);
    auto a = [] { return std::make_tuple(A, PI); };
    auto b = [] (float f) { return static_cast<double>(f*L); };
    auto c = [&val] (double x, double y) { val = x * y; };

    pipe.connect(a);
    pipe.connect<1,0>(a,c);
    pipe.connect<0,0>(a,b);
    pipe.connect<1>(b,c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_functor) {
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
    pipe.connect(a);
    pipe.connect<0>(a, f);
    pipe.connect<0>(f, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_member_fn) {
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
    pipe.connect(a);
    pipe.connect(b);
    pipe.connect<0>(a, func);
    pipe.connect<1>(b, func);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(f.val, expect);
}

CALLGRAPH_TEST(callgraph_connect_static_fn) {
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
    pipe.connect(a);
    pipe.connect(b);
    pipe.connect<0>(a, &functor::run);
    pipe.connect<1>(b, &functor::run);
    pipe.connect<0>(&functor::run, c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

static void callgraph_connect_free_function(int in, int& out) {
    out = in;
}

CALLGRAPH_TEST(callgraph_connect_free_function) {
    int val(0);
    static const int expect(0xdeadbeef);

    auto a = [] { return 0xdeadbeef; };
    auto b = [&val] () -> int& { return val; };

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect(b);
    pipe.connect<0>(a, callgraph_connect_free_function);
    pipe.connect<1>(b, callgraph_connect_free_function);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_object_params) {
    struct type {
        int i,j,k;
    };

    type t = {};
    auto a = [] { return type { 1, 2, 3 }; };
    auto b = [&t] (const type& src) {
        t = src;
    };

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect<0>(a, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(t.i, 1);
    CALLGRAPH_EQUAL(t.j, 2);
    CALLGRAPH_EQUAL(t.k, 3);
}

CALLGRAPH_TEST(callgraph_connect_object_ref) {
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
    pipe.connect(a);
    pipe.connect<0>(a, b);

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

CALLGRAPH_TEST(callgraph_connect_object_const_ref) {
    struct type {
        int i;
    };

    type t = { 1 };
    const type* p = nullptr;

    auto a = [&t] () -> const type& { return t; };
    auto b = [&p] (const type& r) { p = &r; };

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect<0>(a,b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(p, &t);
}

CALLGRAPH_TEST(callgraph_connect_array) {
    std::array<int,3> v = { 1, 2, 3 };
    int x(0), y(0), z(0);

    auto a = [&v] { return v; };
    auto b = [&x] (int X) { x = X; };
    auto c = [&y] (int Y) { y = Y; };
    auto d = [&z] (int Z) { z = Z; };

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect<0,0>(a,b);
    pipe.connect<1,0>(a,c);
    pipe.connect<2,0>(a,d);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(x, 1);
    CALLGRAPH_EQUAL(y, 2);
    CALLGRAPH_EQUAL(z, 3);
}

CALLGRAPH_TEST(callgraph_connect_pair) {
    const auto & v = std::make_pair(1, 2);
    int x(0), y(0);

    auto a = [&v] { return v; };
    auto b = [&x] (int X) { x = X; };
    auto c = [&y] (int Y) { y = Y; };

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect<0,0>(a,b);
    pipe.connect<1,0>(a,c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(x, 1);
    CALLGRAPH_EQUAL(y, 2);
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

CALLGRAPH_TEST(callgraph_connect_custom_gettable) {
   int x(0), y(0), z(0);

   auto a = [] { return vec3i { 1, 2, 3 }; };
   auto b = [&x] (int X) { x = X; };
   auto c = [&y] (int Y) { y = Y; };
   auto d = [&z] (int Z) { z = Z; };

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect<0,0>(a,b);
    pipe.connect<1,0>(a,c);
    pipe.connect<2,0>(a,d);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(x, 1);
    CALLGRAPH_EQUAL(y, 2);
    CALLGRAPH_EQUAL(z, 3);
}

CALLGRAPH_TEST(callgraph_connect_pass_gettable_object) {
   vec3i v = { 0, 0, 0 };
   auto a = [] { return vec3i { 1, 2, 3 }; };
   auto b = [&v] (const vec3i& V) { v = V; };

   callgraph::graph pipe;
   pipe.connect(a);
   pipe.connect<0>(a,b);

   callgraph::graph_runner runner(pipe);
   auto future = runner();
   future.wait_for(std::chrono::seconds(1));
   CALLGRAPH_EQUAL(v.x, 1);
   CALLGRAPH_EQUAL(v.y, 2);
   CALLGRAPH_EQUAL(v.z, 3);
}

CALLGRAPH_TEST(callgraph_connect_unknown) {
    auto a = [] {};
    auto b = [] {};

    callgraph::graph pipe;
    CALLGRAPH_THROWS(pipe.connect(a, b));
}

CALLGRAPH_TEST(callgraph_connect_unknown_params) {
    auto a = [] { return 0; };
    auto b = [] (int i) {};

    callgraph::graph pipe;
    CALLGRAPH_THROWS(pipe.connect<0>(a, b));
}

CALLGRAPH_TEST(callgraph_connect_cycle) {
    auto a = [] {};
    auto b = [] {};

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect(a, b);
    CALLGRAPH_THROWS(pipe.connect(b, a));
}

CALLGRAPH_TEST(callgraph_connect_long_cycle) {
    auto a = [] { return 0; };
    auto b = [] { return 1; };
    auto c = [] (int i, int j) { return i + j; };
    auto d = [] (int k) {};

    callgraph::graph pipe;
    pipe.connect(a);
    pipe.connect(b);
    pipe.connect<0>(a, c);
    pipe.connect<1>(b, c);
    pipe.connect<0>(c, d);

    CALLGRAPH_THROWS(pipe.connect(d, a));
}

CALLGRAPH_TEST(callgraph_connect_node_ref) {
   callgraph::graph pipe;

   bool runa(false), runb(false);
   auto a = [&runa] { runa = true; };
   auto b = [&runb] { runb = true; };

   auto n(pipe.connect(a));
   pipe.connect(n, b);

   callgraph::graph_runner runner(pipe);
   auto future = runner();
   future.wait_for(std::chrono::seconds(1));
   CALLGRAPH_CHECK(runa);
   CALLGRAPH_CHECK(runb);
}

CALLGRAPH_TEST(callgraph_connect_node_ref_param) {
    callgraph::graph pipe;
    static const int expect(0xdeadbeef);

    int val(0);
    auto a = [] ()->int { return 0xdeadbeef; };
    auto b = [&val](int v) { val = v; };

    auto n(pipe.connect(a));
    pipe.connect<0>(n, b);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_node_ref_param_index) {
    const auto & v = std::make_pair(1, 2);
    int x(0), y(0);

    auto a = [&v] { return v; };
    auto b = [&x] (int X) { x = X; };
    auto c = [&y] (int Y) { y = Y; };

    callgraph::graph pipe;
    auto n(pipe.connect(a));
    pipe.connect<0,0>(n,b);
    pipe.connect<1,0>(n,c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(x, 1);
    CALLGRAPH_EQUAL(y, 2);
}

CALLGRAPH_TEST(callgraph_connect_node_ref_function_pointer) {
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
    pipe.connect(a);
    pipe.connect(b);
    auto n(pipe.connect<0>(a, &functor::run));
    pipe.connect<1>(b, n);
    pipe.connect<0>(n, c);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}

CALLGRAPH_TEST(callgraph_connect_std_functor) {
    static const int expect = 88888;
    int val = 0;

    auto p1 = []() { return 22222; };
    auto p2 = []() { return 4; };
    auto end = [&val](int i) { val = i; };
    auto mul = std::multiplies<int>();

    callgraph::graph pipe;
    pipe.connect(p1);
    pipe.connect(p2);
    pipe.connect<0>(p1, mul);
    pipe.connect<1>(p2, mul);
    pipe.connect<0>(mul, end);

    callgraph::graph_runner runner(pipe);
    auto future = runner();
    future.wait_for(std::chrono::seconds(1));
    CALLGRAPH_EQUAL(val, expect);
}
