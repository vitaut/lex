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
#include <vector>

namespace lex {

enum class token { unknown, number, string, identifier, eof };

class lexer {
 private:
  std::vector<char> data_;
  const char* ptr_;
  const char* token_start_ = nullptr;

  const char* end() const { return data_.data() + data_.size(); }

  void skip_line_comment();
  auto skip_block_comment() -> bool;
  void skip_whitespace_or_comment();
  auto lex_doc_comment() -> bool;

 public:
  explicit lexer(const char* path);

  auto get_next_token() -> token;

  auto token_string() const -> std::string_view {
    return {token_start_, static_cast<size_t>(ptr_ - token_start_)};
  }
};

}  // namespace lex
