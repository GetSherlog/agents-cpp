#pragma once

#include <agents-cpp/workflow.h>
#include <vector>
#include <functional>

namespace agents {
namespace workflows {

/**
 * @brief A workflow where a central orchestrator delegates tasks to workers
 * 
 * In the orchestrator-workers workflow, a central LLM dynamically breaks down tasks,
 * delegates them to worker LLMs, and synthesizes their results.
 */
class OrchestratorWorkers : public Workflow {
public:
    /**
     * @brief Worker definition
     */
    struct Worker {
        String name;
        String description;
        String system_prompt;
        std::function<JsonObject(const String&, const JsonObject&)> handler;
    };
    
    OrchestratorWorkers(std::shared_ptr<AgentContext> context);
    ~OrchestratorWorkers() override = default;
    
    // Set the system prompt for the orchestrator
    void setOrchestratorPrompt(const String& orchestrator_prompt);
    
    // Register a worker
    void registerWorker(const Worker& worker);
    
    // Register a simple worker with just a system prompt
    void registerWorker(
        const String& name,
        const String& description,
        const String& system_prompt
    );
    
    // Register a worker with a custom handler
    void registerWorker(
        const String& name,
        const String& description,
        const String& system_prompt,
        std::function<JsonObject(const String&, const JsonObject&)> handler
    );
    
    // Set the result synthesizer function
    void setSynthesizer(std::function<JsonObject(const std::vector<JsonObject>&)> synthesizer);
    
    // Run the orchestrator-workers workflow
    JsonObject run(const String& input) override;
    
    // Get the schema for available workers
    JsonObject getWorkersSchema() const;

private:
    String orchestrator_prompt_;
    std::vector<Worker> workers_;
    std::function<JsonObject(const std::vector<JsonObject>&)> synthesizer_;
    
    // Default synthesizer function
    JsonObject defaultSynthesizer(const std::vector<JsonObject>& results);
    
    // Execute a worker by name
    JsonObject executeWorker(
        const String& worker_name,
        const String& task,
        const JsonObject& context_data
    );
};

} // namespace workflows
} // namespace agents 