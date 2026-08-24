#include "codegen.h"

// backends
#include "sea_of_nodes.h"
#include "pe_file.h"
#include "elf_file.h"

#include "sea_of_nodes.c"
#include "pe_file.c"
#include "elf_file.c"

global u8 text[] = { 0x48, 0x83, 0xEC, 0x28,                                  // sub rsp, 0x28
                     0xB9, 0xF5, 0xFF, 0xFF, 0xFF,                            // mov ecx, -11
                     0xFF, 0x15, 0x59, 0x30, 0x00, 0x00,                      // call [GetStdHandle]
                     0x48, 0x89, 0xC1,                                        // mov rcx, rax
                     0x48, 0x8D, 0x15, 0xE7, 0x0F, 0x00, 0x00,                // lea rdx, [start of rodata]
                     0x41, 0xB8, 0x0E, 0x00, 0x00, 0x00,                      // mov r8d, 0x0E
                     0x4C, 0x8D, 0x0D, 0xDA, 0x1F, 0x00, 0x00,                // lea r9, [start of data]
                     0x48, 0xC7, 0x44, 0x24, 0x20, 0x00, 0x00, 0x00, 0x00,    // mov qword ptr [rsp + 0x20], 0
                     0xFF, 0x15, 0x2B, 0x30, 0x00, 0x00,                      // call [WriteConsoleA]
                     0x31, 0xC9,                                              // xor ecx, ecx
                     0xFF, 0x15, 0x1B, 0x30, 0x00, 0x00};                     // call [ExitProcess]

global u8 rodata[] = "Hello from pe!";

global u8 data[] = { 0, 0, 0, 0 };

global string8 kernel32_functions[] = { s("ExitProcess"), s("WriteConsoleA"), s("GetStdHandle") };

global import imports[] = {
                              {
                               .dll_name = s("Kernel32.dll"),
                               .function_names = kernel32_functions,
                               .function_count = array_count(kernel32_functions),
                              },
                          };
generated_code generate_code_for_x64_pc_windows_from_sea_of_nodes(mem_arena *arena) {
    generated_code code;
    code.text = text;
    code.size_of_text = sizeof(text);
    code.data = data;
    code.size_of_data = sizeof(data);
    code.rodata = rodata;
    code.size_of_rodata = sizeof(rodata) - 1;
    code.size_of_bss = 0;
    code.imports = imports;
    code.import_count = array_count(imports);
    code.num_sections = (sizeof(text) != 0) + (sizeof(data) != 0) + (sizeof(rodata) != 0) + (array_count(imports) != 0);
    return code;
}

generated_code generate_code_for_x64_pc_linux_from_sea_of_nodes(mem_arena *arena) {
    // TODO: Implement this function properly.
    generated_code code;
    code.text = 0;
    code.size_of_text = 0;
    code.data = 0;
    code.size_of_data = 0;
    code.rodata = 0;
    code.size_of_rodata = 0;
    code.size_of_bss = 0;
    code.imports = 0;
    code.import_count = 0;
    code.num_sections = 0;
    return code;
}

// NOTE: Decoupling the codegen from writing out the specific executable
// format was probably a good idea because now we can use the same code to
// write out an executable file for different instruction set architectures.

void generate_code(ast_node *root,
                   string8 output_path,
                   compilation_target target,
                   output_format format) {
    mem_arena *arena = arena_init(gibibytes(8));

    generate_sea_of_nodes(root);
    switch(target) {
        case tt_sea_of_nodes:
            break;
        case tt_bytecode:
            break;
        case tt_x86_pc_windows:
            break;
        case tt_x64_pc_windows: {
            if(format == output_format_pe) {
                generated_code code = generate_code_for_x64_pc_windows_from_sea_of_nodes(arena);
                generate_pe_file(arena, output_path, &code);
            }
        } break;
        case tt_arm64_pc_windows: {

        } break;
        case tt_x86_pc_linux: {

        } break;
        case tt_x64_pc_linux: {
            if(format == output_format_elf) {
                generated_code code = generate_code_for_x64_pc_linux_from_sea_of_nodes(arena);
                generate_elf_file(root, output_path, &code);
            }
        } break;
    }
}
