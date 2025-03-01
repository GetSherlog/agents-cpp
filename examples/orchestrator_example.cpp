#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/workflows/orchestrator_workers.h>
#include <agents-cpp/tools/tool_registry.h>

#include <iostream>
#include <string>

using namespace agents;

int main(int argc, char* argv[]) {
    // Get API key from environment or command line
    String api_key;
    const char* env_api_key = std::getenv("OPENAI_API_KEY");
    if (env_api_key != nullptr) {
        api_key = env_api_key;
    } else if (argc > 1) {
        api_key = argv[1];
    } else {
        std::cerr << "Please provide an API key as an argument or set the OPENAI_API_KEY environment variable." << std::endl;
        return 1;
    }

    // Create LLM
    auto llm = createLLM("openai", api_key, "gpt-4o-2024-05-13");
    
    // Configure LLM options
    LLMOptions options;
    options.temperature = 0.3;
    options.max_tokens = 2048;
    llm->setOptions(options);

    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Register tools
    context->registerTool(tools::createWebSearchTool());
    context->registerTool(tools::createWikipediaTool());
    
    // Create orchestrator-workers workflow
    workflows::OrchestratorWorkers orchestrator(context);
    
    // Set orchestrator prompt
    orchestrator.setOrchestratorPrompt(
        "You are a project manager that breaks down complex tasks into subtasks and assigns them to appropriate specialist workers. "
        "Analyze the user's request carefully, identify what specialists would be needed, and coordinate their work. "
        "Provide a detailed plan for completing the task using the available workers."
    );
    
    // Register workers
    orchestrator.registerWorker(
        "researcher",
        "Gathers factual information and data on specific topics",
        "You are a research specialist focused on gathering accurate, current, and relevant information. "
        "Your task is to find the most important facts, data, statistics, and context on the given topic. "
        "Cite sources when possible."
    );
    
    orchestrator.registerWorker(
        "analyst",
        "Analyzes information, identifies patterns, and draws insights",
        "You are an analytical specialist who excels at examining information critically. "
        "Your task is to identify patterns, trends, insights, and implications from the research. "
        "Focus on depth rather than breadth."
    );
    
    orchestrator.registerWorker(
        "writer",
        "Creates well-written, cohesive content from information and analysis",
        "You are a writing specialist who creates clear, engaging, and informative content. "
        "Your task is to synthesize information and analysis into a cohesive narrative. "
        "Focus on clarity, flow, and presentation."
    );
    
    orchestrator.registerWorker(
        "technical_expert",
        "Provides specialized technical knowledge on complex topics",
        "You are a technical specialist with deep expertise in technical domains. "
        "Your task is to provide accurate technical explanations, clarifications, and context. "
        "Make complex topics accessible without oversimplifying."
    );
    
    orchestrator.registerWorker(
        "critic",
        "Reviews content for accuracy, clarity, and completeness",
        "You are a critical reviewer who evaluates content objectively. "
        "Your task is to identify gaps, inconsistencies, errors, or areas for improvement. "
        "Provide constructive feedback rather than just criticism."
    );
    
    // Set custom synthesizer
    orchestrator.setSynthesizer([](const std::vector<JsonObject>& worker_results) -> JsonObject {
        JsonObject final_result;
        String combined_output = "# Comprehensive Report\n\n";
        
        // Extract each worker's contribution
        for (const auto& result : worker_results) {
            if (result.contains("worker_name") && result.contains("output")) {
                String worker_name = result["worker_name"].get<String>();
                String output = result["output"].get<String>();
                
                combined_output += "## " + worker_name + "'s Contribution\n\n";
                combined_output += output + "\n\n";
            }
        }
        
        // Add a summary section
        combined_output += "## Summary\n\n";
        combined_output += "This report combines the work of multiple specialists to provide a comprehensive response to the original query.";
        
        final_result["answer"] = combined_output;
        return final_result;
    });
    
    // Process user inputs until exit
    std::cout << "Enter complex tasks (or 'exit' to quit):" << std::endl;
    String user_input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, user_input);
        
        if (user_input == "exit") {
            break;
        }
        
        if (user_input.empty()) {
            continue;
        }
        
        try {
            std::cout << "Orchestrating workers..." << std::endl;
            
            // Run the orchestrator-workers workflow
            JsonObject result = orchestrator.run(user_input);
            
            // Display the result
            std::cout << "\nFinal Result:\n" << result["answer"].get<String>() << std::endl;
            std::cout << "--------------------------------------" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
} 