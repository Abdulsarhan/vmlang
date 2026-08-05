#ifndef X64_BACKEND_H
#define X64_BACKEND_H

#include "parser.h"

typedef struct import import;
struct import {
    string8 dll_name;
    string8 *function_names;
    u32 function_count;
};

void generate_pe_file(ast_node *root,
                      string8 output_path,
                      const u8 *text,
                      u32 size_of_text,
                      const u8 *data,
                      u32 size_of_data,
                      const u8 *rodata,
                      u32 size_of_rodata,
                      u32 size_of_bss,
                      const import *imports,
                      u32 import_count);
#endif //X64_BACKEND_H

