#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/workflows/routing.h>
#include <agents-cpp/tools/tool_registry.h>
#include <agents-cpp/logger.h>

#include <iostream>
#include <string>

using namespace agents;

int main(int argc, char* argv[]) {
    // Initialize the logger
    Logger::init(Logger::Level::INFO);
    
    // Get API key from environment or command line
    String api_key;
    const char* env_api_key = std::getenv("OPENAI_API_KEY");
    if (env_api_key != nullptr) {
        api_key = env_api_key;
    } else if (argc > 1) {
        api_key = argv[1];
    } else {
        Logger::error("Please provide an API key as an argument or set the OPENAI_API_KEY environment variable.");
        return 1;
    }

    // Create LLM
    auto llm = createLLM("openai", api_key, "gpt-4o-2024-05-13");
    
    // Configure LLM options
    LLMOptions options;
    options.temperature = 0.2;
    options.max_tokens = 2048;
    llm->setOptions(options);

    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Register some tools
    context->registerTool(tools::createWebSearchTool());
    context->registerTool(tools::createWikipediaTool());
    
    // Create routing workflow
    workflows::Routing router(context);
    
    // Set router prompt
    router.setRouterPrompt(
        "You are a routing assistant that examines user queries and classifies them into appropriate categories. "
        "Determine the most suitable category for handling the user's query based on the available routes."
    );
    
    // Add routes for different query types
    router.addRoute(
        "factual_query",
        "Questions about facts, events, statistics, or general knowledge",
        [context](const String& input, const JsonObject& routing_info) -> JsonObject {
            Logger::info("Handling factual query: {}", input);
            
            auto wiki_tool = tools::createWikipediaTool();
            ToolResult result = wiki_tool->execute({{"query", input}});
            
            JsonObject response;
            response["answer"] = "Based on research: " + result.content;
            return response;
        }
    );
    
    router.addRoute(
        "opinion_query",
        "Questions seeking opinions, evaluations, or judgments on topics",
        [context](const String& input, const JsonObject& routing_info) -> JsonObject {
            Logger::info("Handling opinion query: {}", input);
            
            // Create specific context for opinion handling
            auto opinion_context = std::make_shared<AgentContext>(*context);
            opinion_context->setSystemPrompt(
                "You are a balanced and thoughtful assistant that provides nuanced perspectives on complex topics. "
                "Consider multiple viewpoints and provide balanced opinions."
            );
            
            // Get response from LLM
            LLMResponse llm_response = opinion_context->getLLM()->complete(input);
            String response = llm_response.content;
            
            JsonObject result;
            result["answer"] = "Opinion analysis: " + response;
            return result;
        }
    );
    
    router.addRoute(
        "technical_query",
        "Questions about technical topics, programming, or specialized domains",
        [context](const String& input, const JsonObject& routing_info) -> JsonObject {
            Logger::info("Handling technical query: {}", input);
            
            // Create specific context for technical handling
            auto technical_context = std::make_shared<AgentContext>(*context);
            technical_context->setSystemPrompt(
                "You are a technical expert assistant that provides accurate and detailed information on technical topics. "
                "Focus on clarity, precision, and correctness."
            );
            
            // Get response from LLM
            LLMResponse llm_response = technical_context->getLLM()->complete(input);
            String response = llm_response.content;
            
            JsonObject result;
            result["answer"] = "Technical explanation: " + response;
            return result;
        }
    );
    
    // Set default route
    router.setDefaultRoute([context](const String& input, const JsonObject& routing_info) -> JsonObject {
        Logger::info("Handling with default route: {}", input);
        
        // Get response from LLM
        LLMResponse llm_response = context->getLLM()->complete(input);
        String response = llm_response.content;
        
        JsonObject result;
        result["answer"] = "General response: " + response;
        return result;
    });

    // Process user inputs until exit
    Logger::info("Enter queries (or 'exit' to quit):");
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
            // Run the routing workflow
            JsonObject result = router.run(user_input);
            
            // Display the result
            Logger::info("\nResponse: {}", result["answer"].get<String>());
            Logger::info("--------------------------------------");
        } catch (const std::exception& e) {
            Logger::error("Error: {}", e.what());
        }
    }
    
    return 0;
} 