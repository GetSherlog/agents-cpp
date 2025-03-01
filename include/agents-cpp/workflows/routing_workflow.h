#pragma once

#include <agents-cpp/workflows/actor_workflow.h>
#include <map>

namespace agents {
namespace workflows {

/**
 * @brief Routing workflow using the actor model
 * 
 * This workflow classifies an input and directs it to a specialized follow-up
 * task or handler. It allows for separation of concerns and building more
 * specialized prompts for different types of inputs.
 */
class RoutingWorkflow : public ActorWorkflow {
public:
    // Handler definition for a route
    struct RouteHandler {
        String name;
        String description;
        String prompt_template;
        std::shared_ptr<LLMInterface> llm;  // Optional separate LLM for this route
        std::shared_ptr<Workflow> workflow;  // Optional workflow to delegate to
        
        RouteHandler(
            const String& name,
            const String& description,
            const String& prompt_template = "",
            std::shared_ptr<LLMInterface> llm = nullptr,
            std::shared_ptr<Workflow> workflow = nullptr
        ) : name(name), description(description), 
            prompt_template(prompt_template), llm(llm), workflow(workflow) {}
    };
    
    // Constructor with router LLM interface
    RoutingWorkflow(
        std::shared_ptr<LLMInterface> router_llm,
        const String& router_prompt_template = ""
    );
    
    // Add a route handler
    void addRouteHandler(const RouteHandler& handler);
    
    // Add a route handler with basic params
    void addRouteHandler(
        const String& name,
        const String& description,
        const String& prompt_template = "",
        std::shared_ptr<LLMInterface> handler_llm = nullptr,
        std::shared_ptr<Workflow> workflow = nullptr
    );
    
    // Initialize the workflow
    void init() override;
    
    // Execute the workflow with input
    JsonObject execute(const JsonObject& input) override;
    
    // Set the router prompt template
    void setRouterPromptTemplate(const String& prompt_template);
    
    // Set default handler for unknown routes
    void setDefaultHandler(const RouteHandler& handler);
    
private:
    // Router prompt template
    String router_prompt_template_;
    
    // Map of route handlers
    std::map<String, RouteHandler> route_handlers_;
    
    // Default handler for unknown routes
    RouteHandler default_handler_;
    
    // Actor for the router
    caf::actor router_actor_;
    
    // Map of handler actors
    std::map<String, caf::actor> handler_actors_;
    
    // Setup actor roles for this workflow
    void setupActorSystem() override;
    
    // Create the router system prompt with route descriptions
    String createRouterSystemPrompt() const;
};

} // namespace workflows
} // namespace agents 