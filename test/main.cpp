#define CALLGRAPH_TEST_MAIN 1
#include "test.hpp"

char* ARGV_0;

int main(int argc, char** argv) {
   ARGV_0 = argv[0];
  return callgraph_test::global_test_engine().run_all();
}
