# Agents-CPP: High Performance C++ Framework for LLM Agents

A high-performance, low-memory C++ implementation of agentic systems based on Anthropic's article ["Building effective agents"](https://www.anthropic.com/research/building-effective-agents).

## Features

- **Modular Design**: Composable components for building various agent architectures
- **Multiple LLM Providers**: Support for OpenAI, Anthropic, Google, and Ollama
- **High Performance**: Optimized for efficiency and low memory usage
- **Workflow Patterns**: Implementation of recommended workflow patterns
  - Prompt Chaining
  - Routing
  - Parallelization
  - Orchestrator-Workers
  - Evaluator-Optimizer
- **Autonomous Agents**: Support for fully autonomous agents with various planning strategies
- **Extensible Tools**: Flexible tool system with prebuilt examples

## Configuration

You can configure API keys and other settings in three ways:

1. Using a `.env` file:
   ```bash
   # Copy the template
   cp .env.template .env
   
   # Edit the file with your API keys
   nano .env  # or use any editor
   ```

2. Using environment variables:
   ```bash
   export OPENAI_API_KEY=your_api_key_here
   export ANTHROPIC_API_KEY=your_api_key_here
   ```

3. Passing API keys as command-line arguments (not recommended for production):
   ```bash
   ./bin/examples/simple_agent your_api_key_here
   ```

The framework will check for API keys in the following order:
1. `.env` file
2. Environment variables
3. Command-line arguments

## Requirements

- C++17 compatible compiler
- CMake 3.14+
- libcurl
- Dependencies (automatically fetched via CMake):
  - nlohmann/json
  - CPR (C++ Requests)
  - spdlog

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Here's a simple example of creating and running an autonomous agent:

```cpp
#include <agents-cpp/agent_context.h>
#include <agents-cpp/agents/autonomous_agent.h>
#include <agents-cpp/llm_interface.h>
#include <agents-cpp/tools/tool_registry.h>

using namespace agents;

int main() {
    // Create LLM
    auto llm = createLLM("anthropic", "your_api_key_here", "claude-3-5-sonnet-20240620");
    
    // Create agent context
    auto context = std::make_shared<AgentContext>();
    context->setLLM(llm);
    
    // Register tools
    context->registerTool(tools::createWebSearchTool());
    
    // Create the agent
    AutonomousAgent agent(context);
    agent.setPlanningStrategy(AutonomousAgent::PlanningStrategy::REACT);
    
    // Run the agent
    JsonObject result = agent.run("Research the latest developments in quantum computing");
    
    // Access the result
    std::cout << result["answer"].get<String>() << std::endl;
    
    return 0;
}
```

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed:
- A C++17 compatible compiler (GCC 8+, Clang 7+, or MSVC 2019+)
- CMake 3.14 or higher
- libcurl with development headers
- Git

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/agents-cpp.git
   cd agents-cpp
   ```

2. Create a build directory and compile the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j4  # Adjust the number based on your CPU cores
   ```

