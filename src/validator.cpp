#include "validator.h"

#include "lexer.h"

#include <map>

namespace nabla {

namespace {

struct Scope final
{
  std::map<Token, const DeclNode*, std::less<>> decls;
};

class ValidatorImpl final
  : public Validator
  , public NodeVisitor
{
  std::vector<Diagnostic> diagnostics_;

  bool failed_{ false };

  std::vector<Scope> scope_;

public:
  auto get_diagnostics() -> std::vector<Diagnostic> override { return std::move(diagnostics_); }

  void validate(const std::vector<NodePtr>& nodes, const AnnotationTable& annotations) override
  {
    failed_ = false;

    scope_ = std::vector<Scope>{ Scope{} };

    validate_add_expr(annotations);

    validate_mul_expr(annotations);

    for (const auto& node : nodes) {
      node->accept(*this);
    }
  }

  auto failed() const -> bool override { return failed_; }

protected:
  [[nodiscard]] auto current_scope() -> Scope& { return scope_.at(scope_.size() - 1); }

  [[nodiscard]] auto find_decl(const Token& name) const -> const DeclNode*
  {
    for (size_t i = scope_.size(); i > 0; i--) {
      const auto& scope = scope_.at(i - 1);
      const auto it = scope.decls.find(name);
      if (it != scope.decls.end()) {
        return it->second;
      }
    }

    return nullptr;
  }

  void visit(const DeclNode& node) override
  {
    if (const auto* existing = find_decl(node.get_name()); existing) {
      add_diagnostic("symbol already exists by this name", &node.get_name());
    } else {
      current_scope().decls.emplace(node.get_name(), &node);
    }
  }

  void visit(const FuncNode& node) override
  {
    //
    (void)node;
  }

  void visit(const StructNode&) override {}

  void visit(const ReturnNode&) override {}

  void visit(const PrintNode& node) override
  {
    //
  }

  void add_diagnostic(const std::string& what, const Token* token)
  {
    diagnostics_.emplace_back(Diagnostic{ what, token });
    failed_ = true;
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
