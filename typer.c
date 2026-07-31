#include "typer.h"

/*
Pass 1 — collection: walk the top-level declarations and register all names and
their types into the symbol table, but don't recurse into bodies yet. This is
sometimes called a "forward declaration pass" or "hoisting pass."
Pass 2 — checking: now walk everything again and check bodies, knowing all names
are already in scope.
*/

type_kind is_primitive_type(string8 ident) {
    if(ident.length == 2) {
        if(str_are_strings_equal(ident, STR8_LIT("i8"))) {
            return type_kind_i8;
        } else if(str_are_strings_equal(ident, STR8_LIT("u8"))) {
            return type_kind_u8;
        }
    } else if(ident.length == 3) {
        if(str_are_strings_equal(ident, STR8_LIT("i16"))) {
            return type_kind_i16;
        } else if(str_are_strings_equal(ident, STR8_LIT("i32"))) {
            return type_kind_i32;
        } else if(str_are_strings_equal(ident, STR8_LIT("i64"))) {
            return type_kind_i64;
        } else if(str_are_strings_equal(ident, STR8_LIT("u16"))) {
            return type_kind_u16;
        } else if(str_are_strings_equal(ident, STR8_LIT("u32"))) {
            return type_kind_u32;
        } else if(str_are_strings_equal(ident, STR8_LIT("u64"))) {
            return type_kind_u64;
        }
    } else if(ident.length == 4) {
        if(str_are_strings_equal(ident, STR8_LIT("bool"))) {
            return type_kind_bool;
        } else if(str_are_strings_equal(ident, STR8_LIT("enum"))) {
            return type_kind_enum;
        }
    } else if(str_are_strings_equal(ident, STR8_LIT("struct"))) {
        return type_kind_struct;
    }
    return type_kind_none;
}

b32 is_literal(ast_node *node) {
    switch(node->kind) {
        case NODE_KIND_INT_LIT:
        case NODE_KIND_FLOAT_LIT:
        case NODE_KIND_CHAR_LIT:
        case NODE_KIND_STRING_LIT:
        case NODE_KIND_BOOL_LIT:
            return true;
    }
    return false;
}

type_kind typecheck_operand(typer *tp, ast_node *operand) {
    switch(operand->kind) {
        case NODE_KIND_INT_LIT:    return type_kind_i32;
        case NODE_KIND_FLOAT_LIT:  return type_kind_f32;
        case NODE_KIND_BOOL_LIT:   return type_kind_bool;
        case NODE_KIND_CHAR_LIT:   return type_kind_char;
        case NODE_KIND_STRING_LIT: return type_kind_string;
        case NODE_KIND_IDENT: {
            string8 ident = operand->ident.name;
            symbol *sym = scope_lookup(tp, ident);
            if(!sym) {
                typer_report_error(tp, operand, "Error: use of undeclared identifier %.*s", (int)ident.length, ident.data);
                return type_kind_none;
            }
            return sym->tp.kind;
        }
        case NODE_KIND_BINOP:
            typecheck_expression(tp, operand);
            return operand->binop.binexpr_type;
        default:
            typer_report_error(tp, operand, "Unexpected node kind as operand");
            return type_kind_none;
    }
}

