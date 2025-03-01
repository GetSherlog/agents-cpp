#pragma once

#include <agents-cpp/types.h>
#include <functional>

namespace agents {

// Result of a tool execution
struct ToolResult {
    bool success;
    String content;
    JsonObject data;
};

// Callback type for tool execution
using ToolCallback = std::function<ToolResult(const JsonObject&)>;

/**
 * @brief Interface for tools that an agent can use
 * 
 * Tools are capabilities that the LLM can use to interact with the outside world.
 * Each tool has a name, description, set of parameters, and execution logic.
 */
class Tool {
public:
    Tool(const String& name, const String& description);
    virtual ~Tool() = default;

    // Getters
    const String& getName() const;
    const String& getDescription() const;
    const ParameterMap& getParameters() const;
    const JsonObject& getSchema() const;

    // Add a parameter to the tool
    void addParameter(const Parameter& param);
    
    // Set the execution callback
    void setCallback(ToolCallback callback);
    
    // Execute the tool with the given parameters
    virtual ToolResult execute(const JsonObject& params) const;
    
    // Validate parameters against schema
    bool validateParameters(const JsonObject& params) const;

protected:
    String name_;
    String description_;
    ParameterMap parameters_;
    ToolCallback callback_;
    JsonObject schema_;

    // Update schema when parameters change
    void updateSchema();
};

/**
 * @brief Create a custom tool with a name, description, and callback
 */
std::shared_ptr<Tool> createTool(
    const String& name,
    const String& description,
    const std::vector<Parameter>& parameters,
    ToolCallback callback
);

} // namespace agents 