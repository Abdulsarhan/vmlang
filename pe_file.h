#ifndef PE_FILE_H
#define PE_FILE_H

#include "parser.h"
#include "codegen.h"

void generate_pe_file(mem_arena *arena, string8 output_path, generated_code *code);

#endif //PE_FILE_H
