#pragma once

#include "../annotator.h"
#include "../syntax_tree.h"

namespace nabla {

class AddExprAnnotator final : public Annotator<AddExpr>
{
public:
  auto annotate(const AddExpr&, AnnotationType& annotation, AnnotationTable& table) -> bool override;
};

} // namespace nabla
