#ifndef X64_BACKEND_H
#define X64_BACKEND_H

#include "parser.h"

void generate_pe_file(ast_node *root, string8 output_path, string8 *import_function_names, u32 import_function_count);

#endif //X64_BACKEND_H

