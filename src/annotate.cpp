#include "annotate.h"

#include "annotators/add_expr.h"
#include "annotators/mul_expr.h"

namespace nabla {

namespace {

template<typename T>
auto
add_if_not_exists(const T& object, std::map<const T*, Annotation<T>, std::less<>>& m) -> Annotation<T>*
{
  auto it = m.find(&object);
  if (it == m.end()) {
    it = m.emplace(&object, Annotation<T>{}).first;
  }

  return &it->second;
}

class ExprVisitorImpl final : public ExprVisitor
{
  AnnotationTable* annotations_{ nullptr };

  AddExprAnnotator add_expr_annotator_;

  MulExprAnnotator mul_expr_annotator_;

public:
  ExprVisitorImpl(AnnotationTable* annotations)
    : annotations_(annotations)
  {
  }

  void visit(const IntLiteralExpr& expr) override {}

  void visit(const FloatLiteralExpr& expr) override {}

  void visit(const StringLiteralExpr& expr) override {}

  void visit(const AddExpr& expr) override
  {
    auto* annotation = add_if_not_exists<AddExpr>(expr, annotations_->add_expr);

    add_expr_annotator_.annotate(expr, *annotation, *annotations_);
  }

  void visit(const MulExpr& expr) override
  {
    auto* annotation = add_if_not_exists<MulExpr>(expr, annotations_->mul_expr);

    mul_expr_annotator_.annotate(expr, *annotation, *annotations_);
  }
};

class NodeVisitorImpl final : public NodeVisitor
{
  AnnotationTable* annotations_;

  ExprVisitorImpl expr_visitor_;

public:
  explicit NodeVisitorImpl(AnnotationTable* annotations)
    : annotations_(annotations)
    , expr_visitor_(annotations)
  {
  }

  void visit(const PrintNode& node) override
  {
    for (const auto& arg : node.args()) {
      arg->accept(expr_visitor_);
    }
  }
};

} // namespace

auto
annotate(const std::vector<NodePtr>& nodes) -> AnnotationTable
{
  AnnotationTable annotations;
  NodeVisitorImpl visitor(&annotations);
  for (const auto& n : nodes) {
    n->accept(visitor);
  }
  return annotations;
}

} // namespace nabla
