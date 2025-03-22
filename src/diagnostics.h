#pragma once

#include <stdexcept>
#include <string>

namespace nabla {

class Token;

enum class Severity
{
  error,
  fatal_error
};

struct Diagnostic final
{
  std::string what;

  const Token* token{ nullptr };
};

class FatalError final : public std::runtime_error
{
  Diagnostic diagnostic_;

public:
  explicit FatalError(Diagnostic diagnostic)
    : std::runtime_error(diagnostic.what)
    , diagnostic_(std::move(diagnostic))
  {
  }

  [[nodiscard]] auto diagnostic() const -> const Diagnostic& { return diagnostic_; }
};

} // namespace nabla
