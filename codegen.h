#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

typedef enum compilation_target {
    tt_llvm_ir,
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

void generate_code(ast_node *root, string8 output_path, compilation_target target, output_format format);

#endif //CODEGEN_H
