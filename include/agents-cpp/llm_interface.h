#pragma once

#include <agents-cpp/types.h>
#include <agents-cpp/tool.h>
#include <agents-cpp/coroutine_utils.h>
#include <functional>
#include <vector>
#include <memory>

namespace agents {

/**
 * @brief Options for LLM API calls
 */
struct LLMOptions {
    double temperature = 0.7;
    int max_tokens = 1024;
    double top_p = 1.0;
    double presence_penalty = 0.0;
    double frequency_penalty = 0.0;
    int timeout_ms = 30000; // 30 seconds
    std::vector<String> stop_sequences;
};

/**
 * @brief Interface for language model providers (OpenAI, Anthropic, Google, Ollama)
 */
class LLMInterface {
public:
    virtual ~LLMInterface() = default;

    // Get available models from this provider
    virtual std::vector<String> getAvailableModels() = 0;
    
    // Set the model to use
    virtual void setModel(const String& model) = 0;
    
    // Get current model
    virtual String getModel() const = 0;
    
    // Set API key
    virtual void setApiKey(const String& api_key) = 0;
    
    // Set API base URL (for self-hosted or proxied endpoints)
    virtual void setApiBase(const String& api_base) = 0;
    
    // Set options for API calls
    virtual void setOptions(const LLMOptions& options) = 0;
    
    // Get current options
    virtual LLMOptions getOptions() const = 0;
    
    // Generate completion from a prompt
    virtual LLMResponse complete(const String& prompt);
    
    // Generate completion from a list of messages
    virtual LLMResponse complete(const std::vector<Message>& messages);
    
    // Generate completion with available tools
    virtual LLMResponse completeWithTools(
        const std::vector<Message>& messages,
        const std::vector<JsonObject>& tools_schema
    );
    
    // Generate completion from a list of messages
    virtual LLMResponse chat(const std::vector<Message>& messages) = 0;
    
    // Generate completion with available tools
    virtual LLMResponse chatWithTools(
        const std::vector<Message>& messages,
        const std::vector<std::shared_ptr<Tool>>& tools
    ) = 0;
    
    // Stream results with callback
    virtual void streamChat(
        const std::vector<Message>& messages,
        std::function<void(const String&, bool)> callback
    ) = 0;

    // Coroutine versions of the above methods
    
    // Async complete from a prompt
    virtual Task<LLMResponse> completeAsync(const String& prompt) {
        // Default implementation falls back to synchronous method
        co_return complete(prompt);
    }
    
    // Async complete from a list of messages
    virtual Task<LLMResponse> completeAsync(const std::vector<Message>& messages) {
        // Default implementation falls back to synchronous method
        co_return complete(messages);
    }
    
    // Async chat from a list of messages
    virtual Task<LLMResponse> chatAsync(const std::vector<Message>& messages) {
        // Default implementation falls back to synchronous method
        co_return chat(messages);
    }
    
    // Async chat with tools
    virtual Task<LLMResponse> chatWithToolsAsync(
        const std::vector<Message>& messages,
        const std::vector<std::shared_ptr<Tool>>& tools
    ) {
        // Default implementation falls back to synchronous method
        co_return chatWithTools(messages, tools);
    }
    
    // Stream chat with AsyncGenerator
    virtual AsyncGenerator<String> streamChatAsync(
        const std::vector<Message>& messages
    ) {
        // Default implementation uses the callback-based method
        // and converts it to an AsyncGenerator
        
        // This will be implemented by derived classes for better performance
        co_yield "Not implemented";
    }
};

/**
 * @brief Factory function to create a specific LLM provider
 * 
 * @param provider One of: "anthropic", "openai", "google", "ollama"
 * @param api_key API key for the provider
 * @param model Model to use (provider-specific)
 * @return std::shared_ptr<LLMInterface> 
 */
std::shared_ptr<LLMInterface> createLLM(
    const String& provider,
    const String& api_key,
    const String& model = ""
);

} // namespace agents 