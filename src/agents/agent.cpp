#include <agents-cpp/agent.h>
#include <agents-cpp/logger.h>
#include <stdexcept>

namespace agents {

Agent::Agent(std::shared_ptr<AgentContext> context)
    : context_(context) {
    if (!context_) {
        throw std::invalid_argument("Agent context cannot be null");
    }
}

void Agent::runAsync(
    const String& task,
    std::function<void(const JsonObject&)> callback
) {
    // Default implementation runs synchronously in a separate thread
    std::thread([this, task, callback]() {
        try {
            auto result = run(task);
            if (callback) {
                callback(result);
            }
        } catch (const std::exception& e) {
            if (callback) {
                JsonObject error;
                error["error"] = e.what();
                callback(error);
            }
        }
    }).detach();
}

void Agent::stop() {
    // Base implementation does nothing, derived classes should override
    setState(State::STOPPED);
}

std::shared_ptr<AgentContext> Agent::getContext() const {
    return context_;
}

Agent::State Agent::getState() const {
    return state_;
}

void Agent::setOptions(const Options& options) {
    options_ = options;
}

const Agent::Options& Agent::getOptions() const {
    return options_;
}

void Agent::setStatusCallback(std::function<void(const String&)> callback) {
    status_callback_ = callback;
}

void Agent::provideFeedback(const String& feedback) {
    // Base implementation does nothing, derived classes should override
}

void Agent::setState(State state) {
    state_ = state;
}

void Agent::logStatus(const String& status) {
    if (status_callback_) {
        status_callback_(status);
    } else {
        Logger::info("Agent status: {}", status);
    }
}

} // namespace agents 