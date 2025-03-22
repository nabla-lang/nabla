#include "mul_expr.h"

namespace nabla {

auto
MulExprAnnotator::annotate(const MulExpr& expr, AnnotationType& annotation, AnnotationTable& table) -> bool
{
  if (annotation.result_type) {
    // no change
    return false;
  }

  const auto* l_type = table.resolve_type(expr.left());
  if (!l_type) {
    return false;
  }

  const auto* r_type = table.resolve_type(expr.right());
  if (!r_type) {
    return false;
  }

  const auto lid = l_type->id();
  const auto rid = r_type->id();

  if (lid == TypeID::float_ && rid == TypeID::float_) {
    annotation.result_type = std::make_unique<FloatType>();
    annotation.op = Annotation<MulExpr>::Op::mul_float;
    // indicate a change was made to the annotation
    return true;
  }

  if (lid == TypeID::int_ && rid == TypeID::int_) {
    annotation.result_type = std::make_unique<IntType>();
    annotation.op = Annotation<MulExpr>::Op::mul_int;
    return true;
  }

  // by default, no change to the annotation because the
  // combination of operands is not supported.

  return false;
}

} // namespace nabla
