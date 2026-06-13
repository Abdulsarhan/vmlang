#include "typer.h"

/*
Pass 1 — collection: walk the top-level declarations and register all names and
their types into the symbol table, but don't recurse into bodies yet. This is
sometimes called a "forward declaration pass" or "hoisting pass."
Pass 2 — checking: now walk everything again and check bodies, knowing all names
are already in scope.
*/

void typecheck_expression(typer *tp, ast_node *expr) {
    if(expr->kind == NODE_KIND_ERROR) {
        return NULL;
    }

    switch(expr->kind) {
        case NODE_KIND_IDENT:
            break;
        case NODE_KIND_BINOP:
            if(expr->binop.left->kind == NODE_KIND_INT_LIT) {
                if(expr->binop.right->kind == NODE_KIND_INT_LIT) {
                    expr->binop.type == TYPE_INT_LIT;
                }
            }
            break;
    }
}

void typer_report_error(typer *tp, ast_node *node, const char *fmt, ...) {
    printf("%s:%d:%d ", tp->file_name, node->l0, node->c0);
    printf("Error: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    print_line(node->start_of_line);
    tp->error_count++;
    printf("\n");
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
                typer_report_error(tp, statement, "break statement is not inside of a for loop or a while loop");
            }
            break;
        case NODE_KIND_CONTINUE:
            if(tp->loop_depth == 0) {
                typer_report_error(tp, statement, "continue statement is not inside of a for loop or a while loop");
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
        case NODE_KIND_INT_LIT:
        case NODE_KIND_FLOAT_LIT:
        case NODE_KIND_CHAR_LIT:
        case NODE_KIND_BOOL_LIT:
        case NODE_KIND_STRING_LIT:
            typer_report_error(tp, statement, "expression cannot start with a literal!\n");
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

void typechecker_prepass(typer *tp, ast_node *root) {

}
void typecheck_file(typer *tp, ast_node *root) {
    typechecker_prepass(tp, root);
    for(i32 i = 0; i < da_len(root->file.declarations); i++) {
        ast_node *declaration = root->file.declarations[i];
        typecheck_declaration(tp, declaration);
    }
}
