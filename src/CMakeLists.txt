# Initialize empty source list
set(AGENTS_CPP_SOURCES)

# Function to check and add source files if they exist
function(check_and_add_source file)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        list(APPEND AGENTS_CPP_SOURCES ${file})
        set(AGENTS_CPP_SOURCES ${AGENTS_CPP_SOURCES} PARENT_SCOPE)
    endif()
endfunction()

# Check each potential source file
check_and_add_source(core/agent_context.cpp)
check_and_add_source(core/tool.cpp)
check_and_add_source(core/memory.cpp)
check_and_add_source(llms/llm_interface.cpp)
check_and_add_source(llms/anthropic_llm.cpp)
check_and_add_source(llms/openai_llm.cpp)
check_and_add_source(llms/google_llm.cpp)
check_and_add_source(llms/ollama_llm.cpp)
check_and_add_source(workflows/workflow.cpp)
check_and_add_source(workflows/prompt_chain.cpp)
check_and_add_source(workflows/routing.cpp)
check_and_add_source(workflows/parallelization.cpp)
check_and_add_source(workflows/orchestrator_workers.cpp)
check_and_add_source(workflows/evaluator_optimizer.cpp)
check_and_add_source(agents/agent.cpp)
check_and_add_source(agents/autonomous_agent.cpp)
check_and_add_source(tools/tool_registry.cpp)
check_and_add_source(tools/file_tool.cpp)
check_and_add_source(tools/search_tool.cpp)
check_and_add_source(tools/system_tool.cpp)
check_and_add_source(memory/conversation_memory.cpp)
check_and_add_source(memory/vector_memory.cpp)
check_and_add_source(agents/basic_agent.cpp)
check_and_add_source(workflows/basic_workflow.cpp)
check_and_add_source(workflows/actor_workflow.cpp)
check_and_add_source(workflows/prompt_chaining_workflow.cpp)
check_and_add_source(workflows/parallelization_workflow.cpp)
check_and_add_source(workflows/orchestrator_workflow.cpp)
check_and_add_source(workflows/evaluator_workflow.cpp)
check_and_add_source(workflows/routing_workflow.cpp)
check_and_add_source(agents/actor_agent.cpp)
check_and_add_source(config_loader.cpp)
check_and_add_source(logger.cpp)

# If no source files found, create a stub file
if(NOT AGENTS_CPP_SOURCES)
    message(STATUS "No source files found. Creating stub library.")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stub.cpp "// Stub file\n")
    set(AGENTS_CPP_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/stub.cpp)
endif()

# Create the library
add_library(agents-cpp STATIC ${AGENTS_CPP_SOURCES})

target_include_directories(agents-cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link against required libraries
target_link_libraries(agents-cpp PUBLIC
    nlohmann_json::nlohmann_json
    cpr::cpr
    spdlog::spdlog
    Threads::Threads
    ${CURL_LIBRARIES}
    ${Boost_LIBRARIES}
    Folly::folly
    gflags
    glog::glog
    fmt::fmt
    ZLIB::ZLIB
    double-conversion::double-conversion
    # CAF dependencies commented out until it's properly supported
    caf_core caf_io
)

# Install targets
install(TARGETS agents-cpp
    EXPORT agents-cpp-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/
    DESTINATION include
) 