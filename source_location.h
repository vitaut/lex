// Copyright (c) 2022 - present, Victor Zverovich
//
// For the license information refer to lexer.h.

#include <stdint.h>

#include <string>
#include <string_view>
#include <vector>

namespace lex {

struct source;

class source_manager {
 private:
  struct source_info {
    std::string file_name;
    std::vector<char> text;
    std::vector<uint_least32_t> line_offsets;  // Lazily computed line offsets.
  };
  std::vector<source_info> sources_;

  friend class resolved_location;

 public:
  auto from_file(const char* path) -> source;
};

// A lightweight source location that can be resolved via source_manager.
class source_location {
 private:
  uint_least32_t source_id_;
  uint_least32_t offset_;

  friend class resolved_location;
  friend class source_manager;

  source_location(uint_least32_t source_id, uint_least32_t offset)
      : source_id_(source_id), offset_(offset) {}
};

// A resolved (source) location that provides the file name, line and column.
class resolved_location {
 private:
  const char* file_name_;
  uint_least32_t line_;
  uint_least32_t column_;

 public:
  resolved_location(source_location loc, source_manager& sm);

  // Returns the source file name. It can include directory components and/or be
  // a virtual file name that doesn't have a correspondent entry in the system's
  // directory structure.
  auto file_name() const -> const char* { return file_name_; }

  auto line() const -> uint_least32_t { return line_; }
  auto column() -> uint_least32_t const { return column_; }
};

struct source {
  source_location start;
  std::string_view text;
};

}  // namespace lex