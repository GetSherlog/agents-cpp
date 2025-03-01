#pragma once

#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/coroutine_utils.h>
#include <functional>
#include <memory>

namespace agents {

/**
 * @brief Interface for agents
 * 
 * Agents are LLM-powered systems that can use tools and make decisions
 * to accomplish a task.
 */
class Agent {
public:
    /**
     * @brief Agent execution state
     */
    enum class State {
        READY,      // Ready to start execution
        RUNNING,    // Currently executing
        WAITING,    // Waiting for human input
        COMPLETED,  // Execution completed successfully
        FAILED,     // Execution failed
        STOPPED     // Execution stopped by user
    };
    
    /**
     * @brief Agent execution options
     */
    struct Options {
        int max_iterations = 10;
        int max_consecutive_errors = 3;
        bool human_feedback_enabled = true;
        std::function<bool(const String&, const JsonObject&)> human_in_the_loop;
    };
    
    Agent(std::shared_ptr<AgentContext> context);
    virtual ~Agent() = default;
    
    // Initialize the agent
    virtual void init() = 0;
    
    // Run the agent with a task using coroutines
    virtual Task<JsonObject> run(const String& task) = 0;
    
    // Stop the agent
    virtual void stop();
    
    // Get the agent's context
    std::shared_ptr<AgentContext> getContext() const;
    
    // Get the agent's current state
    State getState() const;
    
    // Set execution options
    void setOptions(const Options& options);
    
    // Get execution options
    const Options& getOptions() const;
    
    // Set a callback for status updates
    void setStatusCallback(std::function<void(const String&)> callback);
    
    // Provide human feedback
    virtual void provideFeedback(const String& feedback);
    
    // Wait for feedback using coroutines
    virtual Task<String> waitForFeedback(
        const String& message, 
        const JsonObject& context
    ) {
        // Placeholder implementation, to be overridden
        co_return "";
    }

protected:
    std::shared_ptr<AgentContext> context_;
    State state_ = State::READY;
    Options options_;
    std::function<void(const String&)> status_callback_;
    
    // Update the agent's state
    void setState(State state);
    
    // Log a status message
    void logStatus(const String& status);
};

} // namespace agents 