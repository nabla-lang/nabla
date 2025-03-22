#pragma once

#include "annotator.h"

#include "annotators/add_expr.h"

namespace nabla {

[[nodiscard]] auto
annotate(const std::vector<NodePtr>& nodes) -> AnnotationTable;

} // namespace nabla
