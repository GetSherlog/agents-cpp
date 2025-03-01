#include <agents-cpp/memory.h>
#include <map>
#include <algorithm>

namespace agents {

// Simple memory implementation
class SimpleMemory : public Memory {
public:
    SimpleMemory() = default;
    ~SimpleMemory() override = default;

    void add(const String& key, const JsonObject& value, MemoryType type = MemoryType::SHORT_TERM) override {
        memory_[static_cast<int>(type)][key] = value;
    }
    
    std::optional<JsonObject> get(const String& key, MemoryType type = MemoryType::SHORT_TERM) const override {
        const auto& memory_map = memory_.find(static_cast<int>(type));
        if (memory_map == memory_.end()) {
            return std::nullopt;
        }
        
        const auto& entry = memory_map->second.find(key);
        if (entry == memory_map->second.end()) {
            return std::nullopt;
        }
        
        return entry->second;
    }
    
    bool has(const String& key, MemoryType type = MemoryType::SHORT_TERM) const override {
        const auto& memory_map = memory_.find(static_cast<int>(type));
        if (memory_map == memory_.end()) {
            return false;
        }
        
        return memory_map->second.find(key) != memory_map->second.end();
    }
    
    void remove(const String& key, MemoryType type = MemoryType::SHORT_TERM) override {
        auto memory_map = memory_.find(static_cast<int>(type));
        if (memory_map != memory_.end()) {
            memory_map->second.erase(key);
        }
    }
    
    void clear(MemoryType type = MemoryType::SHORT_TERM) override {
        memory_[static_cast<int>(type)].clear();
    }
    
    void addMessage(const Message& message) override {
        messages_.push_back(message);
    }
    
    std::vector<Message> getMessages() const override {
        return messages_;
    }
    
    String getConversationSummary(int max_length = 0) const override {
        String summary;
        
        for (const auto& message : messages_) {
            String role_str;
            switch (message.role) {
                case Message::Role::SYSTEM:
                    role_str = "System: ";
                    break;
                case Message::Role::USER:
                    role_str = "User: ";
                    break;
                case Message::Role::ASSISTANT:
                    role_str = "Assistant: ";
                    break;
                case Message::Role::TOOL:
                    role_str = "Tool (" + message.name.value_or("unknown") + "): ";
                    break;
            }
            
            summary += role_str + message.content + "\n\n";
        }
        
        // Truncate if needed
        if (max_length > 0 && summary.length() > static_cast<size_t>(max_length)) {
            summary = summary.substr(0, max_length) + "...";
        }
        
        return summary;
    }
    
    std::vector<std::pair<JsonObject, float>> search(
        const String& query, 
        MemoryType type = MemoryType::LONG_TERM,
        int max_results = 5
    ) const override {
        // Simple implementation: just return the most recent entries
        // In a real implementation, this would use semantic search
        std::vector<std::pair<JsonObject, float>> results;
        
        const auto& memory_map = memory_.find(static_cast<int>(type));
        if (memory_map == memory_.end()) {
            return results;
        }
        
        for (const auto& entry : memory_map->second) {
            // For now, just use a placeholder similarity score
            results.emplace_back(entry.second, 0.5f);
            
            if (results.size() >= static_cast<size_t>(max_results)) {
                break;
            }
        }
        
        return results;
    }

private:
    // Memory storage organized by type and key
    std::map<int, std::map<String, JsonObject>> memory_;
    
    // Conversation history
    std::vector<Message> messages_;
};

std::shared_ptr<Memory> createMemory() {
    return std::make_shared<SimpleMemory>();
}

} // namespace agents 