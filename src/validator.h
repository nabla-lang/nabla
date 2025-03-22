#pragma once

#include <memory>
#include <vector>

#include "annotations.h"
#include "diagnostics.h"
#include "syntax_tree.h"

namespace nabla {

/// @brief Checks that the syntax tree is without errors before converting it to an AST.
class Validator
{
public:
  static auto create() -> std::unique_ptr<Validator>;

  virtual ~Validator() = default;

  [[nodiscard]] virtual auto get_diagnostics() -> std::vector<Diagnostic> = 0;

  virtual void validate(const std::vector<NodePtr>& nodes, const AnnotationTable& annotations) = 0;

  [[nodiscard]] virtual auto failed() const -> bool = 0;
};

} // namespace nabla
