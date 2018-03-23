// test.hpp
// License: BSD-2-Clause
/// \brief Minimal test library for convenience.

#ifndef CALLGRAPH_TEST_HPP
#define CALLGRAPH_TEST_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <vector>
#include <type_traits>

namespace callgraph_test {
    inline std::string test_exception_str(int line,
                                          const std::string& file,
                                          const std::string& test,
                                          const std::string& msg)
    {
        std::ostringstream ss;
        if (!test.empty()) {
            ss << test << " ";
        }
        if (!file.empty()) {
            ss << '[' << file << ':' << line << ']';
        }
        if (!msg.empty()) {
            ss << ": " << msg;
        }
        return ss.str();
    }

    class test_exception : public std::runtime_error {
    public:
        inline test_exception(int line,
                              const std::string& file,
                              const std::string& test,
                              const std::string& msg)
            : std::runtime_error(test_exception_str(line, file, test, msg))
            {
            }
    };

    class test_case_base {
    public:
        virtual ~test_case_base() = default;
        virtual const char* name() const = 0;
        virtual void run() = 0;
    };

    class test_engine {
    public:
        using type = std::function<std::unique_ptr<test_case_base>()>;

        template <typename F>
        void register_test_case(F test) {
            cases_.emplace_back(test);
        }

        int run_all() {
            return run_if([](auto) { return true; });
        }

        int run_matching(const std::regex& re) {
            return run_if([re](test_case_base* t) {
                    return std::regex_match(t->name(), re);
                });
        }

        const char* current() const { return cur_; }

    private:
        template <typename P>
        int run_if(P pred) {
            int run(0);
            std::map<std::string, std::exception_ptr> errors;

            for (auto ctor : cases_) {
                auto test = ctor();
                if (!pred(test.get())) {
                    continue;
                }
                cur_ = test->name();
                std::cout << "Running " << cur_ << "... ";
                try {
                    run++;
                    test->run();
                    std::cout << "OK." << std::endl;
                }
                catch(...) {
                    errors.emplace(test->name(), std::current_exception());
                    std::cout << "failed." << std::endl;
                }
            }

            int failed(static_cast<int>(errors.size()));
            std::cout << (run-failed) << " passed out of " << run << std::endl;

            for (const auto& pair : errors) {
                if (pair.second) {
                    try {
                        std::rethrow_exception(pair.second);
                    }
                    catch (const std::exception& ex) {
                        std::cout << pair.first << ": " << ex.what() << std::endl << std::endl;
                    }
                }
            }

            return failed;
        }

        const char* cur_;
        std::vector<type> cases_;
    };

    inline test_engine& global_test_engine() {
        static test_engine ngn;
        return ngn;
    }

    template <typename T>
    class test_case {
    public:
        test_case() {
            global_test_engine().register_test_case([] {
                    return std::unique_ptr<test_case_base>(new T);
                });
        }
    };

    namespace detail {
        template <typename T, typename U>
        struct is_streamable {
            template <typename V, typename W>
            static auto test(int)
                -> decltype(std::declval<V&>() << std::declval<W>(),
                            std::true_type());

            template <typename, typename>
            static auto test(...) -> std::false_type;

            static const bool value = decltype(test<T, U>(0))::value;
        };

        template <typename T>
        struct is_sstreamable : is_streamable<std::ostringstream, T> {};

        template <typename T>
        struct on_streamable
            : std::enable_if<is_sstreamable<T>::value, std::string>
        {
        };
        template <typename T>
        struct on_unstreamable
            : std::enable_if<!is_sstreamable<T>::value, std::string>
        {
        };

        template <typename T>
        typename on_streamable<typename std::decay<T>::type>::type
        value_of(T&& t) {
            std::ostringstream ss;
            ss << '(' << t << ')';
            return ss.str();
        }

        template <typename T>
        typename on_unstreamable<typename std::decay<T>::type>::type
        value_of(T&&) {
            return "";
        }
    }

    template <typename T>
    std::string value_of (T&& t) {
        return detail::value_of(std::forward<T>(t));
    }
}

#define CALLGRAPH_TEST_STR2(T) #T
#define CALLGRAPH_TEST_STR(T) CALLGRAPH_TEST_STR2(T)

#define CALLGRAPH_TEST(T) class test_case_ ## T :                       \
        public callgraph_test::test_case_base {                         \
public:                                                                 \
        void run() override;                                            \
        const char* name() const override {                             \
            return CALLGRAPH_TEST_STR(T);                               \
        }                                                               \
    };                                                                  \
    namespace {                                                         \
        callgraph_test::test_case<test_case_##T> test_case_##T##_instance; \
    }                                                                   \
    void test_case_ ## T::run()

#define CALLGRAPH_ERROR_STR(T) (CALLGRAPH_TEST_STR(T) +         \
                                callgraph_test::value_of(T))

#define CALLGRAPH_ERROR(m)                                              \
    throw callgraph_test::test_exception(__LINE__,                      \
                                         __FILE__,                      \
                                         callgraph_test::               \
                                         global_test_engine().current(), \
                                         m)

#define CALLGRAPH_CHECK(a) if (!(a)) {              \
        CALLGRAPH_ERROR(CALLGRAPH_ERROR_STR(a));    \
    }

#define CALLGRAPH_EQUAL(a,b) if ((a) != (b)) {          \
        CALLGRAPH_ERROR(CALLGRAPH_ERROR_STR(a) + "==" + \
                        CALLGRAPH_ERROR_STR(b));        \
    }

#define CALLGRAPH_THROWS(a) { bool threw(false);    \
        try {                                       \
            a;                                      \
        } catch(...) {                              \
            threw = true;                           \
        }                                           \
        if (!threw) {                               \
            CALLGRAPH_ERROR("Did not throw.");      \
        }                                           \
    }

#endif // CALLGRAPH_TEST_HPP
