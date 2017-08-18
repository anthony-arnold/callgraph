// callgraph/detail/graph_worker.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_GRAPH_WORKER_HPP
#define CALLGRAPH_DETAIL_GRAPH_WORKER_HPP

#include <callgraph/graph_runner.hpp>
#include <callgraph/detail/graph_node.hpp>

#include <thread>

#ifndef NO_DOC
namespace callgraph {
   namespace detail {
      struct graph_node;

      struct graph_worker {
            graph_worker(graph_runner& runner)
               : runner_(runner),
                 thread_(std::bind(&graph_worker::work, this))
            {
            }
            ~graph_worker()  {
               if (thread_.joinable()) {
                  thread_.join();
               }
            }

            graph_worker(const graph_worker&) = delete;
            graph_worker(graph_worker&&) = default;
            graph_worker& operator=(const graph_worker&) = delete;
            graph_worker& operator=(graph_worker&&) = default;

         private:
            const graph_node* get_task() {
               const graph_node* task(nullptr);
               {
                  std::unique_lock<std::mutex> lk(runner_.queue_mutex_);

                  runner_.queue_avail_.wait(lk, [this] {
                        return !runner_.on_ || !runner_.queue_.empty();
                     });

                  if (runner_.on_ && !runner_.queue_.empty()) {
                     task = runner_.queue_.front();
                     runner_.queue_.pop();
                  }
               }
               return task;
            }
            void run_task(const graph_node* task)  {
               if (task->run(runner_) && task->children_.size() == 0) {
                  // Mark a leaf as done.
                  std::unique_lock<std::mutex> lk(runner_.done_mutex_);
                  runner_.leaves_--;
                  if (runner_.leaves_ == 0) {
                     // Finished.
                     runner_.done_.set_value();
                  }
               }
            }
            void handle_exception() {
               runner_.clear_queue();
               {
                  std::unique_lock<std::mutex> lk(runner_.done_mutex_);
                  try {
                     runner_.done_.set_exception(std::current_exception());
                  }
                  catch(...) {}
               }
            }
            void work() {
               while (runner_.on_) {
                  try {
                     const graph_node* task(get_task());
                     if (!task) {
                        break;
                     }
                     run_task(task);
                  }
                  catch(...) {
                     handle_exception();
                  }
               }
            }

            graph_runner& runner_;
            std::thread thread_;
      };

   } }
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_GRAPH_WORKER_HPP
