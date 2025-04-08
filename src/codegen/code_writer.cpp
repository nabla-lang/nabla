#include "code_writer.h"

#include "../lexer.h"

#include <sstream>

#include <assert.h>

namespace nabla::codegen {

auto
CodeWriter::source() const -> std::string
{
  return source_;
}

void
CodeWriter::indent()
{
  indent_++;
}

void
CodeWriter::dedent()
{
  if (indent_ > 0) {
    indent_--;
  }
}

void
CodeWriter::add_line(const std::string_view& line)
{
  for (size_t i = 0; i < indent_; i++) {
    source_ += "  ";
  }

  source_ += line;

  source_ += '\n';
}

void
CodeWriter::write(const std::string_view& str)
{
  source_ += str;
}

void
CodeWriter::newline()
{
  source_ += '\n';
}

void
CodeWriter::visit(const IntLiteralExpr& expr)
{
  source_ += expr.token().data;
}

void
CodeWriter::visit(const FloatLiteralExpr& expr)
{
  source_ += expr.token().data;
}

void
CodeWriter::visit(const AddExpr& expr)
{
  expr.left().accept(*this);
  source_ += " + ";
  expr.right().accept(*this);
}

void
CodeWriter::visit(const MulExpr& expr)
{
  expr.left().accept(*this);
  source_ += " * ";
  expr.right().accept(*this);
}

void
CodeWriter::visit(const VarExpr& expr)
{
  source_ += expr.get_name().data;
}

void
CodeWriter::visit(const CallExpr& expr)
{
  source_ += expr.name().data;
  source_ += "(";
  const auto& args = expr.args();
  for (size_t i = 0; i < args.size(); i++) {

    // TODO : verify name order
    assert(args[i].first == nullptr);

    args[i].second->accept(*this);
    const auto last = (i + 1) == args.size();
    if (!last) {
      source_ += ", ";
    }
  }

  source_ += ")";
}

namespace {

#if 0
class CXXTypeWriter final : public TypeVisitor
  {
    public:
  };
#endif

} // namespace

void
CXXCodeWriter::visit(const StructNode& node)
{
  add_line("struct " + std::string(node.name().data) + " final {");
  indent();
  for (const auto& field : node.fields()) {
    std::ostringstream stream;
    stream << field->get_type().name().data;
    stream << ' ';
    stream << field->get_name().data;
    stream << "{};";
    add_line(stream.str());
  }
  dedent();
  add_line("};");
}

void
CXXCodeWriter::visit(const FuncNode& node)
{
}

void
CXXCodeWriter::visit(const DeclNode& node)
{
  if (node.is_immutable()) {
    write("const ");
  }

  // TODO : type
  write("int ");

  write(node.get_name().data);

  if (node.has_value()) {
    write(" = ");
    node.get_value().accept(*this);
  }

  write(";");

  newline();
}

} // namespace nabla::codegen
