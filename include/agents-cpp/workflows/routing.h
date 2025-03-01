#pragma once

#include <agents-cpp/workflow.h>
#include <map>
#include <functional>

namespace agents {
namespace workflows {

/**
 * @brief A workflow that routes requests to different handlers
 * 
 * Routing classifies an input and directs it to a specialized followup task.
 */
class Routing : public Workflow {
public:
    /**
     * @brief Handler for a route
     */
    using RouteHandler = std::function<JsonObject(const String&, const JsonObject&)>;
    
    Routing(std::shared_ptr<AgentContext> context);
    ~Routing() override = default;
    
    // Set the system prompt for the router
    void setRouterPrompt(const String& router_prompt);
    
    // Add a route
    void addRoute(const String& route_name, const String& route_description, RouteHandler handler);
    
    // Set a default route for unknown categories
    void setDefaultRoute(RouteHandler handler);
    
    // Run the routing workflow
    JsonObject run(const String& input) override;
    
    // Define available routes as a JSON schema
    JsonObject getRoutesSchema() const;

private:
    String router_prompt_;
    std::map<String, std::pair<String, RouteHandler>> routes_;
    std::optional<RouteHandler> default_route_;
};

} // namespace workflows
} // namespace agents 