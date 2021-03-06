/*
  Copyright (c) 2022 - present, Victor Zverovich

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <string_view>

#include "source_location.h"

namespace lex {

enum class token_kind { unknown, number, string, identifier, eof };

struct token {
  token_kind kind;
  source_location loc;
};

class lexer {
 private:
  std::string_view source_;
  source_location start_;
  const char* ptr_;
  const char* token_start_ = nullptr;

  auto end() const -> const char* { return source_.data() + source_.size(); }

  void skip_line_comment();
  auto skip_block_comment() -> bool;
  void skip_whitespace_or_comment();
  auto lex_doc_comment() -> bool;

 public:
  explicit lexer(source src);

  auto get_next_token() -> token;

  auto token_string() const -> std::string_view {
    return {token_start_, static_cast<size_t>(ptr_ - token_start_)};
  }
};

}  // namespace lex