void typecheck_expression(typer *tp, ast_node *expr) {
    switch(expr->kind) {
        case NODE_KIND_IDENT:
            typer_report_error(tp, expr, "Error: found a freestanding identifier when we expected an expression.");
            break;
        case NODE_KIND_BINOP: {
            ast_node_binop *binop = &expr->binop;
            ast_node *left = binop->left;
            ast_node *right = binop->right;

            switch(binop->kind) {
                case BINOP_COLON: {
                    if(left->kind == NODE_KIND_IDENT && right->kind == NODE_KIND_IDENT) {
                        type_kind kind = is_primitive_type(right->ident.name);
                        if(kind != type_kind_none) {
                            symbol ident = {0};
                            ident.tp.kind = kind;
                            ident.node = expr;
                            ident.name = left->ident.name;
                            scope_add_symbol(tp, ident);
                        } else {
                            string8 ident = left->ident.name;
                            typer_report_error(tp, expr, "Error: Expected a type after %.*s and ':'", (int)ident.length, ident.data);
                        }
                    }
                } break;
                case BINOP_COLON_EQUAL: {
                    if(left->kind == NODE_KIND_IDENT) {
                        type_kind inferred = typecheck_operand(tp, right);
                        binop->binexpr_type = inferred;
 
                        symbol ident = {0};
                        ident.tp.kind = inferred;
                        ident.node = expr;
                        ident.name = left->ident.name;
                        scope_add_symbol(tp, ident);
                    } else {
                        typer_report_error(tp, expr, "Error: Expected a variable before ':='");
                    }
                } break;
                default: {
                    type_kind lt = typecheck_operand(tp, left);
                    type_kind rt = typecheck_operand(tp, right);
                    binop->binexpr_type = (lt == type_kind_f32 || rt == type_kind_f32) ? type_kind_f32 : type_kind_i32;
                } break;
            }
        } break;
        case NODE_KIND_FUNCTION_CALL: {
            ast_node_function_call *call = &expr->function_call;
            string8 callee_name = expr->function_call.callee->ident.name;
            if(call->callee->kind != NODE_KIND_IDENT) {
                typer_report_error(tp, expr, "Error: callee is not an identifier");
                break;
            }
            symbol *callee_sym = scope_lookup(tp, callee_name);
            if(!callee_sym) {
                typer_report_error(tp, expr, "Error: call to undeclared function %.*s", (int)callee_name.length, callee_name.data);
                break;
            }
            if(callee_sym->tp.kind != type_kind_function) {
                typer_report_error(tp, expr, "Error: %.*s is not callable", (int)callee_name.length, callee_name.data);
                break;
            }
            int argc = da_len(call->params);
            int paramc = da_len(callee_sym->tp.param_types);
            if(argc != paramc) {
                typer_report_error(tp, expr, "Error: %.*s expects %d argument(s), got %d",
                                    (int)callee_name.length, callee_name.data, paramc, argc);
                break;
            }
            for(int i = 0; i < argc; i++) {
                type_kind arg_type = typecheck_operand(tp, call->params[i]);
                if(arg_type != callee_sym->tp.param_types[i]) {
                    typer_report_error(tp, call->params[i], "Error: argument %d type mismatch", i + 1);
                }
            }
        } break;
        case NODE_KIND_ERROR:
            typer_report_error(tp, expr, "Got an error node in typecheck_expression()");
            break;
        default:
            typer_report_error(tp, expr, "Unexpected node kind in typecheck_expression()");
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
         case NODE_KIND_BINOP:
            typecheck_expression(tp, statement);
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
            typer_report_error(tp, statement, "Got an error node in typecheck_statement()");
            break;
        case NODE_KIND_STRUCT:
            typer_report_error(tp, statement, "Error: struct declarations are only allowed in the global scope");
            break;
        case NODE_KIND_ENUM:
            typer_report_error(tp, statement, "Error: enum declarations are only allowed in the global scope");
            break;
        case NODE_KIND_UNION:
            typer_report_error(tp, statement, "Error: union declarations are only allowed in the global scope");
            break;
    }
}

void push_scope(typer *tp) {
    scope *scp = &tp->scope_stack[tp->scope_depth++];
    if (tp->scope_depth > tp->highest_scope_depth) {
        hm_init(scp->symbols, string8, symbol, hm_cmp_string8, hm_fnv1a_hash);
        tp->highest_scope_depth = tp->scope_depth;
    } else {
        hm_clear(scp->symbols);
    }
}

void pop_scope(typer *tp) {
    tp->scope_depth--;
}

void scope_add_symbol(typer *tp, symbol sym) {
    scope *cur_scope = &tp->scope_stack[tp->scope_depth - 1];
    hm_push(cur_scope->symbols, sym.name, sym);
}

void typecheck_block(typer *tp, ast_node *block) {
    push_scope(tp);
    for(i32 i = 0; i < da_len(block->block.statements); i++) {
        typecheck_statement(tp, block->block.statements[i]);
    }
    pop_scope(tp);
}

void typecheck_func_decl_params(typer *tp, ast_node **params) {
    for(i32 i = 0; i < da_len(params); i++) {
        typecheck_statement(tp, params[i]);
    }
}

symbol *scope_get_symbol(scope scope, string8 name) {
    return (symbol*)hm_get(scope.symbols, name);
}

symbol *scope_lookup(typer *tp, string8 name) {
    for (int i = tp->scope_depth - 1; i >= 0; i--) {
        symbol *sym = scope_get_symbol(tp->scope_stack[i], name);
        if (sym) {
            return sym;
        }
    }
    printf("failed to find symbol: %.*s\n", (int)name.length, name.data);
    return NULL;
}

