#pragma once

#include <agents-cpp/workflows/actor_workflow.h>
#include <vector>
#include <functional>

namespace agents {
namespace workflows {

/**
 * @brief Prompt chaining workflow using the actor model
 * 
 * This workflow decomposes a task into a sequence of steps, where each LLM call
 * processes the output of the previous one. It can add programmatic checks between
 * steps to ensure the process is on track.
 */
class PromptChainingWorkflow : public ActorWorkflow {
public:
    // Define a step in the workflow
    struct Step {
        // Name of the step
        String name;
        
        // Prompt template for this step
        String prompt_template;
        
        // Function to validate step output (returns true if valid)
        std::function<bool(const JsonObject&)> validator;
        
        // Function to transform step output for the next step
        std::function<JsonObject(const JsonObject&)> transformer;
        
        Step(
            const String& name,
            const String& prompt_template,
            std::function<bool(const JsonObject&)> validator = nullptr,
            std::function<JsonObject(const JsonObject&)> transformer = nullptr
        ) : name(name), prompt_template(prompt_template), 
            validator(validator), transformer(transformer) {}
    };
    
    // Constructor with context
    PromptChainingWorkflow(std::shared_ptr<AgentContext> context);
    
    // Add a step to the workflow
    void addStep(const Step& step);
    
    // Add a step to the workflow with basic params
    void addStep(
        const String& name,
        const String& prompt_template,
        std::function<bool(const JsonObject&)> validator = nullptr,
        std::function<JsonObject(const JsonObject&)> transformer = nullptr
    );
    
    // Initialize the workflow
    void init() override;
    
    // Execute the workflow with input (renamed to match base class)
    JsonObject run(const String& input) override;
    
private:
    // List of steps in the workflow
    std::vector<Step> steps_;
    
    // CAF dependencies commented out until available
    // Actor for the workflow controller
    // caf::actor controller_actor_;
    
    // Setup actor roles for this workflow
    // void setupActorSystem() override;
};

} // namespace workflows
} // namespace agents 