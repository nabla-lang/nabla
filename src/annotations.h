#pragma once

#include <map>

#include <stdint.h>

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

template<>
struct Annotation<VarExpr> final
{
  const DeclNode* decl{ nullptr };
};

template<>
struct Annotation<DeclNode> final
{
  const Type* type{ nullptr };
};

template<>
struct Annotation<TypeInstance> final
{
  TypePtr type;

  std::vector<int32_t> args;
};

template<typename Object>
using AnnotationMap = std::map<const Object*, Annotation<Object>, std::less<>>;

struct AnnotationTable final
{
  AnnotationMap<AddExpr> add_expr;

  AnnotationMap<MulExpr> mul_expr;

  AnnotationMap<VarExpr> var_expr;

  AnnotationMap<DeclNode> decl_node;

  AnnotationMap<TypeInstance> type_instances;

  auto resolve_type(const Expr& expr) const -> const Type*;
};

} // namespace nabla
