#pragma once

#include <memory>

namespace nabla {

class ASTWriter
{
public:
  static auto create() -> std::unique_ptr<ASTWriter>;

  virtual ~ASTWriter() = default;
};

} // namespace nabla
