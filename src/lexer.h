#pragma once

#include <string_view>

#include "diagnostics.h"

namespace nabla {

enum class TokenKind
{
  none,
  space,
  comment,
  incomplete_comment,
  identifier,
  string_literal,
  incomplete_string_literal,
  float_literal,
  int_literal,
  symbol
};

using TK = TokenKind;

struct Token final
{
  TokenKind kind{ TK::none };

  std::string_view data;

  size_t line{ 1 };

  size_t column{ 1 };

  [[nodiscard]] auto operator<(const Token& other) const -> bool { return data < other.data; }

  [[nodiscard]] auto operator<(const std::string_view& other) const -> bool { return data < other; }

  [[nodiscard]] auto operator==(const char c) const -> bool { return (data.size() == 1) && (data[0] == c); }

  [[nodiscard]] auto operator!=(const char c) const -> bool { return (data.size() != 1) || (data[0] != c); }

  [[nodiscard]] auto operator==(const std::string_view& str) const -> bool { return data == str; }

  [[nodiscard]] auto operator!=(const std::string_view& str) const -> bool { return data != str; }

  [[nodiscard]] auto operator==(const TokenKind k) const -> bool { return kind == k; }

  [[nodiscard]] auto operator!=(const TokenKind k) const -> bool { return kind != k; }
};

class Lexer final
{
  std::string_view source_;

  size_t offset_{ 0 };

  size_t line_{ 1 };

  size_t column_{ 1 };

public:
  explicit Lexer(const std::string_view& source)
    : source_(source)
  {
  }

  [[nodiscard]] auto eof() const -> bool { return offset_ >= source_.size(); }

  [[nodiscard]] auto scan() -> Token
  {
    if (eof()) {
      return Token();
    }

    const auto first = at(0);
    if ((first == ' ') || (first == '\t') || (first == '\r') || (first == '\n')) {
      return produce(TK::space, 1);
    }

    if ((first == '/') && (at(1) == '/')) {
      size_t len = 2;
      while (in_bounds(len)) {
        if (at(len) == '\n') {
          break;
        }
        len++;
      }
      return produce(TK::comment, len);
    }

    if ((first == '/') && (at(1) == '*')) {
      size_t len = 2;
      auto terminated{ false };
      while (in_bounds(len)) {
        if ((at(len) == '*') && (at(len + 1) == '/')) {
          len += 2;
          terminated = true;
          break;
        }
        len++;
      }
      if (!terminated) {
        return produce(TK::incomplete_comment, 2);
      }
      return produce(TK::comment, len);
    }

    if (is_nondigit(first)) {
      size_t len = 1;
      while (true) {
        const auto c = at(len);
        if (is_digit(c) || is_nondigit(c)) {
          len++;
        } else {
          break;
        }
      }
      return produce(TK::identifier, len);
    }

    if (is_digit(first)) {
      return scan_number(1, /*is_float=*/false);
    }

    if ((first == '.') && is_digit(at(1))) {
      return scan_number(2, /*is_float=*/true);
    }

    if ((first == '"') || (first == '\'')) {
      size_t len = 1;
      while (true) {
        if (!in_bounds(len)) {
          return produce(TK::incomplete_string_literal, 1);
        }
        // TODO : escape sequences
        const auto c = at(len);
        len++;
        if (c == first) {
          break;
        }
      }
      return produce(TK::string_literal, len);
    }

    return produce(TK::symbol, 1);
  }

protected:
  [[nodiscard]] static auto is_digit(const char c) -> bool { return (c >= '0') && (c <= '9'); }

  [[nodiscard]] static auto is_nondigit(const char c) -> bool
  {
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_');
  }

  [[nodiscard]] auto in_bounds(const size_t offset) const -> bool { return (offset_ + offset) < source_.size(); }

  [[nodiscard]] auto at(const size_t offset) const -> char
  {
    const auto o = offset_ + offset;
    return (o < source_.size()) ? source_[o] : 0;
  }

  [[nodiscard]] auto produce(TK kind, const size_t len) -> Token
  {
    const Token token{ kind, source_.substr(offset_, len), line_, column_ };

    for (size_t i = 0; i < len; i++) {
      const auto c = at(i);
      if (c == '\n') {
        line_++;
        column_ = 1;
      } else {
        column_++;
      }
    }

    offset_ += len;

    return token;
  }

  [[nodiscard]] auto scan_number(size_t len, bool is_float) -> Token
  {
    while (isdigit(at(len))) {
      len++;
    }

    if (at(len) == '.') {
      if (is_float) {
        // A second . is probably an error.
        // For simplicity, we'll just terminate the token where
        // and continue scanning after this point.
        return produce(TK::float_literal, len);
      }
      is_float = true;
      len++;
      while (isdigit(at(len))) {
        len++;
      }
    }

    // Scientific notation
    const auto exp = at(len);
    if ((exp == 'e') || (exp == 'E')) {
      is_float = true;
      len++;
      const auto sign = at(len);
      if ((sign == '+') || (sign == '-')) {
        len++;
      }
      if (!isdigit(at(len))) {
        // This is likely an error - the user isn't specifying a digit
        // after they specifiy the 'e' notation. We'll just ignore it for now
        // in order to keep things simple. Should probably produce an error in the future.
        return produce(TK::float_literal, len);
      }
      while (isdigit(at(len))) {
        len++;
      }
    }

    return is_float ? produce(TK::float_literal, len) : produce(TK::int_literal, len);
  }
};

} // namespace nabla