void typecheck_declaration_body(typer *tp, ast_node *decl) {
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

b32 is_already_declared(typer *tp, ast_node *node, string8 name) {
    symbol *str = scope_lookup(tp, name);
    if(str) {
        switch(node->kind) {
            case NODE_KIND_STRUCT:
                typer_report_error(tp, node, "Error: redeclaration of struct by the name of %.*s", (int)name.length, name.data);
                return true;
                break;
            case NODE_KIND_UNION:
                typer_report_error(tp, node, "Error: redeclaration of union by the name of %.*s", (int)name.length, name.data);
                return true;
                break;
            case NODE_KIND_ENUM:
                typer_report_error(tp, node, "Error: redeclaration of enum by the name of %.*s", (int)name.length, name.data);
                return true;
                break;
        }
    }
    return false;
}

type_kind node_kind_to_type_kind(node_kind nd_kind) {
    switch(nd_kind) {
        case NODE_KIND_STRUCT:
            return type_kind_struct;
            break;
        case NODE_KIND_UNION:
            return type_kind_union;
            break;
        case NODE_KIND_ENUM:
            return type_kind_enum;
            break;
    }
    return type_kind_none;
}

void typecheck_declaration_header(typer *tp, ast_node *decl) {
    switch(decl->kind) {
        case NODE_KIND_FUNCTION_DECLARATION: {
            symbol function_name_sym;
            function_name_sym.name = decl->ident.name;
            function_name_sym.tp.kind = type_kind_function;
            function_name_sym.tp.param_types = NULL;
            function_name_sym.node = decl;
            for(i32 i = 0; i < da_len(decl->func_decl.params); i++) {
                ast_node *param_expr = decl->func_decl.params[i];
                switch(param_expr->kind) {
                    case NODE_KIND_BINOP: {
                        if(param_expr->binop.kind == ':') {
                            ast_node *name_of_param = param_expr->binop.left;
                            symbol param_name_sym;
                            param_name_sym.name = name_of_param->ident.name;
                            param_name_sym.node = name_of_param;
                            scope_add_symbol(tp, param_name_sym);

                            ast_node *type_of_param = param_expr->binop.right;

                            type_kind kind = scope_lookup(tp, type_of_param->ident.name)->tp.kind;

                            da_push(function_name_sym.tp.param_types, kind);
                        } else {
                            typer_report_error(tp, decl, "Expected ':' between param name and param type.");
                        }
                    } break;
                }
            }
            scope_add_symbol(tp, function_name_sym);
        } break;
        case NODE_KIND_STRUCT:
        case NODE_KIND_ENUM:
        case NODE_KIND_UNION: {
            string8 ident = decl->struct_decl.ident->ident.name;
            b32 already_declared = is_already_declared(tp, decl, ident);
            if(!already_declared) {
                symbol sym;
                sym.name = ident;
                sym.tp.kind = node_kind_to_type_kind(decl->kind);
                sym.node = decl;
                scope_add_symbol(tp, sym);
            }
        } break;
    }
}

void make_primitive_type(typer *tp, string8 name) {
    symbol sym;
    memset(&sym, 0, sizeof(symbol));
    sym.name = name;
    sym.tp.kind = type_kind_type;
    scope_add_symbol(tp, sym);
}

void typechecker_prepass(typer *tp, ast_node *root) {
    make_primitive_type(tp, STR8_LIT("i8"));
    make_primitive_type(tp, STR8_LIT("i16"));
    make_primitive_type(tp, STR8_LIT("i32"));
    make_primitive_type(tp, STR8_LIT("i64"));

    make_primitive_type(tp, STR8_LIT("u8"));
    make_primitive_type(tp, STR8_LIT("u16"));
    make_primitive_type(tp, STR8_LIT("u32"));
    make_primitive_type(tp, STR8_LIT("u64"));

    make_primitive_type(tp, STR8_LIT("f32"));
    make_primitive_type(tp, STR8_LIT("f64"));

    make_primitive_type(tp, STR8_LIT("bool"));

    make_primitive_type(tp, STR8_LIT("struct"));
    make_primitive_type(tp, STR8_LIT("enum"));
    make_primitive_type(tp, STR8_LIT("union"));

    for (int i = 0; i < da_len(root->file.declarations); i++) {
        typecheck_declaration_header(tp, root->file.declarations[i]);
    }
}

void typecheck_file(typer *tp, ast_node *root) {
    push_scope(tp);
    typechecker_prepass(tp, root);
    for(i32 i = 0; i < da_len(root->file.declarations); i++) {
        ast_node *declaration = root->file.declarations[i];
        typecheck_declaration_body(tp, declaration);
    }
    pop_scope(tp);
}