3. Obtain API keys:
   - For OpenAI models: Get an API key from [OpenAI's platform](https://platform.openai.com/api-keys)
   - For Anthropic models: Get an API key from [Anthropic's console](https://console.anthropic.com/account/keys)
   - For Google models: Get an API key from [Google AI Studio](https://ai.google.dev/)

### Running Your First Example

The simplest way to start is with the `simple_agent` example, which creates a basic autonomous agent that can use tools to answer questions:

1. From the build directory, run the example:
   ```bash
   ./bin/examples/simple_agent your_api_key_here
   ```
   
   Alternatively, you can set your API key as an environment variable:
   ```bash
   export OPENAI_API_KEY=your_api_key_here
   ./bin/examples/simple_agent
   ```

2. Once running, you'll be prompted to enter a question or task. For example:
   ```
   Enter a question or task for the agent (or 'exit' to quit):
   > What's the current status of quantum computing research?
   ```

3. The agent will:
   - Break down the task into steps
   - Use tools (like web search) to gather information
   - Ask for your approval before proceeding with certain steps (if human-in-the-loop is enabled)
   - Provide a comprehensive answer

4. Example output:
   ```
   Step: Planning how to approach the question
   Status: Completed
   Result: {
     "plan": "1. Search for recent quantum computing research developments..."
   }
   --------------------------------------
   Step: Searching for information on quantum computing research
   Status: Waiting for approval
   Context: {"search_query": "current status quantum computing research 2024"}
   Approve this step? (y/n): y
   ...
   ```

### Configuring the Example

You can modify examples/simple_agent.cpp to explore different configurations:

- Change the LLM provider:
  ```cpp
  // For Anthropic Claude
  auto llm = createLLM("anthropic", api_key, "claude-3-5-sonnet-20240620");
  
  // For Google Gemini
  auto llm = createLLM("google", api_key, "gemini-pro");
  ```

- Add different tools:
  ```cpp
  // Add more built-in tools
  context->registerTool(tools::createCalculatorTool());
  context->registerTool(tools::createPythonCodeExecutionTool());
  ```

- Change the planning strategy:
  ```cpp
  // Use ReAct planning (reasoning + acting)
  agent.setPlanningStrategy(AutonomousAgent::PlanningStrategy::REACT);
  
  // Or use CoT planning (chain-of-thought)
  agent.setPlanningStrategy(AutonomousAgent::PlanningStrategy::COT);
  ```

## Examples

The repository includes several examples demonstrating different workflow patterns:

- `simple_agent`: Basic autonomous agent example
- `prompt_chain_example`: Demonstrates prompt chaining workflow
- `routing_example`: Shows how to implement routing
- `parallel_example`: Parallel execution of tasks
- `orchestrator_example`: Orchestrator-workers pattern
- `evaluator_optimizer_example`: Demonstrates the evaluator-optimizer workflow
- `autonomous_agent_example`: Full-featured autonomous agent

Run examples from the build directory:

```bash
./bin/examples/simple_agent your_api_key_here
```

## Project Structure

- `include/agents-cpp/`: Public headers
  - `types.h`: Common type definitions
  - `agent_context.h`: Context for agent execution
  - `llm_interface.h`: Interface for LLM providers
  - `tool.h`: Tool interface
  - `memory.h`: Agent memory interface
  - `workflow.h`: Base workflow interface
  - `agent.h`: Base agent interface
  - `workflows/`: Workflow pattern implementations
  - `agents/`: Agent implementations
  - `tools/`: Tool implementations
  - `llms/`: LLM provider implementations
- `src/`: Implementation files
- `examples/`: Example applications

## Supported LLM Providers

- **Anthropic Claude**: Claude 3 family models (Opus, Sonnet, Haiku)
- **OpenAI**: GPT-4o, GPT-4, GPT-3.5 Turbo
- **Google**: Gemini family models (Pro, Flash)
- **Ollama**: Local models like Llama, Mistral, etc.

## Extending

### Adding Custom Tools

```cpp
auto custom_tool = createTool(
    "calculator",
    "Evaluates mathematical expressions",
    {
        {"expression", "The expression to evaluate", "string", true}
    },
    [](const JsonObject& params) -> ToolResult {
        String expr = params["expression"];
        // Implement calculation logic here
        double result = evaluate(expr);
        return ToolResult{
            true,
            "Result: " + std::to_string(result),
            {{"result", result}}
        };
    }
);

context->registerTool(custom_tool);
```

### Creating Custom Workflows

You can create custom workflows by extending the `Workflow` base class or combining existing workflows:

```cpp
class CustomWorkflow : public Workflow {
public:
    CustomWorkflow(std::shared_ptr<AgentContext> context)
        : Workflow(context) {}
    
    JsonObject run(const String& input) override {
        // Implement your custom workflow logic here
    }
};
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgements

This implementation is based on Anthropic's article ["Building effective agents"](https://www.anthropic.com/research/building-effective-agents) and draws inspiration from their research and recommendations. 