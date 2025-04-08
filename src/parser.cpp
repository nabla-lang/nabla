#include "parser.h"

#include "diagnostics.h"

namespace nabla {

namespace {

// Note: When getting tokens, be sure to avoid copying them.
//       Use a reference to them instead. The reason is because
//       diagnostics take token pointers, and in order to ensure
//       that the pointer is valid when the diagnostic is thrown
//       in an exception, it has to remain alive until the exception
//       is caught.
//
// In other words, do this:
//   const auto& token = at(0);
// Instead of this:
//   const auto token = at(0);

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
    if (first == "let") {
      next();
      return parse_let_stmt(first);
    } else if (first == "fn") {
      next();
      return parse_fn_def(first);
    } else if (first == "struct") {
      next();
      return parse_struct_decl(first);
    } else if (first == "return") {
      next();
      return parse_return_stmt(first);
    } else if (first == "print") {
      next();
      return parse_print_stmt(first);
    }

    throw_error("unexpected token", &first);

    return nullptr;
  }

protected:
  void throw_error(const char* what, const Token* token) { throw FatalError(Diagnostic{ what, token }); }

  void missing_r_operand(const Token* op_token) { throw_error("missing right operand", op_token); }

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

  [[nodiscard]] auto parse_fn_def(const Token& fn_token) -> NodePtr
  {
    if (eof()) {
      throw_error("expected function name after this", &fn_token);
    }

    const auto& name = at(0);
    if (name != TK::identifier) {
      throw_error("expected this to be a function name", &name);
    }
    next();

    auto params = parse_param_list(name);

    auto body = parse_fn_body(name);

    return std::make_unique<FuncNode>(&name, std::move(params), std::move(body));
  }

  [[nodiscard]] auto parse_fn_body(const Token& name) -> std::vector<NodePtr>
  {
    if (eof()) {
      throw_error("missing function body", &name);
    }

    const auto& l_bracket = at(0);
    if (l_bracket != '{') {
      throw_error("expected '{' here", &l_bracket);
    }
    next();

    std::vector<NodePtr> body;
    while (!eof()) {
      if (at(0) == '}') {
        break;
      }
      auto inner = parse();
      if (!inner) {
        break;
      }
      body.emplace_back(std::move(inner));
    }

    if (eof()) {
      throw_error("missing '}'", &l_bracket);
    }

    const auto& r_bracket = at(0);
    if (r_bracket != '}') {
      throw_error("expected '}' here", &r_bracket);
    }
    next();
    return std::move(body);
  }

  [[nodiscard]] auto parse_param_list(const Token& anchor) -> std::vector<std::unique_ptr<DeclNode>>
  {
    if (eof()) {
      throw_error("expected parameter list after this", &anchor);
    }

    const auto& l_paren = at(0);
    if (l_paren != '(') {
      throw_error("expected a '(' here", &l_paren);
    }
    next();

    std::vector<std::unique_ptr<DeclNode>> params;

    while (!eof()) {

      if (at(0) == ')') {
        break;
      }

      auto param = parse_param_decl();
      if (!param) {
        break;
      }
      params.emplace_back(std::move(param));

      if (eof() || (at(0) == ')')) {
        break;
      }

      const auto& comma = at(0);
      if (comma != ',') {
        throw_error("expected either a ',' or ')' here", &comma);
      }
      next();
    }

    if (eof() || (at(0) != ')')) {
      throw_error("missing ')'", &l_paren);
    }
    next();

    return params;
  }

  [[nodiscard]] auto parse_param_decl() -> std::unique_ptr<DeclNode>
  {
    const auto& name = at(0);
    if (name != TK::identifier) {
      return nullptr;
    }
    next();

    const auto& colon = at(0);
    if (colon != ':') {
      return std::make_unique<DeclNode>(name, /*value=*/nullptr, /*immutable=*/true, /*type=*/nullptr);
    }
    next();

    auto type = parse_type();
    if (!type) {
      throw_error("expected type after this", &colon);
    }

    ExprPtr default_value;

    if (!eof() && at(0) == '=') {
      next();
      default_value = parse_expr();
    }

    return std::make_unique<DeclNode>(DeclNode(name, std::move(default_value), /*immutable=*/true, std::move(type)));
  }

  [[nodiscard]] auto parse_type() -> std::unique_ptr<TypeInstance>
  {
    if (eof()) {
      return nullptr;
    }

    const auto& name = at(0);
    if (name != TK::identifier) {
      throw_error("expected a type name here", &name);
    }
    next();

    std::vector<ExprPtr> args;

    if (!eof() && (at(0) == '<')) {
      const auto& l_bracket = at(0);
      next();

      while (!eof()) {
        if (at(0) == '>') {
          break;
        }

        auto arg = parse_expr();
        if (!arg) {
          break;
        }

        args.emplace_back(std::move(arg));
        if (eof() || (at(0) == '>')) {
          break;
        }
        const auto& comma = at(0);
        if (comma != ',') {
          throw_error("expected either ',' or '>' here", &comma);
        }
        next();
      }

      if (eof()) {
        throw_error("missing '>'", &l_bracket);
      }
      const auto& r_bracket = at(0);
      if (r_bracket != '>') {
        throw_error("expected '>' here", &r_bracket);
      }
      next();
    }

    return std::make_unique<TypeInstance>(&name, std::move(args));
  }

