cmake_minimum_required(VERSION 3.18)

project(nabla)

add_executable(nabla
  src/main.cpp
  src/ast_builder.h
  src/ast_builder.cpp
  src/parser.h
  src/parser.cpp
  src/syntax_tree.h
  src/syntax_tree.cpp
  src/validator.h
  src/validator.cpp
  src/interpreter.h
  src/interpreter.cpp
  src/console.h
  src/console.cpp
  src/annotations.h
  src/annotations.cpp
  src/annotate.cpp
  src/annotators/add_expr.h
  src/annotators/add_expr.cpp
  src/annotators/mul_expr.h
  src/annotators/mul_expr.cpp
  src/annotators/var_expr.h
  src/annotators/var_expr.cpp
  src/codegen/code_writer.h
  src/codegen/code_writer.cpp
  src/codegen/generator.h
  src/codegen/generator.cpp
)
