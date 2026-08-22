#include "vir.h"

// vir is the name of our intermediate representation.
void generate_vir(ast_node *root) {
    if(!root) {
        return;
    }

    switch(root->kind) {
        case NODE_KIND_NONE: {
            printf("(unknown node kind: %d)\n", root->kind);
        } break;
        case NODE_KIND_FILE: {
        } break;
        case NODE_KIND_IF: {
        } break;
        case NODE_KIND_FOR: {
        } break;
        case NODE_KIND_WHILE: {
        } break;
        case NODE_KIND_FUNCTION_DECLARATION: {
        } break;
        case NODE_KIND_FUNCTION_PARAMETER: {
        } break;
        case NODE_KIND_STRUCT: {
        } break;
        case NODE_KIND_UNION: {
        } break;
        case NODE_KIND_BLOCK: {
        } break;
        case NODE_KIND_RETURN: {
        } break;
        case NODE_KIND_BREAK: {
        } break;
        case NODE_KIND_CONTINUE: {
        } break;
        case NODE_KIND_IDENT: {
        } break;
        case NODE_KIND_MULTI_ASSIGN: {
        } break;
        case NODE_KIND_ENUM: {
        } break;
        case NODE_KIND_UNARY: {
        } break;
        case NODE_KIND_BINOP: {
        } break;
        case NODE_KIND_FUNCTION_CALL: {
        } break;
        case NODE_KIND_INT_LIT: {
        } break;
        case NODE_KIND_FLOAT_LIT: {
        } break;
        case NODE_KIND_STRING_LIT: {
        } break;
        case NODE_KIND_CHAR_LIT: {
        } break;
        case NODE_KIND_BOOL_LIT: {
        } break;
        case NODE_KIND_ERROR: {
        } break;
    }
}

