#pragma once

#include <agents-cpp/workflow.h>
#include <vector>
#include <functional>

namespace agents {
namespace workflows {

/**
 * @brief A workflow that chains multiple prompts together
 * 
 * Prompt chaining decomposes a task into a sequence of steps, where 
 * each LLM call processes the output of the previous one.
 */
class PromptChain : public Workflow {
public:
    /**
     * @brief Step in a prompt chain
     */
    struct Step {
        String name;
        String system_prompt;
        std::function<String(const String&, const JsonObject&)> prompt_fn;
        std::function<bool(const String&, const JsonObject&)> gate_fn;
        bool use_tools = false;
    };

    PromptChain(std::shared_ptr<AgentContext> context);
    ~PromptChain() override = default;
    
    // Add a step to the chain
    void addStep(const Step& step);
    
    // Add a simple step with just a system prompt
    void addStep(const String& name, const String& system_prompt);
    
    // Add a step with a custom prompt function
    void addStep(
        const String& name,
        const String& system_prompt,
        std::function<String(const String&, const JsonObject&)> prompt_fn
    );
    
    // Add a step with a custom prompt function and gate function
    void addStep(
        const String& name,
        const String& system_prompt,
        std::function<String(const String&, const JsonObject&)> prompt_fn,
        std::function<bool(const String&, const JsonObject&)> gate_fn
    );
    
    // Run the chain with the given input
    JsonObject run(const String& input) override;

private:
    std::vector<Step> steps_;
    JsonObject step_outputs_;
};

} // namespace workflows
} // namespace agents 