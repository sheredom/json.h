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

UTEST(allow_unquoted_keys, one_key) {
  const char payload[] = "{foo : null}";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_unquoted_keys, 0, 0, 0);
  struct json_object_s* object = 0;
  struct json_value_s* value2 = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_object, value->type);

  object = (struct json_object_s* )value->payload;

  ASSERT_TRUE(object->start);
  ASSERT_EQ(1, object->length);

  ASSERT_TRUE(object->start->name);
  ASSERT_TRUE(object->start->value);
  ASSERT_FALSE(object->start->next); // we have only one element

  ASSERT_TRUE(object->start->name->string);
  ASSERT_STREQ("foo", object->start->name->string);
  ASSERT_EQ(strlen("foo"), object->start->name->string_size);
  ASSERT_EQ(strlen(object->start->name->string), object->start->name->string_size);

  value2 = object->start->value;

  ASSERT_FALSE(value2->payload);
  ASSERT_EQ(json_type_null, value2->type);

  free(value);
}

UTEST(allow_unquoted_keys, mixed_keys) {
  const char payload[] = "{foo : true, \"heyo\" : false}";
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_unquoted_keys, 0, 0, 0);
  struct json_object_s* object = 0;
  struct json_object_element_s* element = 0;
  struct json_value_s* value2 = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_object, value->type);

  object = (struct json_object_s* )value->payload;

  ASSERT_TRUE(object->start);
  ASSERT_EQ(2, object->length);

  element = object->start;

  ASSERT_TRUE(element->name);
  ASSERT_TRUE(element->value);
  ASSERT_TRUE(element->next);

  ASSERT_TRUE(element->name->string);
  ASSERT_STREQ("foo", element->name->string);
  ASSERT_EQ(strlen("foo"), element->name->string_size);
  ASSERT_EQ(strlen(element->name->string), element->name->string_size);

  value2 = element->value;

  ASSERT_FALSE(value2->payload);
  ASSERT_EQ(json_type_true, value2->type);

  element = element->next;

  ASSERT_TRUE(element->name);
  ASSERT_TRUE(element->value);
  ASSERT_FALSE(element->next);

  ASSERT_TRUE(element->name->string);
  ASSERT_STREQ("heyo", element->name->string);
  ASSERT_EQ(strlen("heyo"), element->name->string_size);
  ASSERT_EQ(strlen(element->name->string), element->name->string_size);

  value2 = element->value;

  ASSERT_FALSE(value2->payload);
  ASSERT_EQ(json_type_false, value2->type);

  free(value);
}

UTEST(allow_unquoted_keys, value_unquoted_fails) {
  const char payload[] = "{foo\n: heyo}";
  struct json_parse_result_s result;
  struct json_value_s* value = json_parse_ex(payload, strlen(payload), json_parse_flags_allow_unquoted_keys, 0, 0, &result);

  ASSERT_FALSE(value);

  ASSERT_EQ(json_parse_error_invalid_value, result.error);
  ASSERT_EQ(7, result.error_offset);
  ASSERT_EQ(3, result.error_row_no);
  ASSERT_EQ(2, result.error_line_no);
}
