#pragma once

#include <memory>
#include <string>
#include <vector>

#include <stddef.h>

namespace nabla {

namespace ast {

template<typename T>
class LiteralExpr;

template<typename T>
class AddExpr;

template<typename A, typename B>
class MulExpr;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void visit(const LiteralExpr<int>& expr) = 0;

  virtual void visit(const LiteralExpr<float>& expr) = 0;

  virtual void visit(const LiteralExpr<std::string>& expr) = 0;

  virtual void visit(const AddExpr<int>& expr) = 0;

  virtual void visit(const AddExpr<float>& expr) = 0;

  virtual void visit(const AddExpr<std::string>& expr) = 0;

  virtual void visit(const MulExpr<int, int>& expr) = 0;

  virtual void visit(const MulExpr<float, float>& expr) = 0;
};

class Expr
{
public:
  virtual ~Expr() = default;

  virtual void accept(ExprVisitor& visitor) const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

template<typename Derived>
class ExprBase : public Expr
{
public:
  ~ExprBase() override = default;

  void accept(ExprVisitor& visitor) const override { visitor.visit(static_cast<const Derived&>(*this)); }
};

template<typename T>
class LiteralExpr final : public ExprBase<LiteralExpr<T>>
{
  T value_;

public:
  explicit LiteralExpr(T value)
    : value_(value)
  {
  }

  [[nodiscard]] auto value() const -> T { return value_; }
};

template<typename Derived>
class BinaryExpr : public ExprBase<Derived>
{
  size_t left_;

  size_t right_;

public:
  BinaryExpr(size_t left, size_t right)
    : left_(left)
    , right_(right)
  {
  }

  [[nodiscard]] auto left() const -> size_t { return left_; }

  [[nodiscard]] auto right() const -> size_t { return right_; }
};

template<typename T>
class AddExpr final : public BinaryExpr<AddExpr<T>>
{
public:
  using BinaryExpr<AddExpr<T>>::BinaryExpr;
};

template<typename A, typename B>
class MulExpr final : public BinaryExpr<MulExpr<A, B>>
{
public:
  using BinaryExpr<MulExpr<A, B>>::BinaryExpr;
};

class AssignStmt;
class PrintStmt;
class PrintEndStmt;

class StmtVisitor
{
public:
  virtual ~StmtVisitor() = default;

  virtual void visit(const AssignStmt&) = 0;

  virtual void visit(const PrintStmt&) = 0;

  virtual void visit(const PrintEndStmt&) = 0;
};

class Stmt
{
public:
  virtual ~Stmt() = default;

  virtual void accept(StmtVisitor& visitor) const = 0;
};

using StmtPtr = std::unique_ptr<Stmt>;

template<typename Derived>
class StmtBase : public Stmt
{
public:
  ~StmtBase() override = default;

  void accept(StmtVisitor& visitor) const override { visitor.visit(static_cast<const Derived&>(*this)); }
};

class AssignStmt final : public StmtBase<AssignStmt>
{
  size_t id_;

  ExprPtr value_;

public:
  AssignStmt(size_t id, ExprPtr value)
    : id_(id)
    , value_(std::move(value))
  {
  }

  [[nodiscard]] auto id() const -> size_t { return id_; }

  [[nodiscard]] auto value() const -> const Expr& { return *value_; }
};

class PrintStmt final : public StmtBase<PrintStmt>
{
  size_t id_;

public:
  explicit PrintStmt(size_t id)
    : id_(id)
  {
  }

  [[nodiscard]] auto id() const -> int { return id_; }
};

class PrintEndStmt final : public StmtBase<PrintEndStmt>
{
public:
};

struct Module final
{
  std::vector<StmtPtr> stmts;
};

} // namespace ast

} // namespace nabla