#pragma once

#include "../annotations.h"
#include "../syntax_tree.h"

#include <string>
#include <string_view>

#include <stddef.h>

namespace nabla::codegen {

class CodeWriter
  : public ExprVisitor
  , public NodeVisitor
{
  size_t indent_{ 0 };

  std::string source_;

  const AnnotationTable* annotations_{ nullptr };

public:
  CodeWriter(const AnnotationTable* annotations)
    : annotations_(annotations)
  {
  }

  virtual ~CodeWriter() = default;

  [[nodiscard]] auto source() const -> std::string;

  void indent();

  void dedent();

  void add_line(const std::string_view& line);

  void write(const std::string_view& str);

  void newline();

protected:
  [[nodiscard]] auto annotations() const -> const AnnotationTable& { return *annotations_; }

  void visit(const IntLiteralExpr& expr) override;

  void visit(const FloatLiteralExpr& expr) override;

  void visit(const StringLiteralExpr&) override {}

  void visit(const AddExpr& expr) override;

  void visit(const MulExpr& expr) override;

  void visit(const VarExpr& expr) override;

  void visit(const CallExpr& expr) override;
};

class CXXCodeWriter final : public CodeWriter
{
public:
  CXXCodeWriter(const AnnotationTable* annotations)
    : CodeWriter(annotations)
  {
  }

protected:
  void visit(const FuncNode& node) override;

  void visit(const DeclNode& node) override;

  void visit(const StructNode& node) override;

  void visit(const PrintNode&) override {}

  void visit(const ReturnNode&) override {}
};

} // namespace nabla::codegen
