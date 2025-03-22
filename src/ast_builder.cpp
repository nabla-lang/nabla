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

  std::vector<Diagnostic> diagnostics_;

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
  std::string unescape_string_literal(const Token& token)
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

    auto ast_expr = std::make_unique<ast::LiteralExpr<float>>(value);

    push_assign_expr(std::move(ast_expr));
  }

  void visit(const FloatLiteralExpr& expr) override
  {
    const auto& token = expr.token();

    // TODO : this is really inefficient, but compiler support for from_chars (for floats) is not great yet.
    std::istringstream tmp_stream(std::string(token.data));
    float value{ 0.0F };
    tmp_stream >> value;

    auto ast_expr = std::make_unique<ast::LiteralExpr<float>>(value);

    push_assign_expr(std::move(ast_expr));
  }

  void visit(const AddExpr& expr) override
  {
    expr.left().accept(*this);

    expr.right().accept(*this);

    (void)expr;
    //
  }

  void visit(const MulExpr& expr) override
  {
    (void)expr;
    //
  }

  [[nodiscard]] auto push_expr(ast::Expr* expr) -> size_t
  {
    const auto id = exprs_.size();
    exprs_.emplace_back(expr);
    return id;
  }

  [[maybe_unused]] auto push_assign_expr(std::unique_ptr<ast::Expr> expr) -> size_t
  {
    const auto id = push_expr(expr.get());
    auto stmt = std::make_unique<ast::AssignStmt>(id, std::move(expr));
    module_->stmts.emplace_back(std::move(stmt));
    return id;
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
