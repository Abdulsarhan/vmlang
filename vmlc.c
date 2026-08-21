#define _CRT_SECURE_NO_WARNINGS
#define DS_IMPLEMENTATION
#include "ds.h"

#define PAL_IMPLEMENTATION
#include "pal.h"

#undef DS_IMPLEMENTATION
#undef PAL_IMPLEMENTATION

#include "tokenizer.h"
#include "parser.h"
#include "typer.h"
#include "pretty_printing.h"
#include "codegen.h"
#include "writer.h"
#include "pe_file.h"
#include "elf_file.h"

#include "tokenizer.c"
#include "parser.c"
#include "typer.c"
#include "pretty_printing.c"
#include "codegen.c"
#include "writer.c"
#include "pe_file.c"
#include "elf_file.c"

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: %s [filename]", argv[0]);
    }

    u64 file_size = 0;
    unsigned char *file = pal_read_entire_file(argv[1], &file_size);
    assert(file);

    mem_arena *arena = arena_init(gibibytes(1));
    assert(arena);

    tokenizer tokenizer = {0};
    tokenizer.at = file;
    tokenizer.end = file + file_size;
    tokenizer.tokens = ARENA_PUSH_ARRAY(arena, token, 16384);
    tokenizer.token_count = 0;
    tokenizer.current_line_number = 1;
    tokenizer.current_column_number = 0;
    tokenizer.file_path = (u8*)argv[1];
    tokenizer.start_of_current_line = tokenizer.at;
    tokenizer.error_count = 0;

    token_stream stream = tokenize(&tokenizer);
    assert(stream.at);

    ast tree = {0};
    memory_zero(&tree, sizeof(ast));
    tree.nodes = ARENA_PUSH_ARRAY(arena, ast_node, 16384);
    tree.file_name = stream.file_name;

    ast_node *root = parse_file(&tree, &stream);

    typer tp;
    memset(&tp, 0, sizeof(typer));
    tp.file_name = tree.file_name;
    typecheck_file(&tp, root);

    if(tree.error_count || tp.error_count) {
        return -1;
    } else {
        print_ast(root, 0);
    }

    generate_code(root, s("./test.exe"), tt_x64_pc_windows, output_format_pe);

    return 0;
}
