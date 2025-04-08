#pragma once

#include "annotator.h"

namespace nabla {

[[nodiscard]] auto
annotate(const SyntaxTree& tree) -> AnnotationTable;

} // namespace nabla
