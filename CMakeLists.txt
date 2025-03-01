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

# Boost for various utilities
FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.84.0
)
set(BOOST_INCLUDE_LIBRARIES 
    context
    filesystem
    system
    thread
    program_options
    regex
)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_MakeAvailable(Boost)

# Dependencies for Folly
find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_library(EVENT_LIBRARY event)
find_library(EVENT_CORE_LIBRARY event_core)
find_library(EVENT_EXTRA_LIBRARY event_extra)
find_library(EVENT_SSL_LIBRARY event_ssl)
find_library(EVENT_PTHREADS_LIBRARY event_pthreads)

# Folly for coroutines and async functionality
FetchContent_Declare(
    folly
    GIT_REPOSITORY https://github.com/facebook/folly.git
    GIT_TAG v2024.03.25.00
)
FetchContent_MakeAvailable(folly)

# CAF (C++ Actor Framework) for actor-based concurrency
# Commented out until we have proper support for it
# FetchContent_Declare(
#     caf
#     GIT_REPOSITORY https://github.com/actor-framework/actor-framework.git
#     GIT_TAG 0.19.3
# )
# set(CAF_ENABLE_EXAMPLES OFF CACHE INTERNAL "")
# set(CAF_ENABLE_TESTING OFF CACHE INTERNAL "")
# set(CAF_ENABLE_TOOLS OFF CACHE INTERNAL "")
# FetchContent_MakeAvailable(caf)

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