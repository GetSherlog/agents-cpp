#pragma once

#include <agents-cpp/agent.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/coroutine_utils.h>
#include <caf/all.hpp>
#include <memory>
#include <map>
#include <vector>

namespace agents {

/**
 * @brief Actor-based agent implementation
 * 
 * This class uses the C++ Actor Framework (CAF) to implement a flexible agent
 * that can operate autonomously, use tools, and achieve complex tasks.
 */
class ActorAgent : public Agent {
public:
    // Constructor with agent context
    ActorAgent(std::shared_ptr<AgentContext> context);
    
    // Destructor
    virtual ~ActorAgent();
    
    // Initialize the agent
    void init() override;
    
    // Run the agent with a task using coroutines
    Task<JsonObject> run(const String& task) override;
    
    // Stop the agent
    void stop() override;
    
    // Provide human feedback
    void provideFeedback(const String& feedback) override;
    
    // Set the system prompt for the agent
    void setSystemPrompt(const String& system_prompt);
    
    // Get the current system prompt
    String getSystemPrompt() const;
    
    // Wait for feedback using coroutines
    Task<String> waitForFeedback(const String& message, const JsonObject& context) override;
    
protected:
    // Actor system
    std::unique_ptr<caf::actor_system> actor_system_;
    
    // Main agent actor
    caf::actor agent_actor_;
    
    // Tool actors
    std::map<String, caf::actor> tool_actors_;
    
    // System prompt
    String system_prompt_;
    
    // Current conversation history
    std::vector<Message> conversation_;
    
    // Run interval in ms (how often the agent checks for new messages)
    int run_interval_ms_ = 100;
    
    // Coroutine promise for feedback
    folly::Promise<String> feedback_promise_;
    
    // Callback for when a tool is used
    virtual void onToolUsed(const String& tool_name, const JsonObject& params, const ToolResult& result);
    
    // Callback for when the agent generates a response
    virtual void onResponse(const String& response);
    
    // Callback for when the agent errors
    virtual void onError(const String& error);
    
    // Create and configure the actor system
    virtual void setupActorSystem();
    
    // Setup tool actors
    virtual void setupToolActors();
    
    // Create the agent prompt with available tools
    virtual String createAgentPrompt() const;
    
    // Execute a tool with coroutines
    Task<ToolResult> executeTool(const String& tool_name, const JsonObject& params);
    
    // Process a message with coroutines
    Task<String> processMessage(const String& message);
};

} // namespace agents