#pragma once

#include <agents-cpp/llm_interface.h>

namespace agents {
namespace llms {

/**
 * @brief Implementation of LLMInterface for Ollama models
 */
class OllamaLLM : public LLMInterface {
public:
    OllamaLLM(const String& model = "llama3");
    ~OllamaLLM() override = default;
    
    // Get available models from Ollama
    std::vector<String> getAvailableModels() override;
    
    // Set the model to use
    void setModel(const String& model) override;
    
    // Get current model
    String getModel() const override;
    
    // Set API key (not used for Ollama, but implemented for interface compliance)
    void setApiKey(const String& api_key) override;
    
    // Set API base URL for Ollama server
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
    String api_base_ = "http://localhost:11434/api";
    String model_;
    LLMOptions options_;
    
    // Convert Message list to Ollama API format
    JsonObject messagesToOllamaFormat(const std::vector<Message>& messages);
    
    // Convert Ollama API response to LLMResponse
    LLMResponse parseOllamaResponse(const JsonObject& response);
    
    // Make API call to Ollama
    JsonObject makeApiCall(const JsonObject& request_body, bool stream = false);
    
    // Format message history for models without chat format support
    String formatMessagesAsPrompt(const std::vector<Message>& messages);
    
    // Check if the model supports chat format
    bool modelSupportsChatFormat() const;
    
    // Check if the model supports tool calls
    bool modelSupportsToolCalls() const;
};

} // namespace llms
} // namespace agents 