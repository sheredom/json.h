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

// we do one big allocation via malloc, then cast aligned slices of this for
// our structures - we don't have a way to tell the compiler we know what we
// are doing, so disable the warning instead!
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4711)
#endif

struct json_parse_state_s {
  const char* src;
  size_t size;
  size_t offset;
  size_t line_no;            // line counter for error reporting
  size_t line_offset;        // (offset-line_offset) is the character number (in bytes)
  size_t error;
  char* dom;
  char* data;
  size_t dom_size;
  size_t data_size;
};

static int json_is_hexadecimal_digit(const char c) {
  return (('0' <= c && c <= '9') ||
    ('a' <= c && c <= 'f') ||
    ('A' <= c && c <= 'F'));
}

static int json_skip_whitespace(struct json_parse_state_s* state) {
  // the only valid whitespace according to ECMA-404 is ' ', '\n', '\r' and '\t'
  size_t offset = 0, size = 0;

  for (offset = state->offset, size = state->size; offset < size; offset++) {
    switch (state->src[offset]) {
    default:
      state->offset = offset;
      return 0;
    case '\n':
	  state->line_no++;
	  state->line_offset = offset;
      break;
    case ' ':
    case '\r':
    case '\t':
      break;
    }
  }

  // consumed the whole buffer, we're done!
  return 1;
}

static int json_get_value_size(struct json_parse_state_s* state);

static int json_get_string_size(struct json_parse_state_s* state) {
  const size_t initial_offset = state->offset;

  state->dom_size += sizeof(struct json_string_s);

  if ('"' != state->src[state->offset]) {
	state->error = json_parse_error_expected_opening_quote;
    return 1;
  }

  // skip leading '"'
  state->offset++;

  while (state->offset < state->size && '"' != state->src[state->offset]) {
    if ('\\' == state->src[state->offset]) {
      // skip reverse solidus character
      state->offset++;

      if (state->offset == state->size) {
        // string can't terminate here!
        return 1;
      }

      switch (state->src[state->offset]) {
      default:
		state->error = json_parse_error_invalid_string_escape_sequence;
        return 1;
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        // all valid characters!
        state->offset++;
        break;
      case 'u':
        if (state->offset + 5 < state->size) {
          // invalid escaped unicode sequence!
		  state->error = json_parse_error_invalid_string_escape_sequence;
          return 1;
        } else if (
          !json_is_hexadecimal_digit(state->src[state->offset + 1]) ||
          !json_is_hexadecimal_digit(state->src[state->offset + 2]) ||
          !json_is_hexadecimal_digit(state->src[state->offset + 3]) ||
          !json_is_hexadecimal_digit(state->src[state->offset + 4])) {
          // escaped unicode sequences must contain 4 hexadecimal digits!
          state->error = json_parse_error_invalid_string_escape_sequence;
          return 1;
        }

        // valid sequence!
        state->offset += 5;
        break;
      }
    } else {
      // skip character (valid part of sequence)
      state->offset++;
    }
  }

  // skip trailing '"'
  state->offset++;

  state->data_size += state->offset - initial_offset;

  return 0;
}

