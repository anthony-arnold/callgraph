// callgraph/detail/invoke_once.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_DETAIL_INVOKE_ONCE_HPP
#define CALLGRAPH_DETAIL_INVOKE_ONCE_HPP

#include <atomic>
#include <functional>

#ifndef NO_DOC
namespace callgraph {
    namespace detail {

        struct invoke_once {
            template <typename T>
            invoke_once(T&& t)
                : fn_(std::forward<T>(t)),
                  invoked_(false)
                {
                }

            invoke_once(const invoke_once& src)
                : fn_(src.fn_),
                  invoked_(src.invoked_.load())
                {
                }
            invoke_once(invoke_once&& src)
                : fn_(src.fn_)
                {
                    invoked_.store(src.invoked_.load());
                    src.invoked_.store(false);
                }


            invoke_once& operator=(const invoke_once& src){
                fn_ = src.fn_;
                invoked_.store(src.invoked_.load());
                return *this;
            }

            invoke_once& operator=(invoke_once&& src) {
                using std::swap;
                swap(fn_, src.fn_);
                invoked_.store(src.invoked_.load());
                src.invoked_.store(false);
                return *this;
            }

            bool operator()() const{
                bool invoked(false);
                if (!invoked_.exchange(true)) {
                    fn_();
                    invoked = true;
                }
                return invoked;
            }

            void reset() {
                invoked_.store(false);
            }

        private:
            std::function<void()> fn_;
            mutable std::atomic_bool invoked_;
        };

    }
}
#endif // NO_DOC
#endif // CALLGRAPH_DETAIL_INVOKE_ONCE_HPP
