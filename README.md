# json.h #

[![Build status](https://ci.appveyor.com/api/projects/status/piell6hcrlwrcxp9?svg=true)](https://ci.appveyor.com/project/sheredom/json-h)

[![Build status](https://api.travis-ci.org/repositories/sheredom/json.h.svg)](https://travis-ci.org/sheredom/json.h)

A simple one header/one source solution to parsing JSON in C and C++.

JSON is parsed into a read-only, single allocation buffer.

## Usage ##

Just include json.h, json.c in your code!

The current supported compilers are gcc, clang and msvc.

The current tested compiler versions are gcc 4.8.2, clang 3.5 and MSVC 18.0.21005.1.

## Design ##

The json_parse function calls malloc once, and then slices up this single
allocation to support all the weird and wonderful JSON structures you can
imagine!

The structure of the data is always the json structs first (which encode the
structure of the original JSON), followed by the data.

## License ##

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
