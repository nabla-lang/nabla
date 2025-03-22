#include "parser.h"

#include "diagnostics.h"

namespace nabla {

namespace {

class ParserImpl final : public Parser
{
  const Token* tokens_{ 0 };

  size_t num_tokens_{ 0 };

  size_t offset_{ 0 };

public:
  ParserImpl(const Token* tokens, const size_t num_tokens)
    : tokens_(tokens)
    , num_tokens_(num_tokens)
  {
  }

  [[nodiscard]] auto eof() const -> bool override { return offset_ >= num_tokens_; }

  [[nodiscard]] auto parse() -> NodePtr override
  {
    const auto& first = at(0);
    if (first == "print") {
      next();
      return parse_print_stmt(first);
    }

    throw_error("unexpected token", &first);

    return nullptr;
  }

protected:
  void throw_error(const char* what, const Token* token) { throw FatalError(Diagnostic{ what, token }); }

  void next() { offset_++; }

  [[nodiscard]] auto at(const size_t offset) const -> const Token&
  {
    static const Token null_token;
    const auto o = offset_ + offset;
    if (o >= num_tokens_) {
      return null_token;
    } else {
      return tokens_[o];
    }
  }

  void terminate_stmt()
  {
    if (eof()) {
      // we don't really need to terminate the statement at the end of the file.
      return;
    }

    const auto& tok = at(0);
    if (tok != ';') {
      throw_error("expected ';' here", &tok);
      return;
    }

    next();
  }

  [[nodiscard]] auto parse_print_stmt(const Token& print_token) -> NodePtr
  {
    auto node = std::make_unique<PrintNode>(parse_arg_list(print_token));
    terminate_stmt();
    return node;
  }

  [[nodiscard]] auto parse_arg_list(const Token& func_name) -> std::vector<ExprPtr>
  {
    if (eof()) {
      throw_error("missing argument list", &func_name);
    }

    const auto& l_paren = at(0);
    if (l_paren != '(') {
      throw_error("expected the start of an argument list here", &l_paren);
    }

    next();

    std::vector<ExprPtr> args;

    while (!eof() && (at(0) != ')')) {
      auto arg = parse_expr();
      if (!arg) {
        break;
      }

      args.emplace_back(std::move(arg));

      if (eof() || (at(0) == ')')) {
        break;
      }

      const auto& comma = at(0);
      if (comma != ',') {
        throw_error("expected a ',' or ')' here", &comma);
      }

      next();
    }

    if (eof() || (at(0) != ')')) {
      throw_error("missing ')'", &l_paren);
    }

    next();

    return args;
  }

  [[nodiscard]] auto parse_expr() -> ExprPtr { return parse_add_sub_expr(); }

  [[nodiscard]] auto parse_add_sub_expr() -> ExprPtr
  {
    auto lhs = parse_mul_div_expr();
    while (!eof() && (at(0) == '+' || at(0) == '-')) {
      const auto& op = at(0);
      next();
      auto rhs = parse_mul_div_expr();
      lhs = std::make_unique<AddExpr>(std::move(lhs), std::move(rhs), &op);
    }
    return lhs;
  }

  ExprPtr parse_mul_div_expr()
  {
    auto lhs = parse_primary_expr();
    while (!eof() && (at(0) == '*' || at(0) == '/')) {
      const auto& op = at(0);
      next();
      auto rhs = parse_primary_expr();
      lhs = std::make_unique<MulExpr>(std::move(lhs), std::move(rhs), &op);
    }
    return lhs;
  }

  [[nodiscard]] auto parse_primary_expr() -> ExprPtr
  {
    const auto& first = at(0);
    if (first == TK::string_literal) {
      next();
      return std::make_unique<StringLiteralExpr>(&first);
    } else if (first == TK::int_literal) {
      next();
      return std::make_unique<IntLiteralExpr>(&first);
    } else if (first == TK::float_literal) {
      next();
      return std::make_unique<FloatLiteralExpr>(&first);
    }

    return nullptr;
  }
};

} // namespace

auto
Parser::create(const Token* tokens, const size_t num_tokens) -> std::unique_ptr<Parser>
{
  return std::make_unique<ParserImpl>(tokens, num_tokens);
}

} // namespace nabla
