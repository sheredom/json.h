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

  if (0 != strcmp("first", object->names[0].string)) {
    return 4;
  }

  if (strlen("first") != object->names[0].string_size) {
    return 5;
  }

  if (0 != strcmp("second", object->names[1].string)) {
    return 6;
  }

  if (strlen("second") != object->names[1].string_size) {
    return 7;
  }

  if (0 != strcmp("third", object->names[2].string)) {
    return 8;
  }

  if (strlen("third") != object->names[2].string_size) {
    return 9;
  }

  if (0 != strcmp("fourth", object->names[3].string)) {
    return 10;
  }

  if (strlen("fourth") != object->names[3].string_size) {
    return 11;
  }

  if (json_type_null != object->values[0].type) {
    return 12;
  }

  if (json_type_false != object->values[1].type) {
    return 13;
  }

  if (json_type_true != object->values[2].type) {
    return 14;
  }

  if (json_type_object != object->values[3].type) {
    return 15;
  }

  object = (struct json_object_s* )object->values[3].payload;

  if (0 != object->length) {
    return 16;
  }

  if (0 != object->names) {
    return 17;
  }

  if (0 != object->values) {
    return 18;
  }

  free(value);
  return 0;
}

int test_objects() {
  const char payload[] = "{ \"ahem\\\"\"\n : { \"a\" : false }  , \"inception0\" : { \"inception1\" : {\r \"inception2\" : true\t } } }";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;

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

  if (0 != strcmp("ahem\\\"", object->names[0].string)) {
    return 4;
  }

  if (json_type_object != object->values[0].type) {
    return 5;
  }

  object = (struct json_object_s* )object->values[0].payload;

  if (1 != object->length) {
    return 6;
  }

  if (0 != strcmp("a", object->names[0].string)) {
    return 7;
  }

  if (json_type_false != object->values[0].type) {
    return 8;
  }

  // reset object to parent of json tree again
  object = (struct json_object_s* )value->payload;

  if (0 != strcmp("inception0", object->names[1].string)) {
    return 9;
  }

  if (json_type_object != object->values[1].type) {
    return 10;
  }

  object = (struct json_object_s* )object->values[1].payload;

  if (1 != object->length) {
    return 11;
  }

  if (0 != strcmp("inception1", object->names[0].string)) {
    return 12;
  }

  if (json_type_object != object->values[0].type) {
    return 13;
  }

  object = (struct json_object_s* )object->values[0].payload;

  if (1 != object->length) {
    return 14;
  }

  if (0 != strcmp("inception2", object->names[0].string)) {
    return 15;
  }

  if (json_type_true != object->values[0].type) {
    return 16;
  }

  free(value);
  return 0;
}

static int test_simple_array() {
  const char payload[] = "[ null, false, true, {}, [] ]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;

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

  if (json_type_null != array->values[0].type) {
    return 4;
  }

  if (json_type_false != array->values[1].type) {
    return 5;
  }

  if (json_type_true != array->values[2].type) {
    return 6;
  }

  if (json_type_object != array->values[3].type) {
    return 7;
  }

  if (json_type_array != array->values[4].type) {
    return 8;
  }

  if (0 != ((struct json_object_s* )array->values[3].payload)->length) {
    return 9;
  }

  array = (struct json_array_s* )array->values[4].payload;

  if (0 != array->length) {
    return 10;
  }

  if (0 != array->values) {
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

  for (i = 0; i < 7; i++) {
    if (json_type_number != array->values[i].type) {
      return 4;
    }

    number = (struct json_number_s* )array->values[i].payload;

    if (strlen(cmps[i]) != number->number_size) {
      return 5;
    }

    if (0 != strcmp(cmps[i], number->number)) {
      return 6;
    }
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

  return 0;
}
