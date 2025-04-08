#include "generator.h"

#include "code_writer.h"

#include <string.h>

namespace nabla::codegen {

namespace {

class GeneratorImpl final : public Generator
{
  std::unique_ptr<CodeWriter> writer_;

public:
  GeneratorImpl(std::unique_ptr<CodeWriter> writer)
    : writer_(std::move(writer))
  {
  }

  void generate(const SyntaxTree& tree) override
  {
    for (const auto& node : tree.nodes) {
      node->accept(*writer_);
    }
  }

  auto source() const -> std::string override { return writer_->source(); }
};

} // namespace

auto
Generator::create(const char* lang, const AnnotationTable* annotations) -> std::unique_ptr<Generator>
{
  std::unique_ptr<CodeWriter> writer;

  if ((strcmp(lang, "cxx") == 0) || (strcmp(lang, "c++") == 0) || (strcmp(lang, "cpp") == 0)) {
    writer = std::make_unique<CXXCodeWriter>(annotations);
  }

  return std::make_unique<GeneratorImpl>(std::move(writer));
}

} // namespace nabla::codegen
