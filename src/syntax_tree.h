#pragma once

#include <memory>
#include <vector>

namespace nabla {

class Token;

// type

enum class TypeID
{
  float_,
  int_,
  string
};

[[nodiscard]] auto
to_string(TypeID type_id) -> const char*;

class Type
{
public:
  virtual ~Type() = default;

  [[nodiscard]] virtual auto id() const -> TypeID = 0;
};

using TypePtr = std::unique_ptr<Type>;

template<typename Derived, TypeID ID>
class TypeBase : public Type
{
public:
  ~TypeBase() override = default;

  auto id() const -> TypeID override { return ID; }
};

class FloatType final : public TypeBase<FloatType, TypeID::float_>
{
public:
};

class IntType final : public TypeBase<IntType, TypeID::int_>
{
public:
};

class StringType final : public TypeBase<StringType, TypeID::string>
{
public:
};

// expr

class FloatLiteralExpr;
class StringLiteralExpr;
class IntLiteralExpr;
class AddExpr;
class MulExpr;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void visit(const IntLiteralExpr&) = 0;

  virtual void visit(const FloatLiteralExpr&) = 0;

  virtual void visit(const StringLiteralExpr&) = 0;

  virtual void visit(const AddExpr&) = 0;

  virtual void visit(const MulExpr&) = 0;
};

class Expr
{
public:
  virtual ~Expr() = default;

  virtual void accept(ExprVisitor& v) const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

template<typename Derived>
class ExprBase : public Expr
{
public:
  void accept(ExprVisitor& v) const override { v.visit(static_cast<const Derived&>(*this)); }
};

template<typename Derived>
class LiteralExpr : public ExprBase<Derived>
{
  const Token* token_;

public:
  LiteralExpr(const Token* token)
    : token_(token)
  {
  }

  [[nodiscard]] auto token() const -> const Token& { return *token_; }
};

class IntLiteralExpr final : public LiteralExpr<IntLiteralExpr>
{
public:
  using LiteralExpr<IntLiteralExpr>::LiteralExpr;
};

class FloatLiteralExpr final : public LiteralExpr<FloatLiteralExpr>
{
public:
  using LiteralExpr<FloatLiteralExpr>::LiteralExpr;
};

class StringLiteralExpr final : public LiteralExpr<StringLiteralExpr>
{
public:
  using LiteralExpr<StringLiteralExpr>::LiteralExpr;
};

template<typename Derived>
class BinaryExpr : public ExprBase<Derived>
{
  ExprPtr left_;

  ExprPtr right_;

  const Token* op_token_;

public:
  BinaryExpr(ExprPtr left, ExprPtr right, const Token* op_token)
    : left_(std::move(left))
    , right_(std::move(right))
    , op_token_(op_token)
  {
  }

  ~BinaryExpr() override = default;

  [[nodiscard]] auto left() const -> const Expr& { return *left_; }

  [[nodiscard]] auto right() const -> const Expr& { return *right_; }

  [[nodiscard]] auto op_token() const -> const Token& { return *op_token_; }
};

class AddExpr final : public BinaryExpr<AddExpr>
{
public:
  using BinaryExpr<AddExpr>::BinaryExpr;
};

class MulExpr final : public BinaryExpr<MulExpr>
{
public:
  using BinaryExpr<MulExpr>::BinaryExpr;
};

// nodes

class PrintNode;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit(const PrintNode&) = 0;
};

class Node
{
public:
  virtual ~Node() = default;

  virtual void accept(NodeVisitor&) const = 0;
};

using NodePtr = std::unique_ptr<Node>;

template<typename Derived>
class NodeBase : public Node
{
public:
  ~NodeBase() override = default;

  void accept(NodeVisitor& v) const override { v.visit(static_cast<const Derived&>(*this)); }
};

class PrintNode : public NodeBase<PrintNode>
{
  std::vector<ExprPtr> args_;

public:
  explicit PrintNode(std::vector<ExprPtr> args)
    : args_(std::move(args))
  {
  }

  [[nodiscard]] auto args() const -> const std::vector<ExprPtr>& { return args_; }
};

} // namespace nabla
