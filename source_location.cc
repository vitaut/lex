// Copyright (c) 2022 - present, Victor Zverovich
//
// For the license information refer to lexer.h.

#include "source_location.h"

#include <algorithm>

namespace lex {
namespace {

class file {
 private:
  FILE* file_;

 public:
  file(const char* filename, const char* mode) : file_(fopen(filename, mode)) {
    if (!file_)
      throw std::runtime_error(std::string("failed to open file: ") + filename);
  }
  ~file() {
    int result = fclose(file_);
    assert(result == 0);
  }

  file(const file&) = delete;
  void operator=(const file&) = delete;

  auto read(void* buffer, size_t size, size_t count) -> size_t {
    size_t result = fread(buffer, size, count, file_);
    if (result == 0 && !feof(file_))
      throw std::runtime_error("error reading from file");
    return result;
  }
};

}  // namespace

auto source_manager::from_file(const char* path) -> source {
  auto src = source_info();
  src.file_name = path;

  auto f = file(path, "rb");
  constexpr size_t buffer_size = 4096;
  char buffer[buffer_size];
  while (size_t count = f.read(buffer, 1, buffer_size))
    src.text.insert(src.text.end(), buffer, buffer + count);
  src.text.push_back('\0');
  auto text = std::string_view(src.text.data(), src.text.size());

  src.line_offsets.push_back(0);
  auto begin = text.data(), end = begin + text.size() - 1;
  auto ptr = begin;
  for (;;) {
    ptr = static_cast<const char*>(memchr(ptr, '\n', end - ptr));
    if (!ptr) break;
    ++ptr;
    src.line_offsets.push_back(ptr - begin);
  }

  sources_.push_back(std::move(src));
  return {source_location(sources_.size(), 0), text};
}

resolved_location::resolved_location(source_location loc, source_manager& sm) {
  assert(loc.source_id_ != 0);
  const auto& source = sm.sources_[loc.source_id_ - 1];
  file_name_ = source.file_name.c_str();
  const auto& line_offsets = source.line_offsets;
  auto it =
      std::upper_bound(line_offsets.begin(), line_offsets.end(), loc.offset_);
  line_ = it != line_offsets.end() ? it - line_offsets.begin()
                                   : line_offsets.size();
  column_ = loc.offset_ - line_offsets[line_ - 1] + 1;
}

}  // namespace lex
