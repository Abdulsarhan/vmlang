#include "pretty_printing.h"

void print_ast(ast_node *root, int depth) {
    if(!root) {
        return;
    }

    if(depth > 0) {
        printf("|");
        for(int i = 0; i < depth; i++) {
            printf("  ");
        }
    }

    switch(root->kind) {
        case NODE_KIND_BINOP:
            printf("binop: %c\n", (char)root->binop.kind);
            print_ast(root->binop.left, depth + 1);
            print_ast(root->binop.right, depth + 1);
            break;
        case NODE_KIND_RETURN:
            printf("return\n");
            print_ast(root->return_stmt.expression, depth + 1);
            break;
        case NODE_KIND_FILE:
            printf("file\n");
            for(int i = 0; i < da_len(root->file.declarations); i++) {
                print_ast(root->file.declarations[i], depth + 1);
            }
            break;
        case NODE_KIND_INT_LIT:
            printf("%lld\n", root->int_lit.value);
            break;
        case NODE_KIND_FUNCTION_CALL: {
            printf("fn: %.*s\n", (int)root->func_decl.callee->ident.name.length, root->func_decl.callee->ident.name.data);
            i32 len = da_len(root->function_call.params);
            for(int i = 0; i < len; i++) {
                print_ast(root->func_decl.params[i], depth + 1);
            }
            break;
        }
        case NODE_KIND_FUNCTION_DECLARATION: {
            printf("fn: %.*s\n", (int)root->func_decl.callee->ident.name.length, root->func_decl.callee->ident.name.data);
            i32 len = da_len(root->function_call.params);
            for(int i = 0; i < len; i++) {
                print_ast(root->func_decl.params[i], depth + 1);
            }
            print_ast(root->func_decl.block, depth + 1);
            break;
        }
    }
}

