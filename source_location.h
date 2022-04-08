// Copyright (c) 2022 - present, Victor Zverovich
//
// For the license information refer to lexer.h.

#include <stdint.h>

#include <string>
#include <vector>

namespace lex {

enum class source_id : uint_least32_t {};

class source_manager {
 private:
  struct source {
    std::string file_name;
    std::vector<uint_least32_t> line_offsets;
  };
  std::vector<source> sources_;

  friend class resolved_location;
};

// A lightweight source location that can be resolved via source_manager.
class source_location {
 private:
  source_id source_;
  uint_least32_t offset_;

  friend class resolved_location;
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
  const char* file_name() const { return file_name_; }

  uint_least32_t line() const { return line_; }
  uint_least32_t column() const { return column_; }
};

}  // namespace lex