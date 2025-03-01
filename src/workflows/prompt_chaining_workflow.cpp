#include <agents-cpp/workflows/prompt_chaining_workflow.h>
#include <spdlog/spdlog.h>

namespace agents {
namespace workflows {

// Message atoms specific to prompt chaining - commented out until CAF is available

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
    spdlog::debug("Prompt chaining workflow initialized with {} steps (using CAF)", steps_.size());
    
    // Setup actor roles
    setupActorSystem();
    
    // Initialize controller with steps
    if (actor_system_ && controller_actor_) {
        caf::anon_send(controller_actor_, initialize_atom_v, steps_, llm_);
    }
}

// Execute the workflow with input (renamed to run for base class compatibility)
JsonObject PromptChainingWorkflow::run(const String& input) {
    spdlog::debug("Running prompt chaining workflow with input: {}", input);
    
    try {
        // Create input JSON
        JsonObject input_json = {{"input", input}};
        
        // Process through actor and wait for result
        auto result = caf::request_receive<JsonObject>(*actor_system_, controller_actor_, input_json);
        
        return result;
    } catch (const std::exception& e) {
        spdlog::error("Exception executing prompt chaining workflow: {}", e.what());
        return {{"error", e.what()}};
    }
}


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
        [=](initialize_atom, const std::vector<PromptChainingWorkflow::Step>& steps, 
            std::shared_ptr<LLMInterface> llm) {
            // Initialize state
            self->state.steps = steps;
            self->state.llm = llm;
            self->state.current_step_index = 0;
            spdlog::debug("Controller initialized with {} steps", steps.size());
        },
        [=](const JsonObject& input) -> JsonObject {
            spdlog::debug("Controller received input: {}", input.dump());
            
            // Reset state
            self->state.current_context = input;
            self->state.current_step_index = 0;
            self->state.final_result = JsonObject();
            
            // Start the first step
            self->send(self, step_atom_v);
            
            // Wait for all steps to complete
            return self->state.final_result;
        },
        [=](step_atom) {
            if (self->state.current_step_index >= self->state.steps.size()) {
                // All steps completed
                self->send(self, done_atom_v);
                return;
            }
            
            const auto& step = self->state.steps[self->state.current_step_index];
            spdlog::debug("Executing step {}: {}", self->state.current_step_index, step.name);
            
            try {
                // Format the prompt template with the current context
                String prompt = step.prompt_template;
                // Simple JSON template replacement would go here
                
                // Create message for LLM
                Message msg;
                msg.role = Message::Role::USER;
                msg.content = prompt;
                
                // Call the LLM
                auto llm_response = self->state.llm->chat({msg});
                
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
                    self->send(self, error_atom_v, "Validation failed for step " + step.name);
                    return;
                }
                
                // Transform the result for the next step if a transformer is provided
                JsonObject transformed_result = step_result;
                if (step.transformer) {
                    transformed_result = step.transformer(step_result);
                }
                
                // Update the context for the next step
                self->state.current_context = transformed_result;
                
                // Move to next step
                self->state.current_step_index++;
                
                // Continue with next step
                self->send(self, next_atom_v);
                
            } catch (const std::exception& e) {
                spdlog::error("Error in step {}: {}", step.name, e.what());
                self->send(self, error_atom_v, std::string("Error in step ") + step.name + ": " + e.what());
            }
        },
        [=](next_atom) {
            // Continue with next step
            self->send(self, step_atom_v);
        },
        [=](done_atom) {
            // Workflow completed
            spdlog::debug("Prompt chaining workflow completed");
            self->state.final_result = self->state.current_context;
        },
        [=](error_atom, const std::string& error_msg) {
            // Handle error
            spdlog::error("Workflow error: {}", error_msg);
            self->state.final_result = {{"error", error_msg}};
        }
    };
}

} // namespace workflows
} // namespace agents 