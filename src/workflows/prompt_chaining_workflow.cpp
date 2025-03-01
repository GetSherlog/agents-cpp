#include <agents-cpp/workflows/prompt_chaining_workflow.h>
#include <spdlog/spdlog.h>

namespace agents {
namespace workflows {

// Message atoms specific to prompt chaining - commented out until CAF is available
/*
using step_atom = caf::atom_constant<caf::atom("step")>;
using next_atom = caf::atom_constant<caf::atom("next")>;
using done_atom = caf::atom_constant<caf::atom("done")>;
using error_atom = caf::atom_constant<caf::atom("error")>;

// Define the controller actor state
struct ControllerState {
    std::vector<PromptChainingWorkflow::Step> steps;
    std::shared_ptr<LLMInterface> llm;
    JsonObject current_context;
    JsonObject final_result;
    size_t current_step_index = 0;
    caf::actor supervisor;
};

// Forward declaration of controller behavior
caf::behavior controllerBehavior(caf::stateful_actor<ControllerState>* self);
*/

// Constructor
PromptChainingWorkflow::PromptChainingWorkflow(std::shared_ptr<AgentContext> context)
    : ActorWorkflow(context) {
}

// Add a step to the workflow
void PromptChainingWorkflow::addStep(const Step& step) {
    steps_.push_back(step);
}

// Add a step to the workflow with basic params
void PromptChainingWorkflow::addStep(
    const String& name,
    const String& prompt_template,
    std::function<bool(const JsonObject&)> validator,
    std::function<JsonObject(const JsonObject&)> transformer
) {
    Step step(name, prompt_template, validator, transformer);
    steps_.push_back(step);
}

// Initialize the workflow
void PromptChainingWorkflow::init() {
    // Call base class initialization
    ActorWorkflow::init();
    
    // Initialize with steps - CAF actor creation commented out
    spdlog::debug("Prompt chaining workflow initialized with {} steps (CAF disabled)", steps_.size());
}

// Execute the workflow with input (renamed to run for base class compatibility)
JsonObject PromptChainingWorkflow::run(const String& input) {
    spdlog::debug("Running prompt chaining workflow with input: {}", input);
    
    try {
        // Simplified implementation without CAF actors
        JsonObject current_context = {{"input", input}};
        JsonObject final_result;
        
        // Process each step in sequence
        for (size_t i = 0; i < steps_.size(); i++) {
            const auto& step = steps_[i];
            spdlog::debug("Executing step {}: {}", i, step.name);
            
            // Format the prompt template with the current context
            // This is a simplified version - in a real implementation, you'd use a 
            // template engine to replace variables in the prompt template
            String prompt = step.prompt_template;
            // Simple JSON template replacement would go here
            
            // Create message for LLM
            Message msg;
            msg.role = Message::Role::USER;
            msg.content = prompt;
            
            try {
                // Call the LLM
                auto llm_response = llm_->chat({msg});
                
                // Convert the response to JSON
                JsonObject step_result = {
                    {"name", step.name},
                    {"prompt", prompt},
                    {"response", llm_response.content}
                };
                
                // Validate the result if a validator is provided
                bool valid = true;
                if (step.validator) {
                    valid = step.validator(step_result);
                }
                
                if (!valid) {
                    spdlog::error("Step {} validation failed", step.name);
                    return {{"error", "Validation failed for step " + step.name}};
                }
                
                // Transform the result for the next step if a transformer is provided
                JsonObject transformed_result = step_result;
                if (step.transformer) {
                    transformed_result = step.transformer(step_result);
                }
                
                // Update the context for the next step
                current_context = transformed_result;
                
                // Save the final step result
                if (i == steps_.size() - 1) {
                    final_result = transformed_result;
                }
                
            } catch (const std::exception& e) {
                spdlog::error("Error in step {}: {}", step.name, e.what());
                return {{"error", String("Error in step ") + step.name + ": " + e.what()}};
            }
        }
        
        return final_result.empty() ? current_context : final_result;
        
    } catch (const std::exception& e) {
        spdlog::error("Exception executing prompt chaining workflow: {}", e.what());
        return {{"error", e.what()}};
    }
}

/*
// Setup actor roles for this workflow
void PromptChainingWorkflow::setupActorSystem() {
    // Call base class setup
    ActorWorkflow::setupActorSystem();
    
    // Create controller actor
    if (actor_system_ && !controller_actor_) {
        controller_actor_ = actor_system_->spawn(controllerBehavior);
    }
}

// Controller behavior implementation
caf::behavior controllerBehavior(caf::stateful_actor<ControllerState>* self) {
    return {
        // CAF actor implementation code removed
    };
}
*/

} // namespace workflows
} // namespace agents 