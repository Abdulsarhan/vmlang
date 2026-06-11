#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "parser.h"

typedef struct typer typer;
struct typer {
    i32 loop_depth;
    i32 switch_depth;
    i32 error_count;
};

void typecheck_file(typer *tp, ast_node *root);
void typecheck_declaration(typer *tp, ast_node *decl);
void typecheck_func_decl_params(typer *tp, ast_node **params);
void typecheck_block(typer *tp, ast_node *block);
void typecheck_statement(typer *tp, ast_node *statement);
    
#endif // TYPECHECKER_H
