#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/workflows/prompt_chaining_workflow.h>
#include <agents-cpp/config_loader.h>
#include <agents-cpp/logger.h>

#include <iostream>
#include <string>

using namespace agents;
using namespace agents::workflows;

int main(int argc, char* argv[]) {
    // Initialize the logger
    Logger::init(Logger::Level::INFO);
    
    // Get API key from .env, environment, or command line
    String api_key;
    auto& config = ConfigLoader::getInstance();
    
    // Try to get API key from config or environment
    api_key = config.get("ANTHROPIC_API_KEY", "");
    
    // If not found, check command line
    if (api_key.empty() && argc > 1) {
        api_key = argv[1];
    }
    
    // Still not found, show error and exit
    if (api_key.empty()) {
        Logger::error("API key not found. Please:");
        Logger::error("1. Create a .env file with ANTHROPIC_API_KEY=your_key, or");
        Logger::error("2. Set the ANTHROPIC_API_KEY environment variable, or");
        Logger::error("3. Provide an API key as a command line argument");
        return 1;
    }

    // Create LLM
    auto llm = createLLM("anthropic", api_key, "claude-3-5-sonnet-20240620");
    
    // Configure LLM options
    LLMOptions options;
    options.temperature = 0.3;
    llm->setOptions(options);

    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Create the prompt chain workflow
    PromptChainingWorkflow chain(context);
    
    // Add a step to generate a document outline
    chain.addStep(
        "outline", 
        "You are an expert document planner. Your task is to create a detailed outline for a document on the provided topic. "
        "The outline should include main sections and subsections. Be comprehensive but focused."
    );
    
    // Add a step with a gate function to validate the outline
    chain.addStep(
        "validate_outline",
        "You are a document validator. Your task is to evaluate an outline and determine if it's comprehensive and well-structured. "
        "Check if it covers all important aspects of the topic and has a logical flow.",
        // Validator: check if the outline is approved
        [](const JsonObject& result) -> bool {
            String response = result["response"].get<String>();
            return response.find("approved") != String::npos || 
                   response.find("looks good") != String::npos ||
                   response.find("comprehensive") != String::npos;
        }
    );
    
    // Add a step to write the document based on the outline
    chain.addStep(
        "write_document",
        "You are an expert content writer. Your task is to write a comprehensive document following the provided outline. "
        "Make sure to cover each section in detail and maintain a professional tone."
    );
    
    // Add a step to proofread and improve the document
    chain.addStep(
        "proofread",
        "You are a professional editor. Your task is to proofread and improve the provided document. "
        "Fix any grammatical errors, improve clarity and flow, and ensure consistency."
    );
    
    // Set a callback for intermediate steps
    chain.setStepCallback([](const String& step_name, const JsonObject& result) {
        Logger::info("Completed step: {}", step_name);
        Logger::info("--------------------------------------");
    });
    
    // Get user input
    Logger::info("Enter a topic for document generation:");
    String user_input;
    std::getline(std::cin, user_input);
    
    try {
        // Run the chain
        JsonObject result = chain.run(user_input);
        
        // Display the final document
        Logger::info("\nFinal Document:\n{}", result["proofread"]["response"].get<String>());
        
        // Also show the original outline
        Logger::info("\nOriginal Outline:\n{}", result["outline"]["response"].get<String>());
    } catch (const std::exception& e) {
        Logger::error("Error: {}", e.what());
    }
    
    return 0;
} 