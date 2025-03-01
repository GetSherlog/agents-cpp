#pragma once

#include <agents-cpp/workflow.h>
#include <functional>

namespace agents {
namespace workflows {

/**
 * @brief A workflow where one LLM optimizes output based on another's feedback
 * 
 * In the evaluator-optimizer workflow, one LLM call generates a response while
 * another provides evaluation and feedback in a loop.
 */
class EvaluatorOptimizer : public Workflow {
public:
    EvaluatorOptimizer(std::shared_ptr<AgentContext> context);
    ~EvaluatorOptimizer() override = default;
    
    // Set the system prompt for the optimizer
    void setOptimizerPrompt(const String& optimizer_prompt);
    
    // Set the system prompt for the evaluator
    void setEvaluatorPrompt(const String& evaluator_prompt);
    
    // Set the evaluation criteria
    void setEvaluationCriteria(const std::vector<String>& criteria);
    
    // Set custom optimization function
    void setOptimizer(std::function<String(const String&, const JsonObject&)> optimizer);
    
    // Set custom evaluation function
    void setEvaluator(std::function<JsonObject(const String&, const String&)> evaluator);
    
    // Set the maximum number of iterations
    void setMaxIterations(int max_iterations);
    
    // Set the minimum score to accept a result
    void setMinimumAcceptableScore(double min_score);
    
    // Run the evaluator-optimizer workflow
    JsonObject run(const String& input) override;

private:
    String optimizer_prompt_;
    String evaluator_prompt_;
    std::vector<String> evaluation_criteria_;
    std::function<String(const String&, const JsonObject&)> optimizer_;
    std::function<JsonObject(const String&, const String&)> evaluator_;
    int max_iterations_ = 5;
    double min_acceptable_score_ = 0.8;
    
    // Default optimizer function
    String defaultOptimizer(const String& input, const JsonObject& feedback);
    
    // Default evaluator function
    JsonObject defaultEvaluator(const String& input, const String& output);
};

} // namespace workflows
} // namespace agents 