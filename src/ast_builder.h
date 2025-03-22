#pragma once

#include "annotations.h"
#include "ast.h"
#include "syntax_tree.h"

namespace nabla {

class ASTBuilder
{
public:
  static auto create(ast::Module* m, const AnnotationTable* annotations) -> std::unique_ptr<ASTBuilder>;

  virtual ~ASTBuilder() = default;

  [[nodiscard]] virtual auto build(const Node& node) -> bool = 0;
};

} // namespace nabla
