#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <stdlib.h>

#include "annotate.h"
#include "ast.h"
#include "ast_builder.h"
#include "console.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "validator.h"

namespace {

class Program final
{
public:
  [[nodiscard]] auto compile(const std::filesystem::path& filename, nabla::Console& console) -> bool
  {
    std::ifstream file(filename);
    if (!file.good()) {
      return false;
    }

    const auto source = read_file(file);

    nabla::Lexer lexer(source);

    std::vector<nabla::Token> tokens;

    while (!lexer.eof()) {
      const auto token = lexer.scan();
      if ((token == nabla::TK::comment) || (token == nabla::TK::space)) {
        continue;
      }
      if (token == nabla::TK::incomplete_string_literal) {
        nabla::Diagnostic diagnostic{ "unterminated string", &token };
        console.print_diagnostic(filename.string(), diagnostic, source);
        return false;
      }
      tokens.emplace_back(token);
    }

    auto parser = nabla::Parser::create(tokens.data(), tokens.size());

    std::vector<nabla::NodePtr> nodes;

    while (!parser->eof()) {
      try {
        auto node = parser->parse();
        nodes.emplace_back(std::move(node));
      } catch (const nabla::FatalError& error) {
        const auto& diagnostic = error.diagnostic();
        console.print_diagnostic(filename.string(), diagnostic, source);
        return false;
      }
    }

    const auto annotations = annotate(nodes);

    auto validator = nabla::Validator::create();

    validator->validate(nodes, annotations);

    for (const auto& diagnostic : validator->get_diagnostics()) {
      console.print_diagnostic(filename.string(), diagnostic, source);
    }

    if (validator->failed()) {
      return false;
    }

    nabla::ast::Module mod;

    auto ast_builder = nabla::ASTBuilder::create(&mod, &annotations);

    for (auto& node : nodes) {
      if (!ast_builder->build(*node)) {
        return false;
      }
    }

    nabla::Runtime runtime;

    auto interpreter = nabla::Interpreter::create(&runtime);

    interpreter->exec(mod);

    return true;
  }

protected:
  [[nodiscard]] auto read_file(std::ifstream& file) -> std::string
  {
    file.seekg(0, std::ios::end);

    const auto file_size = file.tellg();

    file.seekg(0, std::ios::beg);

    std::string source;

    source.resize(file_size);

    file.read(source.data(), source.size());

    return source;
  }
};

} // namespace

auto
main(int, char** argv) -> int
{
  Program program;

  auto console = nabla::Console::create(&std::cout);

  console->set_program_name(argv[0]);

  console->set_color_enabled(true);

  for (const auto& entry : std::filesystem::directory_iterator(".")) {
    if (entry.is_regular_file()) {
      if (entry.path().extension() != ".nabla") {
        continue;
      }
      if (!program.compile(entry, *console)) {
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_FAILURE;
}
