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
#include <sstream>
#include <vector>

namespace callgraph_test {
   inline std::string test_exception_str(int line,
                                         const char* file,
                                         const char* test,
                                         const char* msg)
   {
      std::ostringstream ss;
      if (test) {
         ss << test << " ";
      }
      if (file) {
         ss << '[' << file << ':' << line << ']';
      }
      if (msg) {
         ss << ": " << msg;
      }
      return ss.str();
   }

   class test_exception : public std::runtime_error {
      public:
         inline test_exception(int line,
                               const char* file,
                               const char* test,
                               const char* msg)
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
         inline void register_test_case(F test) {
            cases_.emplace_back(test);
         }

         inline int run_all() {
            int run(0);
            std::map<std::string, std::exception_ptr> errors;

            for (type ctor : cases_) {
               auto test = ctor();

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

         inline const char* current() const { return cur_; }

      private:
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
}

#define CALLGRAPH_TEST_STR2(T) #T
#define CALLGRAPH_TEST_STR(T) CALLGRAPH_TEST_STR2(T)
#define CALLGRAPH_TEST(T) class test_case_ ## T :                       \
      public callgraph_test::test_case_base {                           \
   public:                                                              \
      void run() override;                                              \
      const char* name() const override {                               \
         return CALLGRAPH_TEST_STR(T);                                  \
      }                                                                 \
   };                                                                   \
   namespace {                                                          \
      callgraph_test::test_case<test_case_##T> test_case_##T##_instance; \
   }                                                                    \
   void test_case_ ## T::run()

#define CALLGRAPH_ERROR(m)                                              \
   throw callgraph_test::test_exception(__LINE__,                       \
                                        __FILE__,                       \
                                        callgraph_test::                \
                                        global_test_engine().current(), \
                                        m)

#define CALLGRAPH_CHECK(a) if (!(a)) {          \
      CALLGRAPH_ERROR(CALLGRAPH_TEST_STR(a));   \
   }

#define CALLGRAPH_EQUAL(a,b) if ((a) != (b)) {                          \
      CALLGRAPH_ERROR(CALLGRAPH_TEST_STR(a) "==" CALLGRAPH_TEST_STR(b)); \
   }

#define CALLGRAPH_THROWS(a) { bool threw(false);        \
      try {                                             \
         a;                                             \
      } catch(...) {                                    \
         threw = true;                                  \
      }                                                 \
      if (!threw) {                                     \
         CALLGRAPH_ERROR("Did not throw.");             \
      }                                                 \
   }

#endif // CALLGRAPH_TEST_HPP
