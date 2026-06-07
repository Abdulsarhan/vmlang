#include "pretty_printing.h"

static const char *binop_kind_to_string(binop_kind kind) {
    switch(kind) {
        case BINOP_ADD:               return "+";
        case BINOP_SUB:               return "-";
        case BINOP_MUL:               return "*";
        case BINOP_DIV:               return "/";
        case BINOP_MOD:               return "%";
        case BINOP_COLON:             return ":";
        case BINOP_ASSIGN:            return "=";
        case BINOP_BITWISE_AND:       return "&";
        case BINOP_BITWISE_OR:        return "|";
        case BINOP_BITWISE_XOR:       return "^";
        case BINOP_GREATER_THAN:      return ">";
        case BINOP_LESS_THAN:         return "<";
        case BINOP_RANGE:             return "..";
        case BINOP_ARRAY_SUBSCRIPT:   return "[]";
        case BINOP_MEMBER_ACCESS:     return ".";
        case BINOP_LOGICAL_NOT:       return "!=";
        case BINOP_LOGICAL_AND:       return "&&";
        case BINOP_LOGICAL_OR:        return "||";
        case BINOP_RIGHT_SHIFT:       return ">>";
        case BINOP_LEFT_SHIFT:        return "<<";
        case BINOP_ADD_ASSIGN:        return "+=";
        case BINOP_MINUS_ASSIGN:      return "-=";
        case BINOP_DIV_ASSIGN:        return "/=";
        case BINOP_MOD_ASSIGN:        return "%=";
        case BINOP_MUL_ASSIGN:        return "*=";
        case BINOP_XOR_ASSIGN:        return "^=";
        case BINOP_AND_ASSIGN:        return "&=";
        case BINOP_OR_ASSIGN:         return "|=";
        case BINOP_LEFT_SHIFT_ASSIGN: return "<<=";
        case BINOP_RIGHT_SHIFT_ASSIGN:return ">>=";
        case BINOP_GREATER_THAN_ASSIGN: return ">=";
        case BINOP_LESS_THAN_ASSIGN:  return "<=";
        case BINOP_EQUALS:            return "==";
        case BINOP_COLON_COLON:       return "::";
        case BINOP_COLON_EQUAL:       return ":=";
        default:                      return "?";
    }
}

static const char *unary_kind_to_string(unary_kind kind) {
    switch(kind) {
        case UNARY_PLUS:          return "+";
        case UNARY_MINUS:         return "-";
        case UNARY_ADDRESS_OF:    return "&";
        case UNARY_DEREF:         return "*";
        case UNARY_LOGICAL_NOT:   return "!";
        case UNARY_BITWISE_NOT:   return "~";
        case UNARY_PRE_INCREMENT: return "++ (pre)";
        case UNARY_POST_INCREMENT:return "++ (post)";
        case UNARY_PRE_DECREMENT: return "-- (pre)";
        case UNARY_POST_DECREMENT:return "-- (post)";
        default:                  return "?";
    }
}

void print_indent(int depth) {
    for(int i = 0; i < depth; i++) {
        printf("|  ");
    }
}

