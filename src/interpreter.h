#pragma once

#include "ast.h"

#include <iostream>
#include <memory>
#include <string>

namespace nabla {

class Runtime
{
public:
  virtual ~Runtime() = default;

  virtual void print(const std::string& data) { std::cout << data; }

  virtual void print(const int data) { std::cout << data; }

  virtual void print(const float data) { std::cout << data; }

  virtual void print_end() { std::cout << std::endl; }
};

class Interpreter
{
public:
  static auto create(Runtime* runtime) -> std::unique_ptr<Interpreter>;

  virtual ~Interpreter() = default;

  virtual void exec(const ast::Module& m) = 0;
};

} // namespace nabla
