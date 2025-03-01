#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/workflows/prompt_chaining_workflow.h>
#include <agents-cpp/config_loader.h>
#include <agents-cpp/logger.h>

#include <string>

using namespace agents;
using namespace agents::workflows;

int main() {
    // Initialize the logger
    Logger::init(Logger::Level::INFO);
    
    Logger::info("Simple Workflow Example");
    Logger::info("This is a minimal example to demonstrate the agents-cpp library.");
    Logger::info("It uses only the components that are actually implemented.");
    
    // Check if we have any API keys configured
    auto& config = ConfigLoader::getInstance();
    
    bool hasOpenAI = config.has("OPENAI_API_KEY");
    bool hasAnthropic = config.has("ANTHROPIC_API_KEY");
    bool hasGoogle = config.has("GOOGLE_API_KEY");
    
    Logger::info("\nAPI Key Configuration Status:");
    Logger::info("- OpenAI API Key: {}", hasOpenAI ? "Found ✓" : "Not found ✗");
    Logger::info("- Anthropic API Key: {}", hasAnthropic ? "Found ✓" : "Not found ✗");
    Logger::info("- Google API Key: {}", hasGoogle ? "Found ✓" : "Not found ✗");
    
    if (hasOpenAI || hasAnthropic || hasGoogle) {
        Logger::info("\nAPI keys found in configuration!");
        Logger::info("You can now run examples without providing keys on the command line.");
    } else {
        Logger::warn("\nNo API keys found in configuration.");
        Logger::warn("Please create a .env file or set environment variables.");
        Logger::warn("See README.md for instructions.");
    }
    
    // Create agent context
    auto context = std::make_shared<AgentContext>();
    
    // Create the prompt chain workflow
    PromptChainingWorkflow chain(context);
    
    // The workflow needs an LLM to work, but we're not connecting to any external API
    // This example just shows the structure and compilation
    
    Logger::info("\nWorkflow created successfully.");
    Logger::info("This example is just for demonstration purposes.");
    Logger::info("To run a real workflow, you would need to:");
    Logger::info("1. Set an LLM with an API key");
    Logger::info("2. Add steps to the workflow");
    Logger::info("3. Run the workflow with input");
    
    return 0;
} 