static int json_get_object_size(struct json_parse_state_s* state) {
  size_t elements = 0;
  int allow_comma = 0;

  if ('{' != state->src[state->offset]) {
	state->error = json_parse_error_unknown;
    return 1;
  }

  // skip leading '{'
  state->offset++;

  state->dom_size += sizeof(struct json_object_s);

  while (state->offset < state->size) {
    if (json_skip_whitespace(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if ('}' == state->src[state->offset]) {
      // skip trailing '}'
      state->offset++;

      // finished the object!
      break;
    }

    // if we parsed at least once element previously, grok for a comma
    if (allow_comma) {
      if (',' != state->src[state->offset]) {
		state->error = json_parse_error_expected_comma;
        return 1;
      }

      // skip comma
      state->offset++;
	  allow_comma = 0;
      continue;
    }

    if (json_get_string_size(state)) {
      // string parsing failed!
      return 1;
    }

    if (json_skip_whitespace(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if (':' != state->src[state->offset]) {
      state->error = json_parse_error_expected_colon;
      return 1;
    }

    // skip colon
    state->offset++;

    if (json_skip_whitespace(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if (json_get_value_size(state)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed a name/value pair!
    elements++;
	allow_comma = 1;
  }

  state->dom_size += sizeof(struct json_string_s) * elements;
  state->dom_size += sizeof(struct json_value_s) * elements;
  state->dom_size += sizeof(struct json_object_element_s) * elements;

  return 0;
}

static int json_get_array_size(struct json_parse_state_s* state) {
  size_t elements = 0;
  int allow_comma = 0;

  if ('[' != state->src[state->offset]) {
	// expected array to begin with leading '['
	state->error = json_parse_error_unknown;
    return 1;
  }

  // skip leading '['
  state->offset++;

  state->dom_size += sizeof(struct json_array_s);

  while (state->offset < state->size) {
    if (json_skip_whitespace(state)) {
      state->error = json_parse_error_premature_end_of_buffer;
      return 1;
    }

    if (']' == state->src[state->offset]) {
      // skip trailing ']'
      state->offset++;

      // finished the object!
      break;
    }

    // if we parsed at least once element previously, grok for a comma
    if (allow_comma) {
      if (',' != state->src[state->offset]) {
        state->error = json_parse_error_expected_comma;
        return 1;
      }

      // skip comma
      state->offset++;
	  allow_comma = 0;
      continue;
    }

    if (json_get_value_size(state)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed an array element!
    elements++;
	allow_comma = 1;
  }

  state->dom_size += sizeof(struct json_value_s) * elements;
  state->dom_size += sizeof(struct json_array_element_s) * elements;

  return 0;
}

static int json_get_number_size(struct json_parse_state_s* state) {
  size_t initial_offset = state->offset;

  state->dom_size += sizeof(struct json_number_s);

  if ((state->offset < state->size) && ('-' == state->src[state->offset])) {
    // skip valid leading '-'
    state->offset++;
  }

  if ((state->offset < state->size) && ('0' == state->src[state->offset])) {
    // skip valid '0'
    state->offset++;

    if ((state->offset < state->size) &&
      ('0' <= state->src[state->offset] && state->src[state->offset] <= '9')) {
      // a leading '0' must not be immediately followed by any digits!
	  state->error = json_parse_error_invalid_number_format;
      return 1;
    }
  }

  // the main digits of our number next
  while ((state->offset < state->size) &&
   ('0' <= state->src[state->offset] && state->src[state->offset] <= '9')) {
     state->offset++;
  }

  if ((state->offset < state->size) && ('.' == state->src[state->offset])) {
    state->offset++;

    // a decimal point can be followed by more digits of course!
    while ((state->offset < state->size) &&
     ('0' <= state->src[state->offset] && state->src[state->offset] <= '9')) {
       state->offset++;
    }
  }

  if ((state->offset < state->size) && ('e' == state->src[state->offset] ||
    'E' == state->src[state->offset])) {
    // our number has an exponent!

    // skip 'e' or 'E'
    state->offset++;

    if ((state->offset < state->size) && ('-' == state->src[state->offset] ||
      '+' == state->src[state->offset])) {
      // skip optional '-' or '+'
      state->offset++;
    }

    // consume exponent digits
    while ((state->offset < state->size) &&
     ('0' <= state->src[state->offset] && state->src[state->offset] <= '9')) {
       state->offset++;
    }
  }

  state->data_size += state->offset - initial_offset;

  // one more byte for null terminator ending the number string!
  state->data_size++;

  return 0;
}

static int json_get_value_size(struct json_parse_state_s* state) {
  if (json_skip_whitespace(state)) {
	state->error = json_parse_error_premature_end_of_buffer;
    return 1;
  }

  state->dom_size += sizeof(struct json_value_s);

  switch (state->src[state->offset]) {
  case '"':
    return json_get_string_size(state);
  case '{':
    return json_get_object_size(state);
  case '[':
    return json_get_array_size(state);
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return json_get_number_size(state);
  default:
    if ((state->offset + 4) < state->size &&
      't' == state->src[state->offset + 0] &&
      'r' == state->src[state->offset + 1] &&
      'u' == state->src[state->offset + 2] &&
      'e' == state->src[state->offset + 3]) {
      state->offset += 4;
      return 0;
    } else if ((state->offset + 5) < state->size &&
      'f' == state->src[state->offset + 0] &&
      'a' == state->src[state->offset + 1] &&
      'l' == state->src[state->offset + 2] &&
      's' == state->src[state->offset + 3] &&
      'e' == state->src[state->offset + 4]) {
      state->offset += 5;
      return 0;
    } else if ((state->offset + 4) < state->size &&
      'n' == state->src[state->offset + 0] &&
      'u' == state->src[state->offset + 1] &&
      'l' == state->src[state->offset + 2] &&
      'l' == state->src[state->offset + 3]) {
      state->offset += 4;
      return 0;
    }

    // invalid value!
	state->error = json_parse_error_invalid_value;
    return 1;
  }
}

static int json_parse_value(struct json_parse_state_s* state,
  struct json_value_s* value);

static int json_parse_string(struct json_parse_state_s* state,
  struct json_string_s* string) {
  size_t size = 0;

  string->string = state->data;

  if ('"' != state->src[state->offset]) {
    // expected string to begin with '"'!
    return 1;
  }

  // skip leading '"'
  state->offset++;

  while (state->offset < state->size &&
    ('"' != state->src[state->offset] ||
    ('\\' == state->src[state->offset - 1] &&
    '"' == state->src[state->offset]))) {
    state->data[size++] = state->src[state->offset++];
  }

  // skip trailing '"'
  state->offset++;

  // record the size of the string
  string->string_size = size;

  // add null terminator to string
  state->data[size++] = '\0';

  // move data along
  state->data += size;

  return 0;
}

static int json_parse_object(struct json_parse_state_s* state,
  struct json_object_s* object) {
  size_t elements = 0;
  int allow_comma = 0;
  struct json_object_element_s* previous = 0;

  if ('{' != state->src[state->offset]) {
    // expected object to begin with leading '{'
    return 1;
  }

  // skip leading '{'
  state->offset++;

  if (json_skip_whitespace(state)) {
    // consumed the whole buffer when we expected a value!
    return 1;
  }

  if ('}' != state->src[state->offset]) {
    // we have at least one element as we definitely don't have
    // an empty object {   }!
    elements++;
  }

  // reset elements
  elements = 0;

  while (state->offset < state->size) {
	struct json_object_element_s* element = 0;
	struct json_string_s* string = 0;
	struct json_value_s* value = 0;

    if (json_skip_whitespace(state)) {
      // reached end of buffer before object was complete!
      return 1;
    }

    if ('}' == state->src[state->offset]) {
      // skip trailing '}'
      state->offset++;

      // finished the object!
      break;
    }

    // if we parsed at least one element previously, grok for a comma
    if (allow_comma) {
      if (',' != state->src[state->offset]) {
        // expected a comma where there was none!
        return 1;
      }

      // skip comma
      state->offset++;
	  allow_comma = 0;
      continue;
    }

    element = (struct json_object_element_s* )state->dom;

    state->dom += sizeof(struct json_object_element_s);

    if (0 == previous) {
      // this is our first element, so record it in our object
      object->start = element;
    } else {
      previous->next = element;
    }

    previous = element;

    string = (struct json_string_s* )state->dom;

    state->dom += sizeof(struct json_string_s);

    element->name = string;

    if (json_parse_string(state, string)) {
      // string parsing failed!
      return 1;
    }

    if (json_skip_whitespace(state)) {
      // reached end of buffer before object was complete!
      return 1;
    }

    if (':' != state->src[state->offset]) {
      // colon seperating name/value pair was missing!
      return 1;
    }

    // skip colon
    state->offset++;

    if (json_skip_whitespace(state)) {
      // reached end of buffer before object was complete!
      return 1;
    }

    value = (struct json_value_s* )state->dom;

    state->dom += sizeof(struct json_value_s);

    element->value = value;

    if (json_parse_value(state, value)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed a name/value pair!
    elements++;
	allow_comma = 1;
  }

  // if we had at least one element, end the linked list
  if (previous) {
    previous->next = 0;
  }

  if (0 == elements) {
    object->start = 0;
  }

  object->length = elements;

  return 0;
}

static int json_parse_array(struct json_parse_state_s* state,
  struct json_array_s* array) {
  size_t elements = 0;
  int allow_comma = 0;
  struct json_array_element_s* previous = 0;

  if ('[' != state->src[state->offset]) {
    // expected object to begin with leading '['
    return 1;
  }

  // skip leading '['
  state->offset++;

  if (json_skip_whitespace(state)) {
    // consumed the whole buffer when we expected a value!
    return 1;
  }

  // reset elements
  elements = 0;

  while (state->offset < state->size) {
	struct json_array_element_s* element = 0;
	struct json_value_s* value = 0;

    if (json_skip_whitespace(state)) {
      // reached end of buffer before array was complete!
      return 1;
    }

    if (']' == state->src[state->offset]) {
      // skip trailing ']'
      state->offset++;

      // finished the array!
      break;
    }

    // if we parsed at least one element previously, grok for a comma
    if (allow_comma) {
      if (',' != state->src[state->offset]) {
        // expected a comma where there was none!
        return 1;
      }

      // skip comma
      state->offset++;
	  allow_comma = 0;
      continue;
    }

    element = (struct json_array_element_s* )state->dom;

    state->dom += sizeof(struct json_array_element_s);

    if (0 == previous) {
      // this is our first element, so record it in our array
      array->start = element;
    } else {
      previous->next = element;
    }

    previous = element;

    value = (struct json_value_s* )state->dom;

    state->dom += sizeof(struct json_value_s);

    element->value = value;

    if (json_parse_value(state, value)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed an array element!
    elements++;
	allow_comma = 1;
  }

  // end the linked list
  if (previous) {
    previous->next = 0;
  }

  if (0 == elements) {
    array->start = 0;
  }

  array->length = elements;

  return 0;
}

static int json_parse_number(struct json_parse_state_s* state,
  struct json_number_s* number) {
  size_t size = 0;

  number->number = state->data;

  while (state->offset < state->size) {
    switch (state->src[state->offset]) {
    default:
      // we've reached the end of all valid number characters!
      goto json_parse_number_cleanup;
    case '+':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
    case 'e':
    case 'E':
      state->data[size++] = state->src[state->offset++];
      break;
    }
  }

json_parse_number_cleanup:
  // record the size of the number
  number->number_size = size;

  // add null terminator to number string
  state->data[size++] = '\0';

  // move data along
  state->data += size;

  return 0;
}

static int json_parse_value(struct json_parse_state_s* state,
  struct json_value_s* value) {
  if (json_skip_whitespace(state)) {
    // consumed the whole buffer when we expected a value!
    return 1;
  }

  switch (state->src[state->offset]) {
  case '"':
    value->type = json_type_string;
    value->payload = state->dom;
    state->dom += sizeof(struct json_string_s);
    return json_parse_string(state, value->payload);
  case '{':
    value->type = json_type_object;
    value->payload = state->dom;
    state->dom += sizeof(struct json_object_s);
    return json_parse_object(state, value->payload);
  case '[':
    value->type = json_type_array;
    value->payload = state->dom;
    state->dom += sizeof(struct json_array_s);
    return json_parse_array(state, value->payload);
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    value->type = json_type_number;
    value->payload = state->dom;
    state->dom += sizeof(struct json_number_s);
    return json_parse_number(state, value->payload);
  default:
    if ((state->offset + 4) < state->size &&
      't' == state->src[state->offset + 0] &&
      'r' == state->src[state->offset + 1] &&
      'u' == state->src[state->offset + 2] &&
      'e' == state->src[state->offset + 3]) {
      value->type = json_type_true;
      value->payload = 0;
      state->offset += 4;
      return 0;
    } else if ((state->offset + 5) < state->size &&
      'f' == state->src[state->offset + 0] &&
      'a' == state->src[state->offset + 1] &&
      'l' == state->src[state->offset + 2] &&
      's' == state->src[state->offset + 3] &&
      'e' == state->src[state->offset + 4]) {
      value->type = json_type_false;
      value->payload = 0;
      state->offset += 5;
      return 0;
    } else if ((state->offset + 4) < state->size &&
      'n' == state->src[state->offset + 0] &&
      'u' == state->src[state->offset + 1] &&
      'l' == state->src[state->offset + 2] &&
      'l' == state->src[state->offset + 3]) {
      value->type = json_type_null;
      value->payload = 0;
      state->offset += 4;
      return 0;
    }

    // invalid value!
    return 1;
  }
}

struct json_value_s* json_parse_ex(const void* src, size_t src_size, struct json_parse_result_s* result) {
  struct json_parse_state_s state;
  void* allocation;

  if (result) {
	result->error = json_parse_error_none;
	result->error_offset = 0;
	result->error_line_no = 0;
	result->error_row_no = 0;
  }

  if (0 == src || 2 > src_size) {
    // absolute minimum valid json is either "{}" or "[]"
    return 0;
  }

  state.src = src;
  state.size = src_size;
  state.offset = 0;
  state.line_no = 1;
  state.line_offset = 0;
  state.error = json_parse_error_none;
  state.dom_size = 0;
  state.data_size = 0;

  if (json_get_value_size(&state)) {
    // parsing value's size failed (most likely an invalid JSON DOM!)
	if (result) {
	  result->error = state.error;
	  result->error_offset = state.offset;
	  result->error_line_no = state.line_no;
	  result->error_row_no = state.offset - state.line_offset;
	}
    return 0;
  }

  // our total allocation is the combination of the dom and data sizes (we
  // first encode the structure of the JSON, and then the data referenced by
  // the JSON values)
  allocation = malloc(state.dom_size + state.data_size);

  if (0 == allocation) {
    // malloc failed!
    return 0;
  }

  // reset offset so we can reuse it
  state.offset = 0;

  state.dom = allocation;
  state.data = state.dom + state.dom_size;

  state.dom += sizeof(struct json_value_s);

  if (json_parse_value(&state, (struct json_value_s* )allocation)) {
    // really bad chi here
    free(allocation);
    return 0;
  }

  return allocation;
}

struct json_value_s* json_parse(const void* src, size_t src_size) {
  return json_parse_ex(src, src_size, NULL);
}

static int json_write_minified_get_value_size(const struct json_value_s* value, size_t* size);

static int json_write_minified_get_number_size(const struct json_number_s* number, size_t* size) {
  *size += number->number_size; // the actual string of the number

  return 0;
}

static int json_write_minified_get_string_size(const struct json_string_s* string, size_t* size) {
  *size += string->string_size; // the actual string
  *size += 2; // need to encode the surrounding '"' characters

  return 0;
}

static int json_write_minified_get_array_size(const struct json_array_s* array, size_t* size) {
  struct json_array_element_s* element;

  *size += 2; // '[' and ']'

  if (1 < array->length) {
    *size += array->length - 1; // ','s seperate each element
  }

  for (element = array->start; 0 != element; element = element->next) {
    if (json_write_minified_get_value_size(element->value, size)) {
      // value was malformed!
      return 1;
    }
  }

  return 0;
}

static int json_write_minified_get_object_size(const struct json_object_s* object, size_t* size) {
  struct json_object_element_s* element;

  *size += 2; // '{' and '}'

  *size += object->length; // ':'s seperate each name/value pair

  if (1 < object->length) {
    *size += object->length - 1; // ','s seperate each element
  }

  for (element = object->start; 0 != element; element = element->next) {
    if (json_write_minified_get_string_size(element->name, size)) {
      // string was malformed!
      return 1;
    }

    if (json_write_minified_get_value_size(element->value, size)) {
      // value was malformed!
      return 1;
    }
  }

  return 0;
}

static int json_write_minified_get_value_size(const struct json_value_s* value, size_t* size) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 1;
  case json_type_number:
    return json_write_minified_get_number_size((struct json_number_s* )value->payload, size);
  case json_type_string:
    return json_write_minified_get_string_size((struct json_string_s* )value->payload, size);
  case json_type_array:
    return json_write_minified_get_array_size((struct json_array_s* )value->payload, size);
  case json_type_object:
    return json_write_minified_get_object_size((struct json_object_s* )value->payload, size);
  case json_type_true:
    *size += 4; // the string "true"
    return 0;
  case json_type_false:
    *size += 5; // the string "false"
    return 0;
  case json_type_null:
    *size += 4; // the string "null"
    return 0;
  }
}

static char* json_write_minified_value(const struct json_value_s* value, char* data);

static char* json_write_minified_number(const struct json_number_s* number, char* data) {
  size_t i;

  for (i = 0; i < number->number_size; i++) {
    *data++ = number->number[i];
  }

  return data;
}

static char* json_write_minified_string(const struct json_string_s* string, char* data) {
  size_t i;

  *data++ = '"'; // open the string

  for (i = 0; i < string->string_size; i++) {
    *data++ = ((char* )string->string)[i];
  }

  *data++ = '"'; // close the string

  return data;
}

static char* json_write_minified_array(const struct json_array_s* array, char* data) {
  struct json_array_element_s* element = 0;

  *data++ = '['; // open the array

  for (element = array->start; 0 != element; element = element->next) {
    if (element != array->start) {
      *data++ = ','; // ','s seperate each element
    }

    data = json_write_minified_value(element->value, data);

    if (0 == data) {
      // value was malformed!
      return 0;
    }
  }

  *data++ = ']'; // close the array

  return data;
}

static char* json_write_minified_object(const struct json_object_s* object, char* data) {
  struct json_object_element_s* element = 0;

  *data++ = '{'; // open the object

  for (element = object->start; 0 != element; element = element->next) {
    if (element != object->start) {
      *data++ = ','; // ','s seperate each element
    }

    data = json_write_minified_string(element->name, data);

    if (0 == data) {
      // string was malformed!
      return 0;
    }

    *data++ = ':'; // ':'s seperate each name/value pair

    data = json_write_minified_value(element->value, data);

    if (0 == data) {
      // value was malformed!
      return 0;
    }
  }

  *data++ = '}'; // close the object

  return data;
}

static char* json_write_minified_value(const struct json_value_s* value, char* data) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 0;
  case json_type_number:
    return json_write_minified_number((struct json_number_s*)value->payload, data);
  case json_type_string:
    return json_write_minified_string((struct json_string_s*)value->payload, data);
  case json_type_array:
    return json_write_minified_array((struct json_array_s*)value->payload, data);
  case json_type_object:
    return json_write_minified_object((struct json_object_s*)value->payload, data);
  case json_type_true:
    data[0] = 't';
    data[1] = 'r';
    data[2] = 'u';
    data[3] = 'e';
    return data + 4;
  case json_type_false:
    data[0] = 'f';
    data[1] = 'a';
    data[2] = 'l';
    data[3] = 's';
    data[4] = 'e';
    return data + 5;
  case json_type_null:
    data[0] = 'n';
    data[1] = 'u';
    data[2] = 'l';
    data[3] = 'l';
    return data + 4;
  }
}

void* json_write_minified(const struct json_value_s* value, size_t* out_size) {
  size_t size = 0;
  char* data = 0;
  char* data_end = 0;

	if (0 == value) {
    return 0;
	}

  if (json_write_minified_get_value_size(value, &size)) {
    // value was malformed!
    return 0;
  }

  size += 1; // for the '\0' null terminating character

  data = malloc(size);

  if (0 == data) {
    // malloc failed!
    return 0;
  }

  data_end = json_write_minified_value(value, data);

  if (0 == data_end) {
    // bad chi occurred!
    free(data);
    return 0;
  }

  // null terminated the string
  *data_end = '\0';

  if (0 != out_size) {
    *out_size = size;
  }

  return data;
}


static int json_write_pretty_get_value_size(const struct json_value_s* value,
  size_t depth, size_t indent_size, size_t newline_size, size_t* size);

static int json_write_pretty_get_number_size(const struct json_number_s* number, size_t* size) {
  *size += number->number_size; // the actual string of the number

  return 0;
}

static int json_write_pretty_get_string_size(const struct json_string_s* string, size_t* size) {
  *size += string->string_size; // the actual string
  *size += 2; // need to encode the surrounding '"' characters

  return 0;
}

static int json_write_pretty_get_array_size(const struct json_array_s* array,
  size_t depth, size_t indent_size, size_t newline_size, size_t* size) {
  struct json_array_element_s* element;

  *size += 1; // '['
  *size += newline_size; // need a newline next

  if (1 < array->length) {
    *size += (array->length - 1); // ','s seperate each element
  }

  for (element = array->start; 0 != element; element = element->next) {
    // each element gets an indent and newline
    *size += (depth + 1) * indent_size;
    *size += newline_size;
    if (json_write_pretty_get_value_size(
      element->value, depth + 1, indent_size, newline_size, size)) {
      // value was malformed!
      return 1;
    }
  }

  *size += depth * indent_size;
  *size += 1; // ']'
  *size += newline_size; // need a newline next

  return 0;
}

static int json_write_pretty_get_object_size(const struct json_object_s* object,
  size_t depth, size_t indent_size, size_t newline_size, size_t* size) {
  struct json_object_element_s* element;

  *size += 1; // '{'
  *size += newline_size; // need a newline next

  if (1 < object->length) {
    *size += object->length - 1; // ','s seperate each element
  }

  for (element = object->start; 0 != element; element = element->next) {
    // each element gets an indent and newline
    *size += (depth + 1) * indent_size;
    *size += newline_size;

    if (json_write_pretty_get_string_size(element->name, size)) {
      // string was malformed!
      return 1;
    }

    *size += 3; // seperate each name/value pair with " : "

    if (json_write_pretty_get_value_size(element->value, depth + 1, indent_size, newline_size, size)) {
      // value was malformed!
      return 1;
    }
  }

  *size += depth * indent_size;
  *size += 1; // '}'

  if (0 != depth)
  {
    *size += newline_size; // need a newline next
  }

  return 0;
}

static int json_write_pretty_get_value_size(const struct json_value_s* value,
  size_t depth, size_t indent_size, size_t newline_size, size_t* size) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 1;
  case json_type_number:
    return json_write_pretty_get_number_size((struct json_number_s*)value->payload, size);
  case json_type_string:
    return json_write_pretty_get_string_size((struct json_string_s*)value->payload, size);
  case json_type_array:
    return json_write_pretty_get_array_size((struct json_array_s*)value->payload, depth, indent_size, newline_size, size);
  case json_type_object:
    return json_write_pretty_get_object_size((struct json_object_s*)value->payload, depth, indent_size, newline_size, size);
  case json_type_true:
    *size += 4; // the string "true"
    return 0;
  case json_type_false:
    *size += 5; // the string "false"
    return 0;
  case json_type_null:
    *size += 4; // the string "null"
    return 0;
  }
}

static char* json_write_pretty_value(const struct json_value_s* value,
  size_t depth, const char* indent, const char* newline, char* data);

static char* json_write_pretty_number(const struct json_number_s* number, char* data) {
  size_t i;

  for (i = 0; i < number->number_size; i++) {
    *data++ = number->number[i];
  }

  return data;
}

static char* json_write_pretty_string(const struct json_string_s* string, char* data) {
  size_t i;

  *data++ = '"'; // open the string

  for (i = 0; i < string->string_size; i++) {
    *data++ = ((char*)string->string)[i];
  }

  *data++ = '"'; // close the string

  return data;
}

static char* json_write_pretty_array(const struct json_array_s* array,
  size_t depth, const char* indent, const char* newline, char* data) {
  size_t k, m;
  struct json_array_element_s* element;

  *data++ = '['; // open the array

  for (k = 0; '\0' != newline[k]; k++) {
    *data++ = newline[k];
  }

  for (element = array->start; 0 != element; element = element->next) {
    if (element != array->start) {
      *data++ = ','; // ','s seperate each element

      for (k = 0; '\0' != newline[k]; k++) {
        *data++ = newline[k];
      }
    }

    for (k = 0; k < depth + 1; k++) {
      for (m = 0; '\0' != indent[m]; m++) {
        *data++ = indent[m];
      }
    }

    data = json_write_pretty_value(element->value, depth + 1, indent, newline, data);

    if (0 == data) {
      // value was malformed!
      return 0;
    }
  }

  for (k = 0; '\0' != newline[k]; k++) {
    *data++ = newline[k];
  }

  for (k = 0; k < depth; k++) {
    for (m = 0; '\0' != indent[m]; m++) {
      *data++ = indent[m];
    }
  }

  *data++ = ']'; // close the array

  return data;
}

static char* json_write_pretty_object(const struct json_object_s* object,
  size_t depth, const char* indent, const char* newline, char* data) {
  size_t k, m;
  struct json_object_element_s* element;

  *data++ = '{'; // open the object

  for (k = 0; '\0' != newline[k]; k++) {
    *data++ = newline[k];
  }

  for (element = object->start; 0 != element; element = element->next) {
    if (element != object->start) {
      *data++ = ','; // ','s seperate each element

      for (k = 0; '\0' != newline[k]; k++) {
        *data++ = newline[k];
      }
    }

    for (k = 0; k < depth + 1; k++) {
      for (m = 0; '\0' != indent[m]; m++) {
        *data++ = indent[m];
      }
    }

    data = json_write_pretty_string(element->name, data);

    if (0 == data) {
      // string was malformed!
      return 0;
    }

    // " : "s seperate each name/value pair
    *data++ = ' ';
    *data++ = ':';
    *data++ = ' ';

    data = json_write_pretty_value(element->value, depth + 1, indent, newline, data);

    if (0 == data) {
      // value was malformed!
      return 0;
    }
  }

  for (k = 0; '\0' != newline[k]; k++) {
    *data++ = newline[k];
  }

  for (k = 0; k < depth; k++) {
    for (m = 0; '\0' != indent[m]; m++) {
      *data++ = indent[m];
    }
  }

  *data++ = '}'; // close the object

  return data;
}

static char* json_write_pretty_value(const struct json_value_s* value,
  size_t depth, const char* indent, const char* newline, char* data) {
  switch (value->type) {
  default:
    // unknown value type found!
    return 0;
  case json_type_number:
    return json_write_pretty_number((struct json_number_s*)value->payload, data);
  case json_type_string:
    return json_write_pretty_string((struct json_string_s*)value->payload, data);
  case json_type_array:
    return json_write_pretty_array((struct json_array_s*)value->payload, depth, indent, newline, data);
  case json_type_object:
    return json_write_pretty_object((struct json_object_s*)value->payload, depth, indent, newline, data);
  case json_type_true:
    data[0] = 't';
    data[1] = 'r';
    data[2] = 'u';
    data[3] = 'e';
    return data + 4;
  case json_type_false:
    data[0] = 'f';
    data[1] = 'a';
    data[2] = 'l';
    data[3] = 's';
    data[4] = 'e';
    return data + 5;
  case json_type_null:
    data[0] = 'n';
    data[1] = 'u';
    data[2] = 'l';
    data[3] = 'l';
    return data + 4;
  }
}

void* json_write_pretty(const struct json_value_s* value,
  const char* indent,
  const char* newline,
  size_t* out_size) {
  size_t size = 0;
  size_t indent_size = 0;
  size_t newline_size = 0;
  char* data = 0;
  char* data_end = 0;

  if (0 == value) {
    return 0;
  }

  if (0 == indent) {
    indent = "  "; // default to two spaces
  }

  if (0 == newline) {
    newline = "\n"; // default to linux newlines
  }

  while ('\0' != indent[indent_size]) {
    ++indent_size; // skip non-null terminating characters
  }

  while ('\0' != newline[newline_size]) {
    ++newline_size; // skip non-null terminating characters
  }

  if (json_write_pretty_get_value_size(value, 0, indent_size, newline_size, &size)) {
    // value was malformed!
    return 0;
  }

  size += 1; // for the '\0' null terminating character

  data = malloc(size);

  if (0 == data) {
    // malloc failed!
    return 0;
  }

  data_end = json_write_pretty_value(value, 0, indent, newline, data);

  if (0 == data_end) {
    // bad chi occurred!
    free(data);
    return 0;
}

  // null terminated the string
  *data_end = '\0';

  if (0 != out_size) {
    *out_size = size;
  }

  return data;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
