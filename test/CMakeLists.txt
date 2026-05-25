# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

cmake_minimum_required(VERSION 3.15)
project(json)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../README.md readme_md)
string(REPLACE ";" "@" readme_md "${readme_md}")
string(REGEX MATCHALL "```c[^`]*```" snippets "${readme_md}")
string(REPLACE "```c" "{" snippets ${snippets})
string(REPLACE "```" "}\n\n" snippets ${snippets})
string(REPLACE ";" "" snippets "${snippets}")
string(REPLACE "@" ";" snippets "${snippets}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/generated.h "${snippets}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR})

include(CTest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Base warning flags and whether -std= flags are supported.
# MSVC and clang-cl use /std: and don't accept -std=, so we skip
# the standard flag for those compilers.
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(JSON_C_WARNINGS "-Wall -Wextra -Werror")
  set(JSON_CXX_WARNINGS "-Wall -Wextra -Werror")
  set(JSON_USE_STD_FLAG TRUE)
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
  if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(JSON_C_WARNINGS "/Wall /WX /wd4514 /wd5045")
    set(JSON_CXX_WARNINGS "/Wall /WX /wd4514 /wd5045")
    set(JSON_USE_STD_FLAG FALSE)
  else()
    set(JSON_C_WARNINGS "-Wall -Wextra -Weverything -Werror -Wno-poison-system-directories")
    set(JSON_CXX_WARNINGS "-Wall -Wextra -Weverything -Werror -Wno-poison-system-directories")
    set(JSON_USE_STD_FLAG TRUE)
  endif()
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  set(JSON_C_WARNINGS "/Wall /WX /wd4514 /wd5045")
  set(JSON_CXX_WARNINGS "/Wall /WX /wd4514 /wd5045")
  set(JSON_USE_STD_FLAG FALSE)
else()
  message(WARNING "Unknown compiler '${CMAKE_C_COMPILER_ID}'!")
endif()

# Helper: set compile flags for a C source file (warnings + standard).
function(json_set_c_flags FILE STANDARD)
  if(JSON_USE_STD_FLAG)
    set_source_files_properties(${FILE} PROPERTIES
      COMPILE_FLAGS "${JSON_C_WARNINGS} ${STANDARD}")
  else()
    set_source_files_properties(${FILE} PROPERTIES
      COMPILE_FLAGS "${JSON_C_WARNINGS}")
  endif()
endfunction()

# Helper: set compile flags for a C++ source file (warnings + standard + extras).
function(json_set_cxx_flags FILE STANDARD)
  if(JSON_USE_STD_FLAG)
    set(EXTRA_FLAGS ${ARGN})
    set_source_files_properties(${FILE} PROPERTIES
      COMPILE_FLAGS "${JSON_CXX_WARNINGS} ${STANDARD} ${EXTRA_FLAGS}")
  else()
    set_source_files_properties(${FILE} PROPERTIES
      COMPILE_FLAGS "${JSON_CXX_WARNINGS}")
  endif()
endfunction()

# C files — each compiled under a specific C standard.
json_set_c_flags(main.c  "-std=gnu89")
json_set_c_flags(test.c  "-std=gnu89")
json_set_c_flags(test99.c "-std=c99")

list(FIND CMAKE_C_COMPILE_FEATURES c_std_11 IDX)
if(${IDX} GREATER -1)
  json_set_c_flags(test11.c "-std=c11")
endif()

# C++ files — each compiled under a specific C++ standard.
json_set_cxx_flags(test.cpp   "" "-Wno-c++98-compat")
json_set_cxx_flags(test98.cpp "-std=gnu++98")
json_set_cxx_flags(test11.cpp "-std=c++11" "-Wno-c++98-compat")
json_set_cxx_flags(test14.cpp "-std=c++14" "-Wno-c++98-compat")
json_set_cxx_flags(test17.cpp "-std=c++17" "-Wno-c++98-compat")
json_set_cxx_flags(test20.cpp "-std=c++20" "-Wno-c++98-compat")

set(JSON_TEST_SOURCES
  ../json.h
  utest.h
  test_shared.h
  main.c
  test.c
  test11.c
  test99.c
  test.cpp
  test98.cpp
  test11.cpp
  test14.cpp
  test17.cpp
  test20.cpp
)

# Helper: add a test executable and register it with CTest.
function(json_add_test NAME)
  add_executable(${NAME} ${JSON_TEST_SOURCES})
  add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
endfunction()

json_add_test(json_test)

if(MSVC)
  # Build a second test binary linked against the static CRT (/MT)
  # to ensure we work with both static and dynamic libc linkage.
  set_property(TARGET json_test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  json_add_test(json_mt_test)
  set_property(TARGET json_mt_test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Some sanitizer runtimes can't initialise on every host even when
# the toolchain links them fine — e.g. clang's TSan and MSan on
# aarch64 only ship shadow-memory layouts for 39-, 42- and 48-bit
# user VMAs, but Raspberry Pi OS' kernel is configured with
# CONFIG_ARM64_VA_BITS=47. The binary builds, but aborts before
# main with "FATAL: ThreadSanitizer: unsupported VMA range". Probe
# each candidate sanitizer with try_run and skip the ones whose
# runtime can't start. Result is cached so reconfigure is cheap;
# delete CMakeCache.txt to re-probe (e.g. after changing kernels).
function(json_check_sanitizer_runs SAN OUT_VAR)
  set(CACHE_VAR "JSON_SAN_${SAN}_RUNS")
  if(DEFINED ${CACHE_VAR})
    set(${OUT_VAR} ${${CACHE_VAR}} PARENT_SCOPE)
    return()
  endif()

  set(PROBE_SRC "${CMAKE_BINARY_DIR}/json_san_probe.c")
  file(WRITE "${PROBE_SRC}" "int main(void) { return 0; }\n")

  # COMPILE_DEFINITIONS / LINK_OPTIONS are the channels that try_run
  # actually forwards to the inner build's command line. CMAKE_FLAGS
  # with -DCMAKE_C_FLAGS=... silently does nothing here — the inner
  # try_compile project doesn't pick those up by default.
  try_run(RUN_RESULT COMPILE_RESULT
    "${CMAKE_BINARY_DIR}/json_san_probe_${SAN}"
    "${PROBE_SRC}"
    COMPILE_DEFINITIONS "-fsanitize=${SAN}" "-fno-omit-frame-pointer"
    LINK_OPTIONS "-fsanitize=${SAN}"
  )

  if(COMPILE_RESULT AND "${RUN_RESULT}" STREQUAL "0")
    set(RESULT TRUE)
    message(STATUS "Sanitizer ${SAN}: usable")
  else()
    set(RESULT FALSE)
    message(STATUS "Sanitizer ${SAN}: skipping "
                   "(compile=${COMPILE_RESULT}, run=${RUN_RESULT})")
  endif()

  set(${CACHE_VAR} ${RESULT} CACHE INTERNAL
      "Whether the ${SAN} sanitizer runtime starts on this host")
  set(${OUT_VAR} ${RESULT} PARENT_SCOPE)
endfunction()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang" AND NOT "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
  set(JSON_SANITIZER_CANDIDATES
    address
    undefined
    thread
  )

  if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    set(JSON_SANITIZER_CANDIDATES ${JSON_SANITIZER_CANDIDATES} memory)
  endif()

  set(JSON_SANITIZERS)
  foreach(JSON_SANITIZER ${JSON_SANITIZER_CANDIDATES})
    if(CMAKE_CROSSCOMPILING)
      # Can't run a probe under cross-compile — fall back to the old
      # behaviour of registering everything and letting the run fail
      # at ctest time on whoever owns the target environment.
      list(APPEND JSON_SANITIZERS ${JSON_SANITIZER})
    else()
      json_check_sanitizer_runs(${JSON_SANITIZER} JSON_SAN_RUNS)
      if(JSON_SAN_RUNS)
        list(APPEND JSON_SANITIZERS ${JSON_SANITIZER})
      endif()
    endif()
  endforeach()

  foreach(JSON_SANITIZER ${JSON_SANITIZERS})
    json_add_test(json_${JSON_SANITIZER}_test)
    target_compile_options(json_${JSON_SANITIZER}_test PRIVATE -fsanitize=${JSON_SANITIZER} -fno-omit-frame-pointer)
    target_link_options(json_${JSON_SANITIZER}_test PRIVATE -fsanitize=${JSON_SANITIZER})
  endforeach()
endif()
