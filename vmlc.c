#define _CRT_SECURE_NO_WARNINGS
#define DS_IMPLEMENTATION
#include "ds.h"

#define PAL_IMPLEMENTATION
#include "pal.h"

#undef DS_IMPLEMENTATION
#undef PAL_IMPLEMENTATION

#define fatal_error(str, ...) printf(str, __VA_ARGS__); assert(0)

#include "tokenizer.h"
#include "tokenizer.c"

#include "parser.h"
#include "parser.c"

#include "pretty_printing.h"
#include "pretty_printing.c"

#include "codegen.h"
#include "codegen.c"

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

    token_stream stream = tokenize(&tokenizer);
    assert(stream.at);
    assert(stream.end);
    assert(stream.end > stream.at);

    ast ast = {0};
    ast.nodes = ARENA_PUSH_ARRAY(arena, ast_node, 16384);
    ast.node_count = 0;

    ast_node *root = parse_file(&ast, &stream);

    print_ast(root, 0);

    //generate_code(root);

    return 0;
}
