#pragma once

#include <agents-cpp/llm_interface.h>

namespace agents {
namespace llms {

/**
 * @brief Implementation of LLMInterface for OpenAI models
 */
class OpenAILLM : public LLMInterface {
public:
    OpenAILLM(const String& api_key = "", const String& model = "gpt-4o-2024-05-13");
    ~OpenAILLM() override = default;
    
    // Get available models from OpenAI
    std::vector<String> getAvailableModels() override;
    
    // Set the model to use
    void setModel(const String& model) override;
    
    // Get current model
    String getModel() const override;
    
    // Set API key
    void setApiKey(const String& api_key) override;
    
    // Set API base URL (for self-hosted or proxied endpoints)
    void setApiBase(const String& api_base) override;
    
    // Set options for API calls
    void setOptions(const LLMOptions& options) override;
    
    // Get current options
    LLMOptions getOptions() const override;
    
    // Generate completion from a prompt
    LLMResponse complete(const String& prompt) override;
    
    // Generate completion from a list of messages
    LLMResponse chat(const std::vector<Message>& messages) override;
    
    // Generate completion with available tools
    LLMResponse chatWithTools(
        const std::vector<Message>& messages,
        const std::vector<std::shared_ptr<Tool>>& tools
    ) override;
    
    // Stream results with callback
    void streamChat(
        const std::vector<Message>& messages,
        std::function<void(const String&, bool)> callback
    ) override;

private:
    String api_key_;
    String api_base_ = "https://api.openai.com/v1";
    String model_;
    LLMOptions options_;
    
    // Convert Message list to OpenAI API format
    JsonObject messagesToOpenAIFormat(const std::vector<Message>& messages);
    
    // Convert Tool list to OpenAI API format
    JsonObject toolsToOpenAIFormat(const std::vector<std::shared_ptr<Tool>>& tools);
    
    // Convert OpenAI API response to LLMResponse
    LLMResponse parseOpenAIResponse(const JsonObject& response);
    
    // Make API call to OpenAI
    JsonObject makeApiCall(const JsonObject& request_body, bool stream = false, const String& endpoint = "chat/completions");
};

} // namespace llms
} // namespace agents 