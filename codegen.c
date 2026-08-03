#include "codegen.h"
#include "x64_backend.h"

void generate_code(ast_node *root,
                   string8 output_path,
                   compilation_target target,
                   output_format format) {
    switch(target) {
        case tt_llvm_ir:
            break;
        case tt_bytecode:
            break;
        case tt_x86_pc_windows:
            break;
        case tt_x64_pc_windows:
            if(format == output_format_pe) {
                string8 import_function_names[] = {STR8_LIT("ExitProcess"), STR8_LIT("WriteConsoleA"), STR8_LIT("GetStdHandle")};
                generate_pe_file(root, output_path, import_function_names, array_count(import_function_names));
            }
            break;
        case tt_arm64_pc_windows:
            break;
        case tt_x86_pc_linux:
            break;
        case tt_x64_pc_linux:
            break;
    }
}
