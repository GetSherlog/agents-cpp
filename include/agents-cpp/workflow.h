#pragma once

#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <functional>
#include <memory>

namespace agents {

/**
 * @brief Abstract base class for workflows
 * 
 * A workflow is a pattern for executing a series of 
 * LLM operations to accomplish a task.
 */
class Workflow {
public:
    Workflow(std::shared_ptr<AgentContext> context);
    virtual ~Workflow() = default;
    
    // Run the workflow with a user input and return the result
    virtual JsonObject run(const String& input) = 0;
    
    // Run the workflow with a user input asynchronously
    virtual void runAsync(
        const String& input,
        std::function<void(const JsonObject&)> callback
    );
    
    // Get the workflow's context
    std::shared_ptr<AgentContext> getContext() const;
    
    // Set a callback for intermediate steps
    void setStepCallback(std::function<void(const String&, const JsonObject&)> callback);
    
    // Set maximum number of steps
    void setMaxSteps(int max_steps);
    
    // Get maximum number of steps
    int getMaxSteps() const;

protected:
    std::shared_ptr<AgentContext> context_;
    std::function<void(const String&, const JsonObject&)> step_callback_;
    int max_steps_ = 10;
    
    // Log a step with description and result
    void logStep(const String& description, const JsonObject& result);
};

} // namespace agents 