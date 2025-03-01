#pragma once

#include <agents-cpp/types.h>
#include <agents-cpp/tool.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/memory.h>
#include <agents-cpp/coroutine_utils.h>
#include <vector>
#include <memory>
#include <map>

namespace agents {

/**
 * @brief Context for an agent, containing tools, LLM, and memory
 */
class AgentContext {
public:
    AgentContext();
    ~AgentContext() = default;

    // Set the LLM to use
    void setLLM(std::shared_ptr<LLMInterface> llm);
    
    // Get the LLM
    std::shared_ptr<LLMInterface> getLLM() const;
    
    // Set the system prompt
    void setSystemPrompt(const String& system_prompt);
    
    // Get the system prompt
    const String& getSystemPrompt() const;
    
    // Register a tool
    void registerTool(std::shared_ptr<Tool> tool);
    
    // Get a tool by name
    std::shared_ptr<Tool> getTool(const String& name) const;
    
    // Get all tools
    std::vector<std::shared_ptr<Tool>> getTools() const;
    
    // Execute a tool by name using coroutines
    Task<ToolResult> executeTool(const String& name, const JsonObject& params);
    
    // Get memory
    std::shared_ptr<Memory> getMemory() const;
    
    // Add a message to the conversation history
    void addMessage(const Message& message);
    
    // Get all messages in the conversation history
    std::vector<Message> getMessages() const;
    
    // Run a chat completion with the current context using coroutines
    Task<LLMResponse> chat(const String& user_message);
    
    // Run a chat completion with tools using coroutines
    Task<LLMResponse> chatWithTools(const String& user_message);
    
    // Stream chat results with AsyncGenerator
    AsyncGenerator<String> streamChat(const String& user_message);

private:
    std::shared_ptr<LLMInterface> llm_;
    std::shared_ptr<Memory> memory_;
    std::map<String, std::shared_ptr<Tool>> tools_;
    String system_prompt_;
};

} // namespace agents 