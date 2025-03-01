#include <agents-cpp/llm_interface.h>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace agents {

// Ollama implementation of the LLM interface
class OllamaLLM : public LLMInterface {
public:
    OllamaLLM(const String& api_key, const String& model = "llama3") 
        : api_key_(api_key), model_(model), api_base_("http://localhost:11434/api") {
    }

    ~OllamaLLM() override = default;

    std::vector<String> getAvailableModels() override {
        try {
            cpr::Response response = cpr::Get(
                cpr::Url{api_base_ + "/tags"},
                cpr::Header{{"Content-Type", "application/json"}}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Ollama API error when fetching models: {} {}", response.status_code, response.text);
                return {"llama3", "llama3:8b", "llama3:70b"};
            }
            
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            std::vector<String> models;
            
            if (response_json.contains("models")) {
                for (const auto& model : response_json["models"]) {
                    if (model.contains("name")) {
                        models.push_back(model["name"]);
                    }
                }
            }
            
            return models.empty() ? std::vector<String>{"llama3", "llama3:8b", "llama3:70b"} : models;
        } catch (const std::exception& e) {
            spdlog::error("Error fetching Ollama models: {}", e.what());
            return {"llama3", "llama3:8b", "llama3:70b"};
        }
    }
    
    void setModel(const String& model) override {
        model_ = model;
    }
    
    String getModel() const override {
        return model_;
    }
    
    void setApiKey(const String& api_key) override {
        api_key_ = api_key;  // Not used by Ollama but kept for interface compatibility
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
            // Format messages for Ollama API
            nlohmann::json request_body = {
                {"model", model_},
                {"options", {
                    {"temperature", options_.temperature},
                    {"num_predict", options_.max_tokens},
                    {"top_p", options_.top_p}
                }}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["options"]["stop"] = options_.stop_sequences;
            }
            
            // Convert messages to Ollama format
            nlohmann::json ollama_messages = nlohmann::json::array();
            
            // Process messages
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
                        // Ollama doesn't support tool responses natively, we'll format them as user messages
                        role = "user";
                        break;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                // For tool responses, add a prefix
                if (message.role == Message::Role::TOOL && message.name.has_value()) {
                    msg["content"] = "Tool result from " + message.name.value() + ": " + message.content;
                }
                
                ollama_messages.push_back(msg);
            }
            
