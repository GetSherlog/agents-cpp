#include <agents-cpp/llm_interface.h>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace agents {

// Anthropic implementation of the LLM interface
class AnthropicLLM : public LLMInterface {
public:
    AnthropicLLM(const String& api_key, const String& model = "claude-3-opus-20240229") 
        : api_key_(api_key), model_(model), api_base_("https://api.anthropic.com/v1/messages") {
    }

    ~AnthropicLLM() override = default;

    std::vector<String> getAvailableModels() override {
        return {
            "claude-3-opus-20240229",
            "claude-3-sonnet-20240229",
            "claude-3-haiku-20240307",
            "claude-2.1",
            "claude-2.0"
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
            // Format messages for Anthropic API
            nlohmann::json request_body = {
                {"model", model_},
                {"temperature", options_.temperature},
                {"max_tokens", options_.max_tokens},
                {"top_p", options_.top_p}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["stop_sequences"] = options_.stop_sequences;
            }
            
            // Convert messages to Anthropic format
            nlohmann::json anthropic_messages = nlohmann::json::array();
            
            // Handle system message separately (Anthropic has a system field)
            String system_prompt;
            
            for (const auto& message : messages) {
                if (message.role == Message::Role::SYSTEM) {
                    system_prompt = message.content;
                    continue;
                }
                
                String role;
                switch (message.role) {
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "assistant";
                        break;
                    case Message::Role::TOOL:
                        // Skip tool messages as they're not directly supported
                        continue;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                anthropic_messages.push_back(msg);
            }
            
            request_body["messages"] = anthropic_messages;
            
            if (!system_prompt.empty()) {
                request_body["system"] = system_prompt;
            }
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_},
                cpr::Header{
                    {"Content-Type", "application/json"},
                    {"anthropic-version", "2023-06-01"},
                    {"x-api-key", api_key_}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Anthropic API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("Anthropic API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            
            LLMResponse result;
            result.content = response_json["content"][0]["text"];
            
            // Add usage metrics if available
            if (response_json.contains("usage")) {
                result.usage_metrics["input_tokens"] = response_json["usage"]["input_tokens"];
                result.usage_metrics["output_tokens"] = response_json["usage"]["output_tokens"];
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in Anthropic LLM: {}", e.what());
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
            // Format messages for Anthropic API
            nlohmann::json request_body = {
                {"model", model_},
                {"temperature", options_.temperature},
                {"max_tokens", options_.max_tokens},
                {"top_p", options_.top_p}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["stop_sequences"] = options_.stop_sequences;
            }
            
            // Convert messages to Anthropic format
            nlohmann::json anthropic_messages = nlohmann::json::array();
            
            // Handle system message separately
            String system_prompt;
            
            for (const auto& message : messages) {
                if (message.role == Message::Role::SYSTEM) {
                    system_prompt = message.content;
                    continue;
                }
                
                String role;
                switch (message.role) {
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "assistant";
                        break;
                    case Message::Role::TOOL:
                        // Skip tool messages as they're handled separately
                        continue;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                anthropic_messages.push_back(msg);
            }
            
            request_body["messages"] = anthropic_messages;
            
            if (!system_prompt.empty()) {
                request_body["system"] = system_prompt;
            }
            
            // Add tools to request
            nlohmann::json tools_json = nlohmann::json::array();
            
            for (const auto& tool : tools) {
                tools_json.push_back(tool->getSchema());
            }
            
            request_body["tools"] = tools_json;
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_},
                cpr::Header{
                    {"Content-Type", "application/json"},
                    {"anthropic-version", "2023-06-01"},
                    {"x-api-key", api_key_}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Anthropic API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("Anthropic API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            
            LLMResponse result;
            result.content = response_json["content"][0]["text"];
            
            // Extract tool calls if present
            if (response_json.contains("tool_use")) {
                for (const auto& tool_use : response_json["tool_use"]) {
                    String tool_name = tool_use["name"];
                    JsonObject tool_params = tool_use["parameters"];
                    
                    result.tool_calls.emplace_back(tool_name, tool_params);
                }
            }
            
            // Add usage metrics if available
            if (response_json.contains("usage")) {
                result.usage_metrics["input_tokens"] = response_json["usage"]["input_tokens"];
                result.usage_metrics["output_tokens"] = response_json["usage"]["output_tokens"];
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in Anthropic LLM: {}", e.what());
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
            // Format messages for Anthropic API
            nlohmann::json request_body = {
                {"model", model_},
                {"temperature", options_.temperature},
                {"max_tokens", options_.max_tokens},
                {"top_p", options_.top_p},
                {"stream", true}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["stop_sequences"] = options_.stop_sequences;
            }
            
            // Convert messages to Anthropic format
            nlohmann::json anthropic_messages = nlohmann::json::array();
            
            // Handle system message separately
            String system_prompt;
            
            for (const auto& message : messages) {
                if (message.role == Message::Role::SYSTEM) {
                    system_prompt = message.content;
                    continue;
                }
                
                String role;
                switch (message.role) {
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "assistant";
                        break;
                    case Message::Role::TOOL:
                        // Skip tool messages as they're not directly supported
                        continue;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                anthropic_messages.push_back(msg);
            }
            
            request_body["messages"] = anthropic_messages;
            
            if (!system_prompt.empty()) {
                request_body["system"] = system_prompt;
            }
            
            // Make streaming API request
            // Note: This is a simplified implementation that does not handle
            // actual streaming. A real implementation would use a streaming HTTP client.
            // For now, we'll just simulate streaming by breaking up the response.
            
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_},
                cpr::Header{
                    {"Content-Type", "application/json"},
                    {"anthropic-version", "2023-06-01"},
                    {"x-api-key", api_key_}
                },
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Anthropic API error: {} {}", response.status_code, response.text);
                callback("Error: " + response.text, true);
                return;
            }
            
            // Parse response and simulate streaming
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            String full_content = response_json["content"][0]["text"];
            
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
            spdlog::error("Error in Anthropic LLM streaming: {}", e.what());
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
std::shared_ptr<LLMInterface> createAnthropicLLM(const String& api_key, const String& model) {
    return std::make_shared<AnthropicLLM>(api_key, model);
}

} // namespace agents 