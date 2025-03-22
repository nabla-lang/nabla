#include "interpreter.h"

namespace nabla {

namespace {

class Value
{
public:
  virtual ~Value() = default;

  virtual void print(Runtime& runtime) const = 0;
};

template<typename T>
class ValueImpl final : public Value
{
  T value_;

public:
  explicit ValueImpl(T value)
    : value_(value)
  {
  }

  [[nodiscard]] auto value() const -> const T& { return value_; }

  void print(Runtime& runtime) const override { runtime.print(value_); }
};

using IntValue = ValueImpl<int>;

using FloatValue = ValueImpl<float>;

using StringValue = ValueImpl<std::string>;

using ValuePtr = std::unique_ptr<Value>;

class InterpreterImpl final
  : public Interpreter
  , public ast::StmtVisitor
  , public ast::ExprVisitor
{
  Runtime* runtime_;

  std::vector<ValuePtr> values_;

public:
  explicit InterpreterImpl(Runtime* runtime)
    : runtime_(runtime)
  {
  }

  void exec(const ast::Module& mod) override
  {
    for (const auto& stmt : mod.stmts) {
      stmt->accept(*this);
    }
  }

protected:
  void visit(const ast::AssignStmt& stmt) override
  {
    // since we build the value table the same way the expressions are layed out in the AST, we do not need to map them
    // to the IDs.
    stmt.value().accept(*this);
  }

  void visit(const ast::PrintStmt& stmt) override
  {
    const auto id = stmt.id();
    values_.at(id)->print(*runtime_);
  }

  void visit(const ast::PrintEndStmt&) override { runtime_->print_end(); }

  void visit(const ast::LiteralExpr<int>& expr) override
  {
    values_.emplace_back(std::make_unique<IntValue>(expr.value()));
  }

  void visit(const ast::LiteralExpr<float>& expr) override
  {
    values_.emplace_back(std::make_unique<FloatValue>(expr.value()));
  }

  void visit(const ast::LiteralExpr<std::string>& expr) override
  {
    values_.emplace_back(std::make_unique<StringValue>(expr.value()));
  }

  void visit(const ast::AddExpr<int>& expr) override
  {
    (void)expr;
    //
  }

  void visit(const ast::AddExpr<float>&) override {}

  void visit(const ast::AddExpr<std::string>&) override {}

  void visit(const ast::MulExpr<int, int>& expr) override
  {
    const auto* l = static_cast<const IntValue*>(values_.at(expr.left()).get());
    const auto* r = static_cast<const IntValue*>(values_.at(expr.right()).get());
    push_value(std::make_unique<IntValue>(l->value() * r->value()));
  }

  void visit(const ast::MulExpr<float, float>&) override {}

  void push_value(std::unique_ptr<Value> value) { values_.emplace_back(std::move(value)); }
};

} // namespace

auto
Interpreter::create(Runtime* runtime) -> std::unique_ptr<Interpreter>
{
  return std::make_unique<InterpreterImpl>(runtime);
}

} // namespace nabla
