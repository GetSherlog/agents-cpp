#include <agents-cpp/agent_context.h>
#include <agents-cpp/logger.h>
#include <stdexcept>

namespace agents {

AgentContext::AgentContext() : memory_(createMemory()) {
    // Initialize with empty values
}

void AgentContext::setLLM(std::shared_ptr<LLMInterface> llm) {
    llm_ = llm;
}

std::shared_ptr<LLMInterface> AgentContext::getLLM() const {
    return llm_;
}

void AgentContext::setSystemPrompt(const String& system_prompt) {
    system_prompt_ = system_prompt;
}

const String& AgentContext::getSystemPrompt() const {
    return system_prompt_;
}

void AgentContext::registerTool(std::shared_ptr<Tool> tool) {
    tools_[tool->getName()] = tool;
}

std::shared_ptr<Tool> AgentContext::getTool(const String& name) const {
    auto it = tools_.find(name);
    if (it != tools_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Tool>> AgentContext::getTools() const {
    std::vector<std::shared_ptr<Tool>> result;
    for (const auto& pair : tools_) {
        result.push_back(pair.second);
    }
    return result;
}

std::shared_ptr<Memory> AgentContext::getMemory() const {
    return memory_;
}

void AgentContext::addMessage(const Message& message) {
    memory_->addMessage(message);
}

std::vector<Message> AgentContext::getMessages() const {
    return memory_->getMessages();
}

// Coroutine-based implementations

Task<ToolResult> AgentContext::executeTool(const String& name, const JsonObject& params) {
    Logger::debug("Executing tool: {}", name);
    if (!llm_) {
        throw std::runtime_error("LLM not set in agent context");
    }
    
    auto tool = getTool(name);
    if (!tool) {
        throw std::runtime_error("Tool not found: " + name);
    }
    
    // In a real implementation, we would add asynchronous execution of tools
    // For now, just run the tool synchronously
    ToolResult result = tool->execute(params);
    
    co_return result;
}

Task<LLMResponse> AgentContext::chat(const String& user_message) {
    Logger::debug("Chat: {}", user_message);
    if (!llm_) {
        throw std::runtime_error("LLM not set in agent context");
    }
    
    // Create a user message
    Message msg;
    msg.role = Message::Role::USER;
    msg.content = user_message;
    
    // Add the message to history
    if (memory_) {
        memory_->addMessage(msg);
    }
    
    // Prepare messages for the LLM
    std::vector<Message> messages;
    
    // Add system message if set
    if (!system_prompt_.empty()) {
        Message system_msg;
        system_msg.role = Message::Role::SYSTEM;
        system_msg.content = system_prompt_;
        messages.push_back(system_msg);
    }
    
    // Add conversation history from memory if available
    if (memory_) {
        auto history = memory_->getMessages();
        messages.insert(messages.end(), history.begin(), history.end());
    } else {
        // Otherwise just add the current message
        messages.push_back(msg);
    }
    
    // Use the LLM's async method
    auto response = co_await llm_->chatAsync(messages);
    
    // Add the response to memory
    if (memory_) {
        Message response_msg;
        response_msg.role = Message::Role::ASSISTANT;
        response_msg.content = response.content;
        memory_->addMessage(response_msg);
    }
    
    co_return response;
}

Task<LLMResponse> AgentContext::chatWithTools(const String& user_message) {
    Logger::debug("Chat with tools: {}", user_message);
    if (!llm_) {
        throw std::runtime_error("LLM not set in agent context");
    }
    
    // Create a user message
    Message msg;
    msg.role = Message::Role::USER;
    msg.content = user_message;
    
    // Add the message to history
    if (memory_) {
        memory_->addMessage(msg);
    }
    
    // Prepare messages for the LLM
    std::vector<Message> messages;
    
    // Add system message if set
    if (!system_prompt_.empty()) {
        Message system_msg;
        system_msg.role = Message::Role::SYSTEM;
        system_msg.content = system_prompt_;
        messages.push_back(system_msg);
    }
    
    // Add conversation history from memory if available
    if (memory_) {
        auto history = memory_->getMessages();
        messages.insert(messages.end(), history.begin(), history.end());
    } else {
        // Otherwise just add the current message
        messages.push_back(msg);
    }
    
    // Get all tools
    auto tools = getTools();
    
    // Use the LLM's async method
    auto response = co_await llm_->chatWithToolsAsync(messages, tools);
    
    // Add the response to memory
    if (memory_) {
        Message response_msg;
        response_msg.role = Message::Role::ASSISTANT;
        response_msg.content = response.content;
        memory_->addMessage(response_msg);
    }
    
    co_return response;
}

AsyncGenerator<String> AgentContext::streamChat(const String& user_message) {
    Logger::debug("Stream chat: {}", user_message);
    if (!llm_) {
        throw std::runtime_error("LLM not set in agent context");
    }
    
    // Create a user message
    Message msg;
    msg.role = Message::Role::USER;
    msg.content = user_message;
    
    // Add the message to history
    if (memory_) {
        memory_->addMessage(msg);
    }
    
    // Prepare messages for the LLM
    std::vector<Message> messages;
    
    // Add system message if set
    if (!system_prompt_.empty()) {
        Message system_msg;
        system_msg.role = Message::Role::SYSTEM;
        system_msg.content = system_prompt_;
        messages.push_back(system_msg);
    }
    
    // Add conversation history from memory if available
    if (memory_) {
        auto history = memory_->getMessages();
        messages.insert(messages.end(), history.begin(), history.end());
    } else {
        // Otherwise just add the current message
        messages.push_back(msg);
    }
    
    // Use the LLM's stream method and forward chunks
    auto generator = llm_->streamChatAsync(messages);
    
    // Create an async generator that forwards from the LLM generator
    // but also adds the final message to memory
    String full_response;
    
    // Return a new generator that yields chunks and collects them
    for co_await (auto chunk : generator) {
        full_response += chunk;
        co_yield chunk;
    }
    
    // After streaming is complete, add the response to memory
    if (memory_ && !full_response.empty()) {
        Message response_msg;
        response_msg.role = Message::Role::ASSISTANT;
        response_msg.content = full_response;
        memory_->addMessage(response_msg);
    }
}

} // namespace agents 