# Add example executables 
message(STATUS "Building examples with implemented functionality")

# When the library implementation is complete, uncomment these lines to build examples
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simple_agent.cpp)
    add_executable(simple_agent ${CMAKE_CURRENT_SOURCE_DIR}/simple_agent.cpp)
    target_link_libraries(simple_agent PRIVATE agents-cpp)
    list(APPEND EXAMPLE_TARGETS simple_agent)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/prompt_chain_example.cpp)
    add_executable(prompt_chain_example ${CMAKE_CURRENT_SOURCE_DIR}/prompt_chain_example.cpp)
    target_link_libraries(prompt_chain_example PRIVATE agents-cpp)
    list(APPEND EXAMPLE_TARGETS prompt_chain_example)
endif()

# Add our new simple workflow example
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/simple_workflow.cpp)
    add_executable(simple_workflow ${CMAKE_CURRENT_SOURCE_DIR}/simple_workflow.cpp)
    target_link_libraries(simple_workflow PRIVATE agents-cpp)
    list(APPEND EXAMPLE_TARGETS simple_workflow)
endif()

# Actor-based agent example
add_executable(actor_agent_example ${CMAKE_CURRENT_SOURCE_DIR}/actor_agent_example.cpp)
target_link_libraries(actor_agent_example PRIVATE agents-cpp)
list(APPEND EXAMPLE_TARGETS actor_agent_example)

# Routing example
add_executable(routing_example ${CMAKE_CURRENT_SOURCE_DIR}/routing_example.cpp)
target_link_libraries(routing_example PRIVATE agents-cpp)
list(APPEND EXAMPLE_TARGETS routing_example)

# Autonomous agent example
add_executable(autonomous_agent_example ${CMAKE_CURRENT_SOURCE_DIR}/autonomous_agent_example.cpp)
target_link_libraries(autonomous_agent_example PRIVATE agents-cpp)
list(APPEND EXAMPLE_TARGETS autonomous_agent_example)

# Coroutine example with Folly
add_executable(coroutine_example ${CMAKE_CURRENT_SOURCE_DIR}/coroutine_example.cpp)
target_link_libraries(coroutine_example PRIVATE agents-cpp)
list(APPEND EXAMPLE_TARGETS coroutine_example)

# Install example executables if any are defined
if(DEFINED EXAMPLE_TARGETS)
    install(TARGETS ${EXAMPLE_TARGETS} RUNTIME DESTINATION bin/examples)
endif() 