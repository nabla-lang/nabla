#include "ast_writer.h"

namespace nabla {

namespace {

class ASTWriterImpl final : public ASTWriter
{
public:
};

} // namespace

auto
ASTWriter::create() -> std::unique_ptr<ASTWriter>
{
  return std::make_unique<ASTWriterImpl>();
}

} // namespace nabla
