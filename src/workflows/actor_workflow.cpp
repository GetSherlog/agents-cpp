#include <agents-cpp/workflows/actor_workflow.h>
#include <spdlog/spdlog.h>

namespace agents {
namespace workflows {

// CAF dependencies commented out until the library is available

// Define actor types and messages
using workflow_actor_type = caf::typed_actor<
    caf::replies_to<JsonObject>::with<JsonObject>,
    caf::replies_to<caf::atom<execute_atom>, JsonObject>::with<void>
>;

// Message atoms
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(JsonObject)
CAF_ADD_ATOM(execute_atom, execute)
CAF_ADD_ATOM(stop_atom, stop)
CAF_ADD_ATOM(initialize_atom, init)

// Base actor behavior
workflow_actor_type::behavior_type baseWorkflowBehavior(workflow_actor_type::stateful_pointer<JsonObject> self) {
    return {
        [=](const JsonObject& input) -> JsonObject {
            // Store input in state
            self->state = input;
            
            // Default implementation - just return the input
            spdlog::debug("Base workflow actor received input: {}", input.dump());
            return input;
        },
        [=](initialize_atom) {
            spdlog::debug("Base workflow actor initialized");
        },
        [=](stop_atom) {
            spdlog::debug("Base workflow actor stopped");
            self->quit();
        }
    };
}


// Constructor
ActorWorkflow::ActorWorkflow(std::shared_ptr<AgentContext> context)
    : Workflow(context) {
    // Get LLM from context if available
    llm_ = context->getLLM();
    
    // Setup the actor system
    setupActorSystem();
}

// Destructor
ActorWorkflow::~ActorWorkflow() {
    stop();
}

// Initialize the workflow
void ActorWorkflow::init() {
    spdlog::debug("Actor workflow initialized (using CAF)");
    
    // Send initialize message to the workflow actor
    caf::anon_send(workflow_actor_, initialize_atom_v);
}

// Execute the workflow with input (renamed to run to match base class)
JsonObject ActorWorkflow::run(const String& input) {
    spdlog::debug("Running actor workflow with input: {}", input);
    
    try {
        // Create input JSON
        JsonObject input_json = {{"input", input}};
        
        // Process through actor and wait for result
        auto result = caf::request_receive<JsonObject>(*actor_system_, workflow_actor_, input_json);
        
        return result;
    } catch (const std::exception& e) {
        spdlog::error("Exception executing workflow: {}", e.what());
        return {{"error", e.what()}};
    }
}

// Stop the workflow
void ActorWorkflow::stop() {
    spdlog::debug("Actor workflow stopped (using CAF)");
    
    // Send stop message to the workflow actor
    if (actor_system_ && workflow_actor_) {
        caf::anon_send(workflow_actor_, stop_atom_v);
    }
}

// Get the workflow status
String ActorWorkflow::getStatus() const {
    return "Running (using CAF)";
}


// Create and configure the actor system
void ActorWorkflow::setupActorSystem() {
    if (!actor_system_) {
        // Create actor system config
        caf::actor_system_config cfg;
        
        // Create actor system
        actor_system_ = std::make_unique<caf::actor_system>(cfg);
        
        // Create workflow actor
        workflow_actor_ = actor_system_->spawn(baseWorkflowBehavior);
    }
}

} // namespace workflows
} // namespace agents 