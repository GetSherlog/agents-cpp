#include <agents-cpp/tools/tool_registry.h>
#include <agents-cpp/tool.h>
#include <stdexcept>

namespace agents {
namespace tools {

void ToolRegistry::registerTool(std::shared_ptr<Tool> tool) {
    if (!tool) {
        throw std::invalid_argument("Cannot register null tool");
    }
    
    tools_[tool->getName()] = tool;
}

std::shared_ptr<Tool> ToolRegistry::getTool(const String& name) const {
    auto it = tools_.find(name);
    if (it == tools_.end()) {
        return nullptr;
    }
    
    return it->second;
}

std::vector<std::shared_ptr<Tool>> ToolRegistry::getAllTools() const {
    std::vector<std::shared_ptr<Tool>> result;
    result.reserve(tools_.size());
    
    for (const auto& pair : tools_) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool ToolRegistry::hasTool(const String& name) const {
    return tools_.find(name) != tools_.end();
}

void ToolRegistry::removeTool(const String& name) {
    tools_.erase(name);
}

void ToolRegistry::clear() {
    tools_.clear();
}

JsonObject ToolRegistry::getToolSchemas() const {
    JsonObject schemas;
    std::vector<JsonObject> toolsArray;
    
    for (const auto& pair : tools_) {
        toolsArray.push_back(pair.second->getSchema());
    }
    
    schemas["tools"] = toolsArray;
    return schemas;
}

ToolRegistry& ToolRegistry::global() {
    static ToolRegistry instance;
    return instance;
}

// Standard tools implementation
void registerStandardTools(ToolRegistry& registry) {
    registry.registerTool(createShellCommandTool());
    registry.registerTool(createWebSearchTool());
    registry.registerTool(createWikipediaTool());
    registry.registerTool(createPythonTool());
    registry.registerTool(createFileReadTool());
    registry.registerTool(createFileWriteTool());
}

std::shared_ptr<Tool> createShellCommandTool() {
    auto tool = std::make_shared<Tool>("shell", "Execute shell commands on the system");
    
    Parameter command;
    command.name = "command";
    command.description = "The shell command to execute";
    command.type = "string";
    command.required = true;
    
    tool->addParameter(command);
    
    tool->setCallback([](const JsonObject& params) {
        ToolResult result;
        result.success = true;
        result.content = "Shell command executed: " + params["command"].get<String>();
        // In a real implementation, this would actually execute the command
        return result;
    });
    
    return tool;
}

std::shared_ptr<Tool> createWebSearchTool() {
    auto tool = std::make_shared<Tool>("web_search", "Search the web for information");
    
    Parameter query;
    query.name = "query";
    query.description = "The search query";
    query.type = "string";
    query.required = true;
    
    tool->addParameter(query);
    
    tool->setCallback([](const JsonObject& params) {
        ToolResult result;
        result.success = true;
        result.content = "Web search results for: " + params["query"].get<String>();
        // In a real implementation, this would perform an actual web search
        return result;
    });
    
    return tool;
}

std::shared_ptr<Tool> createWikipediaTool() {
    auto tool = std::make_shared<Tool>("wikipedia", "Search Wikipedia for information");
    
    Parameter query;
    query.name = "query";
    query.description = "The Wikipedia article to search for";
    query.type = "string";
    query.required = true;
    
    tool->addParameter(query);
    
    tool->setCallback([](const JsonObject& params) {
        ToolResult result;
        result.success = true;
        result.content = "Wikipedia results for: " + params["query"].get<String>();
        // In a real implementation, this would fetch Wikipedia content
        return result;
    });
    
    return tool;
}

std::shared_ptr<Tool> createPythonTool() {
    auto tool = std::make_shared<Tool>("python", "Execute Python code");
    
    Parameter code;
    code.name = "code";
    code.description = "The Python code to execute";
    code.type = "string";
    code.required = true;
    
    tool->addParameter(code);
    
    tool->setCallback([](const JsonObject& params) {
        ToolResult result;
        result.success = true;
        result.content = "Python code executed: " + params["code"].get<String>();
        // In a real implementation, this would execute Python code
        return result;
    });
    
    return tool;
}

std::shared_ptr<Tool> createFileReadTool() {
    auto tool = std::make_shared<Tool>("file_read", "Read a file from the filesystem");
    
    Parameter path;
    path.name = "path";
    path.description = "The path to the file to read";
    path.type = "string";
    path.required = true;
    
    tool->addParameter(path);
    
    tool->setCallback([](const JsonObject& params) {
        ToolResult result;
        result.success = true;
        result.content = "File read from: " + params["path"].get<String>();
        // In a real implementation, this would read the file content
        return result;
    });
    
    return tool;
}

std::shared_ptr<Tool> createFileWriteTool() {
    auto tool = std::make_shared<Tool>("file_write", "Write to a file in the filesystem");
    
    Parameter path;
    path.name = "path";
    path.description = "The path to the file to write";
    path.type = "string";
    path.required = true;
    
    Parameter content;
    content.name = "content";
    content.description = "The content to write to the file";
    content.type = "string";
    content.required = true;
    
    tool->addParameter(path);
    tool->addParameter(content);
    
    tool->setCallback([](const JsonObject& params) {
        ToolResult result;
        result.success = true;
        result.content = "File written to: " + params["path"].get<String>();
        // In a real implementation, this would write the content to the file
        return result;
    });
    
    return tool;
}

} // namespace tools
} // namespace agents 