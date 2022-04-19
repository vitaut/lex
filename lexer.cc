// Copyright (c) 2022 - present, Victor Zverovich
//
// For the license information refer to lexer.h.

#include "lexer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace lex {

namespace {

auto is_whitespace(char c) -> bool {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

auto is_letter(char c) -> bool {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

auto is_bin_digit(char c) -> bool { return c == '0' || c == '1'; }
auto is_oct_digit(char c) -> bool { return c >= '0' && c <= '7'; }
auto is_dec_digit(char c) -> bool { return c >= '0' && c <= '9'; }
auto is_hex_digit(char c) -> bool {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

// Lexes a decimal constant of the form [0-9]+. Returns a pointer past the end
// if the constant has been lexed; nullptr otherwise.
auto lex_dec_constant(const char* p) -> const char* {
  if (!is_dec_digit(*p)) return nullptr;
  do ++p;
  while (is_dec_digit(*p));
  return p;
}

// Lexes a float exponent of the form [eE][+-]?[0-9]+. Returns a pointer past
// the end if the exponent has been lexed; nullptr otherwise.
auto lex_float_exponent(const char* p) -> const char* {
  if (*p != 'e' && *p != 'E') return nullptr;
  ++p;
  if (*p == '+' || *p == '-') ++p;
  return lex_dec_constant(p);
}

// Lexes a float constant in the form [0-9]+ followed by an exponent. Returns a
// pointer past the end if the constant has been lexed; nullptr otherwise.
auto lex_float_constant(const char* p) -> const char* {
  p = lex_dec_constant(p);
  return p ? lex_float_exponent(p) : nullptr;
}

}  // namespace

lexer::lexer(source src) : src_(std::move(src)) { ptr_ = src_.text.data(); }

void lexer::skip_line_comment() { ptr_ = std::find(ptr_, end() - 1, '\n'); }

auto lexer::skip_block_comment() -> bool {
  assert(strncmp(ptr_, "/*", 2) == 0);
  const char* p = ptr_ + 3;  // Skip "/*.".
  for (;;) {
    p = std::find(p, end() - 1, '*');
    if (!*p) return false;  // EOF while lexing a block comment.
    ++p;                    // Skip "*".
    if (*p == '/') {
      ptr_ = p + 1;
      return true;
    }
  }
}

auto lexer::lex_doc_comment() -> bool {
  assert(strncmp(ptr_, "///", 3) == 0);
  bool is_inline = ptr_[3] == '<';
  const char* prefix = ptr_;
  size_t prefix_size = is_inline ? 4 : 3;
  bool matched = false;
  do {
    if (!is_inline && ptr_[3] == '/') return matched;
    matched = true;
    ptr_ += prefix_size;  // Skip "///" or "///<".
    skip_line_comment();
    while (is_whitespace(*ptr_)) ++ptr_;
  } while (strncmp(ptr_, prefix, prefix_size) == 0);
  return true;
}

void lexer::skip_whitespace_or_comment() {
  for (;;) {
    switch (*ptr_) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      ++ptr_;
      break;
    case '/':
      if (ptr_[1] == '/') {
        if (ptr_[2] != '/' || !lex_doc_comment()) {
          ptr_ += 2;  // Skip "//".
          skip_line_comment();
        }
        break;
      }
      if (ptr_[1] == '*' && skip_block_comment()) break;
      return;
    case '#':
      ++ptr_;
      skip_line_comment();
      break;
    default:
      return;
    }
  }
}

auto lexer::get_next_token() -> token {
  skip_whitespace_or_comment();

  token_start_ = ptr_;

  char c = *ptr_++;
  if (is_letter(c)) {
    // Lex an identifier.
    while (is_letter(*ptr_)) ++ptr_;
    return token::identifier;
  } else if (c == '.') {
    if (const char* p = lex_float_constant(ptr_)) {
      ptr_ = p;
      return token::number;
    }
  } else if (is_dec_digit(c)) {
    if (c == '0') {
      switch (*ptr_) {
      case 'x':
      case 'X':
        // Lex a hexadecimal constant.
        if (!is_hex_digit(*ptr_)) break;
        ptr_ += 2;
        while (is_hex_digit(*ptr_)) ++ptr_;
        return token::number;
      case 'b':
      case 'B':
        // Lex a binary constant.
        if (!is_bin_digit(*ptr_)) break;
        ptr_ += 2;
        while (is_bin_digit(*ptr_)) ++ptr_;
        return token::number;
      }
    }
    // Lex a decimal, octal or floating-point constant.
    const char* dec_end = lex_dec_constant(ptr_);
    if (!dec_end) dec_end = ptr_;
    switch (*dec_end) {
    case '.':
      if (const char* p = lex_float_constant(dec_end + 1)) {
        ptr_ = p;
        return token::number;
      }
      break;
    case 'e':
    case 'E':
      if (const char* p = lex_float_exponent(dec_end)) {
        ptr_ = p;
        return token::number;
      }
      break;
    }
    if (c != '0') {
      ptr_ = dec_end;
    } else {
      while (is_oct_digit(*ptr_)) ++ptr_;
    }
    return token::number;
  } else if (c == '"' || c == '\'') {
    // Lex a string literal.
    const char* p = std::find(ptr_, end() - 1, c);
    if (*p) {
      ptr_ = p + 1;
      return token::string;
    }
  } else if (!c && ptr_ == end()) {
    --ptr_;  // Put '\0' back in case get_next_token() is called again.
    return token::eof;
  }
  return token::unknown;
}

}  // namespace lex

auto main(int argc, char** argv) -> int {
  if (argc != 2) {
    printf("usage: %s FILE\n", argv[0]);
    return 0;
  }
  auto source_mgr = lex::source_manager();
  auto lexer = lex::lexer(source_mgr.from_file(argv[1]));
  auto token = lex::token();
  while ((token = lexer.get_next_token()) != lex::token::eof)
    printf("token: %d %s\n", token, std::string(lexer.token_string()).c_str());
}
