#include "validator.h"

namespace nabla {

namespace {

class ValidatorImpl final : public Validator
{
  std::vector<Diagnostic> diagnostics_;

  bool failed_{ false };

public:
  auto get_diagnostics() -> std::vector<Diagnostic> override { return std::move(diagnostics_); }

  void validate(const std::vector<NodePtr>&, const AnnotationTable& annotations) override
  {
    failed_ = false;

    validate_add_expr(annotations);

    validate_mul_expr(annotations);
  }

  auto failed() const -> bool override { return failed_; }

protected:
  void add_diagnostic(const std::string& what, const Token* token)
  {
    diagnostics_.emplace_back(Diagnostic{ what, token });
  }

  void unresolved_operator(const Token* token)
  {
    add_diagnostic("unresolved operator", token);
    failed_ = true;
  }

  void validate_add_expr(const AnnotationTable& annotations)
  {
    for (const auto& annotation : annotations.add_expr) {
      if (!annotation.second.result_type) {
        unresolved_operator(&annotation.first->op_token());
      }
    }
  }

  void validate_mul_expr(const AnnotationTable& annotations)
  {
    for (const auto& annotation : annotations.mul_expr) {
      if (!annotation.second.result_type) {
        unresolved_operator(&annotation.first->op_token());
      }
    }
  }
};

} // namespace

auto
Validator::create() -> std::unique_ptr<Validator>
{
  return std::make_unique<ValidatorImpl>();
}

} // namespace nabla
