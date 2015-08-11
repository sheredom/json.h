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

#ifndef __JSON_H__
#define __JSON_H__

#include <stddef.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4820)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct json_value_s;

// Parse a JSON text file, returning a pointer to the root of the JSON
// structure. json_parse performs 1 call to malloc for the entire encoding.
// Returns 0 if an error occurred (malformed JSON input, or malloc failed)
struct json_value_s* json_parse(
  const void* src, size_t src_size);

// Write out a minified JSON utf-8 string. This string is an encoding of the
// minimal string characters required to still encode the same data.
// json_write_minified performs 1 call to malloc for the entire encoding.
// Return 0 if an error occurred (malformed JSON input, or malloc failed).
// The out_size parameter is optional as the utf-8 string is null terminated.
void* json_write_minified(const struct json_value_s* value, size_t* out_size);

// Write out a pretty JSON utf-8 string. This string is encoded such that the 
// resultant JSON is pretty in that it is easily human readable. The indent and 
// newline parameters allow a user to specify what kind of indentation and 
// newline they want (two spaces / three spaces / tabs? \r, \n, \r\n ?). Both
// indent and newline can be NULL, indent defaults to two spaces ("  "), and 
// newline defaults to linux newlines ('\n' as the newline character).
// json_write_pretty performs 1 call to malloc for the entire encoding.
// Return 0 if an error occurred (malformed JSON input, or malloc failed).
// The out_size parameter is optional as the utf-8 string is null terminated.
void* json_write_pretty(const struct json_value_s* value, 
  char* indent,
  char* newline,
  size_t* out_size);

// The various types JSON values can be. Used to identify what a value is
enum json_type_e {
  json_type_string,
  json_type_number,
  json_type_object,
  json_type_array,
  json_type_true,
  json_type_false,
  json_type_null
};

// A JSON string value
struct json_string_s {
  // utf-8 string
  void* string;
  // the size (in bytes) of the string
  size_t string_size;
};

// a JSON number value
struct json_number_s {
  // ASCII string containing representation of the number
  char* number;
  // the size (in bytes) of the number
  size_t number_size;
};

// a JSON object value
struct json_object_s {
  // the names of the elements in the object
  struct json_string_s* names;
  // the values of the named elements in the object
  struct json_value_s* values;
  // the length of names and values (number of elements in the object)
  size_t length;
};

// a JSON array value
struct json_array_s {
  // the values of the elements in the array
  struct json_value_s* values;
  // the length of values (number of elements in the array)
  size_t length;
};

// a JSON value
struct json_value_s {
  // a pointer to either a json_string_s, json_number_s, json_object_s, or
  // json_array_s. Should be cast to the appropriate struct type based on what
  // the type of this value is
  void* payload;
  // Must be one of json_type_e. If type is json_type_true, json_type_false, or
  // json_type_null, payload will be NULL
  size_t type;
};

#ifdef __cplusplus
} // extern "C"
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif//__JSON_H__
