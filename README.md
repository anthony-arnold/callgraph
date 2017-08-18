Callgraph
=========

Callgraph is a simple library for creating dynamic and extensible execution graphs. Callable objects are reprented by nodes in a directed acyclic graph, with edges representing dependencies. A callgraph will manage the order of execution so as to satisfy these dependencies, and will even run multiple dependencies in parallel where appropriate.

Theory
------

We will define a callgraph here. Let `G` be a connected acyclic direct graph (DAG). An edge from one node to another `E(n1, n2)` indicates that `n1` is a dependency of `n2`. Imbue each node in the graph with a property, *executed*, which can have a value of either `true` or `false`. The rules for a callgraph are as follows:

 1. All nodes start with `executed = false`.
 2. Nodes with no dependencies immediately set `executed = true`.
 3. When all of a node's dependencies have `executed = true`, it will also set `executed = true`.
 4. When all nodes in the graph have `executed = true`, the callgraph has finished.

### Execution Order

The order in which nodes in a graph set `executed = true` is called the *execution order*. According to the rules above, only some execution orders are valid. For an execution order to be valid, **no node may precede its dependencies**.

Below is an algorithm for determining a valid execution order:

    GetExecutionOrder (G, L):
       for each node in G:
           if child.dependencies is empty:
               AddToExecutionOrder(node, L)

    AddToExecutionOrder(node, L):
        L.add(node)

        for each child in node:
            child.dependencies.erase(node)
            if child.dependencies is empty:
               AddToExecutionOrder(node, L)

### Depth

The *depth* of callgraph is defined as the greatest number of dependents of any node. This defnition will be useful later.

Simple Example
--------------

Consider two functions, `void a()`, and `void b()`. The definitions of these functions are not important. Suppose the execution of `b` depends on `a`; a callgraph `G` can be constructed to represent `b`'s dependency on `a`:

    callgraph::graph G;
    G.connect(a);
    G.connect(a, b);

Internally, Callgraph will determine a valid *execution order* and enqueue each node. Executing the graph will now call `a()`, followed by `b()`. The result is a `std::future<void>` which will be satisfied once all nodes in the graph have finished executing.

    callgraph::graph_runner R(G);
    std::future<void> f(R.execute());
    f.wait();

Multiple Dependencies
---------------------

Let's split `a` into multiple definitions, `void a1()` and `void a2()`. Let `b` depend on both of them, so `b` now has two dependencies:

    callgraph::graph G;
    G.connect(a1);
    G.connect(a2);
    G.connect(a1, b);
    G.connect(a2, b);

Note that neither of `a1` or `a2` depends on the other, so the order of their evaluation is not important. Both `a1, a2, b` and `a2, a1, b` are valid execution orders. In fact, `a1` and `a2` can be executed in parallel, as long as they both complete before `b` is executed.

The Callgraph library automatically manages the creation of multiple worker threads to execute nodes in parallel where appropriate.

Reducing Worker Threads
-----------------------

Recall the original graph, where we have nodes `a` and `b`, with `a` as a dependency of `b`. Introduce now a new node, `c`. Let both `a` *and* `b` be dependencies of `c`.

    callgraph::graph G;
    G.connect(a);
    G.connect(a, b);
    G.connect(a, c);
    G.connect(b, c);

Unlike in the previous example, we cannot run `a` and `b` in parallel while `c` waits for them to complete. This is because `b` also depends on `a`. There is only one valid execution order in this example: `a, b, c`.

This isn't a big problem, especially not for a trivial graph; the Callgraph library will ensure all dependencies are met before executing a node. However, there is a potential for improved efficiency here. The `graph_runner` will spawn a number of worker threads equal to the *depth* of the graph. In this case, the graph has a depth of **2** (because `a` has 2 dependents.) This is undesirable, because only one thread will ever be working, leaving a thread always waiting for work. Why use two threads when one will do?

The `graph::reduce` function will perform a *transitive reduction* on the graph, removing edges between nodes where there is another path between them of greater length. In the case of our example graph, it will remove the dependency between `a` and `c`. Performing a reduction on the graph reduces the *depth* to 1, which reduces the number of spawned worker threads.

Note that nothing about the graph changes, for practical purposes. The execution order is still `a, b, c`, and `c` is still dependent upon `a` albeit only implicitly. A reduction is simply a useful operation to reduce the number of worker threads that may be unnecessary.

    G.reduce();

    callgraph::graph_runner R(G);
    std::future<void> f(R.execute());
    f.wait();

Passing Parameters
------------------

Documentation forthcoming...

Exploding Return Values
-----------------------

Documentation forthcoming...

Things to Watch Out For
-----------------------

Documentation forthcoming...
