#include <agents-cpp/workflow.h>
#include <thread>
#include <spdlog/spdlog.h>

namespace agents {

Workflow::Workflow(std::shared_ptr<AgentContext> context)
    : context_(context) {
}

void Workflow::runAsync(
    const String& input,
    std::function<void(const JsonObject&)> callback
) {
    // Simple implementation that runs in a new thread
    std::thread thread([this, input, callback]() {
        try {
            auto result = this->run(input);
            callback(result);
        } catch (const std::exception& e) {
            spdlog::error("Error in workflow: {}", e.what());
            JsonObject error_result;
            error_result["error"] = e.what();
            callback(error_result);
        }
    });
    
    // Detach the thread to let it run independently
    thread.detach();
}

std::shared_ptr<AgentContext> Workflow::getContext() const {
    return context_;
}

void Workflow::setStepCallback(std::function<void(const String&, const JsonObject&)> callback) {
    step_callback_ = callback;
}

void Workflow::setMaxSteps(int max_steps) {
    max_steps_ = max_steps;
}

int Workflow::getMaxSteps() const {
    return max_steps_;
}

void Workflow::logStep(const String& description, const JsonObject& result) {
    // Log to console
    spdlog::info("Workflow step: {}", description);
    
    // Call the callback if set
    if (step_callback_) {
        step_callback_(description, result);
    }
}

} // namespace agents 