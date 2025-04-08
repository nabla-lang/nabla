#pragma once

#include "../annotator.h"
#include "../syntax_tree.h"

namespace nabla {

class VarExprAnnotator final : public Annotator<VarExpr>
{
  const SyntaxTree* tree_{ nullptr };

public:
  explicit VarExprAnnotator(const SyntaxTree& tree)
    : tree_(&tree)
  {
  }

  auto annotate(const VarExpr&, AnnotationType& annotation, AnnotationTable& table) -> bool override;
};

} // namespace nabla
