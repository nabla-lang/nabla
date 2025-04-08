#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <stdlib.h>

#include "annotate.h"
#include "codegen/generator.h"
#include "console.h"
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
      if (token == nabla::TK::incomplete_comment) {
        nabla::Diagnostic diagnostic{ "unterminated comment", &token };
        console.print_diagnostic(filename.string(), diagnostic, source);
        return false;
      }
      tokens.emplace_back(token);
    }

    auto parser = nabla::Parser::create(tokens.data(), tokens.size());

    nabla::SyntaxTree tree;

    while (!parser->eof()) {
      try {
        auto node = parser->parse();
        tree.nodes.emplace_back(std::move(node));
      } catch (const nabla::FatalError& error) {
        const auto& diagnostic = error.diagnostic();
        console.print_diagnostic(filename.string(), diagnostic, source);
        return false;
      }
    }

    const auto annotations = annotate(tree);

    auto validator = nabla::Validator::create();

    validator->validate(tree.nodes, annotations);

    for (const auto& diagnostic : validator->get_diagnostics()) {
      console.print_diagnostic(filename.string(), diagnostic, source);
    }

    if (validator->failed()) {
      return false;
    }

    auto generator = nabla::codegen::Generator::create("c++", &annotations);

    generator->generate(tree);

    std::cout << generator->source();

    return true;
  }

protected:
  [[nodiscard]] static auto read_file(std::ifstream& file) -> std::string
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

  if (!std::filesystem::exists("src")) {
    console->print_error("no src/ directory exists in the current directory");
    return EXIT_FAILURE;
  }

  std::vector<std::filesystem::path> directory_queue{ "src", "deps" };

  while (!directory_queue.empty()) {

    std::filesystem::path current = directory_queue[0];

    directory_queue.erase(directory_queue.begin());

    if (!std::filesystem::exists(current)) {
      continue;
    }

    for (const auto& entry : std::filesystem::directory_iterator(current)) {
      if (entry.is_regular_file()) {
        if (entry.path().extension() != ".nabla") {
          continue;
        }
        if (!program.compile(entry, *console)) {
          return EXIT_FAILURE;
        }
      }

      if (entry.is_directory()) {
        directory_queue.emplace_back(current / entry);
      }
    }
  }

  return EXIT_FAILURE;
}
