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

#if defined(__cplusplus) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4820)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__clang__) || defined(__GNUC__)
#define json_pure __attribute__((pure))
#define json_weak __attribute__((weak))
#elif  defined(_MSC_VER)
#define json_pure
#define json_weak __inline
#else
#error Non clang, non gcc, non MSVC compiler found!
#endif

struct json_value_s;

json_pure struct json_value_s* json_parse(
  const void* src, size_t src_size);

#undef json_pure
#undef json_weak

enum json_type_e {
  json_type_string,
  json_type_number,
  json_type_object,
  json_type_array,
  json_type_true,
  json_type_false,
  json_type_null
};

struct json_string_s {
  void* string;
  size_t string_size;
};

struct json_number_s {
  char* number;
  size_t number_size;
};

struct json_object_s {
  struct json_string_s* names;
  struct json_value_s* values;
  size_t length;
};

struct json_array_s {
  struct json_value_s* values;
  size_t length;
};

struct json_value_s {
  void* payload;
  enum json_type_e type;
};

#ifdef __cplusplus
} // extern "C"
#endif

#if defined(__cplusplus) && defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif//__JSON_H__
