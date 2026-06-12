#include "typer.h"

void typecheck_expression(typer *tp, ast_node *expr) {
}

void typecheck_statement(typer *tp, ast_node *statement) {
    switch(statement->kind) {
        case NODE_KIND_FOR:
            tp->loop_depth++;
            typecheck_expression(tp, statement->for_loop.range);
            typecheck_block(tp, statement->for_loop.block);
            tp->loop_depth--;
            break;
        case NODE_KIND_WHILE:
            tp->loop_depth++;
            typecheck_expression(tp, statement->while_loop.cond);
            typecheck_block(tp, statement->while_loop.block);
            tp->loop_depth--;
            break;
        case NODE_KIND_IF:
            typecheck_expression(tp, statement->if_stmt.cond);
            typecheck_block(tp, statement->if_stmt.then_block);
            if(statement->if_stmt.else_part != NULL) {
                typecheck_block(tp, statement->if_stmt.else_part);
            }
            break;
        case NODE_KIND_BREAK:
            if(tp->loop_depth == 0) {
                printf("error: you can't have a break statement outside of a for loop or a while loop.\n");
                tp->error_count++;
            }
            break;
        case NODE_KIND_CONTINUE:
            if(tp->loop_depth == 0) {
                printf("error: you can't have a continue statement outside of a for loop or a while loop.\n");
                tp->error_count++;
            }
            break;
        case NODE_KIND_RETURN:
            typecheck_expression(tp, statement->return_stmt.expression);
            break;
        case NODE_KIND_BLOCK:
            typecheck_block(tp, statement);
            break;
        case NODE_KIND_IDENT:
            typecheck_expression(tp, statement);
            break;
        case NODE_KIND_ERROR:
            break;
    }
}

void typecheck_block(typer *tp, ast_node *block) {
    for(i32 i = 0; i < da_len(block->block.statements); i++) {
        typecheck_statement(tp, block->block.statements[i]);
    }
}

void typecheck_func_decl_params(typer *tp, ast_node **params) {
    for(i32 i = 0; i < da_len(params); i++) {
        typecheck_statement(tp, params[i]);
    }
}

void typecheck_declaration(typer *tp, ast_node *decl) {
    switch(decl->kind) {
        case NODE_KIND_FUNCTION_DECLARATION:
            typecheck_block(tp, decl->func_decl.block);
            typecheck_func_decl_params(tp, decl->func_decl.params);
            break;
        case NODE_KIND_STRUCT:
            typecheck_block(tp, decl->struct_decl.block);
            break;
        case NODE_KIND_ENUM:
            typecheck_block(tp, decl->enum_decl.block);
            break;
        case NODE_KIND_UNION:
            typecheck_block(tp, decl->union_decl.block);
            break;
    }
}

void typecheck_file(typer *tp, ast_node *root) {
    for(i32 i = 0; i < da_len(root->file.declarations); i++) {
        ast_node *declaration = root->file.declarations[i];
        typecheck_declaration(tp, declaration);
    }
}
