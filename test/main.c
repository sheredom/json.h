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

#include "json.h"

#include <string.h>

static int test_empty() {
  const char payload[] = "{}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;

  if (0 == value) {
    return 1;
  }

  if (json_type_object != value->type) {
    return 2;
  }

  object = (struct json_object_s* )value->payload;

  if (0 != object->length) {
    return 3;
  }

  free(value);
  return 0;
}

static int test_simple_object() {
  const char payload[] = "{ \"first\" : null, \"second\" : false, \"third\" : true, \"fourth\" : {} }";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;
  struct json_object_element_s* element = 0;

  if (0 == value) {
    return 1;
  }

  if (json_type_object != value->type) {
    return 2;
  }

  object = (struct json_object_s* )value->payload;

  if (4 != object->length) {
    return 3;
  }

  element = object->start;

  if (0 != strcmp("first", element->name->string)) {
    return 4;
  }

  if (strlen("first") != element->name->string_size) {
    return 5;
  }

  if (json_type_null != element->value->type) {
    return 12;
  }

  element = element->next;

  if (0 != strcmp("second", element->name->string)) {
    return 6;
  }

  if (strlen("second") != element->name->string_size) {
    return 7;
  }

  if (json_type_false != element->value->type) {
    return 13;
  }

  element = element->next;

  if (0 != strcmp("third", element->name->string)) {
    return 8;
  }

  if (strlen("third") != element->name->string_size) {
    return 9;
  }

  if (json_type_true != element->value->type) {
    return 14;
  }

  element = element->next;

  if (0 != strcmp("fourth", element->name->string)) {
    return 10;
  }

  if (strlen("fourth") != element->name->string_size) {
    return 11;
  }

  if (json_type_object != element->value->type) {
    return 15;
  }

  object = (struct json_object_s* )element->value->payload;

  if (0 != object->length) {
    return 16;
  }

  if (0 != object->start) {
    return 17;
  }

  free(value);
  return 0;
}

int test_objects() {
  const char payload[] = "{ \"ahem\\\"\"\n : { \"a\" : false }  , \"inception0\" : { \"inception1\" : {\r \"inception2\" : true\t } } }";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;
  struct json_object_element_s* element = 0;

  if (0 == value) {
    return 1;
  }

  if (json_type_object != value->type) {
    return 2;
  }

  object = (struct json_object_s* )value->payload;

  if (2 != object->length) {
    return 3;
  }

  element = object->start;

  if (0 != strcmp("ahem\\\"", element->name->string)) {
    return 4;
  }

  if (json_type_object != element->value->type) {
    return 5;
  }

  object = (struct json_object_s* )element->value->payload;

  if (1 != object->length) {
    return 6;
  }

  element = object->start;

  if (0 != strcmp("a", element->name->string)) {
    return 7;
  }

  if (json_type_false != element->value->type) {
    return 8;
  }

  // reset object to parent of json tree again
  object = (struct json_object_s* )value->payload;

  element = object->start->next;

  if (0 != strcmp("inception0", element->name->string)) {
    return 9;
  }

  if (json_type_object != element->value->type) {
    return 10;
  }

  object = (struct json_object_s* )element->value->payload;

  element = object->start;

  if (1 != object->length) {
    return 11;
  }

  if (0 != strcmp("inception1", element->name->string)) {
    return 12;
  }

  if (json_type_object != element->value->type) {
    return 13;
  }

  object = (struct json_object_s* )element->value->payload;

  element = object->start;

  if (1 != object->length) {
    return 14;
  }

  if (0 != strcmp("inception2", element->name->string)) {
    return 15;
  }

  if (json_type_true != element->value->type) {
    return 16;
  }

  free(value);
  return 0;
}

static int test_simple_array() {
  const char payload[] = "[ null, false, true, {}, [] ]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_array_element_s* element = 0;

  if (0 == value) {
    return 1;
  }

  if (json_type_array != value->type) {
    return 2;
  }

  array = (struct json_array_s* )value->payload;

  if (5 != array->length) {
    return 3;
  }

  element = array->start;

  if (json_type_null != element->value->type) {
    return 4;
  }

  element = element->next;

  if (json_type_false != element->value->type) {
    return 5;
  }

  element = element->next;

  if (json_type_true != element->value->type) {
    return 6;
  }

  element = element->next;

  if (json_type_object != element->value->type) {
    return 7;
  }

  if (0 != ((struct json_object_s* )element->value->payload)->length) {
    return 9;
  }

  element = element->next;

  if (json_type_array != element->value->type) {
    return 8;
  }

  array = (struct json_array_s* )element->value->payload;

  if (0 != array->length) {
    return 10;
  }

  if (0 != array->start) {
    return 11;
  }

  free(value);
  return 0;
}

