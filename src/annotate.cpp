#include "annotate.h"

#include "annotators/add_expr.h"
#include "annotators/mul_expr.h"
#include "annotators/var_expr.h"

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

  const SyntaxTree* tree_{ nullptr };

  AddExprAnnotator add_expr_annotator_;

  MulExprAnnotator mul_expr_annotator_;

  VarExprAnnotator var_expr_annotator_;

  bool annotated_{ false };

public:
  ExprVisitorImpl(AnnotationTable* annotations, const SyntaxTree* tree)
    : annotations_(annotations)
    , tree_(tree)
    , var_expr_annotator_(*tree)
  {
  }

  [[nodiscard]] auto annotate(const Expr& expr) -> bool
  {
    annotated_ = false;
    expr.accept(*this);
    return annotated_;
  }

  void visit(const IntLiteralExpr& expr) override {}

  void visit(const FloatLiteralExpr& expr) override {}

  void visit(const StringLiteralExpr& expr) override {}

  void visit(const AddExpr& expr) override
  {
    annotate_binary(expr);

    auto* annotation = add_if_not_exists<AddExpr>(expr, annotations_->add_expr);

    annotated_ |= add_expr_annotator_.annotate(expr, *annotation, *annotations_);
  }

  void visit(const MulExpr& expr) override
  {
    annotate_binary(expr);

    auto* annotation = add_if_not_exists<MulExpr>(expr, annotations_->mul_expr);

    annotated_ |= mul_expr_annotator_.annotate(expr, *annotation, *annotations_);
  }

  void visit(const VarExpr& expr) override
  {
    auto* annotation = add_if_not_exists(expr, annotations_->var_expr);

    annotated_ |= var_expr_annotator_.annotate(expr, *annotation, *annotations_);
  }

  void visit(const CallExpr& expr) override
  {
    (void)expr;
    // TODO
  }

protected:
  template<typename Derived>
  void annotate_binary(const BinaryExpr<Derived>& expr)
  {
    expr.left().accept(*this);

    expr.right().accept(*this);
  }
};

class NodeVisitorImpl final : public NodeVisitor
{
  AnnotationTable* annotations_;

  const SyntaxTree* tree_;

  ExprVisitorImpl expr_visitor_;

  bool annotated_{ false };

public:
  explicit NodeVisitorImpl(AnnotationTable* annotations, const SyntaxTree* tree)
    : annotations_(annotations)
    , tree_(tree)
    , expr_visitor_(annotations, tree)
  {
  }

  [[nodiscard]] auto annotate(const Node& node) -> bool
  {
    annotated_ = false;
    node.accept(*this);
    return annotated_;
  }

  void visit(const PrintNode& node) override
  {
    for (const auto& arg : node.args()) {
      annotated_ |= expr_visitor_.annotate(*arg);
    }
  }

  void visit(const DeclNode& node) override
  {
    node.get_value().accept(expr_visitor_);

    auto* annotation = add_if_not_exists(node, annotations_->decl_node);

    if (!annotation->type) {
      annotation->type = annotations_->resolve_type(node.get_value());
      annotated_ |= (annotation->type != nullptr);
    }
  }

  void visit(const FuncNode& node) override
  {
    // TODO : validate name

    // TODO : default initializers?

    for (const auto& inner_node : node.body()) {
      inner_node->accept(*this);
    }
  }

  void visit(const StructNode&) override {}

  void visit(const ReturnNode&) override {}
};

} // namespace

auto
annotate(const SyntaxTree& tree) -> AnnotationTable
{
  AnnotationTable annotations;

  NodeVisitorImpl visitor(&annotations, &tree);

  // we loop and iteratively annotate the tree until there is nothing left to do.
  while (true) {

    auto annotated{ false };

    for (const auto& n : tree.nodes) {
      annotated |= visitor.annotate(*n);
    }

    if (!annotated) {
      break;
    }
  }

  return annotations;
}

} // namespace nabla
