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

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/routing_example.cpp)
#     add_executable(routing_example ${CMAKE_CURRENT_SOURCE_DIR}/routing_example.cpp)
#     target_link_libraries(routing_example PRIVATE agents-cpp)
#     list(APPEND EXAMPLE_TARGETS routing_example)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/parallel_example.cpp)
#     add_executable(parallel_example ${CMAKE_CURRENT_SOURCE_DIR}/parallel_example.cpp)
#     target_link_libraries(parallel_example PRIVATE agents-cpp)
#     list(APPEND EXAMPLE_TARGETS parallel_example)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/orchestrator_example.cpp)
#     add_executable(orchestrator_example ${CMAKE_CURRENT_SOURCE_DIR}/orchestrator_example.cpp)
#     target_link_libraries(orchestrator_example PRIVATE agents-cpp)
#     list(APPEND EXAMPLE_TARGETS orchestrator_example)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/evaluator_optimizer_example.cpp)
#     add_executable(evaluator_optimizer_example ${CMAKE_CURRENT_SOURCE_DIR}/evaluator_optimizer_example.cpp)
#     target_link_libraries(evaluator_optimizer_example PRIVATE agents-cpp)
#     list(APPEND EXAMPLE_TARGETS evaluator_optimizer_example)
# endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/autonomous_agent_example.cpp)
#     add_executable(autonomous_agent_example ${CMAKE_CURRENT_SOURCE_DIR}/autonomous_agent_example.cpp)
#     target_link_libraries(autonomous_agent_example PRIVATE agents-cpp)
#     list(APPEND EXAMPLE_TARGETS autonomous_agent_example)
# endif()

# New actor-based agent example
# add_executable(actor_agent_example actor_agent_example.cpp)
# target_link_libraries(actor_agent_example PRIVATE agents-cpp)

# Install example executables if any are defined
if(DEFINED EXAMPLE_TARGETS)
    install(TARGETS ${EXAMPLE_TARGETS} RUNTIME DESTINATION bin/examples)
endif()

# Install explicitly defined examples
# install(TARGETS 
#     actor_agent_example
#     RUNTIME DESTINATION bin/examples
# ) 

add_executable(routing_example ${CMAKE_CURRENT_SOURCE_DIR}/routing_example.cpp)
target_link_libraries(routing_example PRIVATE agents-cpp)

add_executable(autonomous_agent_example ${CMAKE_CURRENT_SOURCE_DIR}/autonomous_agent_example.cpp)
target_link_libraries(autonomous_agent_example PRIVATE agents-cpp)

add_executable(actor_agent_example ${CMAKE_CURRENT_SOURCE_DIR}/actor_agent_example.cpp)
target_link_libraries(actor_agent_example PRIVATE agents-cpp)

add_executable(coroutine_example ${CMAKE_CURRENT_SOURCE_DIR}/coroutine_example.cpp)
target_link_libraries(coroutine_example PRIVATE agents-cpp) 