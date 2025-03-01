#include <agents-cpp/types.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/workflows/parallelization.h>

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

    // Ask the user which parallelization mode to use
    std::cout << "Select parallelization mode (1 for SECTIONING, 2 for VOTING): ";
    int mode_choice;
    std::cin >> mode_choice;
    std::cin.ignore(); // Clear the newline

    workflows::Parallelization::Mode mode = (mode_choice == 2) ?
        workflows::Parallelization::Mode::VOTING :
        workflows::Parallelization::Mode::SECTIONING;

    std::cout << "Using mode: " << (mode == workflows::Parallelization::Mode::VOTING ? "VOTING" : "SECTIONING") << std::endl;

    // Create LLM
    auto llm = createLLM("openai", api_key, "gpt-4o-2024-05-13");
    
    // Configure LLM options
    LLMOptions options;
    options.temperature = 0.7; // Higher temperature for diversity
    options.max_tokens = 2048;
    llm->setOptions(options);

    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Create parallelization workflow
    workflows::Parallelization parallel(context, mode);
    
    if (mode == workflows::Parallelization::Mode::SECTIONING) {
        // Add tasks for sectioning mode
        parallel.addTask(
            "research",
            "You are a research assistant focused on gathering factual information. "
            "Present only verified facts and data, citing sources when possible.",
            [](const String& input) -> String {
                return "Research task: " + input + 
                      "\nFocus on finding the most relevant facts and data points about this topic.";
            },
            [](const String& output) -> JsonObject {
                JsonObject result;
                result["research"] = output;
                return result;
            }
        );
        
        parallel.addTask(
            "analysis",
            "You are an analytical assistant that excels at critical thinking. "
            "Analyze information objectively, identifying patterns, trends, and insights.",
            [](const String& input) -> String {
                return "Analysis task: " + input + 
                      "\nProvide a thoughtful analysis, including implications and significance.";
            },
            [](const String& output) -> JsonObject {
                JsonObject result;
                result["analysis"] = output;
                return result;
            }
        );
        
        parallel.addTask(
            "recommendations",
            "You are a recommendation assistant that provides practical advice. "
            "Suggest actionable steps based on the query.",
            [](const String& input) -> String {
                return "Recommendation task: " + input + 
                      "\nProvide concrete, actionable recommendations related to this topic.";
            },
            [](const String& output) -> JsonObject {
                JsonObject result;
                result["recommendations"] = output;
                return result;
            }
        );
        
        // Set a custom aggregator for sectioning mode
        parallel.setAggregator([](const std::vector<JsonObject>& results) -> JsonObject {
            JsonObject combined;
            String research, analysis, recommendations;
            
            for (const auto& result : results) {
                if (result.contains("research")) {
                    research = result["research"].get<String>();
                } else if (result.contains("analysis")) {
                    analysis = result["analysis"].get<String>();
                } else if (result.contains("recommendations")) {
                    recommendations = result["recommendations"].get<String>();
                }
            }
            
            combined["answer"] = 
                "# Research Findings\n\n" + research + 
                "\n\n# Analysis\n\n" + analysis +
                "\n\n# Recommendations\n\n" + recommendations;
            
            return combined;
        });
    } else {
        // VOTING mode - multiple identical tasks with different parameters
        for (int i = 0; i < 5; i++) {
            parallel.addTask(
                "agent_" + std::to_string(i+1),
                "You are assistant " + std::to_string(i+1) + ". "
                "Provide your best answer to the query, thinking independently.",
                [i](const String& input) -> String {
                    return "Task for agent " + std::to_string(i+1) + ": " + input;
                },
                [](const String& output) -> JsonObject {
                    JsonObject result;
                    result["response"] = output;
                    return result;
                }
            );
        }
        
        // Using default voting aggregator
    }

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
            std::cout << "Running parallel tasks..." << std::endl;
            
            // Run the parallelization workflow
            JsonObject result = parallel.run(user_input);
            
            // Display the result
            std::cout << "\nResult:\n" << result["answer"].get<String>() << std::endl;
            std::cout << "--------------------------------------" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
} 