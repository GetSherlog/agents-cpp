#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <optional>
#include <any>
#include <nlohmann/json.hpp>

namespace agents {

using json = nlohmann::json;

// Common type definitions
using String = std::string;
using StringMap = std::map<String, String>;
using JsonObject = json;

// Parameter types for tools and LLM calls
struct Parameter {
    String name;
    String description;
    String type;
    bool required;
    std::optional<json> default_value;
};

using ParameterMap = std::map<String, Parameter>;

// Response from an LLM
struct LLMResponse {
    String content;
    std::vector<std::pair<String, json>> tool_calls;
    std::map<String, double> usage_metrics;
};

// Message in a conversation
struct Message {
    enum class Role {
        SYSTEM,
        USER,
        ASSISTANT,
        TOOL
    };

    Role role;
    String content;
    std::optional<String> name;
    std::optional<String> tool_call_id;
    std::vector<std::pair<String, json>> tool_calls;
};

// Memory types
enum class MemoryType {
    SHORT_TERM,
    LONG_TERM,
    WORKING
};

} // namespace agents 