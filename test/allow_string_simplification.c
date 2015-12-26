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

TESTCASE(allow_string_simplification, quotation) {
  const char payload[] = "[ \"\\\"\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\"", string->string);
  ASSERT_EQ(strlen("\""), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, reverse_solidus) {
  const char payload[] = "[ \"\\\\\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\\", string->string);
  ASSERT_EQ(strlen("\\"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, solidus) {
  const char payload[] = "[ \"\\/\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("/", string->string);
  ASSERT_EQ(strlen("/"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, backspace) {
  const char payload[] = "[ \"\\b\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\b", string->string);
  ASSERT_EQ(strlen("\b"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, formfeed) {
  const char payload[] = "[ \"\\f\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\f", string->string);
  ASSERT_EQ(strlen("\f"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, newline) {
  const char payload[] = "[ \"\\n\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\n", string->string);
  ASSERT_EQ(strlen("\n"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, carriage_return) {
  const char payload[] = "[ \"\\r\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\r", string->string);
  ASSERT_EQ(strlen("\r"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(allow_string_simplification, horizontal_tab) {
  const char payload[] = "[ \"\\t\" ]";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_string_simplification, 0);
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("\t", string->string);
  ASSERT_EQ(strlen("\t"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}