int test_numbers() {
  const char* cmps[] = {
    "0", "123", "0.1", "-456.7", "-98e4", "-0.9E+1", "42E-42"};
  const char payload[] = "[ 0, 123, 0.1, -456.7, -98e4, -0.9E+1, 42E-42 ]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_array_element_s* element = 0;
  struct json_number_s* number = 0;
  size_t i;

  if (0 == value) {
    return 1;
  }

  if (json_type_array != value->type) {
    return 2;
  }

  array = (struct json_array_s* )value->payload;

  if (7 != array->length) {
    return 3;
  }

  element = array->start;
  for (i = 0; i < 7; i++) {
    if (json_type_number != element->value->type) {
      return 4;
    }

    number = (struct json_number_s* )element->value->payload;

    if (strlen(cmps[i]) != number->number_size) {
      return 5;
    }

    if (0 != strcmp(cmps[i], number->number)) {
      return 6;
    }

    element = element->next;
  }

  free(value);
  return 0;
}

int test_trailing_commas_in_object() {
  const char no_element[] = "{,}";
  const char one_element[] = "{\"a\" : true,}";
  const char few_element[] = "{\"a\" : true, \"b\" : false,}";

  struct json_value_s* value = 0;

  // negative test, should fail!
  value = json_parse(no_element, strlen(no_element));
  if (0 != value) {
    return 1;
  }

  // negative test, should fail!
  value = json_parse(one_element, strlen(one_element));
  if (0 != value) {
    return 2;
  }

  // negative test, should fail!
  value = json_parse(few_element, strlen(few_element));
  if (0 != value) {
    return 3;
  }

  // negative test, should fail? At present an empty array/object, with a comma,
  // is considered bad chi. Is this a special case we should allow or not?
  value = json_parse_ex(no_element, strlen(no_element),
    json_parse_flags_allow_trailing_comma, 0);
  if (0 != value) {
    return 4;
  }

  // negative test, should fail!
  value = json_parse_ex(one_element, strlen(one_element),
    json_parse_flags_allow_trailing_comma, 0);
  if (0 == value) {
    return 5;
  }
  free(value);

  // negative test, should fail!
  value = json_parse_ex(few_element, strlen(few_element),
    json_parse_flags_allow_trailing_comma, 0);
  if (0 == value) {
    return 6;
  }
  free(value);

  return 0;
}

int test_trailing_commas_in_array() {
  const char no_element[] = "[,]";
  const char one_element[] = "[true,]";
  const char few_element[] = "[true, false,]";

  struct json_value_s* value = 0;

  // negative test, should fail!
  value = json_parse(no_element, strlen(no_element));
  if (0 != value) {
    return 1;
  }

  // negative test, should fail!
  value = json_parse(one_element, strlen(one_element));
  if (0 != value) {
    return 2;
  }

  // negative test, should fail!
  value = json_parse(few_element, strlen(few_element));
  if (0 != value) {
    return 3;
  }

  // negative test, should fail? At present an empty array/object, with a comma,
  // is considered bad chi. Is this a special case we should allow or not?
  value = json_parse_ex(no_element, strlen(no_element),
    json_parse_flags_allow_trailing_comma, 0);
  if (0 != value) {
    return 4;
  }

  // negative test, should fail!
  value = json_parse_ex(one_element, strlen(one_element),
    json_parse_flags_allow_trailing_comma, 0);
  if (0 == value) {
    return 5;
  }
  free(value);

  // negative test, should fail!
  value = json_parse_ex(few_element, strlen(few_element),
    json_parse_flags_allow_trailing_comma, 0);
  if (0 == value) {
    return 6;
  }
  free(value);

  return 0;
}

int main() {
  const int error_addition = 20;
  int result = test_empty();

  if (0 != result) {
    return result;
  }

  result = test_simple_object();

  if (0 != result) {
    return (1 * error_addition) + result;
  }

  result = test_objects();

  if (0 != result) {
    return (2 * error_addition) + result;
  }

  result = test_simple_array();

  if (0 != result) {
    return (3 * error_addition) + result;
  }

  result = test_numbers();

  if (0 != result) {
    return (4 * error_addition) + result;
  }

  result = test_trailing_commas_in_object();

  if (0 != result) {
    return (5 * error_addition) + result;
  }

  result = test_trailing_commas_in_array();

  if (0 != result) {
    return (6 * error_addition) + result;
  }

  return 0;
}