void print_ast(ast_node *root, int depth) {
    if(!root) {
        return;
    }

    print_indent(depth);

    switch(root->kind) {
        case NODE_KIND_FILE:
            printf("file\n");
            for(int i = 0; i < da_len(root->file.declarations); i++) {
                print_ast(root->file.declarations[i], depth + 1);
            }
            return;
        case NODE_KIND_FUNCTION_DECLARATION: {
            printf("fn decl: %.*s", (int)root->func_decl.callee->ident.name.length,
                                        root->func_decl.callee->ident.name.data);
            if(root->func_decl.return_type) {
                printf(" -> %.*s", (int)root->func_decl.return_type->ident.name.length,
                                       root->func_decl.return_type->ident.name.data);
            }
            printf("\n");
            int len = da_len(root->func_decl.params);
            for(int i = 0; i < len; i++) {
                print_ast(root->func_decl.params[i], depth + 1);
            }
            print_ast(root->func_decl.block, depth + 1);
            return;
        }
        case NODE_KIND_FUNCTION_CALL: {
            printf("fn call: %.*s\n", (int)root->function_call.callee->ident.name.length,
                                          root->function_call.callee->ident.name.data);
            int len = da_len(root->function_call.params);
            for(int i = 0; i < len; i++) {
                print_ast(root->function_call.params[i], depth + 1);
            }
            return;
        }
        case NODE_KIND_BLOCK: {
            printf("block\n");
            int len = da_len(root->block.statements);
            for(int i = 0; i < len; i++) {
                print_ast(root->block.statements[i], depth + 1);
            }
            return;
        }
        case NODE_KIND_IF:
            printf("if\n");
            print_indent(depth + 1); printf("cond:\n");
            print_ast(root->if_stmt.cond, depth + 2);
            print_indent(depth + 1); printf("then:\n");
            print_ast(root->if_stmt.then_block, depth + 2);
            if(root->if_stmt.else_part) {
                print_indent(depth + 1); printf("else:\n");
                print_ast(root->if_stmt.else_part, depth + 2);
            }
            return;
        case NODE_KIND_WHILE:
            printf("while\n");
            print_indent(depth + 1); printf("cond:\n");
            print_ast(root->while_loop.cond, depth + 2);
            print_indent(depth + 1); printf("block:\n");
            print_ast(root->while_loop.block, depth + 2);
            return;
        case NODE_KIND_FOR:
            printf("for\n");
            print_indent(depth + 1); printf("range:\n");
            print_ast(root->for_loop.range, depth + 2);
            print_indent(depth + 1); printf("block:\n");
            print_ast(root->for_loop.block, depth + 2);
            return;
        case NODE_KIND_RETURN:
            printf("return\n");
            print_ast(root->return_stmt.expression, depth + 1);
            return;
        case NODE_KIND_BREAK:
            printf("break\n");
            return;
        case NODE_KIND_CONTINUE:
            printf("continue\n");
            return;
        case NODE_KIND_STRUCT:
            printf("struct: %.*s\n", (int)root->struct_decl.ident->ident.name.length,
                                         root->struct_decl.ident->ident.name.data);
            print_ast(root->struct_decl.block, depth + 1);
            return;
        case NODE_KIND_UNION:
            printf("union: %.*s\n", (int)root->union_decl.ident->ident.name.length,
                                        root->union_decl.ident->ident.name.data);
            print_ast(root->union_decl.block, depth + 1);
            return;
        case NODE_KIND_ENUM:
            printf("enum: %.*s\n", (int)root->enum_decl.ident->ident.name.length,
                                       root->enum_decl.ident->ident.name.data);
            print_ast(root->enum_decl.block, depth + 1);
            return;
        case NODE_KIND_BINOP:
            printf("binop: %s\n", binop_kind_to_string(root->binop.kind));
            print_ast(root->binop.left, depth + 1);
            print_ast(root->binop.right, depth + 1);
            return;
        case NODE_KIND_UNARY:
            printf("unary: %s\n", unary_kind_to_string(root->unary.kind));
            print_ast(root->unary.operand, depth + 1);
            return;
        case NODE_KIND_IDENT:
            printf("ident: %.*s\n", (int)root->ident.name.length,
                                        root->ident.name.data);
            return;
        case NODE_KIND_INT_LIT:
            printf("int: %lld\n", root->int_lit.value);
            return;
        case NODE_KIND_FLOAT_LIT:
            printf("float: %f\n", root->float_lit.value);
            return;
        case NODE_KIND_STRING_LIT:
            printf("string: \"%.*s\"\n", (int)root->str_lit.value.length,
                                              root->str_lit.value.data);
            return;
        case NODE_KIND_CHAR_LIT:
            printf("char: '%c'\n", root->char_lit.value);
            return;
        case NODE_KIND_BOOL_LIT:
            printf("bool: %s\n", root->int_lit.value ? "true" : "false");
            return;
        case NODE_KIND_NONE:
        default:
            printf("(unknown node kind: %d)\n", root->kind);
            return;
    }
}
