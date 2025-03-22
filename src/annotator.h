#pragma once

#include "annotations.h"

namespace nabla {

template<typename Object>
class Annotator
{
public:
  using AnnotationType = Annotation<Object>;

  virtual ~Annotator() = default;

  [[nodiscard]] virtual auto annotate(const Object& object, AnnotationType& annotation, AnnotationTable& table)
    -> bool = 0;
};

} // namespace nabla
