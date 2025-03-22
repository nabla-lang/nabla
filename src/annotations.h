#pragma once

#include <map>

#include "syntax_tree.h"

namespace nabla {

template<typename Object>
struct Annotation final
{};

template<>
struct Annotation<AddExpr> final
{
  TypePtr result_type;

  enum class Op
  {
    none,
    add_float,
    add_int
  };

  Op op{ Op::none };
};

template<>
struct Annotation<MulExpr> final
{
  TypePtr result_type;

  enum class Op
  {
    none,
    mul_float,
    mul_int
  };

  Op op{ Op::none };
};

template<typename Object>
using AnnotationMap = std::map<const Object*, Annotation<Object>, std::less<>>;

struct AnnotationTable final
{
  AnnotationMap<AddExpr> add_expr;

  AnnotationMap<MulExpr> mul_expr;

  auto resolve_type(const Expr& expr) const -> const Type*;
};

} // namespace nabla
