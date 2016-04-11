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

#include <string>

#include "utest.h"

#include "json.h"

namespace {
static const char* makeNumber(int sign, int zero, int many_whole_digits, int decimal, int exponent, int exponent_sign) {
  static char payload[512] = {0};

  sprintf(payload, "%s%s%s%s%s%s%s",
    (sign) ? "-" : "",
    (zero) ? "0" : "5",
    (many_whole_digits) ? "423" : "",
    (decimal) ? "." : "",
    (decimal) ? ((exponent) ? "E" : "e") : "",
    (decimal) ? ((exponent_sign < 0) ? "-" : ((exponent_sign > 0) ? "+" : "")) : "",
    (decimal) ? "123456789" : "");

  return payload;
}
}

struct numbers {
  std::string string;
};

UTEST_I_SETUP(numbers) {}

UTEST_I_TEARDOWN(numbers) {
  const char *payload = utest_fixture->string.c_str();
  struct json_value_s* value = json_parse(payload, strlen(payload));
  struct json_number_s* number = 0;

  ASSERT_TRUE(value);
  ASSERT_TRUE(value->payload);
  ASSERT_EQ(json_type_number, value->type);

  number = (struct json_number_s* )value->payload;

  ASSERT_TRUE(number->number);
  ASSERT_STREQ(payload, number->number);
  ASSERT_EQ(strlen(payload), number->number_size);
  ASSERT_EQ(strlen(number->number), number->number_size);

  free(value);
}

UTEST_I(numbers, zero, 1) {
  utest_fixture->string = "0";
}

UTEST_I(numbers, negative_zero, 1) {
  utest_fixture->string = "-0";
}

UTEST_I(numbers, digit, 1000) {
  utest_fixture->string = std::to_string(utest_index);
}

UTEST_I(numbers, negative_digit, 1000) {
  utest_fixture->string = "-" + std::to_string(utest_index);
}

UTEST_I(numbers, decimal, 2000) {
  utest_fixture->string = std::to_string(utest_index) + "." +
    std::to_string(2000 - utest_index);
}

UTEST_I(numbers, negative_decimal, 2000) {
  utest_fixture->string = "-" + std::to_string(utest_index) + "." +
    std::to_string(2000 - utest_index);
}

UTEST_I(numbers, e, 1000) {
  utest_fixture->string = "0.1e" + std::to_string(utest_index);
}

UTEST_I(numbers, E, 1000) {
  utest_fixture->string = "0.1E" + std::to_string(utest_index);
}

UTEST_I(numbers, e_nowt, 1000) {
  utest_fixture->string = "0.1e" + std::to_string(utest_index);
}

UTEST_I(numbers, e_plus, 1000) {
  utest_fixture->string = "0.1e+" + std::to_string(utest_index);
}

UTEST_I(numbers, e_minus, 1000) {
  utest_fixture->string = "0.1e-" + std::to_string(utest_index);
}

struct invalid_numbers {
  std::string string;
};

UTEST_I_SETUP(invalid_numbers) {}

UTEST_I_TEARDOWN(invalid_numbers) {
  const char *payload = utest_fixture->string.c_str();
  struct json_value_s* value = json_parse(payload, strlen(payload));

  ASSERT_FALSE(value);
}

UTEST_I(invalid_numbers, double_negative, 1) {
  utest_fixture->string = "--";
}
