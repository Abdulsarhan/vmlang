#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "parser.h"

typedef struct type type;
struct type {
    type_kind kind;
    type_kind pointee; // for pointers
    type_kind return_type;
    type_kind *param_types; // dynamic array
};

typedef struct symbol symbol;
struct symbol {
    string8 name;
    // for now, we have the type in the symbol directly.
    // in the future, it might make sense to have the types in a
    // separate type table so that we don't have duplicate types.
    // this gets us faster compares between different types since
    // comparing two pointers is faster than comparing types byte
    // by byte. Also, it would give us better memory usage because
    // the type struct is kind of big.
    type tp;
    ast_node *node;
};

typedef struct scope scope;
struct scope {
    symbol *symbols; // hashmap of symbols
};

typedef struct typer typer;
struct typer {
    i32 error_count;
    i32 loop_depth;
    i32 switch_depth;

    scope scope_stack[1024];
    i32 scope_depth;
    i32 highest_scope_depth;

    u8 *file_name;

    type *current_func_return_type;
};

// phase 0
void typecheck_file(typer *tp, ast_node *root);

// phase 1
void typechecker_prepass(typer *tp, ast_node *root);
void typecheck_declaration_header(typer *tp, ast_node *decl);

// phase 2
void typecheck_declaration_body(typer *tp, ast_node *decl);
void typecheck_func_decl_params(typer *tp, ast_node **params);
void typecheck_block(typer *tp, ast_node *block);
void typecheck_statement(typer *tp, ast_node *statement);
void typecheck_expression(typer *tp, ast_node *expr);

// error reporting
void typer_report_error(typer *tp, ast_node *node, const char *fmt, ...);

// scope management & lookup
void scope_add_symbol(typer *tp, symbol sym);
symbol *scope_lookup(typer *tp, string8 name);
symbol *scope_get_symbol(scope scope, string8 name);

#endif // TYPECHECKER_H
