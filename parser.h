#ifndef PARSER_H
#define PARSER_H

#include "ds.h"
#include "tokenizer.h"

// declared in order of root to leaves
typedef enum node_kind {
    NODE_KIND_FILE,
    NODE_KIND_IF,
    NODE_KIND_FOR,
    NODE_KIND_WHILE,
    NODE_KIND_RETURN,
    NODE_KIND_BREAK,
    NODE_KIND_CONTINUE,
    NODE_KIND_IDENT,
    NODE_KIND_FUNCTION_DECLARATION,
    NODE_KIND_FUNCTION_PARAMETER,
    NODE_KIND_STRUCT,
    NODE_KIND_UNION,
    NODE_KIND_BLOCK,
    NODE_KIND_ENUM,
    NODE_KIND_UNARY,
    NODE_KIND_BINOP,
    NODE_KIND_FUNCTION_CALL,
    NODE_KIND_INT_LIT,
    NODE_KIND_FLOAT_LIT,
    NODE_KIND_STRING_LIT,
    NODE_KIND_CHAR_LIT,
    NODE_KIND_BOOL_LIT,
}node_kind;

// maybe sort these by precedence?
typedef enum binop_kind {
    BINOP_NONE = 0,

    //single character operators.
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_COLON,
    BINOP_ASSIGN,
    BINOP_BITWISE_AND,
    BINOP_BITWISE_OR,
    BINOP_BITWISE_XOR,
    BINOP_GREATER_THAN,
    BINOP_LESS_THAN,

    BINOP_RANGE,
    BINOP_ARRAY_SUBSCRIPT,
    BINOP_MEMBER_ACCESS,

    BINOP_LOGICAL_NOT, // !=
    BINOP_LOGICAL_AND, // &&
    BINOP_LOGICAL_OR, // ||

    BINOP_RIGHT_SHIFT, // >>
    BINOP_LEFT_SHIFT, // <<

    BINOP_ADD_ASSIGN, // +=
    BINOP_MINUS_ASSIGN, // -=
    BINOP_DIV_ASSIGN, // /=
    BINOP_MOD_ASSIGN, // %=
    BINOP_MUL_ASSIGN, // *=

    BINOP_XOR_ASSIGN, // ^=
    BINOP_AND_ASSIGN, // &=
    BINOP_OR_ASSIGN, // |=
    BINOP_LEFT_SHIFT_ASSIGN, // <<=
    BINOP_RIGHT_SHIFT_ASSIGN,// >>=
    BINOP_GREATER_THAN_ASSIGN,// >=
    BINOP_LESS_THAN_ASSIGN,// <=

    BINOP_EQUALS, // ==
    BINOP_COLON_COLON,
    BINOP_COLON_EQUAL, // :=
    BINOP_RIGHT_ARROW, // ->
}binop_kind;

typedef enum unary_kind {
    UNARY_NONE = 0,

    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_ADDRESS_OF,
    UNARY_DEREF, // not sure which character I will use for this quite yet.
    UNARY_LOGICAL_NOT,
    UNARY_BITWISE_NOT,
    UNARY_PRE_INCREMENT,
    UNARY_POST_INCREMENT,
    UNARY_PRE_DECREMENT,
    UNARY_POST_DECREMENT,
}unary_kind;

typedef struct ast_node ast_node;

typedef struct ast_node_unary ast_node_unary;
struct ast_node_unary {
    unary_kind kind;
    ast_node *operand;
};

typedef struct ast_node_ident ast_node_ident;
struct ast_node_ident {
    string8 name;
};

typedef struct ast_node_binop ast_node_binop;
struct ast_node_binop {
    binop_kind kind; // we need this to specify what kind of binop this is.
    ast_node *left;
    ast_node *right;
};

// Not sure if I like this. Might just have the blocks
// in the nodes that belong to them.

typedef struct ast_node_block ast_node_block;
struct ast_node_block {
    ast_node **statements; // dynamic array.
};

typedef struct ast_node_function_declaration ast_node_function_declaration;
struct ast_node_function_declaration {
    ast_node *callee;
    ast_node **params; // dynamic array.
    ast_node *block;
    ast_node *return_type;
};

typedef struct ast_node_function_call ast_node_function_call;
struct ast_node_function_call {
    ast_node *callee;
    ast_node **params; // dynamic array.
};

typedef struct ast_node_return ast_node_return;
struct ast_node_return {
    ast_node *expression;
};

typedef struct ast_node_if ast_node_if;
struct ast_node_if {
    ast_node *cond;
    ast_node *then_block;
    ast_node *else_part;
};

typedef struct ast_node_while ast_node_while;
struct ast_node_while {
    ast_node *cond;
    ast_node *block;
};

typedef struct ast_node_for ast_node_for;
struct ast_node_for {
    ast_node *range;
    ast_node *block;
};

typedef struct ast_node_file ast_node_file;
struct ast_node_file {
    ast_node **declarations;
};

typedef struct ast_node_int_lit ast_node_int_lit;
struct ast_node_int_lit {
    i64 value;
};

typedef struct ast_node_float_lit ast_node_float_lit;
struct ast_node_float_lit {
    f64 value;
};

typedef struct ast_node_str_lit ast_node_str_lit;
struct ast_node_str_lit {
    string8 value;
};

typedef struct ast_node_char_lit ast_node_char_lit;
struct ast_node_char_lit {
    u8 value;
};

typedef struct ast_node_union ast_node_union;
struct ast_node_union {
    ast_node *ident;
    ast_node *block;
};

typedef struct ast_node_struct ast_node_struct;
struct ast_node_struct {
    ast_node *ident;
    ast_node *block;
};

typedef struct ast_node_enum ast_node_enum;
struct ast_node_enum {
    ast_node *ident;
    ast_node *block;
};

struct ast_node {
    node_kind kind;
    union {
        ast_node_file file;

        ast_node_for for_loop;
        ast_node_while while_loop;
        ast_node_if if_stmt;
        ast_node_return return_stmt;
        ast_node_function_declaration func_decl;
        ast_node_block block;
        ast_node_binop binop;
        ast_node_unary unary;
        ast_node_function_call function_call;
        ast_node_enum enum_decl;
        ast_node_union union_decl;
        ast_node_struct struct_decl;
        ast_node_int_lit int_lit;
        ast_node_float_lit float_lit;
        ast_node_str_lit str_lit;
        ast_node_char_lit char_lit;
        ast_node_ident ident;
    };
};

typedef struct ast ast;
struct ast {
    ast_node *nodes;
    size_t node_count;
};

ast_node *parse_file(ast *ast, token_stream *tok_stream);
ast_node *parse_infix_and_postfix(ast *ast, token_stream *tok_stream, i32 info, ast_node *left);
ast_node *parse_expression(ast *ast, token_stream *tok_stream, int min_prec);
ast_node *parse_statement(ast *ast, token_stream *tok_stream);
ast_node *parse_else_or_else_if(ast *ast, token_stream *tok_stream);
ast_node *parse_if(ast *ast, token_stream *tok_stream);

#endif /* PARSER_H */
