#include "annotations.h"

namespace nabla {

namespace {

const FloatType float_type_;

const IntType int_type_;

const StringType string_type_;

class SimpleTypeResolver final : public ExprVisitor
{
  const Type* resolved_type_{ nullptr };

public:
  void visit(const IntLiteralExpr&) override { resolved_type_ = &int_type_; }

  void visit(const FloatLiteralExpr&) override { resolved_type_ = &float_type_; }

  void visit(const StringLiteralExpr&) override { resolved_type_ = &string_type_; }

  void visit(const AddExpr&) override {}

  void visit(const MulExpr&) override {}

  auto result() const -> const Type* { return resolved_type_; }
};

} // namespace

auto
AnnotationTable::resolve_type(const Expr& expr) const -> const Type*
{
  if (auto it = add_expr.find(&expr); it != add_expr.end()) {
    return it->second.result_type.get();
  }

  if (auto it = mul_expr.find(&expr); it != mul_expr.end()) {
    return it->second.result_type.get();
  }

  // The expression might be a literal type.
  // Attempt to resolve the type manually.

  SimpleTypeResolver resolver;
  expr.accept(resolver);
  return resolver.result();
}

} // namespace nabla
