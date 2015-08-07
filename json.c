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
#endif

struct json_parse_state_s {
  const char* src;
  size_t size;
  size_t offset;
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
    case ' ':
    case '\n':
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
    // expected string to begin with '"'!
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
        // invalid escaped sequence in string!
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
          return 1;
        } else if (
          !json_is_hexadecimal_digit(state->src[state->offset + 1]) ||
          !json_is_hexadecimal_digit(state->src[state->offset + 2]) ||
          !json_is_hexadecimal_digit(state->src[state->offset + 3]) ||
          !json_is_hexadecimal_digit(state->src[state->offset + 4])) {
          // escaped unicode sequences must contain 4 hexadecimal digits!
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

  if ('{' != state->src[state->offset]) {
    // expected object to begin with leading '{'
    return 1;
  }

  // skip leading '{'
  state->offset++;

  state->dom_size += sizeof(struct json_object_s);

  while (state->offset < state->size) {
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

    // if we parsed at least once element previously, grok for a comma
    if (0 < elements) {
      if (',' != state->src[state->offset]) {
        // expected a comma where there was none!
        return 1;
      }

      // skip comma
      state->offset++;

      if (json_skip_whitespace(state)) {
        // reached end of buffer before object was complete!
        return 1;
      }
    }

    if (json_get_string_size(state)) {
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

    if (json_get_value_size(state)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed a name/value pair!
    elements++;
  }

  state->dom_size += sizeof(struct json_string_s) * elements;
  state->dom_size += sizeof(struct json_value_s) * elements;

  return 0;
}

static int json_get_array_size(struct json_parse_state_s* state) {
  size_t elements = 0;

  if ('[' != state->src[state->offset]) {
    // expected array to begin with leading '['
    return 1;
  }

  // skip leading '['
  state->offset++;

  state->dom_size += sizeof(struct json_array_s);

  while (state->offset < state->size) {
    if (json_skip_whitespace(state)) {
      // reached end of buffer before array was complete!
      return 1;
    }

    if (']' == state->src[state->offset]) {
      // skip trailing ']'
      state->offset++;

      // finished the object!
      break;
    }

    // if we parsed at least once element previously, grok for a comma
    if (0 < elements) {
      if (',' != state->src[state->offset]) {
        // expected a comma where there was none!
        return 1;
      }

      // skip comma
      state->offset++;

      if (json_skip_whitespace(state)) {
        // reached end of buffer before array was complete!
        return 1;
      }
    }

    if (json_get_value_size(state)) {
      // value parsing failed!
      return 1;
    }

    // successfully parsed an array element!
    elements++;
  }

  state->dom_size += sizeof(struct json_value_s) * elements;

  return 0;
}

static int json_get_number_size(struct json_parse_state_s* state) {
  (void)state;
  return 1;
}

static int json_get_value_size(struct json_parse_state_s* state) {
  if (json_skip_whitespace(state)) {
    // consumed the whole buffer when we expected a value!
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
  size_t count_offset = 0;
  size_t object_depth = 1;
  size_t array_depth = 0;

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

  count_offset = state->offset;

  // need to count number of name/value pairs first
  while(0 != object_depth) {
    if ('"' == state->src[count_offset]) {
      // skip string separately incase they have '{' or '}' in them
      do {
        count_offset++;
      } while (('"' != state->src[count_offset]) ||
        ('\\' == state->src[count_offset - 1] &&
        '"' == state->src[count_offset]));

      // skip trailing '"'
      count_offset++;
    } else if ('[' == state->src[count_offset]) {
      // entering a child array
      array_depth++;
    } else if (']' == state->src[count_offset]) {
      // leaving an array
      array_depth--;
    } else if ('{' == state->src[count_offset]) {
      // entering a child object
      object_depth++;
    } else if ('}' == state->src[count_offset]) {
      // leaving an object
      object_depth--;
    } else if (1 == object_depth && 0 == array_depth &&
      ',' == state->src[count_offset]) {
      // if we are on our parent object, and not within an array,
      // if we find a comma we have found another element
      elements++;
    }

    count_offset++;
  }

  object->length = elements;
  if (0 == elements) {
    object->names = 0;
    object->values = 0;

    if (json_skip_whitespace(state)) {
      // reached end of buffer before object was complete!
      return 1;
    }

    if ('}' != state->src[state->offset]) {
      // expected an empty object!
      return 1;
    }

    // skip trailing '}'
    state->offset++;
  } else {
    object->names = (struct json_string_s* )state->dom;
    state->dom += sizeof(struct json_string_s) * elements;
    object->values = (struct json_value_s* )state->dom;
    state->dom += sizeof(struct json_value_s) * elements;

    // reset elements
    elements = 0;

    while (state->offset < state->size) {
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

      // if we parsed at least once element previously, grok for a comma
      if (0 < elements) {
        if (',' != state->src[state->offset]) {
          // expected a comma where there was none!
          return 1;
        }

        // skip comma
        state->offset++;

        if (json_skip_whitespace(state)) {
          // reached end of buffer before object was complete!
          return 1;
        }
      }

      if (json_parse_string(state, object->names + elements)) {
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

      if (json_parse_value(state, object->values + elements)) {
        // value parsing failed!
        return 1;
      }

      // successfully parsed a name/value pair!
      elements++;
    }
  }

  return 0;
}

static int json_parse_array(struct json_parse_state_s* state,
  struct json_array_s* array) {
  size_t elements = 0;
  size_t count_offset = 0;
  size_t array_depth = 1;
  size_t object_depth = 0;

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

  if (']' != state->src[state->offset]) {
    // we have at least one element as we definitely don't have
    // an empty array [   ]!
    elements++;
  }

  count_offset = state->offset;

  // need to count number of array elements first
  while(0 != array_depth) {
    if ('"' == state->src[count_offset]) {
      // skip string separately incase they have '{' or '}' in them
      do {
        count_offset++;
      } while (('"' != state->src[count_offset]) ||
        ('\\' == state->src[count_offset - 1] &&
        '"' == state->src[count_offset]));

      // skip trailing '"'
      count_offset++;
    } else if ('[' == state->src[count_offset]) {
      // entering a child array
      array_depth++;
    } else if (']' == state->src[count_offset]) {
      // leaving an array
      array_depth--;
    } else if ('{' == state->src[count_offset]) {
      // entering a child object
      object_depth++;
    } else if ('}' == state->src[count_offset]) {
      // leaving an object
      object_depth--;
    } else if (1 == array_depth && 0 == object_depth &&
      ',' == state->src[count_offset]) {
      // if we are on our parent array, and not within an object,
      // if we find a comma we have found another element
      elements++;
    }

    count_offset++;
  }

  array->length = elements;
  if (0 == elements) {
    array->values = 0;

    if (json_skip_whitespace(state)) {
      // reached end of buffer before array was complete!
      return 1;
    }

    if (']' != state->src[state->offset]) {
      // expected empty array!
      return 1;
    }

    // skip trailing ']'
    state->offset++;
  } else {
    array->values = (struct json_value_s* )state->dom;
    state->dom += sizeof(struct json_value_s) * elements;

    // reset elements
    elements = 0;

    while (state->offset < state->size) {
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

      // if we parsed at least once element previously, grok for a comma
      if (0 < elements) {
        if (',' != state->src[state->offset]) {
          // expected a comma where there was none!
          return 1;
        }

        // skip comma
        state->offset++;

        if (json_skip_whitespace(state)) {
          // reached end of buffer before array was complete!
          return 1;
        }
      }

      if (json_parse_value(state, array->values + elements)) {
        // value parsing failed!
        return 1;
      }

      // successfully parsed an array element!
      elements++;
    }
  }

  return 0;
}

static int json_parse_number(struct json_parse_state_s* state,
  struct json_number_s* number) {
  (void)state; (void)number;
  return 1;
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

struct json_value_s* json_parse(const void* src, size_t src_size) {
  struct json_parse_state_s state;
  void* allocation;

  if (0 == src || 2 > src_size) {
    // absolute minimum valid json is either "{}" pr "[]"
    return 0;
  }

  state.src = src;
  state.size = src_size;
  state.offset = 0;
  state.dom_size = 0;
  state.data_size = 0;

  if (json_get_value_size(&state)) {
    // parsing value's size failed (most likely an invalid JSON DOM!)
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

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
