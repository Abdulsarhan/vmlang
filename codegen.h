#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

typedef enum compilation_target {
    tt_sea_of_nodes,
    tt_bytecode,

    tt_x86_pc_windows,
    tt_x64_pc_windows,
    tt_arm64_pc_windows,

    tt_x86_pc_linux,
    tt_x64_pc_linux,
}compilation_target;

typedef enum output_format {
    output_format_asm,
    output_format_pe,
    output_format_elf,
}output_format;

typedef struct import import;
struct import {
    string8 dll_name;
    string8 *function_names;
    u32 function_count;
};

typedef struct generated_code generated_code;
struct generated_code {
    u8 *text;
    u32 size_of_text;

    u8 *data;
    u32 size_of_data;

    u8 *rodata;
    u32 size_of_rodata;

    u32 size_of_bss;

    import *imports;
    u32 import_count;

    u32 num_sections;
};

void generate_code(ast_node *root, string8 output_path, compilation_target target, output_format format);

#endif //CODEGEN_H
