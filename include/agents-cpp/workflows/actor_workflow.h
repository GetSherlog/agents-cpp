#pragma once

#include <agents-cpp/workflow.h>
#include <agents-cpp/llm_interface.h>

// Comment out CAF includes to allow building without the actor framework
// #include <caf/all.hpp>

namespace agents {
namespace workflows {

/**
 * @brief Base class for actor-based workflows
 * 
 * This class implements the Workflow interface using the C++ Actor Framework (CAF).
 * It provides a foundation for building more complex workflows like:
 * - Prompt chaining
 * - Routing
 * - Parallelization
 * - Orchestrator-workers
 * - Evaluator-optimizer
 */
class ActorWorkflow : public Workflow {
public:
    // Constructor with LLM interface
    ActorWorkflow(std::shared_ptr<AgentContext> context);
    
    // Destructor
    virtual ~ActorWorkflow();
    
    // Run the workflow with input
    virtual JsonObject run(const String& input) override;
    
    // Initialize the workflow
    virtual void init();
    
    // Stop the workflow
    virtual void stop();
    
    // Get the workflow status
    virtual String getStatus() const;
    
protected:
    // LLM interface for the workflow
    std::shared_ptr<LLMInterface> llm_;
    
    // Create and configure the actor system - commented out until CAF is available
    /*
    // CAF actor system
    std::unique_ptr<caf::actor_system> actor_system_;
    
    // Core actor that manages the workflow
    caf::actor workflow_actor_;
    
    virtual void setupActorSystem();
    */
};

} // namespace workflows
} // namespace agents 