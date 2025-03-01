#include <agents-cpp/llm_interface.h>
#include <cpr/cpr.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

namespace agents {

// Google AI Gemini implementation of the LLM interface
class GoogleLLM : public LLMInterface {
public:
    GoogleLLM(const String& api_key, const String& model = "gemini-1.5-pro") 
        : api_key_(api_key), model_(model), api_base_("https://generativelanguage.googleapis.com/v1beta/models/") {
    }

    ~GoogleLLM() override = default;

    std::vector<String> getAvailableModels() override {
        return {
            "gemini-1.5-pro",
            "gemini-1.5-flash",
            "gemini-1.0-pro",
            "gemini-1.0-ultra"
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
            // Build the endpoint URL with model and API key
            String endpoint = api_base_ + model_ + ":generateContent?key=" + api_key_;
            
            // Format messages for Google AI API
            nlohmann::json request_body = {
                {"generationConfig", {
                    {"temperature", options_.temperature},
                    {"maxOutputTokens", options_.max_tokens},
                    {"topP", options_.top_p}
                }}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["generationConfig"]["stopSequences"] = options_.stop_sequences;
            }
            
            // Convert messages to Google Gemini format
            nlohmann::json google_messages = nlohmann::json::array();
            
            // Handle system message differently
            String system_prompt;
            
            for (const auto& message : messages) {
                if (message.role == Message::Role::SYSTEM) {
                    system_prompt = message.content;
                    continue;  // Skip system messages for regular processing
                }
                
                String role;
                switch (message.role) {
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "model";
                        break;
                    case Message::Role::TOOL:
                        // Google doesn't support tool responses directly
                        continue;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"parts", {
                        {{"text", message.content}}
                    }}
                };
                
                google_messages.push_back(msg);
            }
            
            // Add system prompt as a preamble if present
            if (!system_prompt.empty()) {
                nlohmann::json system_content = {
                    {"role", "user"},
                    {"parts", {
                        {{"text", system_prompt}}
                    }}
                };
                google_messages.insert(google_messages.begin(), system_content);
            }
            
            request_body["contents"] = google_messages;
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{endpoint},
                cpr::Header{{"Content-Type", "application/json"}},
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Google AI API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("Google AI API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            
            LLMResponse result;
            
            if (response_json.contains("candidates") && !response_json["candidates"].empty() &&
                response_json["candidates"][0].contains("content") && 
                response_json["candidates"][0]["content"].contains("parts") &&
                !response_json["candidates"][0]["content"]["parts"].empty()) {
                
                result.content = response_json["candidates"][0]["content"]["parts"][0]["text"];
            } else {
                result.content = "";
            }
            
            // Add usage metrics if available
            if (response_json.contains("usageMetadata")) {
                result.usage_metrics["prompt_tokens"] = response_json["usageMetadata"]["promptTokenCount"];
                result.usage_metrics["completion_tokens"] = response_json["usageMetadata"]["candidatesTokenCount"];
                result.usage_metrics["total_tokens"] = response_json["usageMetadata"]["totalTokenCount"];
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in Google AI LLM: {}", e.what());
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
            // Build the endpoint URL with model and API key
            String endpoint = api_base_ + model_ + ":generateContent?key=" + api_key_;
            
            // Format messages for Google AI API
            nlohmann::json request_body = {
                {"generationConfig", {
                    {"temperature", options_.temperature},
                    {"maxOutputTokens", options_.max_tokens},
                    {"topP", options_.top_p}
                }}
            };
            
            if (!options_.stop_sequences.empty()) {
                request_body["generationConfig"]["stopSequences"] = options_.stop_sequences;
            }
            
            // Convert messages to Google Gemini format
            nlohmann::json google_messages = nlohmann::json::array();
            
            // Handle system message differently
            String system_prompt;
            
            for (const auto& message : messages) {
                if (message.role == Message::Role::SYSTEM) {
                    system_prompt = message.content;
                    continue;  // Skip system messages for regular processing
                }
                
                String role;
                switch (message.role) {
                    case Message::Role::USER:
                        role = "user";
                        break;
                    case Message::Role::ASSISTANT:
                        role = "model";
                        break;
                    case Message::Role::TOOL:
                        role = "user";  // Use user role for tool responses
                        break;
                    default:
                        continue;
                }
                
                nlohmann::json msg = {
                    {"role", role},
                    {"parts", {
                        {{"text", message.content}}
                    }}
                };
                
                // For tool responses, add a prefix
                if (message.role == Message::Role::TOOL && message.name.has_value()) {
                    msg["parts"][0]["text"] = "Tool result from " + message.name.value() + ": " + message.content;
                }
                
                google_messages.push_back(msg);
            }
            
            // Add system prompt with tools info as a preamble if present
            if (!system_prompt.empty() || !tools.empty()) {
                String full_system_prompt = system_prompt;
                
                // If we have tools, append their descriptions to the system prompt
                if (!tools.empty()) {
                    if (!full_system_prompt.empty()) {
                        full_system_prompt += "\n\n";
                    }
                    
                    full_system_prompt += "You have access to the following tools:\n\n";
                    
                    for (const auto& tool : tools) {
                        full_system_prompt += "Tool: " + tool->getName() + "\n";
                        full_system_prompt += "Description: " + tool->getDescription() + "\n";
                        
                        // Add parameters info
                        full_system_prompt += "Parameters:\n";
                        for (const auto& param_pair : tool->getParameters()) {
                            const Parameter& param = param_pair.second;
                            full_system_prompt += "  - " + param.name + " (" + param.type + "): " + 
                                param.description + (param.required ? " (Required)" : "") + "\n";
                        }
                        
                        full_system_prompt += "\n";
                    }
                    
                    full_system_prompt += "When you need to use a tool, format your response exactly like this:\n";
                    full_system_prompt += "ACTION: tool_name\n";
                    full_system_prompt += "ACTION_INPUT: {\"param1\": \"value1\", \"param2\": \"value2\"}\n\n";
                    full_system_prompt += "After receiving the tool result, continue the conversation normally.";
                }
                
                nlohmann::json system_content = {
                    {"role", "user"},
                    {"parts", {
                        {{"text", full_system_prompt}}
                    }}
                };
                google_messages.insert(google_messages.begin(), system_content);
            }
            
            request_body["contents"] = google_messages;
            
            // Make API request
            cpr::Response response = cpr::Post(
                cpr::Url{endpoint},
                cpr::Header{{"Content-Type", "application/json"}},
                cpr::Body{request_body.dump()},
                cpr::Timeout{options_.timeout_ms}
            );
            
            if (response.status_code != 200) {
                spdlog::error("Google AI API error: {} {}", response.status_code, response.text);
                throw std::runtime_error("Google AI API error: " + response.text);
            }
            
            // Parse response
            nlohmann::json response_json = nlohmann::json::parse(response.text);
            
            LLMResponse result;
            
            if (response_json.contains("candidates") && !response_json["candidates"].empty() &&
                response_json["candidates"][0].contains("content") && 
                response_json["candidates"][0]["content"].contains("parts") &&
                !response_json["candidates"][0]["content"]["parts"].empty()) {
                
                String content = response_json["candidates"][0]["content"]["parts"][0]["text"];
                
                // Parse content to extract tool calls if present
                // Example format: "ACTION: tool_name\nACTION_INPUT: {\"param1\": \"value1\"}\n"
                // TODO: Improve this parsing with a more robust regex-based approach
                if (content.find("ACTION:") != String::npos && content.find("ACTION_INPUT:") != String::npos) {
                    size_t action_pos = content.find("ACTION:");
                    size_t input_pos = content.find("ACTION_INPUT:");
                    
                    if (action_pos != String::npos && input_pos != String::npos && input_pos > action_pos) {
                        String tool_name = content.substr(action_pos + 8, input_pos - action_pos - 9);
                        tool_name = tool_name.substr(0, tool_name.find_first_of("\n"));
                        tool_name = tool_name.substr(tool_name.find_first_not_of(" "));
                        tool_name = tool_name.substr(0, tool_name.find_last_not_of(" ") + 1);
                        
                        String json_str = content.substr(input_pos + 13);
                        json_str = json_str.substr(0, json_str.find("\n"));
                        json_str = json_str.substr(json_str.find_first_not_of(" "));
                        
                        try {
                            // Fix: Ensure string is valid JSON before parsing
                            JsonObject params = nlohmann::json::parse(json_str);
                            result.tool_calls.emplace_back(tool_name, params);
                            
                            // Remove the tool call portion from the content
                            result.content = content.substr(0, action_pos);
                        } catch (const std::exception& e) {
                            spdlog::error("Error parsing tool input JSON: {}", e.what());
                            result.content = content;
                        }
                    } else {
                        result.content = content;
                    }
                } else {
                    result.content = content;
                }
            } else {
                result.content = "";
            }
            
            // Add usage metrics if available
            if (response_json.contains("usageMetadata")) {
                result.usage_metrics["prompt_tokens"] = response_json["usageMetadata"]["promptTokenCount"];
                result.usage_metrics["completion_tokens"] = response_json["usageMetadata"]["candidatesTokenCount"];
                result.usage_metrics["total_tokens"] = response_json["usageMetadata"]["totalTokenCount"];
            }
            
            return result;
        } catch (const std::exception& e) {
            spdlog::error("Error in Google AI LLM: {}", e.what());
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
            // For now, we'll just execute a non-streaming request and simulate streaming
            auto response = chat(messages);
            
            // Simulate streaming by breaking up the response
            const int chunk_size = 10;
            for (size_t i = 0; i < response.content.length(); i += chunk_size) {
                size_t remaining = response.content.length() - i;
                size_t size = remaining < chunk_size ? remaining : chunk_size;
                String chunk = response.content.substr(i, size);
                
                bool is_last = (i + size >= response.content.length());
                callback(chunk, is_last);
                
                // Add a small delay to simulate streaming
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const std::exception& e) {
            spdlog::error("Error in Google AI LLM streaming: {}", e.what());
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
std::shared_ptr<LLMInterface> createGoogleLLM(const String& api_key, const String& model) {
    return std::make_shared<GoogleLLM>(api_key, model);
}

} // namespace agents 