#include <agents-cpp/tool.h>
#include <stdexcept>

namespace agents {

Tool::Tool(const String& name, const String& description)
    : name_(name), description_(description) {
    // Initialize tool with name and description
    updateSchema();
}

const String& Tool::getName() const {
    return name_;
}

const String& Tool::getDescription() const {
    return description_;
}

const ParameterMap& Tool::getParameters() const {
    return parameters_;
}

const JsonObject& Tool::getSchema() const {
    return schema_;
}

void Tool::addParameter(const Parameter& param) {
    parameters_[param.name] = param;
    updateSchema();
}

void Tool::setCallback(ToolCallback callback) {
    callback_ = callback;
}

ToolResult Tool::execute(const JsonObject& params) const {
    // Validate parameters
    if (!validateParameters(params)) {
        ToolResult result;
        result.success = false;
        result.content = "Invalid parameters";
        return result;
    }
    
    // No callback set
    if (!callback_) {
        ToolResult result;
        result.success = false;
        result.content = "No execution callback set for tool: " + name_;
        return result;
    }
    
    return callback_(params);
}

bool Tool::validateParameters(const JsonObject& params) const {
    // Check if all required parameters are present
    for (const auto& param_pair : parameters_) {
        const Parameter& param = param_pair.second;
        
        if (param.required) {
            if (params.find(param.name) == params.end()) {
                return false;
            }
        }
    }
    
    return true;
}

void Tool::updateSchema() {
    // Build tool schema for LLM function calling
    schema_["name"] = name_;
    schema_["description"] = description_;
    
    // Add parameters to schema
    JsonObject properties;
    JsonObject parameter_schema;
    std::vector<String> required_params;
    
    for (const auto& param_pair : parameters_) {
        const Parameter& param = param_pair.second;
        
        // Create parameter schema
        JsonObject param_schema;
        param_schema["type"] = param.type;
        param_schema["description"] = param.description;
        
        // Add default value if present
        if (param.default_value.has_value()) {
            param_schema["default"] = param.default_value.value();
        }
        
        // Add to properties
        properties[param.name] = param_schema;
        
        // Add to required list if needed
        if (param.required) {
            required_params.push_back(param.name);
        }
    }
    
    // Build final schema
    parameter_schema["type"] = "object";
    parameter_schema["properties"] = properties;
    parameter_schema["required"] = required_params;
    
    schema_["parameters"] = parameter_schema;
}

std::shared_ptr<Tool> createTool(
    const String& name,
    const String& description,
    const std::vector<Parameter>& parameters,
    ToolCallback callback
) {
    auto tool = std::make_shared<Tool>(name, description);
    
    // Add parameters
    for (const auto& param : parameters) {
        tool->addParameter(param);
    }
    
    // Set callback
    tool->setCallback(callback);
    
    return tool;
}

} // namespace agents 