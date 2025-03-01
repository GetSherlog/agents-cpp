#pragma once

#include <agents-cpp/workflows/actor_workflow.h>
#include <vector>
#include <functional>

namespace agents {
namespace workflows {

/**
 * @brief Parallelization workflow using the actor model
 * 
 * This workflow allows LLMs to work simultaneously on a task and have their outputs
 * aggregated. It supports two key variations:
 * - Sectioning: Breaking a task into independent subtasks run in parallel
 * - Voting: Running the same task multiple times to get diverse outputs
 */
class ParallelizationWorkflow : public ActorWorkflow {
public:
    // Enum for parallelization strategy
    enum class Strategy {
        SECTIONING,  // Break task into independent subtasks
        VOTING       // Run same task multiple times for consensus
    };
    
    // Task definition for parallel execution
    struct Task {
        String name;
        String prompt_template;
        JsonObject context;
        
        Task(
            const String& name,
            const String& prompt_template,
            const JsonObject& context = JsonObject()
        ) : name(name), prompt_template(prompt_template), context(context) {}
    };
    
    // Constructor with LLM interface
    ParallelizationWorkflow(
        std::shared_ptr<LLMInterface> llm, 
        Strategy strategy = Strategy::SECTIONING
    );
    
    // Add a task to the workflow
    void addTask(const Task& task);
    
    // Add a task to the workflow with basic params
    void addTask(
        const String& name,
        const String& prompt_template,
        const JsonObject& context = JsonObject()
    );
    
    // Set the aggregation function for task results
    void setAggregator(std::function<JsonObject(const std::vector<JsonObject>&)> aggregator);
    
    // Set the strategy
    void setStrategy(Strategy strategy);
    
    // Initialize the workflow
    void init() override;
    
    // Execute the workflow with input
    JsonObject execute(const JsonObject& input) override;
    
private:
    // List of tasks to execute in parallel
    std::vector<Task> tasks_;
    
    // Strategy to use
    Strategy strategy_;
    
    // Aggregation function for results
    std::function<JsonObject(const std::vector<JsonObject>&)> aggregator_;
    
    // Actor for the workflow coordinator
    caf::actor coordinator_actor_;
    
    // Setup actor roles for this workflow
    void setupActorSystem() override;
    
    // Default aggregator for sectioning strategy
    static JsonObject defaultSectioningAggregator(const std::vector<JsonObject>& results);
    
    // Default aggregator for voting strategy
    static JsonObject defaultVotingAggregator(const std::vector<JsonObject>& results);
};

} // namespace workflows
} // namespace agents 