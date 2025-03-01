cmake_minimum_required(VERSION 3.14)
project(agents-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(AGENTS_CPP_BUILD_EXAMPLES "Build example applications" ON)
option(AGENTS_CPP_BUILD_TESTS "Build tests" OFF)

# Find packages
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# Set required C++ standard for coroutine support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Add external dependencies
include(FetchContent)

# nlohmann/json for JSON handling
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# CPR for HTTP requests (C++ Requests wrapper around libcurl)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.4
)
set(CPR_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(cpr)

# spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Use system-installed Boost instead of building from source
find_package(Boost 1.74.0 REQUIRED COMPONENTS 
    context
    filesystem
    system
    thread
    program_options
    regex
)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Folly and its dependencies
# First, find required system dependencies
find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(double-conversion REQUIRED)

# Fetch and build Folly from source
FetchContent_Declare(
    folly
    GIT_REPOSITORY https://github.com/facebook/folly.git
    GIT_TAG v2023.05.15.00  # Use a specific stable version tag
)
# Configure Folly build options
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")  # Build as static library
set(BUILD_TESTS OFF CACHE INTERNAL "")
set(FOLLY_BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(folly)

# CAF (C++ Actor Framework) for actor-based concurrency
FetchContent_Declare(
    caf
    GIT_REPOSITORY https://github.com/actor-framework/actor-framework.git
    GIT_TAG 0.19.3
)
set(CAF_ENABLE_EXAMPLES OFF CACHE INTERNAL "")
set(CAF_ENABLE_TESTING OFF CACHE INTERNAL "")
set(CAF_ENABLE_TOOLS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(caf)

# Add library targets
add_subdirectory(src)

# Build examples if enabled
if(AGENTS_CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tests if enabled
if(AGENTS_CPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set(SOURCE_FILES
    src/llm_interface.cpp
    src/agent_context.cpp
    src/tools/tool_registry.cpp
    src/tools/web_search_tool.cpp
    src/tools/wikipedia_tool.cpp
    src/workflows/prompt_chaining_workflow.cpp
    src/config_loader.cpp
    src/logger.cpp
) 