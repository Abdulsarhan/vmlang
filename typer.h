#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "parser.h"

typedef struct typer typer;
struct typer {
    i32 loop_depth;
    i32 switch_depth;
    i32 error_count;
    u8 *file_name;
    string8 *symbols; // dynamic array.
};

typedef enum type_kind {
    TYPE_KIND_INT_LIT,
    TYPE_KIND_FLOAT_LIT,
    TYPE_KIND_CHAR_LIT,
    TYPE_KIND_BOOL_LIT,
    TYPE_KIND_STR_LIT,
}type_kind;

typedef struct type type;
struct type {
    type_kind kind;

};

void typecheck_file(typer *tp, ast_node *root);
void typecheck_declaration(typer *tp, ast_node *decl);
void typecheck_func_decl_params(typer *tp, ast_node **params);
void typecheck_block(typer *tp, ast_node *block);
void typecheck_statement(typer *tp, ast_node *statement);
    
#endif // TYPECHECKER_H
