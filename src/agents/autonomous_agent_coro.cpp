#include <agents-cpp/agents/autonomous_agent.h>
#include <agents-cpp/logger.h>
#include <stdexcept>

namespace agents {

// Implement the coroutine-based version of run
Task<JsonObject> AutonomousAgent::runCoro(const String& task) {
    Logger::info("Running autonomous agent with task: {}", task);
    
    setState(State::RUNNING);
    logStatus("Starting task execution");
    
    try {
        // Clear previous steps
        steps_.clear();
        
        // Execute the task using coroutines
        auto result = co_await executeTaskCoro(task);
        
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

// Coroutine-based implementation for waitForFeedback
Task<String> AutonomousAgent::waitForFeedbackCoro(const String& message, const JsonObject& context) {
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

// Coroutine-based implementation of executeTask
Task<JsonObject> AutonomousAgent::executeTaskCoro(const String& task) {
    Logger::debug("Executing task with coroutines: {}", task);
    
    // Add the task to the context
    context_->addMessage(Message{Message::Role::USER, task});
    
    // Plan the task based on the strategy
    JsonObject plan;
    switch (planning_strategy_) {
        case PlanningStrategy::ZERO_SHOT:
            plan = co_await planZeroShotCoro(task);
            break;
        case PlanningStrategy::TREE_OF_THOUGHT:
            plan = co_await planTreeOfThoughtCoro(task);
            break;
        case PlanningStrategy::PLAN_AND_EXECUTE:
            plan = co_await planAndExecuteCoro(task);
            break;
        case PlanningStrategy::REFLEXION:
            plan = co_await planReflexionCoro(task);
            break;
        case PlanningStrategy::REACT:
        default:
            plan = co_await planReactCoro(task);
            break;
    }
    
    // Create the final answer
    JsonObject answer;
    answer["answer"] = plan["answer"].get<String>();
    answer["steps"] = plan["steps"];
    
    co_return answer;
}

// Coroutine-based implementation of a step execution
Task<AutonomousAgent::Step> AutonomousAgent::executeStepCoro(const String& step_description, const JsonObject& context) {
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
            String feedback = co_await waitForFeedbackCoro(message, context);
            
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
        auto response = co_await context_->chatWithToolsAsync(step_prompt);
        
        // Record the result
        step.result = {
            {"output", response.content}
        };
        
        // Check for tool calls
        if (!response.tool_calls.empty()) {
            JsonObject tool_results;
            
            for (const auto& tool_call : response.tool_calls) {
                // Execute the tool
                auto tool_result = co_await context_->executeToolAsync(tool_call.name, tool_call.parameters);
                
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

// Implement specific planning strategies with coroutines
Task<JsonObject> AutonomousAgent::planReactCoro(const String& task) {
    Logger::debug("Planning with ReAct strategy (coroutine)");
    
    // Initialize context and steps
    JsonObject context = {{"task", task}};
    int iteration = 0;
    JsonArray steps_array;
    
    while (iteration < options_.max_iterations) {
        // Check if should stop
        if (should_stop_) {
            break;
        }
        
        // Generate the next step
        String thinking_prompt = "Task: " + task + "\n\n";
        thinking_prompt += "Think about what to do next. Current status:\n";
        thinking_prompt += JsonObject({{"context", context}}).dump(2);
        
        auto thinking_response = co_await context_->chatAsync(thinking_prompt);
        
        // Extract the next step
        String next_step = thinking_response.content;
        
        // Execute the step
        auto step_result = co_await executeStepCoro(next_step, context);
        
        // Add to steps array
        JsonObject step_json;
        step_json["description"] = step_result.description;
        step_json["result"] = step_result.result;
        step_json["success"] = step_result.success;
        steps_array.push_back(step_json);
        
        // Update context with step result
        context["last_step"] = step_json;
        
        // Check if we have an answer
        if (step_result.result.contains("answer")) {
            context["answer"] = step_result.result["answer"];
            break;
        }
        
        // Check if step was unsuccessful
        if (!step_result.success) {
            // Generate a recovery step
            String recovery_prompt = "The previous step failed. Let's try to recover.\n\n";
            recovery_prompt += "Task: " + task + "\n\n";
            recovery_prompt += "Failed step: " + step_result.description + "\n\n";
            recovery_prompt += "Error: " + step_result.result["error"].get<String>() + "\n\n";
            recovery_prompt += "What should we do next to recover and continue the task?";
            
            auto recovery_response = co_await context_->chatAsync(recovery_prompt);
            String recovery_step = recovery_response.content;
            
            // Execute recovery
            auto recovery_result = co_await executeStepCoro(recovery_step, context);
            
            // Add to steps array
            JsonObject recovery_json;
            recovery_json["description"] = recovery_result.description;
            recovery_json["result"] = recovery_result.result;
            recovery_json["success"] = recovery_result.success;
            steps_array.push_back(recovery_json);
            
            // Update context
            context["last_step"] = recovery_json;
        }
        
        iteration++;
    }
    
    // Generate final answer if not already set
    if (!context.contains("answer")) {
        String answer_prompt = "Task: " + task + "\n\n";
        answer_prompt += "Based on all the steps taken so far, provide a final answer or solution to the task.";
        
        auto answer_response = co_await context_->chatAsync(answer_prompt);
        context["answer"] = answer_response.content;
    }
    
    // Build result
    JsonObject result;
    result["answer"] = context["answer"];
    result["steps"] = steps_array;
    
    co_return result;
}

// Placeholder implementations for other planning strategies
Task<JsonObject> AutonomousAgent::planZeroShotCoro(const String& task) {
    Logger::debug("Planning with Zero Shot strategy (coroutine)");
    
    // Simple implementation: just ask the LLM to solve the task directly
    auto response = co_await context_->chatAsync(task);
    
    JsonObject result;
    result["answer"] = response.content;
    result["steps"] = JsonArray{};
    
    co_return result;
}

Task<JsonObject> AutonomousAgent::planTreeOfThoughtCoro(const String& task) {
    // Placeholder
    co_return planReactCoro(task);
}

Task<JsonObject> AutonomousAgent::planAndExecuteCoro(const String& task) {
    // Placeholder
    co_return planReactCoro(task);
}

Task<JsonObject> AutonomousAgent::planReflexionCoro(const String& task) {
    // Placeholder
    co_return planReactCoro(task);
}

} // namespace agents 