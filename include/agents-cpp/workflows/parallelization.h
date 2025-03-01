#pragma once

#include <agents-cpp/workflow.h>
#include <vector>
#include <functional>
#include <thread>
#include <future>

namespace agents {
namespace workflows {

/**
 * @brief A workflow that runs multiple tasks in parallel
 * 
 * Parallelization can manifest in two key variations:
 * 1. Sectioning: Breaking a task into independent subtasks run in parallel.
 * 2. Voting: Running the same task multiple times to get diverse outputs.
 */
class Parallelization : public Workflow {
public:
    /**
     * @brief Task to run in parallel
     */
    struct Task {
        String name;
        String system_prompt;
        std::function<String(const String&)> prompt_fn;
        std::function<JsonObject(const String&)> result_parser;
    };
    
    enum class Mode {
        SECTIONING,  // Break task into subtasks
        VOTING       // Run multiple identical tasks
    };
    
    Parallelization(std::shared_ptr<AgentContext> context, Mode mode = Mode::SECTIONING);
    ~Parallelization() override = default;
    
    // Add a task to run in parallel
    void addTask(const Task& task);
    
    // Add a simple task with just a system prompt
    void addTask(const String& name, const String& system_prompt);
    
    // Add a task with a custom prompt function
    void addTask(
        const String& name,
        const String& system_prompt,
        std::function<String(const String&)> prompt_fn
    );
    
    // Add a task with a custom prompt function and result parser
    void addTask(
        const String& name,
        const String& system_prompt,
        std::function<String(const String&)> prompt_fn,
        std::function<JsonObject(const String&)> result_parser
    );
    
    // Set the aggregation function for combining results
    void setAggregator(std::function<JsonObject(const std::vector<JsonObject>&)> aggregator);
    
    // Set the voting threshold (for VOTING mode)
    void setVotingThreshold(double threshold);
    
    // Run the parallelization workflow
    JsonObject run(const String& input) override;

private:
    Mode mode_;
    std::vector<Task> tasks_;
    std::function<JsonObject(const std::vector<JsonObject>&)> aggregator_;
    double voting_threshold_ = 0.5;
    
    // Run tasks in parallel (implementation uses std::async)
    std::vector<JsonObject> runTasksInParallel(const String& input);
    
    // Default aggregators
    JsonObject defaultSectionAggregator(const std::vector<JsonObject>& results);
    JsonObject defaultVotingAggregator(const std::vector<JsonObject>& results);
};

} // namespace workflows
} // namespace agents 