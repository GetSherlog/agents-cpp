#include <agents-cpp/workflows/prompt_chain.h>
#include <spdlog/spdlog.h>

namespace agents {
namespace workflows {

PromptChain::PromptChain(std::shared_ptr<AgentContext> context)
    : Workflow(context) {
}

void PromptChain::addStep(const Step& step) {
    steps_.push_back(step);
}

void PromptChain::addStep(const String& name, const String& system_prompt) {
    Step step;
    step.name = name;
    step.system_prompt = system_prompt;
    
    // Default prompt function just passes the input through
    step.prompt_fn = [](const String& input, const JsonObject&) {
        return input;
    };
    
    // Default gate function always allows the step to run
    step.gate_fn = [](const String&, const JsonObject&) {
        return true;
    };
    
    addStep(step);
}

void PromptChain::addStep(
    const String& name,
    const String& system_prompt,
    std::function<String(const String&, const JsonObject&)> prompt_fn
) {
    Step step;
    step.name = name;
    step.system_prompt = system_prompt;
    step.prompt_fn = prompt_fn;
    
    // Default gate function always allows the step to run
    step.gate_fn = [](const String&, const JsonObject&) {
        return true;
    };
    
    addStep(step);
}

void PromptChain::addStep(
    const String& name,
    const String& system_prompt,
    std::function<String(const String&, const JsonObject&)> prompt_fn,
    std::function<bool(const String&, const JsonObject&)> gate_fn
) {
    Step step;
    step.name = name;
    step.system_prompt = system_prompt;
    step.prompt_fn = prompt_fn;
    step.gate_fn = gate_fn;
    
    addStep(step);
}

JsonObject PromptChain::run(const String& input) {
    // Initialize step outputs
    step_outputs_ = JsonObject();
    
    // Store the input
    step_outputs_["input"] = input;
    
    // Initialize result
    String current_result = input;
    
    // Run each step in the chain
    for (size_t i = 0; i < steps_.size(); ++i) {
        const auto& step = steps_[i];
        
        // Check if the step should be skipped
        if (!step.gate_fn(current_result, step_outputs_)) {
            spdlog::info("Skipping step {} based on gate function", step.name);
            continue;
        }
        
        // Format the prompt using the custom function
        String formatted_prompt = step.prompt_fn(current_result, step_outputs_);
        
        // Set the system prompt for this step
        context_->setSystemPrompt(step.system_prompt);
        
        // Decide whether to use tools or not for this step
        LLMResponse response;
        if (step.use_tools) {
            response = context_->chatWithTools(formatted_prompt);
        } else {
            response = context_->chat(formatted_prompt);
        }
        
        // Store the result
        current_result = response.content;
        
        // Store the step output
        JsonObject step_output;
        step_output["prompt"] = formatted_prompt;
        step_output["response"] = current_result;
        if (!response.tool_calls.empty()) {
            JsonObject tool_calls;
            for (const auto& tool_call : response.tool_calls) {
                tool_calls[tool_call.first] = tool_call.second;
            }
            step_output["tool_calls"] = tool_calls;
        }
        
        step_outputs_[step.name] = step_output;
        
        // Log the step
        logStep(step.name, step_output);
    }
    
    // Create final result
    JsonObject result;
    result["steps"] = step_outputs_;
    result["final_output"] = current_result;
    
    return result;
}

} // namespace workflows
} // namespace agents 