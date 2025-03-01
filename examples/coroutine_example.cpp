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

// Example coroutine that performs a multi-step task using tools
Task<JsonObject> performResearchTask(std::shared_ptr<AgentContext> context, const String& topic) {
    Logger::info("Starting research on topic: {}", topic);
    
    // Perform a search to get initial information
    auto search_tool = tools::createWebSearchTool();
    auto search_result = co_await context->executeToolAsync("web_search", {{"query", topic}});
    
    // Extract key points from the search result
    Logger::info("Extracting key points from search results...");
    auto extract_prompt = "Extract the key points from this search result about " + topic + ":\n\n" + search_result.content;
    auto extract_response = co_await context->chatAsync(extract_prompt);
    
    // Get more detailed information from Wikipedia
    Logger::info("Getting more information from Wikipedia...");
    auto wiki_tool = tools::createWikipediaTool();
    auto wiki_result = co_await context->executeToolAsync("wikipedia", {{"query", topic}});
    
    // Combine and summarize all information
    Logger::info("Summarizing all information...");
    auto summarize_prompt = "Synthesize and summarize the following information about " + topic + ":\n\n";
    summarize_prompt += "Key Points:\n" + extract_response.content + "\n\n";
    summarize_prompt += "Wikipedia Information:\n" + wiki_result.content;
    
    auto summary_response = co_await context->chatAsync(summarize_prompt);
    
    // Provide a final answer
    JsonObject result = {
        {"topic", topic},
        {"summary", summary_response.content},
        {"search_results", search_result.content},
        {"wiki_results", wiki_result.content}
    };
    
    Logger::info("Research complete!");
    co_return result;
}

// Example coroutine that generates content in parallel
Task<JsonObject> generateContentInParallel(std::shared_ptr<AgentContext> context, const String& topic) {
    Logger::info("Generating content for topic: {}", topic);
    
    // Create a prompt for the introduction
    String intro_prompt = "Write an introduction paragraph for an article about " + topic + ".";
    
    // Create a prompt for the main body
    String body_prompt = "Write three key points about " + topic + " with detailed explanations.";
    
    // Create a prompt for the conclusion
    String conclusion_prompt = "Write a conclusion paragraph for an article about " + topic + ".";
    
    // Launch all tasks concurrently
    auto intro_task = context->chatAsync(intro_prompt);
    auto body_task = context->chatAsync(body_prompt);
    auto conclusion_task = context->chatAsync(conclusion_prompt);
    
    // Await all tasks (this happens in parallel)
    auto intro_response = co_await std::move(intro_task);
    auto body_response = co_await std::move(body_task);
    auto conclusion_response = co_await std::move(conclusion_task);
    
    // Now we have all the pieces, combine them
    String article = intro_response.content + "\n\n" + body_response.content + "\n\n" + conclusion_response.content;
    
    // Create a prompt for a title
    String title_prompt = "Create a catchy title for this article:\n\n" + article;
    auto title_response = co_await context->chatAsync(title_prompt);
    
    // Return the complete article
    JsonObject result = {
        {"title", title_response.content},
        {"introduction", intro_response.content},
        {"body", body_response.content},
        {"conclusion", conclusion_response.content},
        {"full_article", article}
    };
    
    Logger::info("Content generation complete!");
    co_return result;
}

// Example showing streaming text with coroutines
Task<void> streamText(std::shared_ptr<AgentContext> context, const String& prompt) {
    Logger::info("Streaming response for prompt: {}", prompt);
    
    // Get a streaming generator
    auto generator = context->streamChatAsync(prompt);
    
    std::cout << "Response: ";
    
    // Process each chunk as it arrives
    auto it = co_await generator.begin();
    while (it != generator.end()) {
        String chunk = *it;
        std::cout << chunk << std::flush;
        co_await ++it;
    }
    
    std::cout << std::endl;
    Logger::info("Streaming complete!");
    co_return;
}

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
    options.temperature = 0.7;
    options.max_tokens = 2048;
    llm->setOptions(options);

    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Register tools
    context->registerTool(tools::createWebSearchTool());
    context->registerTool(tools::createWikipediaTool());
    
    // Menu-driven example to demonstrate various coroutines
    while (true) {
        std::cout << "\n========== COROUTINE EXAMPLES ==========\n";
        std::cout << "1. Run autonomous agent with coroutines\n";
        std::cout << "2. Perform research with parallel tool use\n";
        std::cout << "3. Generate content in parallel\n";
        std::cout << "4. Stream text example\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear the newline
        
        if (choice == 5) {
            break;
        }
        
        // Get topic from user
        std::string topic;
        if (choice >= 1 && choice <= 4) {
            std::cout << "Enter a topic: ";
            std::getline(std::cin, topic);
        }
        
        try {
            switch (choice) {
                case 1: {
                    // Example 1: Autonomous agent with coroutines
                    Logger::info("Running autonomous agent with coroutines");
                    
                    // Create and configure agent
                    AutonomousAgent agent(context);
                    agent.setSystemPrompt("You are a helpful assistant that can use tools to perform tasks.");
                    agent.setPlanningStrategy(AutonomousAgent::PlanningStrategy::REACT);
                    agent.setStepCallback(stepCallback);
                    
                    // Configure options
                    AutonomousAgent::Options agent_options;
                    agent_options.max_iterations = 10;
                    agent_options.human_feedback_enabled = true;
                    agent_options.human_in_the_loop = humanApproval;
                    agent.setOptions(agent_options);
                    
                    // Initialize
                    agent.init();
                    
                    // Run the agent using coroutines
                    auto result = blockingWait(agent.runCoro(topic));
                    
                    // Display result
                    if (result.contains("answer")) {
                        Logger::info("\nFinal Answer: {}", result["answer"].get<String>());
                    } else {
                        Logger::info("\nResult: {}", result.dump(2));
                    }
                    break;
                }
                case 2: {
                    // Example 2: Research task with parallel tool use
                    Logger::info("Performing research with coroutines");
                    auto result = blockingWait(performResearchTask(context, topic));
                    Logger::info("\nResearch Summary: {}", result["summary"].get<String>());
                    break;
                }
                case 3: {
                    // Example 3: Generate content in parallel
                    Logger::info("Generating content in parallel");
                    auto result = blockingWait(generateContentInParallel(context, topic));
                    Logger::info("\nTitle: {}", result["title"].get<String>());
                    Logger::info("\nFull Article:\n{}", result["full_article"].get<String>());
                    break;
                }
                case 4: {
                    // Example 4: Stream text
                    Logger::info("Streaming text example");
                    blockingWait(streamText(context, "Write a short story about " + topic));
                    break;
                }
                default:
                    Logger::error("Invalid choice");
            }
        } catch (const std::exception& e) {
            Logger::error("Error: {}", e.what());
        }
    }
    
    return 0;
} 