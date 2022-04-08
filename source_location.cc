// Copyright (c) 2022 - present, Victor Zverovich
//
// For the license information refer to lexer.h.

#include "source_location.h"

#include <algorithm>

namespace lex {

resolved_location::resolved_location(source_location loc, source_manager& sm) {
  auto id = static_cast<uint_least32_t>(loc.source_);
  assert(id != 0);
  const auto& source = sm.sources_[id - 1];
  file_name_ = source.file_name.c_str();
  const auto& line_offsets = source.line_offsets;
  auto it =
      std::lower_bound(line_offsets.begin(), line_offsets.end(), loc.offset_);
  line_ = it != line_offsets.end() ? it - line_offsets.begin()
                                   : line_offsets.size();
  column_ = line_offsets[line_ - 1];
}

}  // namespace lex
