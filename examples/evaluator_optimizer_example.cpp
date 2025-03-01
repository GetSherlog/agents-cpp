#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/workflows/evaluator_optimizer.h>

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
    options.temperature = 0.4;
    options.max_tokens = 2048;
    llm->setOptions(options);

    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Create evaluator-optimizer workflow
    workflows::EvaluatorOptimizer workflow(context);
    
    // Set optimizer prompt
    workflow.setOptimizerPrompt(
        "You are an optimizer assistant that produces high-quality responses to user queries. "
        "Your task is to generate the best possible response to the user's query. "
        "If you receive feedback, use it to improve your response."
    );
    
    // Set evaluator prompt
    workflow.setEvaluatorPrompt(
        "You are an evaluator assistant that critically assesses the quality of responses. "
        "Your task is to provide honest, detailed feedback on the response to help improve it. "
        "Focus on specific areas where the response could be enhanced."
    );
    
    // Set evaluation criteria
    workflow.setEvaluationCriteria({
        "Accuracy: Is the information provided accurate and factually correct?",
        "Completeness: Does the response address all aspects of the query?",
        "Clarity: Is the response clear, well-organized, and easy to understand?",
        "Relevance: Is the response directly relevant to the query?",
        "Actionability: Does the response provide practical, actionable information where appropriate?"
    });
    
    // Set maximum iterations
    workflow.setMaxIterations(3);
    
    // Set minimum acceptable score
    workflow.setMinimumAcceptableScore(0.85);
    
    // Set custom evaluator and optimizer functions (optional)
    workflow.setEvaluator([](const String& input, const String& output) -> JsonObject {
        // This is a custom evaluator function that would normally implement
        // specialized evaluation logic, but here we'll let the default LLM-based
        // evaluator do the work by returning an empty result
        return JsonObject();
    });
    
    workflow.setOptimizer([](const String& input, const JsonObject& feedback) -> String {
        // This is a custom optimizer function that would normally implement
        // specialized optimization logic, but here we'll let the default LLM-based
        // optimizer do the work by returning an empty string
        return "";
    });
    
    // Process user inputs until exit
    std::cout << "Enter queries (or 'exit' to quit):" << std::endl;
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
            std::cout << "Starting evaluator-optimizer workflow..." << std::endl;
            
            // Run the workflow
            JsonObject result = workflow.run(user_input);
            
            // Display the final result
            std::cout << "\nFinal Response:" << std::endl;
            std::cout << result["final_response"].get<String>() << std::endl;
            
            // Display evaluation information
            std::cout << "\nEvaluation Information:" << std::endl;
            std::cout << "Iterations: " << result["iterations"].get<int>() << std::endl;
            std::cout << "Final Score: " << result["final_score"].get<double>() << std::endl;
            
            if (result.contains("evaluations")) {
                std::cout << "\nEvaluation History:" << std::endl;
                for (const auto& eval : result["evaluations"]) {
                    std::cout << "Iteration " << eval["iteration"].get<int>() << ": Score = " 
                              << eval["score"].get<double>() << std::endl;
                    std::cout << "Feedback: " << eval["feedback"].get<String>() << std::endl;
                    std::cout << "----------" << std::endl;
                }
            }
            
            std::cout << "--------------------------------------" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
} 