            request_body["messages"] = ollama_messages;
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_ + "/chat"},
                cpr::Header{{"Content-Type", "application/json"}},
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Ollama API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("Ollama API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            
            LLMResponse result;
            
            if (response_json.contains("message") && response_json["message"].contains("content")) {
                result.content = response_json["message"]["content"];
            } else {
                result.content = "";
            }
            
            // Add usage metrics if available
            if (response_json.contains("prompt_eval_count")) {
                result.usage_metrics["prompt_tokens"] = response_json["prompt_eval_count"];
            }
            if (response_json.contains("eval_count")) {
                result.usage_metrics["completion_tokens"] = response_json["eval_count"];
            }
            if (response_json.contains("prompt_eval_count") && response_json.contains("eval_count")) {
                result.usage_metrics["total_tokens"] = static_cast<int>(response_json["prompt_eval_count"]) + 
                                                    static_cast<int>(response_json["eval_count"]);
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in Ollama LLM: {}", e.what());
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
            // Ollama doesn't natively support function/tool calling
            // We'll add the tool descriptions to the system prompt
            
            std::vector<Message> augmented_messages = messages;
            
            // Find system message if it exists, or create a new one
            bool has_system_message = false;
            for (auto& message : augmented_messages) {
                if (message.role == Message::Role::SYSTEM) {
                    has_system_message = true;
                    
                    // Append tool descriptions to the system message
                    message.content += "\n\nYou have access to the following tools:\n\n";
                    
                    for (const auto& tool : tools) {
                        message.content += "Tool: " + tool->getName() + "\n";
                        message.content += "Description: " + tool->getDescription() + "\n";
                        
                        // Add parameters info
                        message.content += "Parameters:\n";
                        for (const auto& param_pair : tool->getParameters()) {
                            const Parameter& param = param_pair.second;
                            message.content += "  - " + param.name + " (" + param.type + "): " + 
                                param.description + (param.required ? " (Required)" : "") + "\n";
                        }
                        
                        message.content += "\n";
                    }
                    
                    message.content += "When you need to use a tool, format your response exactly like this:\n";
                    message.content += "```json\n{\"tool\": \"tool_name\", \"parameters\": {\"param1\": \"value1\", \"param2\": \"value2\"}}\n```\n";
                    message.content += "After receiving the tool result, continue the conversation normally.";
                    
                    break;
                }
            }
            
            // If no system message exists, create one
            if (!has_system_message) {
                Message system_msg;
                system_msg.role = Message::Role::SYSTEM;
                system_msg.content = "You are a helpful assistant with access to tools.\n\n";
                system_msg.content += "You have access to the following tools:\n\n";
                
                for (const auto& tool : tools) {
                    system_msg.content += "Tool: " + tool->getName() + "\n";
                    system_msg.content += "Description: " + tool->getDescription() + "\n";
                    
                    // Add parameters info
                    system_msg.content += "Parameters:\n";
                    for (const auto& param_pair : tool->getParameters()) {
                        const Parameter& param = param_pair.second;
                        system_msg.content += "  - " + param.name + " (" + param.type + "): " + 
                            param.description + (param.required ? " (Required)" : "") + "\n";
                    }
                    
                    system_msg.content += "\n";
                }
                
                system_msg.content += "When you need to use a tool, format your response exactly like this:\n";
                system_msg.content += "```json\n{\"tool\": \"tool_name\", \"parameters\": {\"param1\": \"value1\", \"param2\": \"value2\"}}\n```\n";
                system_msg.content += "After receiving the tool result, continue the conversation normally.";
                
                augmented_messages.insert(augmented_messages.begin(), system_msg);
            }
            
            // Get the response
            auto response = chat(augmented_messages);
            
            // Parse the response to look for tool calls
            // Example: {"tool": "tool_name", "parameters": {"param1": "value1"}}
            String content = response.content;
            
            // Look for JSON blocks with tool calls
            size_t json_start = content.find("```json");
            if (json_start != String::npos) {
                size_t json_content_start = content.find("\n", json_start) + 1;
                size_t json_end = content.find("```", json_content_start);
                
                if (json_content_start != String::npos && json_end != String::npos) {
                    String json_str = content.substr(json_content_start, json_end - json_content_start);
                    
                    try {
                        // Fix: Ensure string is valid JSON before parsing
                        nlohmann::json tool_call = nlohmann::json::parse(json_str);
                        
                        if (tool_call.contains("tool") && tool_call.contains("parameters")) {
                            String tool_name = tool_call["tool"];
                            JsonObject params = tool_call["parameters"];
                            
                            response.tool_calls.emplace_back(tool_name, params);
                            
                            // Remove the tool call portion from the content
                            response.content = content.substr(0, json_start);
                            if (json_end + 3 < content.length()) {
                                response.content += content.substr(json_end + 3);
                            }
                        }
                    } catch (const std::exception& e) {
                        spdlog::error("Error parsing tool input JSON: {}", e.what());
                        // Keep original content
                    }
                }
            }
            
            return response;
        } catch (const std::exception& e) {
            spdlog::error("Error in Ollama LLM: {}", e.what());
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
            // Format messages for Ollama API - stream mode
            nlohmann::json request_body = {
                {"model", model_},
                {"stream", true},
                {"options", {
                    {"temperature", options_.temperature},
                    {"num_predict", options_.max_tokens},
                    {"top_p", options_.top_p}
                }}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["options"]["stop"] = options_.stop_sequences;
            }
            
            // Convert messages to Ollama format
            nlohmann::json ollama_messages = nlohmann::json::array();
            
            // Process messages
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
                        role = "user";
                        break;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"content", message.content}
                };
                
                // For tool responses, add a prefix
                if (message.role == Message::Role::TOOL && message.name.has_value()) {
                    msg["content"] = "Tool result from " + message.name.value() + ": " + message.content;
                }
                
                ollama_messages.push_back(msg);
            }
            
            request_body["messages"] = ollama_messages;
            
            // For now, we'll just simulate streaming by breaking up the response
            // Ideally this would use a proper streaming HTTP client
            
            // Make a non-streaming API request and then simulate streaming
            request_body["stream"] = false;
            
            cpr::Response response = cpr::Post(
                cpr::Url{api_base_ + "/chat"},
                cpr::Header{{"Content-Type", "application/json"}},
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Ollama API error: {} {}", response.status_code, response.text);
                callback("Error: " + response.text, true);
                return;
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            String content;
            
            if (response_json.contains("message") && response_json["message"].contains("content")) {
                content = response_json["message"]["content"];
            } else {
                content = "";
            }
            
            // Simulate streaming by breaking up the response
            const int chunk_size = 10;
            for (size_t i = 0; i < content.length(); i += chunk_size) {
                size_t remaining = content.length() - i;
                size_t size = remaining < chunk_size ? remaining : chunk_size;
                String chunk = content.substr(i, size);
                
                bool is_last = (i + size >= content.length());
                callback(chunk, is_last);
                
                // Add a small delay to simulate streaming
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const std::exception& e) {
            spdlog::error("Error in Ollama LLM streaming: {}", e.what());
            callback("Error: " + String(e.what()), true);
        }
    }

private:
    String api_key_;  // Not used by Ollama but kept for interface compatibility
    String model_;
    String api_base_;
    LLMOptions options_;
};

// Export the LLM creation function
std::shared_ptr<LLMInterface> createOllamaLLM(const String& api_key, const String& model) {
    return std::make_shared<OllamaLLM>(api_key, model);
}

} // namespace agents 