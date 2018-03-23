// callgraph/graph_runner.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_GRAPH_RUNNER_HPP
#define CALLGRAPH_GRAPH_RUNNER_HPP

#include <callgraph/graph.hpp>
#include <callgraph/detail/graph_node.hpp>

#include <condition_variable>
#include <iterator>
#include <mutex>
#include <future>
#include <queue>
#include <vector>

namespace callgraph {
    namespace detail {
        struct graph_worker;
    }

/// \brief A graph runner is a non-copyable type
/// which launches worker threads to run a callgraph.
///
/// A graph runner may be invoked many times in succession.
    class graph_runner {
    public:
        /// \brief Construct a callgraph runner which wraps a graph.
        graph_runner(graph& g)
            : graph_(g),
              on_(true),
              max_leaves_(graph_.leaves())
            {
            }

        /// \brief Callgraph runner destructor. Wait for all
        /// worker threads to finish.
        ~graph_runner() {
            {
                std::unique_lock<std::mutex> lk(queue_mutex_);
                on_ = false;
                queue_ = std::queue<const graph_node_type*>();
            }
            // Wake up workers
            queue_avail_.notify_all();
        }

        graph_runner(const graph_runner&) = delete;
        graph_runner& operator=(const graph_runner&) = delete;

        /// \brief Move construct a callgraph runner.
        graph_runner(graph_runner&&) = default;
        /// \brief Move assign a callgraph runner.
        graph_runner& operator=(graph_runner&&) = default;

        /// \brief Execute the call graph asynchronously.
        /// \return A future which can be used to wait for the call to finish or
        /// to catch any exception thrown.
        /// \warning Subsequent executions must not be invoked until previous calls
        /// have finished.
        std::future<void> operator()() {
            return execute();
        }

        /// \brief Execute the call graph asynchronously.
        /// \return A future which can be used to wait for the call to finish or
        /// to catch any exception thrown.
        /// \warning Subsequent executions must not be invoked until previous calls
        /// have finished.
        std::future<void> execute() {
            std::unique_lock<std::mutex> lk(done_mutex_);
            for (auto& node : graph_.nodes_) {
                node.reset();
            }
            leaves_ = max_leaves_;

            size_t min_workers(graph_.depth());
            workers_.reserve(min_workers);
            while (workers_.size() < min_workers) {
                workers_.emplace_back(std::make_shared<graph_worker_type>(*this));
            }

            done_ = std::promise<void>();

            enqueue_node(&graph_.root_node_);
            return done_.get_future();
        }

    private:
        using graph_node_type = callgraph::detail::graph_node;
        using graph_worker_type = callgraph::detail::graph_worker;
        friend graph_node_type;
        friend graph_worker_type;

        void enqueue_node(const graph_node_type* node) {
            {
                std::unique_lock<std::mutex> lk(queue_mutex_);
                queue_.push(node);
            }
            queue_avail_.notify_one();
        }

        void clear_queue() {
            std::unique_lock<std::mutex> lk(queue_mutex_);
            queue_ = std::queue<const graph_node_type*>();
        }
        graph& graph_;
        bool on_;

        // Order is important here! Anything that might lock these
        // needs to be destructed (i.e. unlock mutex) before the
        // mutex is destructed.
        std::mutex done_mutex_;
        std::mutex queue_mutex_;

        std::vector<std::shared_ptr<graph_worker_type>> workers_;
        std::queue<const graph_node_type*> queue_;
        std::condition_variable queue_avail_;
        std::promise<void> done_;
        size_t max_leaves_;
        size_t leaves_;
    };
}
#include <callgraph/detail/graph_worker.hpp>

#endif // CALLGRAPH_GRAPH_RUNNER_HPP