  [[nodiscard]] auto parse_struct_decl(const Token& struct_keyword) -> std::unique_ptr<StructNode>
  {
    if (eof()) {
      throw_error("expected name after this", &struct_keyword);
    }
    const auto& name = at(0);
    if (name != TK::identifier) {
      throw_error("expected this to be an struct name", &name);
    }
    next();

    if (eof()) {
      throw_error("expected struct body after this", &name);
    }

    if (at(0) == '<') {
      // TODO: type parameters
    }

    const auto& l_bracket = at(0);
    if (l_bracket != '{') {
      throw_error("expected '{' here", &l_bracket);
    }
    next();

    std::vector<std::unique_ptr<DeclNode>> fields;

    while (!eof()) {
      if (at(0) == '}') {
        break;
      }
      const auto& name = at(0);
      if (name != TK::identifier) {
        throw_error("expected field name or '}' here", &name);
      }
      next();

      if (eof() || (at(0) != ':')) {
        throw_error("expected ':' after field name", &name);
      }
      const auto& colon = at(0);
      next();

      auto type = parse_type();
      if (!type) {
        throw_error("expected type after this", &colon);
      }

      auto field = std::make_unique<DeclNode>(name, nullptr, /*immutable=*/false, std::move(type));

      fields.emplace_back(std::move(field));

      if (eof()) {
        break;
      }
      if (at(0) == '}') {
        break;
      }
      const auto& comma = at(0);
      if (comma != ',') {
        throw_error("expected either ',' or '}' here", &comma);
      }
      next();
    }

    if (eof()) {
      throw_error("missing '}'", &l_bracket);
    }

    const auto& r_bracket = at(0);
    if (r_bracket != '}') {
      throw_error("expected this to be '}'", &r_bracket);
    }
    next();

    return std::make_unique<StructNode>(&name, std::move(fields));
  }

  [[nodiscard]] auto parse_return_stmt(const Token& return_token) -> std::unique_ptr<ReturnNode>
  {
    auto value = parse_expr();
    terminate_stmt();
    return std::make_unique<ReturnNode>(std::move(value));
  }

  [[nodiscard]] auto parse_let_stmt(const Token& let_token) -> NodePtr
  {
    if (eof()) {
      throw_error("missing variable name", &let_token);
    }

    const auto& name = at(0);
    if (name != TK::identifier) {
      throw_error("expected this to be a variable name", &name);
    }
    next();

    const auto& equals = at(0);
    if (equals != '=') {
      throw_error("expected '=' here", &equals);
    }
    next();

    auto value = parse_expr();

    terminate_stmt();

    return std::make_unique<DeclNode>(name, std::move(value), /*immutable=*/true);
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
      if (eof()) {
        missing_r_operand(&op);
      }
      auto rhs = parse_mul_div_expr();
      lhs = std::make_unique<AddExpr>(std::move(lhs), std::move(rhs), &op);
    }
    return lhs;
  }

  [[nodiscard]] auto parse_mul_div_expr() -> ExprPtr
  {
    auto lhs = parse_primary_expr();
    while (!eof() && (at(0) == '*' || at(0) == '/')) {
      const auto& op = at(0);
      next();
      if (eof()) {
        missing_r_operand(&op);
      }
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
    } else if (first == TK::identifier) {
      next();
      if (!eof() && (at(0) == '(')) {
        const auto& l_paren = at(0);
        next();
        return parse_call_expr(first, l_paren);
      }
      return std::make_unique<VarExpr>(first);
    } else {
      throw_error("expected an expression here", &first);
    }

    return nullptr;
  }

  [[nodiscard]] auto parse_call_expr(const Token& name, const Token& l_paren) -> std::unique_ptr<CallExpr>
  {
    std::vector<CallExpr::NamedArg> args;

    while (!eof()) {
      // TODO : named args

      if (at(0) == ')') {
        break;
      }

      auto value = parse_expr();

      args.emplace_back(CallExpr::NamedArg(nullptr, std::move(value)));
      if (eof()) {
        break;
      }
      const auto& comma = at(0);
      if (comma != ',') {
        break;
      }
      next();
    }

    if (eof()) {
      throw_error("missing ')'", &l_paren);
    }

    const auto& r_paren = at(0);
    if (r_paren != ')') {
      throw_error("expected ')' here", &r_paren);
    }
    next();

    return std::make_unique<CallExpr>(&name, std::move(args));
  }
};

} // namespace

auto
Parser::create(const Token* tokens, const size_t num_tokens) -> std::unique_ptr<Parser>
{
  return std::make_unique<ParserImpl>(tokens, num_tokens);
}

} // namespace nabla
