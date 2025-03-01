#include <agents-cpp/llm_interface.h>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace agents {

// OpenAI implementation of the LLM interface
class OpenAILLM : public LLMInterface {
public:
    OpenAILLM(const String& api_key, const String& model = "gpt-4o") 
        : api_key_(api_key), model_(model), api_base_("https://api.openai.com/v1/chat/completions") {
    }

    ~OpenAILLM() override = default;

    std::vector<String> getAvailableModels() override {
        return {
            "gpt-4o",
            "gpt-4-turbo",
            "gpt-4",
            "gpt-3.5-turbo"
        };
    }
    
    void setModel(const String& model) override {
        model_ = model;
    }
    
    String getModel() const override {
        return model_;
    }
    
    void setApiKey(const String& api_key) override {
        api_key_ = api_key;
    }
    
    void setApiBase(const String& api_base) override {
        api_base_ = api_base;
    }
    
    void setOptions(const LLMOptions& options) override {
        options_ = options;
    }
    
    LLMOptions getOptions() const override {
        return options_;
    }
    
    LLMResponse complete(const String& prompt) override {
        // Convert prompt to a message and use chat
        Message msg;
        msg.role = Message::Role::USER;
        msg.content = prompt;
        return chat({msg});
    }
    
    LLMResponse chat(const std::vector<Message>& messages) override {
        try {
            // Format messages for OpenAI API
            nlohmann::json request_body = {
                {"model", model_},
                {"temperature", options_.temperature},
                {"max_tokens", options_.max_tokens},
                {"top_p", options_.top_p},
                {"frequency_penalty", options_.frequency_penalty},
                {"presence_penalty", options_.presence_penalty}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["stop"] = options_.stop_sequences;
            }
            
            // Convert messages to OpenAI format
            nlohmann::json openai_messages = nlohmann::json::array();
            
            for (const auto& message : messages) {
                String role;
                switch (message.role) {
                    case Message::Role::SYSTEM:
                        role = "system";
                        break;
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "assistant";
                        break;
                    case Message::Role::TOOL:
                        role = "tool";
                        break;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                // Add tool name if present for function responses
                if (message.role == Message::Role::TOOL && message.name.has_value()) {
                    msg["name"] = message.name.value();
                }
                
                openai_messages.push_back(msg);
            }
            
            request_body["messages"] = openai_messages;
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_},
                cpr::Header{
                    {"Content-Type", "application/json"},
                    {"Authorization", "Bearer " + api_key_}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("OpenAI API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("OpenAI API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            
            LLMResponse result;
            result.content = response_json["choices"][0]["message"]["content"];
            
            // Add usage metrics if available
            if (response_json.contains("usage")) {
                result.usage_metrics["prompt_tokens"] = response_json["usage"]["prompt_tokens"];
                result.usage_metrics["completion_tokens"] = response_json["usage"]["completion_tokens"];
                result.usage_metrics["total_tokens"] = response_json["usage"]["total_tokens"];
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in OpenAI LLM: {}", e.what());
            LLMResponse error_response;
            error_response.content = "Error: " + String(e.what());
            return error_response;
        }
    }
    
    LLMResponse chatWithTools(
        const std::vector<Message>& messages,
        const std::vector<std::shared_ptr<Tool>>& tools
    ) override {
        try {
            // Format messages for OpenAI API
            nlohmann::json request_body = {
                {"model", model_},
                {"temperature", options_.temperature},
                {"max_tokens", options_.max_tokens},
                {"top_p", options_.top_p},
                {"frequency_penalty", options_.frequency_penalty},
                {"presence_penalty", options_.presence_penalty}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["stop"] = options_.stop_sequences;
            }
            
            // Convert messages to OpenAI format
            nlohmann::json openai_messages = nlohmann::json::array();
            
            for (const auto& message : messages) {
                String role;
                switch (message.role) {
                    case Message::Role::SYSTEM:
                        role = "system";
                        break;
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "assistant";
                        break;
                    case Message::Role::TOOL:
                        role = "tool";
                        break;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                // Add tool name if present for function responses
                if (message.role == Message::Role::TOOL && message.name.has_value()) {
                    msg["name"] = message.name.value();
                }
                
                // Add tool calls if this is an assistant message with tool calls
                if (message.role == Message::Role::ASSISTANT && !message.tool_calls.empty()) {
                    nlohmann::json tool_calls = nlohmann::json::array();
                    for (const auto& tool_call : message.tool_calls) {
                        nlohmann::json call = {
                            {"type", "function"},
                            {"function", {
                                {"name", tool_call.first},
                                {"arguments", tool_call.second.dump()}
                            }}
                        };
                        tool_calls.push_back(call);
                    }
                    msg["tool_calls"] = tool_calls;
                }
                
                openai_messages.push_back(msg);
            }
            
            request_body["messages"] = openai_messages;
            
            // Add tools to request
            nlohmann::json tools_json = nlohmann::json::array();
            
            for (const auto& tool : tools) {
                nlohmann::json tool_json = {
                    {"type", "function"},
                    {"function", tool->getSchema()}
                };
                tools_json.push_back(tool_json);
            }
            
            if (!tools.empty()) {
                request_body["tools"] = tools_json;
                request_body["tool_choice"] = "auto";
            }
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_},
                cpr::Header{
                    {"Content-Type", "application/json"},
                    {"Authorization", "Bearer " + api_key_}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("OpenAI API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("OpenAI API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            nlohmann::json message = response_json["choices"][0]["message"];
            
            LLMResponse result;
            result.content = message.contains("content") ? message["content"] : "";
            
            // Extract tool calls if present
            if (message.contains("tool_calls")) {
                for (const auto& tool_call : message["tool_calls"]) {
                    if (tool_call["type"] == "function") {
                        String name = tool_call["function"]["name"];
                        // Fix: Convert to string first, then parse
                        String args_str = tool_call["function"]["arguments"].get<String>();
                        JsonObject args = nlohmann::json::parse(args_str);
                        result.tool_calls.emplace_back(name, args);
                    }
                }
            }
            
            // Add usage metrics if available
            if (response_json.contains("usage")) {
                result.usage_metrics["prompt_tokens"] = response_json["usage"]["prompt_tokens"];
                result.usage_metrics["completion_tokens"] = response_json["usage"]["completion_tokens"];
                result.usage_metrics["total_tokens"] = response_json["usage"]["total_tokens"];
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in OpenAI LLM: {}", e.what());
            LLMResponse error_response;
            error_response.content = "Error: " + String(e.what());
            return error_response;
        }
    }
    
    void streamChat(
        const std::vector<Message>& messages,
        std::function<void(const String&, bool)> callback
    ) override {
        try {
            // Format messages for OpenAI API
            nlohmann::json request_body = {
                {"model", model_},
                {"temperature", options_.temperature},
                {"max_tokens", options_.max_tokens},
                {"top_p", options_.top_p},
                {"frequency_penalty", options_.frequency_penalty},
                {"presence_penalty", options_.presence_penalty},
                {"stream", true}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["stop"] = options_.stop_sequences;
            }
            
            // Convert messages to OpenAI format
            nlohmann::json openai_messages = nlohmann::json::array();
            
            for (const auto& message : messages) {
                String role;
                switch (message.role) {
                    case Message::Role::SYSTEM:
                        role = "system";
                        break;
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "assistant";
                        break;
                    case Message::Role::TOOL:
                        role = "tool";
                        break;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                // Add tool name if present for function responses
                if (message.role == Message::Role::TOOL && message.name.has_value()) {
                    msg["name"] = message.name.value();
                }
                
                openai_messages.push_back(msg);
            }
            
            request_body["messages"] = openai_messages;
            
            // Make streaming API request
            // Note: This is a simplified implementation that does not handle
            // actual streaming. A real implementation would use a streaming HTTP client.
            // For now, we'll just simulate streaming by breaking up the response.
            
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_},
                cpr::Header{
                    {"Content-Type", "application/json"},
                    {"Authorization", "Bearer " + api_key_}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("OpenAI API error: {} {}", response.status_code, response.text);
                callback("Error: " + response.text, true);
                return;
            }
            
            // Parse response and simulate streaming
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            String full_content = response_json["choices"][0]["message"]["content"];
            
            // Simulate streaming by breaking up the response
            const int chunk_size = 10;
            for (size_t i = 0; i < full_content.length(); i += chunk_size) {
                size_t remaining = full_content.length() - i;
                size_t size = remaining < chunk_size ? remaining : chunk_size;
                String chunk = full_content.substr(i, size);
                
                bool is_last = (i + size >= full_content.length());
                callback(chunk, is_last);
                
                // Add a small delay to simulate streaming
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const std::exception& e) {
            spdlog::error("Error in OpenAI LLM streaming: {}", e.what());
            callback("Error: " + String(e.what()), true);
        }
    }

private:
    String api_key_;
    String model_;
    String api_base_;
    LLMOptions options_;
};

// Export the LLM creation function
std::shared_ptr<LLMInterface> createOpenAILLM(const String& api_key, const String& model) {
    return std::make_shared<OpenAILLM>(api_key, model);
}

} // namespace agents 