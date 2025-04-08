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
  string,
  struct_
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

class DeclNode;

class StructType final : public TypeBase<StructType, TypeID::struct_>
{
  std::vector<std::unique_ptr<DeclNode>> fields_;

public:
  explicit StructType(std::vector<std::unique_ptr<DeclNode>> fields)
    : fields_(std::move(fields))
  {
  }

  [[nodiscard]] auto fields() const -> const std::vector<std::unique_ptr<DeclNode>>& { return fields_; }
};

// expr

class FloatLiteralExpr;
class StringLiteralExpr;
class IntLiteralExpr;
class VarExpr;
class CallExpr;
class AddExpr;
class MulExpr;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void visit(const IntLiteralExpr&) = 0;

  virtual void visit(const FloatLiteralExpr&) = 0;

  virtual void visit(const StringLiteralExpr&) = 0;

  virtual void visit(const VarExpr&) = 0;

  virtual void visit(const CallExpr&) = 0;

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

class VarExpr final : public ExprBase<VarExpr>
{
  const Token* name_{ nullptr };

public:
  explicit VarExpr(const Token& name)
    : name_(&name)
  {
  }

  [[nodiscard]] auto get_name() const -> const Token& { return *name_; }
};

class CallExpr final : public ExprBase<CallExpr>
{
  const Token* name_{ nullptr };

  std::vector<std::pair<const Token*, ExprPtr>> args_;

public:
  /// @brief A type alias for named arguments.
  ///
  /// @note This is also used for positional arguments, where the name is left out.
  ///       When the name is left out, the token pointer is null.
  using NamedArg = std::pair<const Token*, ExprPtr>;

  CallExpr(const Token* name, std::vector<NamedArg> args)
    : name_(name)
    , args_(std::move(args))
  {
  }

  [[nodiscard]] auto name() const -> const Token& { return *name_; }

  [[nodiscard]] auto args() const -> const std::vector<NamedArg>& { return args_; }
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
class DeclNode;
class FuncNode;
class ReturnNode;
class StructNode;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;

  virtual void visit(const PrintNode&) = 0;

  virtual void visit(const DeclNode&) = 0;

  virtual void visit(const FuncNode&) = 0;

  virtual void visit(const StructNode&) = 0;

  virtual void visit(const ReturnNode&) = 0;
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

class TypeInstance final
{
  const Token* name_{ nullptr };

  std::vector<ExprPtr> args_;

public:
  TypeInstance(const Token* name, std::vector<ExprPtr> args)
    : name_(name)
    , args_(std::move(args))
  {
  }

  [[nodiscard]] auto name() const -> const Token& { return *name_; }

  [[nodiscard]] auto args() const -> const std::vector<ExprPtr>& { return args_; }
};

class DeclNode final : public NodeBase<DeclNode>
{
  const Token* name_{ nullptr };

  /// @brief This is the value used to initialize the declaration.
  ///
  /// @note For function parameters and struct fields, this might be null.
  ExprPtr value_{ nullptr };

  const bool immutable_{ true };

  /// @brief If the declaration node has a type annotation, it is placed here.
  ///
  /// @note This field might be null if the type is inferred.
  std::unique_ptr<TypeInstance> type_{ nullptr };

public:
  DeclNode(const Token& name, ExprPtr value, const bool immutable, std::unique_ptr<TypeInstance> type = nullptr)
    : name_(&name)
    , value_(std::move(value))
    , immutable_(immutable)
    , type_(std::move(type))
  {
  }

  [[nodiscard]] auto get_name() const -> const Token& { return *name_; }

  [[nodiscard]] auto get_value() const -> const Expr& { return *value_; }

  [[nodiscard]] auto has_value() const -> bool { return value_.get() != nullptr; }

  [[nodiscard]] auto get_type() const -> const TypeInstance& { return *type_; }

  [[nodiscard]] auto has_type() const -> bool { return type_.get() != nullptr; }

  [[nodiscard]] auto is_immutable() const -> bool { return immutable_; }
};

class FuncNode final : public NodeBase<FuncNode>
{
  const Token* name_{ nullptr };

  std::vector<std::unique_ptr<DeclNode>> params_;

  std::vector<NodePtr> body_;

public:
  FuncNode(const Token* name, std::vector<std::unique_ptr<DeclNode>> params, std::vector<NodePtr> body)
    : name_(name)
    , params_(std::move(params))
    , body_(std::move(body))
  {
  }

  [[nodiscard]] auto name() const -> const Token& { return *name_; }

  [[nodiscard]] auto params() const -> const std::vector<std::unique_ptr<DeclNode>>& { return params_; }

  [[nodiscard]] auto body() const -> const std::vector<NodePtr>& { return body_; }
};

class StructNode final : public NodeBase<StructNode>
{
  const Token* name_{ nullptr };

  std::vector<std::unique_ptr<DeclNode>> fields_;

public:
  StructNode(const Token* name, std::vector<std::unique_ptr<DeclNode>> fields)
    : name_(name)
    , fields_(std::move(fields))
  {
  }

  [[nodiscard]] auto name() const -> const Token& { return *name_; }

  [[nodiscard]] auto fields() const -> const std::vector<std::unique_ptr<DeclNode>>& { return fields_; }
};

class ReturnNode final : public NodeBase<ReturnNode>
{
  ExprPtr value_;

public:
  explicit ReturnNode(ExprPtr value)
    : value_(std::move(value))
  {
  }

  [[nodiscard]] auto value() const -> const Expr& { return *value_; }
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

struct SyntaxTree final
{
  std::vector<NodePtr> nodes;
};

} // namespace nabla
