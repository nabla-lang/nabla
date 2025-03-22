#pragma once

#include <iosfwd>
#include <memory>
#include <string_view>

namespace nabla {

struct Diagnostic;

class Console
{
public:
  static auto create(std::ostream* output) -> std::unique_ptr<Console>;

  virtual ~Console() = default;

  virtual void set_color_enabled(bool enabled) = 0;

  virtual void set_program_name(const std::string_view& arg0) = 0;

  virtual void print_error(const std::string_view& what) = 0;

  virtual void print_file_error(const std::string_view& filename, const std::string_view& what) = 0;

  virtual void print_diagnostic(const std::string_view& filename,
                                const Diagnostic& diagnostic,
                                const std::string_view& source) = 0;
};

} // namespace nabla
