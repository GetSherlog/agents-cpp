#include <agents-cpp/agents/autonomous_agent.h>
#include <agents-cpp/logger.h>
#include <stdexcept>

namespace agents {

AutonomousAgent::AutonomousAgent(std::shared_ptr<AgentContext> context)
    : Agent(context) {
    // Initialize with default values
}

void AutonomousAgent::init() {
    setState(State::READY);
    steps_.clear();
    logStatus("Agent initialized");
}

Task<JsonObject> AutonomousAgent::run(const String& task) {
    Logger::info("Running autonomous agent with task: {}", task);
    
    setState(State::RUNNING);
    logStatus("Starting task execution");
    
    try {
        // Clear previous steps
        steps_.clear();
    
        auto result = co_await executeTask(task);
        
        // Mark as completed
        setState(State::COMPLETED);
        logStatus("Task completed successfully");
        
        co_return result;
    } catch (const std::exception& e) {
        setState(State::FAILED);
        logStatus("Task failed: " + String(e.what()));
        
        // Return error
        JsonObject error;
        error["error"] = e.what();
        co_return error;
    }
}

void AutonomousAgent::stop() {
    should_stop_ = true;
    
    if (getState() == State::RUNNING) {
        setState(State::STOPPED);
        logStatus("Task stopped by user");
    }
}

void AutonomousAgent::provideFeedback(const String& feedback) {
    // Resolve the promise if it exists
    feedback_promise_.setValue(feedback);
}

void AutonomousAgent::setSystemPrompt(const String& system_prompt) {
    system_prompt_ = system_prompt;
}

void AutonomousAgent::setPlanningStrategy(PlanningStrategy strategy) {
    planning_strategy_ = strategy;
}

std::vector<AutonomousAgent::Step> AutonomousAgent::getSteps() const {
    return steps_;
}

void AutonomousAgent::setStepCallback(std::function<void(const Step&)> callback) {
    step_callback_ = callback;
}

Task<String> AutonomousAgent::waitForFeedback(const String& message, const JsonObject& context) {
    Logger::debug("Waiting for feedback...");
    
    // Set state to waiting
    setState(State::WAITING);
    logStatus("Waiting for human feedback");
    
    if (!options_.human_feedback_enabled || !options_.human_in_the_loop) {
        // Auto-approve if human feedback is not enabled
        setState(State::RUNNING);
        co_return "";
    }
    
    // Create a promise to wait for feedback
    folly::Promise<String> promise;
    auto future = promise.getFuture();
    
    // Store the promise for later resolution
    feedback_promise_ = std::move(promise);
    
    // Call the human-in-the-loop function
    bool approved = options_.human_in_the_loop(message, context);
    
    if (approved) {
        // If approved, we can move on
        setState(State::RUNNING);
        co_return "";
    } else {
        // If not approved, we need to wait for feedback
        // This would be resolved by calling provideFeedback
        auto feedback = co_await std::move(future);
        
        setState(State::RUNNING);
        co_return feedback;
    }
}

// Implementation of task execution using coroutines
Task<JsonObject> AutonomousAgent::executeTask(const String& task) {
    Logger::debug("Executing task with coroutines: {}", task);
    
    // Add the task to the context
    context_->addMessage(Message{Message::Role::USER, task});
    
    // Plan the task based on the strategy
    JsonObject plan;
    switch (planning_strategy_) {
        case PlanningStrategy::ZERO_SHOT:
            plan = co_await planZeroShot(task);
            break;
        case PlanningStrategy::TREE_OF_THOUGHT:
            plan = co_await planTreeOfThought(task);
            break;
        case PlanningStrategy::PLAN_AND_EXECUTE:
            plan = co_await planAndExecute(task);
            break;
        case PlanningStrategy::REFLEXION:
            plan = co_await planReflexion(task);
            break;
        case PlanningStrategy::REACT:
        default:
            plan = co_await planReact(task);
            break;
    }
    
    // Create the final answer
    JsonObject answer;
    answer["answer"] = plan["answer"].get<String>();
    answer["steps"] = plan["steps"];
    
    co_return answer;
}

// Implementation of a step execution using coroutines
Task<AutonomousAgent::Step> AutonomousAgent::executeStep(const String& step_description, const JsonObject& context) {
    Logger::debug("Executing step: {}", step_description);
    
    // Create a step record
    Step step;
    step.description = step_description;
    step.status = "Running";
    
    try {
        // Check if human approval is required
        if (options_.human_feedback_enabled && options_.human_in_the_loop) {
            // Ask for approval
            String message = "Step: " + step_description;
            String feedback = co_await waitForFeedback(message, context);
            
            if (!feedback.empty()) {
                // Incorporate feedback
                Logger::info("Incorporating feedback: {}", feedback);
                // TODO: Modify the step based on feedback
            }
        }
        
        // Execute the step using the LLM
        auto tools = context_->getTools();
        String step_prompt = "Execute the following step: " + step_description;
        
        // Add any context information
        if (!context.empty()) {
            step_prompt += "\n\nContext: " + context.dump(2);
        }
        
        // Create appropriate message
        Message msg;
        msg.role = Message::Role::USER;
        msg.content = step_prompt;
        
        // Get response with tools
        auto response = co_await context_->chatWithTools(step_prompt);
        
        // Record the result
        step.result = {
            {"output", response.content}
        };
        
        // Check for tool calls
        if (!response.tool_calls.empty()) {
            JsonObject tool_results;
            
            for (const auto& tool_call : response.tool_calls) {
                // Execute the tool
                auto tool_result = co_await context_->executeTool(tool_call.name, tool_call.parameters);
                
                // Add to results
                tool_results[tool_call.name] = {
                    {"success", tool_result.success},
                    {"result", tool_result.content},
                    {"data", tool_result.data}
                };
            }
            
            step.result["tool_results"] = tool_results;
        }
        
        // Mark as success
        step.success = true;
        step.status = "Completed";
    } catch (const std::exception& e) {
        // Mark as failure
        step.success = false;
        step.status = "Failed: " + String(e.what());
        step.result = {{"error", e.what()}};
    }
    
    // Record the step
    recordStep(step);
    
    co_return step;
}

void AutonomousAgent::recordStep(const Step& step) {
    steps_.push_back(step);
    
    if (step_callback_) {
        step_callback_(step);
    }
}

String AutonomousAgent::getToolDescriptions() const {
    String descriptions;
    
    for (const auto& tool : context_->getTools()) {
        descriptions += "Tool: " + tool->getName() + "\n";
        descriptions += "Description: " + tool->getDescription() + "\n";
        descriptions += "Parameters: " + tool->getSchema().dump() + "\n\n";
    }
    
    return descriptions;
}

// Planning strategy implementations using coroutines
Task<JsonObject> AutonomousAgent::planZeroShot(const String& task) {
    Logger::debug("Planning with Zero Shot strategy (coroutine)");
    
    // Simple implementation: just ask the LLM to solve the task directly
    auto response = co_await context_->chat(task);
    
    JsonObject result;
    result["answer"] = response.content;
    result["steps"] = JsonArray{};
    
    co_return result;
}

Task<JsonObject> AutonomousAgent::planTreeOfThought(const String& task) {
    // Placeholder implementation
    co_return planReact(task);
}

Task<JsonObject> AutonomousAgent::planAndExecute(const String& task) {
    // Placeholder implementation
    co_return planReact(task);
}

Task<JsonObject> AutonomousAgent::planReflexion(const String& task) {
    // Placeholder implementation
    co_return planReact(task);
}

Task<JsonObject> AutonomousAgent::planReact(const String& task) {
    Logger::debug("Planning with REACT strategy (coroutine)");
    
    // Initial prompt to get the agent thinking about the task
    String prompt = "Task: " + task + "\n\n";
    prompt += "Think about how to approach this task step by step. ";
    prompt += "You can use the following tools to help you:\n\n";
    prompt += getToolDescriptions();
    
    // Get initial response
    auto response = co_await context_->chat(prompt);
    
    // Execute steps in sequence
    std::vector<Step> recorded_steps;
    JsonArray step_array;
    
    // Extract steps from the response (in a real implementation, 
    // this would parse structured output from the LLM)
    String steps_text = response.content;
    std::vector<String> step_descriptions;
    
    // For this example, we'll just create a single step from the response
    step_descriptions.push_back("Execute the task based on initial analysis");
    
    // Execute each step with human feedback as needed
    for (const auto& step_desc : step_descriptions) {
        if (should_stop_) {
            break;
        }
        
        auto step = co_await executeStep(step_desc, {});
        recorded_steps.push_back(step);
        
        JsonObject step_json;
        step_json["description"] = step.description;
        step_json["success"] = step.success;
        step_json["result"] = step.result;
        step_array.push_back(step_json);
    }
    
    // Create final result with steps
    JsonObject result;
    result["answer"] = response.content;
    result["steps"] = step_array;
    
    co_return result;
}

} // namespace agents 