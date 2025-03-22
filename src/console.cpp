#include "console.h"

#include "diagnostics.h"
#include "lexer.h"

#include <ostream>
#include <sstream>

namespace nabla {

namespace {

class ConsoleImpl final : public Console
{
  std::string program_name_{ "nabla" };

  std::ostream* output_;

  bool color_enabled_{ false };

public:
  explicit ConsoleImpl(std::ostream* output)
    : output_(output)
  {
  }

  void set_program_name(const std::string_view& arg0) override { program_name_ = arg0; }

  void set_color_enabled(const bool enabled) override { color_enabled_ = enabled; }

  void print_error(const std::string_view& what) override
  {
    out() << program_name_ << ": error: " << what << std::endl;
  }

  void print_file_error(const std::string_view& filename, const std::string_view& what) override
  {
    out() << filename << ": error: " << what << std::endl;
  }

  void print_diagnostic(const std::string_view& filename,
                        const Diagnostic& diagnostic,
                        const std::string_view& source) override
  {
    if (!diagnostic.token) {
      print_file_error(filename, diagnostic.what);
      return;
    }

    const auto& token = *diagnostic.token;

    const auto lp = line_prefix(token.line);
    const auto ls = line_space(token.line);
    const auto cs = column_space(token.column);
    out() << lp << get_line(token.line, source) << std::endl;
    out() << ls << cs << "^" << std::string(token.data.size() - 1, '~') << std::endl;
    out() << ls << cs << std::string(token.data.size(), ' ') << '`' << diagnostic.what << std::endl;
  }

protected:
  auto out() -> std::ostream& { return *output_; }

  static auto get_line(const size_t line, const std::string_view& source) -> std::string_view
  {
    size_t l{ 1 };
    size_t start = 0;
    size_t len = 0;

    for (size_t i = 0; i < source.size(); i++) {
      const auto c = source[i];
      if (c == '\n') {
        if (l == line) {
          break;
        }
        l++;
        len = 0;
        start = i + 1;
      } else {
        len++;
      }
    }

    return source.substr(start, len);
  }

  static auto column_space(const size_t column) -> std::string
  {
    return std::string(column >= 1 ? (column - 1) : 0, ' ');
  }

  static auto line_space(const size_t line) -> std::string
  {
    std::ostringstream tmp_stream;
    tmp_stream << ' ' << line;
    const auto tmp = tmp_stream.str();

    std::ostringstream stream;
    stream << std::string(tmp.size(), ' ');
    stream << " | ";
    return stream.str();
  }

  static auto line_prefix(const size_t line) -> std::string
  {
    std::ostringstream stream;
    stream << ' ' << line << " | ";
    return stream.str();
  }
};

} // namespace

auto
Console::create(std::ostream* output) -> std::unique_ptr<Console>
{
  return std::make_unique<ConsoleImpl>(output);
}

} // namespace nabla
