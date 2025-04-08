#pragma once

#include <memory>
#include <string>

namespace nabla {

struct SyntaxTree;
struct AnnotationTable;

} // namespace nabla

namespace nabla::codegen {

class Generator
{
public:
  static auto create(const char* lang, const AnnotationTable* annotations) -> std::unique_ptr<Generator>;

  virtual ~Generator() = default;

  virtual void generate(const SyntaxTree& m) = 0;

  virtual auto source() const -> std::string = 0;
};

} // namespace nabla::codegen
