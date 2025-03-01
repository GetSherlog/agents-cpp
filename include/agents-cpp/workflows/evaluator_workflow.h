#pragma once

#include <agents-cpp/workflows/actor_workflow.h>

namespace agents {
namespace workflows {

/**
 * @brief Evaluator-Optimizer workflow using the actor model
 * 
 * In this workflow, one LLM call generates a response while another provides
 * evaluation and feedback in a loop. This pattern is effective when we have
 * clear evaluation criteria and iterative refinement provides value.
 */
class EvaluatorWorkflow : public ActorWorkflow {
public:
    // Constructor with LLM interfaces for optimizer and evaluator
    EvaluatorWorkflow(
        std::shared_ptr<LLMInterface> optimizer_llm,
        std::shared_ptr<LLMInterface> evaluator_llm = nullptr,
        const String& optimizer_prompt_template = "",
        const String& evaluator_prompt_template = ""
    );
    
    // Initialize the workflow
    void init() override;
    
    // Execute the workflow with input
    JsonObject execute(const JsonObject& input) override;
    
    // Set the evaluation criteria for the evaluator
    void setEvaluationCriteria(const std::vector<String>& criteria);
    
    // Set the max number of feedback iterations
    void setMaxIterations(int max_iterations);
    
    // Set the improvement threshold (minimum score to accept a response)
    void setImprovementThreshold(double threshold);
    
    // Set the optimizer prompt template
    void setOptimizerPromptTemplate(const String& prompt_template);
    
    // Set the evaluator prompt template
    void setEvaluatorPromptTemplate(const String& prompt_template);
    
private:
    // LLM for the evaluator (if nullptr, use the main LLM)
    std::shared_ptr<LLMInterface> evaluator_llm_;
    
    // Prompt templates
    String optimizer_prompt_template_;
    String evaluator_prompt_template_;
    
    // Evaluation criteria
    std::vector<String> evaluation_criteria_;
    
    // Max iterations for the feedback loop
    int max_iterations_ = 3;
    
    // Improvement threshold for accepting a response
    double improvement_threshold_ = 0.8;  // 0.0 to 1.0
    
    // Actor for the optimizer
    caf::actor optimizer_actor_;
    
    // Actor for the evaluator
    caf::actor evaluator_actor_;
    
    // Setup actor roles for this workflow
    void setupActorSystem() override;
    
    // Create the evaluator system prompt with criteria
    String createEvaluatorSystemPrompt() const;
};

} // namespace workflows
} // namespace agents 