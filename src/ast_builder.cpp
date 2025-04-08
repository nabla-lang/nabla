#include "ast_builder.h"

#include "diagnostics.h"
#include "lexer.h"

#include <charconv>
#include <sstream>

#include <stddef.h>

namespace nabla {

namespace {

class ASTBuilderImpl final
  : public ASTBuilder
  , public NodeVisitor
  , public ExprVisitor
{
  ast::Module* module_;

  const AnnotationTable* annotations_;

  /// @brief Used internally to track all values computed by each expression.
  /// @details The indices of each element in this vector correspond to the value IDs used in the AST.
  std::vector<ast::Expr*> exprs_;

  std::map<const DeclNode*, size_t> decl_ids_;

  std::vector<Diagnostic> diagnostics_;

  size_t last_expr_id_{ 0 };

public:
  ASTBuilderImpl(ast::Module* m, const AnnotationTable* annotations)
    : module_(m)
    , annotations_(annotations)
  {
  }

  [[nodiscard]] auto build(const Node& node) -> bool override
  {
    node.accept(*this);
    //
    return true;
  }

protected:
  [[nodiscard]] static auto unescape_string_literal(const Token& token) -> std::string
  {
    const auto& data = token.data;

    std::string result;
    result.reserve(data.size()); // estimate

    for (size_t i = 1; i < data.size() - 1; ++i) {
      char c = data.at(i);
      if (c == '\\') {
        if (i + 1 >= data.size() - 1) {
          throw std::invalid_argument("Invalid escape sequence at end of string");
        }
        char next = data[++i];
        switch (next) {
          case 'n':
            result += '\n';
            break;
          case 't':
            result += '\t';
            break;
          case 'r':
            result += '\r';
            break;
          case '\\':
            result += '\\';
            break;
          case '\'':
            result += '\'';
            break;
          case '"':
            result += '\"';
            break;
          case '0':
            result += '\0';
            break;
          case 'b':
            result += '\b';
            break;
          case 'f':
            result += '\f';
            break;
          case 'v':
            result += '\v';
            break;
          default:
            throw std::invalid_argument(std::string("Unknown escape sequence: \\") + next);
        }
      } else {
        result += c;
      }
    }

    return result;
  }

  // nodes

  void visit(const PrintNode& node) override
  {
    for (const auto& expr : node.args()) {
      expr->accept(*this);
      const auto last_expr_id = exprs_.size() - 1;
      auto stmt = std::make_unique<ast::PrintStmt>(last_expr_id);
      module_->stmts.emplace_back(std::move(stmt));
    }
    module_->stmts.emplace_back(std::make_unique<ast::PrintEndStmt>());
  }

  void visit(const DeclNode& node) override
  {
    node.get_value().accept(*this);

    const auto id = exprs_.size() - 1;

    decl_ids_.emplace(&node, id);
  }

  void visit(const FuncNode& node) override
  {
    (void)node;
    //
  }

  void visit(const StructNode&) override {}

  void visit(const ReturnNode&) override {}

  // expressions

  void visit(const StringLiteralExpr& expr) override
  {
    const auto& token = expr.token();
    auto value = unescape_string_literal(token);
    auto ast_expr = std::make_unique<ast::LiteralExpr<std::string>>(std::move(value));
    push_assign_expr(std::move(ast_expr));
  }

  void visit(const IntLiteralExpr& expr) override
  {
    const auto& token = expr.token();

    int value{ 0 };

    const auto result = std::from_chars(token.data.data(), token.data.data() + token.data.size(), value);
    if (result.ptr != (token.data.data() + token.data.size())) {
      add_diagnostic("unable to parse integer", &token);
    }

    auto ast_expr = std::make_unique<ast::LiteralExpr<int>>(value);

    push_assign_expr(std::move(ast_expr));
  }

  void visit(const FloatLiteralExpr& expr) override
  {
    const auto& token = expr.token();

    // TODO : this is really inefficient, but compiler support for from_chars for floats is not great (yet).
    std::istringstream tmp_stream(std::string(token.data));
    float value{ 0.0F };
    tmp_stream >> value;

    auto ast_expr = std::make_unique<ast::LiteralExpr<float>>(value);

    push_assign_expr(std::move(ast_expr));
  }

  void visit(const VarExpr& expr) override
  {
    auto& annotation = annotations_->var_expr.at(&expr);

    const auto* decl = annotation.decl;

    const auto id = decl_ids_.at(decl);

    last_expr_id_ = id;
  }

  void visit(const CallExpr& expr) override
  {
    (void)expr;
    // TODO
  }

  void visit(const AddExpr& expr) override
  {
    const auto l = build_expr(expr.left());
    const auto r = build_expr(expr.right());

    const auto& annotation = annotations_->add_expr.at(&expr);

    switch (annotation.op) {
      case Annotation<AddExpr>::Op::none:
        break;
      case Annotation<AddExpr>::Op::add_int:
        last_expr_id_ = push_assign_expr(std::make_unique<ast::AddExpr<int>>(l, r));
        break;
      case Annotation<AddExpr>::Op::add_float:
        last_expr_id_ = push_assign_expr(std::make_unique<ast::AddExpr<float>>(l, r));
        break;
    }
  }

  void visit(const MulExpr& expr) override
  {
    const auto l = build_expr(expr.left());
    const auto r = build_expr(expr.right());

    const auto& annotation = annotations_->mul_expr.at(&expr);

    switch (annotation.op) {
      case Annotation<MulExpr>::Op::none:
        break;
      case Annotation<MulExpr>::Op::mul_int:
        push_assign_expr(std::make_unique<ast::MulExpr<int, int>>(l, r));
        break;
      case Annotation<MulExpr>::Op::mul_float:
        push_assign_expr(std::make_unique<ast::MulExpr<float, float>>(l, r));
        break;
    }
  }

  [[nodiscard]] auto build_expr(const Expr& expr) -> size_t
  {
    expr.accept(*this);
    return last_expr_id_;
  }

  [[maybe_unused]] auto push_assign_expr(std::unique_ptr<ast::Expr> expr) -> size_t
  {
    return push_assign_expr(std::move(expr), exprs_.size());
  }

  [[maybe_unused]] auto push_assign_expr(std::unique_ptr<ast::Expr> expr, const size_t expr_id) -> size_t
  {
    exprs_.emplace_back(expr.get());
    auto stmt = std::make_unique<ast::AssignStmt>(expr_id, std::move(expr));
    module_->stmts.emplace_back(std::move(stmt));
    last_expr_id_ = expr_id;
    return expr_id;
  }

  void add_diagnostic(const std::string& what, const Token* token)
  {
    diagnostics_.emplace_back(Diagnostic{ what, token });
  }
};

} // namespace

auto
ASTBuilder::create(ast::Module* m, const AnnotationTable* annotations) -> std::unique_ptr<ASTBuilder>
{
  return std::make_unique<ASTBuilderImpl>(m, annotations);
}

} // namespace nabla
