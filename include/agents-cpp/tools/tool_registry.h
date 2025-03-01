#pragma once

#include <agents-cpp/tool.h>
#include <map>
#include <vector>
#include <memory>

namespace agents {
namespace tools {

/**
 * @brief Registry for tools that agents can use
 * 
 * The ToolRegistry provides a central place to register, retrieve,
 * and manage tools that agents can use.
 */
class ToolRegistry {
public:
    ToolRegistry() = default;
    ~ToolRegistry() = default;
    
    // Register a tool
    void registerTool(std::shared_ptr<Tool> tool);
    
    // Get a tool by name
    std::shared_ptr<Tool> getTool(const String& name) const;
    
    // Get all registered tools
    std::vector<std::shared_ptr<Tool>> getAllTools() const;
    
    // Check if a tool is registered
    bool hasTool(const String& name) const;
    
    // Remove a tool
    void removeTool(const String& name);
    
    // Clear all tools
    void clear();
    
    // Get tool schemas as JSON
    JsonObject getToolSchemas() const;
    
    // Get the global tool registry
    static ToolRegistry& global();

private:
    std::map<String, std::shared_ptr<Tool>> tools_;
};

/**
 * @brief Create and register standard tools
 * 
 * @param registry The tool registry to register tools with
 */
void registerStandardTools(ToolRegistry& registry);

/**
 * @brief Creates a tool for executing shell commands
 */
std::shared_ptr<Tool> createShellCommandTool();

/**
 * @brief Creates a tool for performing web searches
 */
std::shared_ptr<Tool> createWebSearchTool();

/**
 * @brief Creates a tool for retrieving information from Wikipedia
 */
std::shared_ptr<Tool> createWikipediaTool();

/**
 * @brief Creates a tool for running Python code
 */
std::shared_ptr<Tool> createPythonTool();

/**
 * @brief Creates a tool for reading files
 */
std::shared_ptr<Tool> createFileReadTool();

/**
 * @brief Creates a tool for writing files
 */
std::shared_ptr<Tool> createFileWriteTool();

} // namespace tools
} // namespace agents 