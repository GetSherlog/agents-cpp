#include <agents-cpp/workflows/prompt_chaining_workflow.h>
#include <agents-cpp/workflows/parallelization_workflow.h>
#include <agents-cpp/agent_context.h>
#include <agents-cpp/agents/actor_agent.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/tool.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>

// Replace blanket using directive with specific ones
using agents::String;
using agents::JsonObject;
using agents::ToolResult;
using agents::Message;
using agents::Tool;
using agents::LLMOptions;
using agents::LLMResponse;
using agents::LLMInterface;
using agents::Agent;
using agents::AgentContext;
using agents::createLLM;
using agents::createTool;
using agents::agents::ActorAgent;
using agents::workflows::PromptChainingWorkflow;
using agents::workflows::ParallelizationWorkflow;

// Example tool: Calculator
ToolResult calculatorTool(const JsonObject& params) {
    try {
        if (params.contains("expression")) {
            String expr = params["expression"];
            // Very simple calculator for demo purposes
            // In a real-world scenario, you'd use a proper expression evaluator
            double result = 0.0;
            
            // Just a dummy implementation for demo purposes
            if (expr == "1+1") {
                result = 2.0;
            } else if (expr == "2*3") {
                result = 6.0;
            } else {
                // Default response
                result = 42.0;
            }
            
            return {
                true,
                "Calculated result: " + std::to_string(result),
                {{"result", result}}
            };
        } else {
            return {
                false,
                "Missing expression parameter",
                {{"error", "Missing expression parameter"}}
            };
        }
    } catch (const std::exception& e) {
        return {
            false,
            "Error calculating result: " + String(e.what()),
            {{"error", e.what()}}
        };
    }
}

// Example tool: Weather
ToolResult weatherTool(const JsonObject& params) {
    try {
        if (params.contains("location")) {
            String location = params["location"];
            
            // Just a dummy implementation for demo purposes
            String weather = "sunny";
            double temperature = 22.0;
            
            return {
                true,
                "Weather in " + location + ": " + weather + ", " + std::to_string(temperature) + "°C",
                {
                    {"location", location},
                    {"weather", weather},
                    {"temperature", temperature}
                }
            };
        } else {
            return {
                false,
                "Missing location parameter",
                {{"error", "Missing location parameter"}}
            };
        }
    } catch (const std::exception& e) {
        return {
            false,
            "Error getting weather: " + String(e.what()),
            {{"error", e.what()}}
        };
    }
}

int main(int argc, char* argv[]) {
    // Set up logging
    spdlog::set_level(spdlog::level::debug);
    
    // Check for API key
    const char* api_key = std::getenv("ANTHROPIC_API_KEY");
    if (!api_key) {
        std::cerr << "Error: ANTHROPIC_API_KEY environment variable not set\n";
        return 1;
    }
    
    try {
        // Create LLM interface
        auto llm = createLLM("anthropic", api_key, "claude-3-opus-20240229");
        
        // Set up options
        LLMOptions options;
        options.temperature = 0.7;
        options.max_tokens = 1000;
        llm->setOptions(options);
        
        // Create tools
        auto calculator = createTool(
            "calculator", 
            "Calculate mathematical expressions", 
            {
                {"expression", "string", "The mathematical expression to calculate", true}
            },
            calculatorTool
        );
        
        auto weather = createTool(
            "weather", 
            "Get weather information for a location", 
            {
                {"location", "string", "The location to get weather for", true}
            },
            weatherTool
        );
        
        // Create agent context
        auto context = std::make_shared<AgentContext>();
        context->setLLM(llm);
        context->registerTool(calculator);
        context->registerTool(weather);
        
        // Example 1: Using the prompt chaining workflow
        std::cout << "\n=== Example 1: Prompt Chaining Workflow ===\n\n";
        
        auto chaining_workflow = std::make_shared<PromptChainingWorkflow>(llm);
        
        // Add steps to the workflow
        chaining_workflow->addStep(
            "brainstorm", 
            "Brainstorm 3 creative ideas for a short story about space exploration. Return them as a JSON array."
        );
        
        chaining_workflow->addStep(
            "select", 
            "From these ideas, select the most interesting one and explain why you chose it:\n{{response}}"
        );
        
        chaining_workflow->addStep(
            "outline", 
            "Create a brief outline for a story based on this idea:\n{{response}}"
        );
        
        // Initialize and execute the workflow
        chaining_workflow->init();
        auto result = chaining_workflow->execute({});
        
        std::cout << "Prompt chaining result: " << result.dump(2) << "\n\n";
        
        // Example 2: Using the parallelization workflow
        std::cout << "\n=== Example 2: Parallelization Workflow (Sectioning) ===\n\n";
        
        auto parallel_workflow = std::make_shared<ParallelizationWorkflow>(
            llm, ParallelizationWorkflow::Strategy::SECTIONING
        );
        
        // Add tasks to the workflow
        parallel_workflow->addTask(
            "characters", 
            "Create 2 interesting characters for a sci-fi story set on Mars."
        );
        
        parallel_workflow->addTask(
            "setting", 
            "Describe the environment and setting of a Mars colony in the year 2150."
        );
        
        parallel_workflow->addTask(
            "plot", 
            "Create a plot outline for a mystery story set on Mars."
        );
        
        // Initialize and execute the workflow
        parallel_workflow->init();
        result = parallel_workflow->execute({});
        
        std::cout << "Parallelization result: " << result.dump(2) << "\n\n";
        
        // Example 3: Using the actor agent
        std::cout << "\n=== Example 3: Actor Agent with Tools ===\n\n";
        
        auto agent = std::make_shared<ActorAgent>(context);
        
        // Set system prompt
        agent->setSystemPrompt(
            "You are a helpful assistant that can answer questions and use tools to get information. "
            "When using tools, make sure to include all necessary parameters."
        );
        
        // Set options
        Agent::Options agent_options;
        agent_options.max_iterations = 5;
        agent_options.human_feedback_enabled = false;
        agent->setOptions(agent_options);
        
        // Register status callback
        agent->setStatusCallback([](const String& status) {
            std::cout << "Agent status: " << status << "\n";
        });
        
        // Initialize and run the agent
        agent->init();
        
        // Run the agent with multiple tasks
        std::vector<String> tasks = {
            "What is 1+1?",
            "What's the weather like in New York?",
            "Tell me a short story about a robot learning to feel emotions."
        };
        
        for (const auto& task : tasks) {
            std::cout << "\nTask: " << task << "\n";
            result = agent->run(task);
            std::cout << "Result: " << result.dump(2) << "\n";
            
            // Small delay between tasks
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
} 