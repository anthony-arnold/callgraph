# License: BSD-2-Clause
# Main CMake project for the callgraph library.
cmake_minimum_required(VERSION 3.1)

set(NAME callgraph)
set(VERSION 0.0.0)
project(${NAME} VERSION ${VERSION})

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Target definition
add_library(callgraph INTERFACE)
target_include_directories(callgraph INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS ${NAME} EXPORT ${NAME}-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
install(DIRECTORY callgraph DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/callgraph/callgraph-version.cmake"
  VERSION ${VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT callgraph-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/callgraph/callgraph-targets.cmake"
       NAMESPACE callgraph::)

configure_file(cmake/callgraph-config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/callgraph/callgraph-config.cmake"
  COPYONLY
)

install(EXPORT callgraph-targets
        FILE callgraph-targets.cmake
        NAMESPACE callgraph::
        DESTINATION lib/cmake/callgraph)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/callgraph/callgraph-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/callgraph/callgraph-version.cmake"
        DESTINATION lib/cmake/callgraph
        COMPONENT Devel)

# Build the tests
enable_testing()
add_subdirectory(test)

# Build the documentation
find_package(Doxygen)

if (DOXYGEN_FOUND)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
                  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(doc ALL
       COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
       COMMENT "Generating documentation."
       VERBATIM)
   install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc DESTINATION share/callgraph)
endif()
