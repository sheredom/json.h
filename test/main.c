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

TESTCASE(object, empty) {
  const char payload[] = "{}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_object, value->type);

  object = (struct json_object_s* )value->payload;

  ASSERT_FALSE(object->start);
  ASSERT_EQ(0, object->length);

  free(value);
}

TESTCASE(object, string) {
  const char payload[] = "{\"foo\" : \"Heyo, gaia?\"}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;
  struct json_value_s* value2 = 0;
  struct json_string_s* string = 0;

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

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_string, value2->type);

  string = (struct json_string_s* )value2->payload;

  ASSERT_TRUE(string->string);
  ASSERT_STREQ("Heyo, gaia?", string->string);
  ASSERT_EQ(strlen("Heyo, gaia?"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(object, number) {
  const char payload[] = "{\"foo\" : -0.123e-42}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;
  struct json_value_s* value2 = 0;
  struct json_number_s* number = 0;

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

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_number, value2->type);

  number = (struct json_number_s* )value2->payload;

  ASSERT_TRUE(number->number);
  ASSERT_STREQ("-0.123e-42", number->number);
  ASSERT_EQ(strlen("-0.123e-42"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(object, object) {
  const char payload[] = "{\"foo\" : {}}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;
  struct json_value_s* value2 = 0;
  struct json_object_s* object2 = 0;

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

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_object, value2->type);

  object2 = (struct json_object_s* )value2->payload;

  ASSERT_FALSE(object2->start);
  ASSERT_EQ(0, object2->length);

  free(value);
}

TESTCASE(object, array) {
  const char payload[] = "{\"foo\" : []}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_object_s* object = 0;
  struct json_value_s* value2 = 0;
  struct json_array_s* array = 0;

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

  ASSERT_TRUE(value2->payload);
  ASSERT_EQ(json_type_array, value2->type);

  array = (struct json_array_s* )value2->payload;

  ASSERT_FALSE(array->start);
  ASSERT_EQ(0, array->length);

  free(value);
}

TESTCASE(object, true) {
  const char payload[] = "{\"foo\" : true}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
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
  ASSERT_EQ(json_type_true, value2->type);

  free(value);
}

TESTCASE(object, false) {
  const char payload[] = "{\"foo\" : false}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
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
  ASSERT_EQ(json_type_false, value2->type);

  free(value);
}

TESTCASE(object, null) {
  const char payload[] = "{\"foo\" : null}";
  struct json_value_s* value = json_parse(payload, strlen(payload));
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

TESTCASE(array, empty) {
  const char payload[] = "[]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_FALSE(array->start);
  ASSERT_EQ(0, array->length);

  free(value);
}

TESTCASE(array, string) {
  const char payload[] = "[\"Heyo, gaia?\"]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
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
  ASSERT_STREQ("Heyo, gaia?", string->string);
  ASSERT_EQ(strlen("Heyo, gaia?"), string->string_size);
  ASSERT_EQ(strlen(string->string), string->string_size);

  free(value);
}

TESTCASE(array, number) {
  const char payload[] = "[-0.123e-42]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;
  struct json_number_s* number = 0;

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
  ASSERT_EQ(json_type_number, value2->type);

  number = (struct json_number_s* )value2->payload;

  ASSERT_TRUE(number->number);
  ASSERT_STREQ("-0.123e-42", number->number);
  ASSERT_EQ(strlen("-0.123e-42"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(array, true) {
  const char payload[] = "[true]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_FALSE(value2->payload);
  ASSERT_EQ(json_type_true, value2->type);

  free(value);
}

TESTCASE(array, false) {
  const char payload[] = "[false]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_FALSE(value2->payload);
  ASSERT_EQ(json_type_false, value2->type);

  free(value);
}

TESTCASE(array, null) {
  const char payload[] = "[null]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_value_s* value2 = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_FALSE(array->start->next); // we have only one element

  value2 = array->start->value;

  ASSERT_FALSE(value2->payload);
  ASSERT_EQ(json_type_null, value2->type);

  free(value);
}

TESTCASE(no_global_object, empty) {
  const char payload[] = "";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  ASSERT_FALSE(value);
}

TESTCASE(number, zero) {
  const char payload[] = "[0]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("0", number->number);
  ASSERT_EQ(strlen("0"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, positive) {
  const char payload[] = "[42]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("42", number->number);
  ASSERT_EQ(strlen("42"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, minus) {
  const char payload[] = "[-0]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("-0", number->number);
  ASSERT_EQ(strlen("-0"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, decimal) {
  const char payload[] = "[0.4]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("0.4", number->number);
  ASSERT_EQ(strlen("0.4"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, smalle) {
  const char payload[] = "[1e4]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("1e4", number->number);
  ASSERT_EQ(strlen("1e4"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, bige) {
  const char payload[] = "[1E4]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("1E4", number->number);
  ASSERT_EQ(strlen("1E4"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, eplus) {
  const char payload[] = "[1e+4]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("1e+4", number->number);
  ASSERT_EQ(strlen("1e+4"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

TESTCASE(number, eminus) {
  const char payload[] = "[1e-4]";
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_array_s* array = 0;
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_array, value->type);

  array = (struct json_array_s* )value->payload;

  ASSERT_TRUE(array->start);
  ASSERT_EQ(1, array->length);

  ASSERT_TRUE(array->start->value);
  ASSERT_TRUE(array->start->value->payload);
  ASSERT_EQ(json_type_number, array->start->value->type);

  number = (struct json_number_s* )array->start->value->payload;

  ASSERT_TRUE(number->number);

  ASSERT_STREQ("1e-4", number->number);
  ASSERT_EQ(strlen("1e-4"), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

UTEST_MAIN();
