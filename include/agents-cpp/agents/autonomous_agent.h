#pragma once

#include <agents-cpp/agent.h>
#include <agents-cpp/coroutine_utils.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <folly/futures/Promise.h>

namespace agents {

/**
 * @brief An agent that operates autonomously to complete a task
 * 
 * Autonomous agents start with a task, plan steps to accomplish it,
 * and use tools to execute those steps. They can be configured with
 * various strategies and human-in-the-loop options.
 */
class AutonomousAgent : public Agent {
public:
    /**
     * @brief Step in the agent's execution
     */
    struct Step {
        String description;
        String status;
        JsonObject result;
        bool success;
    };
    
    /**
     * @brief Planning strategy for the agent
     */
    enum class PlanningStrategy {
        ZERO_SHOT,   // Generate actions without explicit planning
        TREE_OF_THOUGHT, // Generate multiple reasoning paths
        PLAN_AND_EXECUTE, // Generate a plan then execute it
        REFLEXION,   // Reflect on past steps for improvement
        REACT        // Reasoning and acting
    };
    
    AutonomousAgent(std::shared_ptr<AgentContext> context);
    ~AutonomousAgent() override = default;
    
    // Initialize the agent
    void init() override;
    
    // Run the agent with a task using coroutines
    Task<JsonObject> run(const String& task) override;
    
    // Stop the agent
    void stop() override;
    
    // Provide human feedback
    void provideFeedback(const String& feedback) override;
    
    // Set the system prompt
    void setSystemPrompt(const String& system_prompt);
    
    // Set the planning strategy
    void setPlanningStrategy(PlanningStrategy strategy);
    
    // Get the steps executed so far
    std::vector<Step> getSteps() const;
    
    // Set a callback for when a step is completed
    void setStepCallback(std::function<void(const Step&)> callback);
    
    // Wait for feedback using coroutines
    Task<String> waitForFeedback(const String& message, const JsonObject& context) override;

private:
    String system_prompt_;
    PlanningStrategy planning_strategy_ = PlanningStrategy::REACT;
    std::vector<Step> steps_;
    std::function<void(const Step&)> step_callback_;
    
    // Execution state
    std::atomic<bool> should_stop_{false};
    
    // Promise for coroutine-based feedback
    folly::Promise<String> feedback_promise_;
    
    // Execute the agent's task using coroutines
    Task<JsonObject> executeTask(const String& task);
    
    // Execute a step using coroutines
    Task<Step> executeStep(const String& step_description, const JsonObject& context);
    
    // Record a completed step
    void recordStep(const Step& step);
    
    // Get tool descriptions for prompts
    String getToolDescriptions() const;
    
    // Planning strategies using coroutines
    Task<JsonObject> planZeroShot(const String& task);
    Task<JsonObject> planTreeOfThought(const String& task);
    Task<JsonObject> planAndExecute(const String& task);
    Task<JsonObject> planReflexion(const String& task);
    Task<JsonObject> planReact(const String& task);
};

} // namespace agents 