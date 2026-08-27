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
#include "writer.h"
#include "codegen.h"

#include "tokenizer.c"
#include "parser.c"
#include "typer.c"
#include "pretty_printing.c"
#include "writer.c"
#include "codegen.c"

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: %s [filename]", argv[0]);
        return -1;
    }

    u64 file_size = 0;
    u8 *file = pal_read_entire_file(argv[1], &file_size);
    assert(file);

    mem_arena *arena = arena_init(gibibytes(1));
    assert(arena);

    tokenizer tokenizer;
    memory_zero(&tokenizer, sizeof(tokenizer));
    tokenizer.file = file;
    tokenizer.at = file;
    tokenizer.end = file + file_size;
    da_reserve(tokenizer.token_kinds, sizeof(token_kind) * 16384);
    da_reserve(tokenizer.token_positions, sizeof(u32) * 16384);
    assert(tokenizer.token_kinds);
    assert(tokenizer.token_positions);
    tokenizer.current_token = 0;
    tokenizer.current_line_number = 1;
    tokenizer.current_column_number = 0;
    tokenizer.file_path = (u8*)argv[1];
    tokenizer.start_of_current_line = tokenizer.at;
    tokenizer.error_count = 0;

    tokenize(&tokenizer);
    ast tree;
    memory_zero(&tree, sizeof(ast));
    tree.nodes = arena_push_array(arena, ast_node, 16384);
    tree.file_name = tokenizer.file_path;

    ast_node *root = parse_file(&tree, &tokenizer);

    typer tp;
    memory_zero(&tp, sizeof(typer));
    tp.file_name = tree.file_name;
    typecheck_file(&tp, &tokenizer, root);

    if(tree.error_count || tp.error_count) {
        return -1;
    } else {
        print_ast(root, 0);
    }

    printf("size_of_enum: %zu", sizeof(token_kind));
    generate_code(root, s("./test.exe"), tt_x64_pc_windows, output_format_pe);

    return 0;
}
