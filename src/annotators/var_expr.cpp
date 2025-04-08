#include "var_expr.h"

#include "../lexer.h"

namespace nabla {

namespace {

class VarResolver final
  : public NodeVisitor
  , public ExprVisitor
{
  const VarExpr* expr_{ nullptr };

  const DeclNode* decl_{ nullptr };

  bool done_{ false };

public:
  explicit VarResolver(const VarExpr* expr)
    : expr_(expr)
  {
  }

  auto done() const -> bool { return done_; }

  auto decl() const -> const DeclNode* { return decl_; }

protected:
  void visit(const PrintNode& node) override
  {
    for (const auto& arg : node.args()) {
      arg->accept(*this);
    }
  }

  void visit(const DeclNode& node) override
  {
    // Check whether this declaration contains the expression we're resolving.
    // If it does, we can't use it to resolve the reference. That would be like resolving:
    // let foo = foo;

    node.get_value().accept(*this);
    if (done()) {
      return;
    }

    if (node.get_name().data == expr_->get_name().data) {
      // We've found a match.
      // Since there may be another match within a nested scope, continue the search.
      decl_ = &node;
    }
  }

  void visit(const FuncNode& node) override
  {
    for (const auto& inner : node.body()) {
      inner->accept(*this);
    }
  }

  void visit(const StructNode& node) override {}

  void visit(const ReturnNode&) override {}

  void visit(const IntLiteralExpr&) override {}

  void visit(const FloatLiteralExpr&) override {}

  void visit(const StringLiteralExpr&) override {}

  void visit(const VarExpr& expr) override
  {
    // If we've reached the variable we're trying to resolve, terminate the search.
    done_ |= &expr == expr_;
  }

  void visit(const CallExpr& expr) override
  {
    // check for termination condition, we wont find variable declarations here
    for (const auto& arg : expr.args()) {
      arg.second->accept(*this);
    }
  }

  void visit(const AddExpr& expr) override { visit_binary(expr); }

  void visit(const MulExpr& expr) override { visit_binary(expr); }

  template<typename Derived>
  void visit_binary(const BinaryExpr<Derived>& expr)
  {
    expr.left().accept(*this);

    expr.right().accept(*this);
  }
};

} // namespace

auto
VarExprAnnotator::annotate(const VarExpr& expr, AnnotationType& annotation, AnnotationTable& table) -> bool
{
  if (annotation.decl) {
    // no change
    return false;
  }

  VarResolver resolver(&expr);

  for (const auto& node : tree_->nodes) {
    node->accept(resolver);
    if (resolver.done()) {
      break;
    }
  }

  const auto* decl = resolver.decl();
  if (decl) {
    annotation.decl = decl;
    return true;
  }

  return false;
}

} // namespace nabla
