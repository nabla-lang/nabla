#pragma once

#include "../annotator.h"
#include "../syntax_tree.h"

namespace nabla {

class MulExprAnnotator final : public Annotator<MulExpr>
{
public:
  auto annotate(const MulExpr& expr, AnnotationType& annotation, AnnotationTable& table) -> bool override;
};

} // namespace nabla
