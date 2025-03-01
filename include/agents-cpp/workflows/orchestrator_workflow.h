#pragma once

#include <agents-cpp/workflows/actor_workflow.h>
#include <vector>
#include <functional>
#include <map>

namespace agents {
namespace workflows {

/**
 * @brief Orchestrator-Workers workflow using the actor model
 * 
 * In this workflow, a central LLM (orchestrator) dynamically breaks down tasks,
 * delegates them to worker LLMs, and synthesizes their results. This is well-suited
 * for complex tasks where subtasks can't be predetermined.
 */
class OrchestratorWorkflow : public ActorWorkflow {
public:
    // Worker definition
    struct Worker {
        String name;
        String description;
        String prompt_template;
        std::shared_ptr<LLMInterface> llm;  // Optional separate LLM for this worker
        
        Worker(
            const String& name,
            const String& description,
            const String& prompt_template,
            std::shared_ptr<LLMInterface> llm = nullptr
        ) : name(name), description(description), 
            prompt_template(prompt_template), llm(llm) {}
    };
    
    // Constructor with orchestrator LLM interface
    OrchestratorWorkflow(
        std::shared_ptr<LLMInterface> orchestrator_llm,
        const String& orchestrator_prompt_template
    );
    
    // Add a worker to the workflow
    void addWorker(const Worker& worker);
    
    // Add a worker with basic params
    void addWorker(
        const String& name,
        const String& description,
        const String& prompt_template,
        std::shared_ptr<LLMInterface> worker_llm = nullptr
    );
    
    // Initialize the workflow
    void init() override;
    
    // Execute the workflow with input
    JsonObject execute(const JsonObject& input) override;
    
    // Set the max number of iterations
    void setMaxIterations(int max_iterations);
    
private:
    // Orchestrator prompt template
    String orchestrator_prompt_template_;
    
    // List of available workers
    std::vector<Worker> workers_;
    
    // Actor for the orchestrator
    caf::actor orchestrator_actor_;
    
    // Map of worker actors
    std::map<String, caf::actor> worker_actors_;
    
    // Max number of iterations
    int max_iterations_ = 5;
    
    // Setup actor roles for this workflow
    void setupActorSystem() override;
    
    // Create the orchestrator system prompt with worker descriptions
    String createOrchestratorSystemPrompt() const;
};

} // namespace workflows
} // namespace agents 