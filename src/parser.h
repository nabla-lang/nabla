#pragma once

#include "lexer.h"
#include "syntax_tree.h"

#include <memory>

#include <stddef.h>

namespace nabla {

class Parser
{
public:
  static auto create(const Token* tokens, const size_t num_tokens) -> std::unique_ptr<Parser>;

  virtual ~Parser() = default;

  [[nodiscard]] virtual auto eof() const -> bool = 0;

  [[nodiscard]] virtual auto parse() -> NodePtr = 0;
};

} // namespace nabla
