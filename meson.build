###################################################################################
#                                                                                 #
# NAME: meson.build                                                               #
#                                                                                 #
# AUTHOR: Boris Claas Kiefer.                                                     #
# WRITTEN BY: Michael Brockus.                                                    #
#                                                                                 #
# License: Unlicense                                                              #
#                                                                                 #
###################################################################################



project('json.h', 'c', 'cpp',
    license         : 'Unlicense',
    meson_version   : '>=0.50.0',
    default_options : 
    [ 
        'werror=true',
        'optimization=3', 
        'warning_level=3', 
        'b_sanitize=address,undefined', 
        'b_lto=true', 
        'b_lundef=true',
        'cpp_std=c++17'
    ])
cpp = meson.get_compiler('cpp')
cc = meson.get_compiler('c')



##
#
# Meson: Add compiler flags
#
##
if cc.get_id()  == 'clang' and cpp.get_id() == 'clang'
    list_of_clang_flags = [
            '-Wweak-vtables', 
            '-Wexit-time-destructors',
            '-Wglobal-constructors', 
            '-Wmissing-noreturn' 
            ]

    add_project_arguments(
        cpp.get_supported_arguments(list_of_clang_flags), 
        language: 'cpp')

    add_project_arguments(
        cc.get_supported_arguments(list_of_clang_flags), 
        language: 'c')
endif

if cc.get_argument_syntax() == 'gcc' and cpp.get_argument_syntax() == 'gcc'
    list_of_gcc_flags = [
            '-Wunreachable-code', 
            '-Wmissing-declarations',
            '-Wmissing-prototypes',
            '-Wredundant-decls',
            '-Wundef',
            '-Wwrite-strings',
            '-Wformat',
            '-Wformat-nonliteral',
            '-Wformat-security',
            '-Wold-style-definition',
            '-Winit-self',
            '-Wmissing-include-dirs',
            '-Waddress',
            '-Waggregate-return',
            '-Wno-multichar',
            '-Wno-parentheses',
            '-Wno-unused-parameter',
            '-Wno-type-limits',
            '-Wdeclaration-after-statement',
            '-Wvla',
            '-Wpointer-arith'
            ]
    
    add_project_arguments(
        cpp.get_supported_arguments(list_of_gcc_flags), 
        language: 'cpp')

    add_project_arguments(
        cc.get_supported_arguments(list_of_gcc_flags), 
        language: 'c')
endif

if cc.get_id() == 'msvc' and cpp.get_id() == 'msvc'
    list_of_msvc_flags = [
            '/w44265', 
            '/w44061', 
            '/w44062', 
            '/wd4018',
            '/wd4146',
            '/wd4244',
            '/wd4305'
        ]
    add_project_arguments(
        cpp.get_supported_arguments(), 
        language: 'cpp')

    add_project_arguments(
        cc.get_supported_arguments(), 
        language: 'c')
endif

json_h_dir = include_directories('.')

json_h_lib = static_library(meson.project_name(), 
    sources: ['json.c'],
    include_directories: json_h_dir)

json_h_dep = declare_dependency(
    link_with: json_h_lib,
    include_directories: json_h_dir)
