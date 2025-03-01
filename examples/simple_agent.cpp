#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/agents/autonomous_agent.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/tools/tool_registry.h>
#include <agents-cpp/logger.h>
#include <agents-cpp/coroutine_utils.h>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

using namespace agents;

// Simple callback to print agent steps
void stepCallback(const AutonomousAgent::Step& step) {
    Logger::info("Step: {}", step.description);
    Logger::info("Status: {}", step.status);
    if (step.success) {
        Logger::info("Result: {}", step.result.dump(2));
    } else {
        Logger::error("Failed!");
    }
    Logger::info("--------------------------------------");
}

// Simple callback for human-in-the-loop
bool humanApproval(const String& message, const JsonObject& context) {
    Logger::info("\n{}", message);
    Logger::info("Context: {}", context.dump(2));
    Logger::info("Approve this step? (y/n): ");
    char response;
    std::cin >> response;
    return (response == 'y' || response == 'Y');
}

// Main agent task
Task<int> runAgentApp() {
    // Initialize the logger
    Logger::init(Logger::Level::INFO);
    
    // Get API key from environment or command line
    String api_key;
    const char* env_api_key = std::getenv("OPENAI_API_KEY");
    if (env_api_key != nullptr) {
        api_key = env_api_key;
    } else {
        Logger::error("Please set the OPENAI_API_KEY environment variable.");
        co_return 1;
    }
    
    // Create the context
    auto context = std::make_shared<AgentContext>();
    
    // Configure the LLM
    auto llm = std::make_shared<llms::OpenAILLM>();
    llm->setApiKey(api_key);
    llm->setModel("gpt-4o");
    llm->setTemperature(0.7);
    context->setLLM(llm);
    
    // Register some tools
    context->registerTool(tools::createWebSearchTool());
    context->registerTool(tools::createWikipediaTool());
    
    // Create the agent
    AutonomousAgent agent(context);
    
    // Configure the agent
    agent.setSystemPrompt(
        "You are a research assistant that helps users find information and answer questions. "
        "Use the tools available to you to gather information and provide comprehensive answers. "
        "When searching for information, try multiple queries if necessary."
    );
    
    // Set the planning strategy
    agent.setPlanningStrategy(AutonomousAgent::PlanningStrategy::REACT);
    
    // Set up options
    AutonomousAgent::Options agent_options;
    agent_options.max_iterations = 15;
    agent_options.human_feedback_enabled = true;
    agent_options.human_in_the_loop = humanApproval;
    agent.setOptions(agent_options);
    
    // Set up callbacks
    agent.setStepCallback(stepCallback);
    agent.setStatusCallback([](const String& status) {
        Logger::info("Agent status: {}", status);
    });
    
    // Initialize the agent
    agent.init();
    
    // Get user input
    Logger::info("Enter a question or task for the agent (or 'exit' to quit):");
    String user_input;
    while (true) {
        Logger::info("> ");
        std::getline(std::cin, user_input);
        
        if (user_input == "exit") {
            break;
        }
        
        if (user_input.empty()) {
            continue;
        }
        
        try {
            // Run the agent with coroutines
            JsonObject result = co_await agent.run(user_input);
            
            // Display the final result
            Logger::info("\nFinal Result:\n{}", result["answer"].get<String>());
        } catch (const std::exception& e) {
            Logger::error("Error: {}", e.what());
        }
    }
    
    co_return 0;
}

// Entry point
int main() {
    // Use folly::coro::blockingWait to execute the coroutine
    return folly::coro::blockingWait(runAgentApp());
} 