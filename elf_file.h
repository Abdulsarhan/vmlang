#ifndef ELF_FILE_H
#define ELF_FILE_H

#include "ds.h"
#include "parser.h"
#include "codegen.h"

void generate_elf_file(ast_node *root, string8 output_path, generated_code *code);

#endif //ELF_FILE_H
