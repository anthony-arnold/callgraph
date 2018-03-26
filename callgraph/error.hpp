// callgraph/error.hpp
// License: BSD-2-Clause

#ifndef CALLGRAPH_ERROR_HPP
#define CALLGRAPH_ERROR_HPP

#include <stdexcept>
#include <sstream>

namespace callgraph {
#ifndef NO_DOC
    namespace detail {
        inline std::string file_line(const char* msg,
                                     const char* file,
                                     int line) {
            std::ostringstream ss;
            ss << msg << "(" << file << ":" << line << ")";
            return ss.str();
        }
    }
#endif

/// \brief An error thrown if connecting a node would cause a cycle.
    class cycle_error : public std::runtime_error {
    public:
        /// \brief Construct a new exception.
        cycle_error()
            : runtime_error("cycle detected")
            {
            }
        /// \brief Construct a new exception thrown at the given location.
        /// \param file The source file containing the throw site.
        /// \param line The line of the throw site.
        cycle_error(const char* file, int line)
            : runtime_error(detail::file_line("cycle detected", file, line))
            {
            }
    };

/// \brief An error thrown if the source node in a connection
/// request is not found.
    class source_node_not_found : public std::runtime_error {
    public:
        /// \brief Construct a new exception.
        source_node_not_found()
            : runtime_error("source node not found")
            {
            }
        /// \brief Construct a new exception thrown at the given location.
        /// \param file The source file containing the throw site.
        /// \param line The line of the throw site.
        source_node_not_found(const char* file, int line)
            : runtime_error(detail::file_line("source node not found",
                                              file,
                                              line))
            {
            }
    };

/// \brief An error thrown if a node's required parameters
/// are not supplied.
    class node_parameter_missing : public std::runtime_error {
    public:
        /// \brief Construct a new exception.
        node_parameter_missing()
            : runtime_error("parameter not set")
            {
            }
        /// \brief Construct a new exception thrown at the given location.
        /// \param file The source file containing the throw site.
        /// \param line The line of the throw site.
        node_parameter_missing(const char* file, int line)
            : runtime_error(detail::file_line("parameter not set",
                                              file,
                                              line))
            {
            }
    };
}

#undef CALLGRAPH_THROW
#define CALLGRAPH_THROW(x) throw x(__FILE__, __LINE__)

#endif // CALLGRAPH_ERROR_HPP
