#pragma once

#include <agents-cpp/types.h>
#include <vector>
#include <memory>
#include <optional>

namespace agents {

/**
 * @brief Interface for agent memory storage
 * 
 * Memory stores information that can be retrieved by the agent,
 * such as conversation history, retrieved documents, or 
 * intermediate results.
 */
class Memory {
public:
    virtual ~Memory() = default;

    // Add a memory entry
    virtual void add(const String& key, const JsonObject& value, MemoryType type = MemoryType::SHORT_TERM) = 0;
    
    // Get a memory entry by key
    virtual std::optional<JsonObject> get(const String& key, MemoryType type = MemoryType::SHORT_TERM) const = 0;
    
    // Check if a memory entry exists
    virtual bool has(const String& key, MemoryType type = MemoryType::SHORT_TERM) const = 0;
    
    // Remove a memory entry
    virtual void remove(const String& key, MemoryType type = MemoryType::SHORT_TERM) = 0;
    
    // Clear all memory of a specific type
    virtual void clear(MemoryType type = MemoryType::SHORT_TERM) = 0;
    
    // Add a conversation message to memory
    virtual void addMessage(const Message& message) = 0;
    
    // Get all conversation messages
    virtual std::vector<Message> getMessages() const = 0;
    
    // Get conversation summary as a string
    virtual String getConversationSummary(int max_length = 0) const = 0;
    
    // Semantic search in memory
    virtual std::vector<std::pair<JsonObject, float>> search(
        const String& query, 
        MemoryType type = MemoryType::LONG_TERM,
        int max_results = 5
    ) const = 0;
};

/**
 * @brief Create a Memory instance
 */
std::shared_ptr<Memory> createMemory();

} // namespace agents 