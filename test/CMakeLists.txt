# License: BSD-2-Clause
# A CMake project for the callgraph tests.
cmake_minimum_required(VERSION 3.1)

set(NAME callgraph_test)

project(${NAME})

# Set required standard
set(CMAKE_CXX_STANDARD 14)

if (MSVC)
   # Turn off some warnings that can safely be ignored.
   add_definitions(/D_SCL_SECURE_NO_WARNINGS)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275")

   # Set some useful MSVC flags
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

# Find required packages
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CALLGRAPH_TESTS_SOURCES
  callgraph_depth_test.cpp
  callgraph_connect_test.cpp
  callgraph_validity_test.cpp
  callgraph_run_test.cpp
  callgraph_functional_test.cpp
  callgraph_thread_test.cpp)

set(TEST_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(callgraph_tests ${CALLGRAPH_TESTS_SOURCES} ${TEST_MAIN})
add_test(NAME callgraph_tests COMMAND callgraph_tests)
target_link_libraries(callgraph_tests callgraph Threads::Threads)
