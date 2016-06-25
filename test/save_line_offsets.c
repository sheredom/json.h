// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#include "utest.h"

#include "json.h"

TESTCASE(save_line_offsets, single_line) {
  const char payload[] = "{}";
  struct json_parse_result_s parse_result;
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_save_line_offsets, 0, 0, &parse_result);

  ASSERT_EQ(1, parse_result.line_count);
  ASSERT_EQ(0, parse_result.line_offsets[0]);

  free(value);
}

TESTCASE(save_line_offsets, two_lines) {
  const char payload[] = "{\n}";
  struct json_parse_result_s parse_result;
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_save_line_offsets, 0, 0, &parse_result);

  ASSERT_EQ(2, parse_result.line_count);
  ASSERT_EQ(0, parse_result.line_offsets[0]);
  ASSERT_EQ(1, parse_result.line_offsets[1]);

  free(value);
}
