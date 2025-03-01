#include <agents-cpp/llm_interface.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <algorithm>
#include <cctype>

// Forward declarations of LLM provider classes
namespace agents {
class AnthropicLLM;
class OpenAILLM;
class GoogleLLM;
class OllamaLLM;
}

namespace agents {

// Default implementation of complete that falls back to chat
LLMResponse LLMInterface::complete(const String& prompt) {
    // Convert prompt to a message and use chat
    Message msg;
    msg.role = Message::Role::USER;
    msg.content = prompt;
    return chat({msg});
}

// Default implementation for chat with vector of messages
LLMResponse LLMInterface::complete(const std::vector<Message>& messages) {
    return chat(messages);
}

// Default implementation for completeWithTools that uses a schema
LLMResponse LLMInterface::completeWithTools(
    const std::vector<Message>& messages,
    const std::vector<JsonObject>& tools_schema
) {
    // Create tool objects from schema
    std::vector<std::shared_ptr<Tool>> tools;
    for (const auto& schema : tools_schema) {
        auto tool = std::make_shared<Tool>(
            schema["name"].get<String>(),
            schema["description"].get<String>()
        );
        tools.push_back(tool);
    }
    
    return chatWithTools(messages, tools);
}

// Forward declaration of LLM provider constructors
extern std::shared_ptr<LLMInterface> createAnthropicLLM(const String& api_key, const String& model);
extern std::shared_ptr<LLMInterface> createOpenAILLM(const String& api_key, const String& model);
extern std::shared_ptr<LLMInterface> createGoogleLLM(const String& api_key, const String& model);
extern std::shared_ptr<LLMInterface> createOllamaLLM(const String& api_key, const String& model);

// Helper function to lowercase a string
String toLower(const String& str) {
    String result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::shared_ptr<LLMInterface> createLLM(
    const String& provider,
    const String& api_key,
    const String& model
) {
    // Convert provider name to lowercase for case-insensitive comparison
    String provider_lower = toLower(provider);
    
    if (provider_lower == "anthropic") {
        return createAnthropicLLM(api_key, model.empty() ? "claude-3-opus-20240229" : model);
    } else if (provider_lower == "openai") {
        return createOpenAILLM(api_key, model.empty() ? "gpt-4o" : model);
    } else if (provider_lower == "google") {
        return createGoogleLLM(api_key, model.empty() ? "gemini-1.5-pro" : model);
    } else if (provider_lower == "ollama") {
        return createOllamaLLM(api_key, model.empty() ? "llama3" : model);
    } else {
        throw std::runtime_error("Unknown LLM provider: " + provider);
    }
}

} // namespace agents 