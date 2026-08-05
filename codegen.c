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
                const u8 text[] = {0x48, 0x83, 0xEC, 0x28,                                  // sub rsp, 0x28
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
                const u32 size_of_text = sizeof(text);

                const u8 rodata[] = "Hello from pe!";
                const u32 size_of_rodata = sizeof(rodata) - 1;

                const u8 data[] = {0,0,0,0};
                const u32 size_of_data = sizeof(data);


                const u32 size_of_uninit_data = 0;

                string8 kernel32_functions[] = {
                                                   STR8_LIT("ExitProcess"),
                                                   STR8_LIT("WriteConsoleA"),
                                                   STR8_LIT("GetStdHandle"),
                                               };

                import imports[] = {
                                       {
                                        .dll_name = STR8_LIT("Kernel32.dll"),
                                        .function_names = kernel32_functions,
                                        .function_count = array_count(kernel32_functions),
                                       },
                                   };

                generate_pe_file(root,
                                 output_path,
                                 text,
                                 size_of_text,
                                 data,
                                 size_of_data,
                                 rodata,
                                 size_of_rodata,
                                 size_of_uninit_data,
                                 imports,
                                 array_count(imports));
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
