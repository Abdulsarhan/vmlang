#include <stdarg.h>

#include "parser.h"
#include "tokenizer.h"

typedef enum operator_pos {
    PREFIX,
    POSTFIX,
}operator_pos;

token peek_token(token_stream *stream) {
    return *stream->at;
}

token peek_next_token(token_stream *stream) {
    return *(stream->at + 1);
}

token eat_token(token_stream *stream) {
    token current_tok = *stream->at;
    if(stream->at->kind != TOKEN_KIND_END_OF_STREAM) {
        stream->at++;
        return current_tok;
    }
    printf("Error: %s(): Tried to eat past end of file\n", __func__);
    return current_tok;
}

b32 match_and_eat_token(token_stream *stream, token_kind kind) {
    token tok = peek_token(stream);
    if(tok.kind == kind || tok.kind == TOKEN_KIND_ERROR) {
        eat_token(stream);
        return true;
    }
    return false;
}

void copy_source_location_from_token_to_ast_node(token tok, ast_node *node) {
    node->l0 = tok.l0;
    node->c0 = tok.c0;
    node->l1 = tok.l1;
    node->c1 = tok.c1;
    node->start_of_line = tok.start_of_line;
}

ast_node *error_node(ast *ast, token tok, error_kind kind) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_ERROR;
    node->error.kind = kind;
    copy_source_location_from_token_to_ast_node(tok, node);
    return node;
}

ast_node *binop_node(ast *ast, binop_kind kind, ast_node *left, ast_node *right) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BINOP;
    node->binop.left = left;
    node->binop.right = right;
    node->binop.kind = kind;
    return node;
}

ast_node *break_node(ast *ast) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BREAK;
    return node;
}

ast_node *continue_node(ast *ast) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_CONTINUE;
    return node;
}

ast_node *return_node(ast *ast, ast_node *expression) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_RETURN;
    node->return_stmt.expression = expression;
    return node;
}

ast_node *if_node(ast *ast, ast_node *cond, ast_node *then_block, ast_node *else_part) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_IF;
    node->if_stmt.cond = cond;
    node->if_stmt.then_block = then_block;
    node->if_stmt.else_part = else_part;
    return node;
}

ast_node *for_node(ast *ast, ast_node *range, ast_node *block) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FOR;
    node->for_loop.range = range;
    node->for_loop.block = block;
    return node;
}

ast_node *while_node(ast *ast, ast_node *cond, ast_node *block) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_WHILE;
    node->while_loop.cond = cond;
    node->while_loop.block = block;
    return node;
}

ast_node *unary_node(ast *ast, unary_kind kind, ast_node *operand) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_UNARY;
    node->unary.kind = kind;
    node->unary.operand = operand;
    return node;
}

ast_node *function_call_node(ast *ast, ast_node *function_name, ast_node **params) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FUNCTION_CALL;
    node->function_call.callee = function_name;
    node->function_call.params = params;
    return node;
}

ast_node *ident_node(ast *ast, string8 value) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_IDENT;
    node->ident.name = value;
    return node;
}

ast_node *multi_assign_node(ast *ast, binop_kind kind, ast_node **left, ast_node *right) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_MULTI_ASSIGN;
    node->multi_assign.kind = kind;
    node->multi_assign.left = left;
    node->multi_assign.right = right;
    return node;
}

ast_node *struct_declaration_node(ast *ast, ast_node *ident, ast_node *block) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_STRUCT;
    node->struct_decl.ident = ident;
    node->struct_decl.block = block;
    return node;
}

ast_node *union_declaration_node(ast *ast, ast_node *ident, ast_node *block) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_UNION;
    node->union_decl.ident = ident;
    node->union_decl.block = block;
    return node;
}

ast_node *enum_declaration_node(ast *ast, ast_node *ident, ast_node *block) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_ENUM;
    node->enum_decl.ident = ident;
    node->enum_decl.block = block;
    return node;
}

ast_node *function_declaration_node(ast *ast, ast_node *function_name, ast_node **params, ast_node *block, ast_node *return_type) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FUNCTION_DECLARATION;
    node->func_decl.function_name = function_name;
    node->func_decl.params = params;
    node->func_decl.block = block;
    node->func_decl.return_type = return_type;
    return node;
}

void print_line(const u8 *start_of_line) {
    printf("                | ");
    const u8 *at = start_of_line;
    while(*at != '\n') {
        at++;
    }
    printf("%.*s", (int)(at - start_of_line), start_of_line);
    printf("\n");
}

void print_filename_line_and_column(const token_stream *tok_stream, const ast_node *node) {
    printf("%s:%d:%d ", tok_stream->file, node->l0, node->c0);
}

void report_error_new(ast *ast, const token_stream *tok_stream, const ast_node *node, const char *fmt, ...) {
    print_filename_line_and_column(tok_stream, node);
    printf("Error: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    print_line(node->start_of_line);
    ast->error_count++;
    printf("\n");
}

i32 get_operator_precedence(token tok) {
    i32 prec = 0;
    switch(tok.kind) {
        case ':':
        case TOKEN_KIND_COLON_COLON:
        case TOKEN_KIND_DOT_DOT:
            prec = 50;
            break;
        case '=':
        case TOKEN_KIND_PLUS_EQUAL:
        case TOKEN_KIND_MINUS_EQUAL:
        case TOKEN_KIND_MULTIPLY_EQUAL:
        case TOKEN_KIND_DIVIDE_EQUAL:
        case TOKEN_KIND_MOD_EQUAL:
        case TOKEN_KIND_XOR_EQUAL:
        case TOKEN_KIND_AND_EQUAL:
        case TOKEN_KIND_OR_EQUAL:
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL:
        case TOKEN_KIND_COLON_EQUAL:
            prec = 100;
            break;
        case '&':
        case '^':
        case '|':
            prec = 110;
            break;
        case TOKEN_KIND_AND_AND:
        case TOKEN_KIND_OR_OR:
            prec = 120;
            break;
        case TOKEN_KIND_NOT_EQUAL:
        case TOKEN_KIND_EQUAL_EQUAL:
            prec = 130;
            break;
        case '>':
        case '<':
        case TOKEN_KIND_GREATER_THAN_EQUAL:
        case TOKEN_KIND_LESS_THAN_EQUAL:
            prec = 140;
            break;
        case TOKEN_KIND_RIGHT_SHIFT:
        case TOKEN_KIND_LEFT_SHIFT:
            prec = 150;
            break;
        case '+':
        case '-':
            prec = 200;
            break;
        case '*':
        case '/':
        case '%':
            prec = 400;
            break;
        case '(':
        case '[':
        case '.':
        case TOKEN_KIND_RIGHT_ARROW:
            prec = 800;
            break;
    }
    return prec;
}

binop_kind binop_from_token(token tok) {
    switch(tok.kind) {
        case '+': return BINOP_ADD;
        case '-': return BINOP_SUB;
        case '/': return BINOP_DIV;
        case '%': return BINOP_MOD;
        case '*': return BINOP_MUL;
        case '=': return BINOP_ASSIGN;
        case ':': return BINOP_COLON;

        case TOKEN_KIND_DOT_DOT:            return BINOP_RANGE;
        case TOKEN_KIND_NOT_EQUAL:          return BINOP_LOGICAL_NOT;
        case TOKEN_KIND_EQUAL_EQUAL:        return BINOP_EQUALS;
        case TOKEN_KIND_PLUS_EQUAL:         return BINOP_ADD_ASSIGN;
        case TOKEN_KIND_MINUS_EQUAL:        return BINOP_MINUS_ASSIGN;
        case TOKEN_KIND_DIVIDE_EQUAL:       return BINOP_DIV_ASSIGN;
        case TOKEN_KIND_MOD_EQUAL:          return BINOP_MOD_ASSIGN;
        case TOKEN_KIND_MULTIPLY_EQUAL:     return BINOP_MUL_ASSIGN;
        case TOKEN_KIND_XOR_EQUAL:          return BINOP_XOR_ASSIGN;
        case TOKEN_KIND_AND_EQUAL:          return BINOP_AND_ASSIGN;
        case TOKEN_KIND_OR_EQUAL:           return BINOP_OR_ASSIGN;
        case TOKEN_KIND_COLON_EQUAL:        return BINOP_COLON_EQUAL;
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:   return BINOP_LEFT_SHIFT_ASSIGN;
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL:  return BINOP_RIGHT_SHIFT_ASSIGN;
        case TOKEN_KIND_GREATER_THAN_EQUAL: return BINOP_GREATER_THAN_ASSIGN;
        case TOKEN_KIND_LESS_THAN_EQUAL:    return BINOP_LESS_THAN_ASSIGN;
        case TOKEN_KIND_RIGHT_SHIFT:        return BINOP_RIGHT_SHIFT;
        case TOKEN_KIND_LEFT_SHIFT:         return BINOP_LEFT_SHIFT;
        case TOKEN_KIND_COLON_COLON:        return BINOP_COLON_COLON;
        case TOKEN_KIND_AND_AND:            return BINOP_LOGICAL_AND;
        case TOKEN_KIND_OR_OR:              return BINOP_LOGICAL_OR;
        default:                            return BINOP_NONE;
    }
    return BINOP_NONE;
}

binop_kind assignment_binop_from_token(token tok) {
    switch (tok.kind) {
        case TOKEN_KIND_COLON_EQUAL:       return BINOP_COLON_EQUAL;
        case '=':                          return BINOP_ASSIGN;
        case TOKEN_KIND_PLUS_EQUAL:        return BINOP_ADD_ASSIGN;
        case TOKEN_KIND_MINUS_EQUAL:       return BINOP_MINUS_ASSIGN;
        case TOKEN_KIND_MULTIPLY_EQUAL:    return BINOP_MUL_ASSIGN;
        case TOKEN_KIND_DIVIDE_EQUAL:      return BINOP_DIV_ASSIGN;
        case TOKEN_KIND_MOD_EQUAL:         return BINOP_MOD_ASSIGN;
        case TOKEN_KIND_XOR_EQUAL:         return BINOP_XOR_ASSIGN;
        case TOKEN_KIND_AND_EQUAL:         return BINOP_AND_ASSIGN;
        case TOKEN_KIND_OR_EQUAL:          return BINOP_OR_ASSIGN;
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:  return BINOP_LEFT_SHIFT_ASSIGN;
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL: return BINOP_RIGHT_SHIFT_ASSIGN;
        default:                           return BINOP_NONE;
    }
    return BINOP_NONE;
}

ast_node *node_int_lit(ast *ast, i64 value) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_INT_LIT;
    node->int_lit.value = value;
    return node;
}

ast_node *node_float_lit(ast *ast, f64 value) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FLOAT_LIT;
    node->float_lit.value = value;
    return node;
}

ast_node *node_char_lit(ast *ast, u8 value) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_CHAR_LIT;
    node->char_lit.value = value;
    return node;
}

ast_node *node_string_lit(ast *ast, string8 value) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_STRING_LIT;
    node->str_lit.value = value;
    return node;
}

ast_node *node_bool_lit(ast *ast, b64 value) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BOOL_LIT;
    node->int_lit.value = value;
    return node;
}

unary_kind unary_from_token(token tok, operator_pos pos) {
    unary_kind kind = UNARY_NONE;
    switch(tok.kind) {
        case TOKEN_KIND_MINUS_MINUS:
            if(pos == PREFIX) {
                kind = UNARY_PRE_DECREMENT;
            } else if(pos == POSTFIX) {
                kind = UNARY_POST_DECREMENT;
            }
            break;
        case TOKEN_KIND_PLUS_PLUS:
            if(pos == PREFIX) {
                kind = UNARY_PRE_INCREMENT;
            } else if(pos == POSTFIX) {
                kind = UNARY_POST_INCREMENT;
            }
            break;
        case '-':
            kind = UNARY_MINUS;
            break;
        case '+':
            kind = UNARY_PLUS;
            break;
        case '!':
            kind = UNARY_LOGICAL_NOT;
            break;
        case '~':
            kind = UNARY_BITWISE_NOT;
            break;
    }
    return kind;
}

ast_node *parse_prefix(ast *ast, token_stream *tok_stream) {
    token tok = eat_token(tok_stream);
    switch(tok.kind) {
        case TOKEN_KIND_FLOAT_LITERAL:
            return node_float_lit(ast, tok.float_value);
            break;
        case TOKEN_KIND_CHAR_LITERAL:
            return node_char_lit(ast, tok.char_value);
            break;
        case TOKEN_KIND_STRING_LITERAL:
            return node_string_lit(ast, tok.string_value);
            break;
        case TOKEN_KIND_BOOL_LITERAL:
            return node_bool_lit(ast, tok.bool_value);
            break;
        case TOKEN_KIND_INT_LITERAL:
            return node_int_lit(ast, tok.integer_value);
            break;
        case '(': // for parenthesized expressions.
            ast_node *expr = parse_expression(ast, tok_stream, -9999);
            b32 matched = match_and_eat_token(tok_stream, ')');
            if(!matched) {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "Expected closing parenthesis to terminate parenthesized expression.");
                return err_node;
            }
            return expr;
            break;
        case '+':
        case '-':
        case '~':
        case '!':
        case TOKEN_KIND_MINUS_MINUS:
        case TOKEN_KIND_PLUS_PLUS:
            ast_node *operand = parse_prefix(ast, tok_stream);
            unary_kind kind = unary_from_token(tok, PREFIX);
            return unary_node(ast, kind, operand);
            break;
        case TOKEN_KIND_IDENTIFIER:
            return ident_node(ast, tok.ident_string);
            break;
        case TOKEN_KIND_ERROR:
            return error_node(ast, tok, ERROR_KIND_LEX_ERROR);
    }
    return error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
}

ast_node *parse_expression(ast *ast, token_stream *tok_stream, int min_prec) {
    ast_node *left = parse_prefix(ast, tok_stream);
    while (1) {
        token next_token = peek_token(tok_stream);
        i32 prec = get_operator_precedence(next_token);
        // prec == 0 means that the token is not an operator or we hit end of stream.
        if (prec == 0 || prec < min_prec) {
            break;
        }

        left = parse_infix_and_postfix(ast, tok_stream, prec, left);
    }
    return left;
}

ast_node **parse_function_parameters(ast *ast, token_stream *tok_stream) {
    ast_node **params = NULL;
    while(1) {
        token tok = peek_token(tok_stream);
        if(tok.kind == ')') {
            break;
        }
        switch(tok.kind) {
            case ',':
                eat_token(tok_stream);
                break;
            default:
                ast_node *param = parse_expression(ast, tok_stream, -9999);
                da_push(params, param);
                break;
        }
    }
    match_and_eat_token(tok_stream, ')');
    return params;
}

ast_node *parse_infix_and_postfix(ast *ast, token_stream *tok_stream, i32 prec, ast_node *left) {
    token tok = eat_token(tok_stream);
    switch(tok.kind) {
        case '+':
        case '-':
        case '/':
        case '%':
        case '*':
        case '>':
        case '<':
        case '&':
        case '^':
        case '|':
        case ':':
        case TOKEN_KIND_AND_AND:
        case TOKEN_KIND_OR_OR:

        case TOKEN_KIND_NOT_EQUAL:
        case TOKEN_KIND_EQUAL_EQUAL:
        case TOKEN_KIND_GREATER_THAN_EQUAL:
        case TOKEN_KIND_LESS_THAN_EQUAL:
        case TOKEN_KIND_RIGHT_SHIFT:
        case TOKEN_KIND_DOT_DOT:
        case TOKEN_KIND_LEFT_SHIFT: {
            ast_node *right = parse_expression(ast, tok_stream, prec + 1); // +1 for left-assoc
            binop_kind kind = binop_from_token(tok);
            return binop_node(ast, kind, left, right);
            break;
        }
        case '[': {
            ast_node *index = parse_expression(ast, tok_stream, -9999);
            match_and_eat_token(tok_stream, ']');
            return binop_node(ast, BINOP_ARRAY_SUBSCRIPT, left, index);
            break;
        }
        case '.': {
            ast_node *right = parse_prefix(ast, tok_stream);
            return binop_node(ast, BINOP_MEMBER_ACCESS, left, right);
            break;
        }
        case '=':
        case TOKEN_KIND_PLUS_EQUAL:
        case TOKEN_KIND_DIVIDE_EQUAL:
        case TOKEN_KIND_MINUS_EQUAL:
        case TOKEN_KIND_MULTIPLY_EQUAL:
        case TOKEN_KIND_MOD_EQUAL:
        case TOKEN_KIND_XOR_EQUAL:
        case TOKEN_KIND_OR_EQUAL:
        case TOKEN_KIND_AND_EQUAL:
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL:
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:
        case TOKEN_KIND_COLON_COLON:
        case TOKEN_KIND_COLON_EQUAL: {
            ast_node *right = parse_expression(ast, tok_stream, prec - 1); // -1 for right-assoc
            binop_kind kind = binop_from_token(tok);
            mem_arena *arena = arena_get_scratch();
            string8 str = token_to_string(arena, tok);
            const char *cstr = str_to_cstr(arena, str);

            if(right->kind == NODE_KIND_ERROR) {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "expected expression after '%s'", cstr);
                return err_node;
            } else {
                return binop_node(ast, kind, left, right);
            }
            break;
        }
        case '(':
            ast_node **params = parse_function_parameters(ast, tok_stream);
            return function_call_node(ast, left, params);
            break;
        case TOKEN_KIND_ERROR:
            return error_node(ast, tok, ERROR_KIND_LEX_ERROR);
            break;
        default:
            return error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
            break;
    }
}

ast_node *parse_block(ast *ast, token_stream *tok_stream) {
    match_and_eat_token(tok_stream, '{');
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BLOCK;
    node->block.statements = NULL;
    while (peek_token(tok_stream).kind != '}' &&
           peek_token(tok_stream).kind != TOKEN_KIND_END_OF_STREAM) {
        ast_node *stmt = parse_statement(ast, tok_stream);
        da_push(node->block.statements, stmt);
    }
    match_and_eat_token(tok_stream, '}');
    return node;
}

ast_node *parse_if(ast *ast, token_stream *tok_stream) {
    match_and_eat_token(tok_stream, TOKEN_KIND_IF);
    ast_node *cond = parse_expression(ast, tok_stream, -9999);
    ast_node *then_block = parse_block(ast, tok_stream);
    ast_node *else_part = parse_else_or_else_if(ast, tok_stream);
    return if_node(ast, cond, then_block, else_part);
}

ast_node *parse_else_or_else_if(ast *ast, token_stream *tok_stream) {
    token tok = peek_token(tok_stream);
    ast_node *node = NULL;
    if(match_and_eat_token(tok_stream, TOKEN_KIND_ELSE)) {
        tok = peek_token(tok_stream);
        if(tok.kind == TOKEN_KIND_IF) {
            node = parse_if(ast, tok_stream);
        } else if(tok.kind == '{') {
            node = parse_block(ast, tok_stream);
        } else {
            ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
            report_error_new(ast, tok_stream, err_node, "expected '{' after else if statement");
            return err_node;
        }
    } else {
        node = NULL;
    }
    return node;
}

ast_node *parse_statement(ast *ast, token_stream *tok_stream) {
    token tok = peek_token(tok_stream);
    switch(tok.kind) {
        case TOKEN_KIND_IF: {
            tok = peek_next_token(tok_stream);
            if(tok.kind == TOKEN_KIND_IDENTIFIER || tok.kind == TOKEN_KIND_INT_LITERAL) {
                return parse_if(ast, tok_stream);
            } else {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "expected '{' after if statement");
                return err_node;
            }
            break;
        }
        case TOKEN_KIND_WHILE: {
            eat_token(tok_stream);
            ast_node *cond = parse_expression(ast, tok_stream, -9999);
            ast_node *block = parse_block(ast, tok_stream);
            return while_node(ast, cond, block);
            break;
        }
        case TOKEN_KIND_FOR: {
            eat_token(tok_stream);
            ast_node *range_or_count = parse_expression(ast, tok_stream, -9999);
            ast_node *block = parse_block(ast, tok_stream);
            return for_node(ast, range_or_count, block);
            break;
        }
        case TOKEN_KIND_CONTINUE: {
            eat_token(tok_stream);
            b32 matched = match_and_eat_token(tok_stream, ';');
            if(!matched) {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "expected ';' after continue statement");
                return err_node;
            }
            return continue_node(ast);
            break;
        }
        case TOKEN_KIND_BREAK: {
            eat_token(tok_stream);
            b32 matched = match_and_eat_token(tok_stream, ';');
            if(!matched) {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "expected ';' after break statement");
                return err_node;
            }
            return break_node(ast);
            break;
        }
        case TOKEN_KIND_RETURN: {
            eat_token(tok_stream);
            ast_node *expression = parse_expression(ast, tok_stream, -9999);
            b32 matched = match_and_eat_token(tok_stream, ';');
            if(!matched) {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "expected ';' after return statement");
                return err_node;
            }
            return return_node(ast, expression);
            break;
        }
        case '{': {
            return parse_block(ast, tok_stream);
            break;
        }
        case TOKEN_KIND_IDENTIFIER:
            tok = peek_next_token(tok_stream);
            if (tok.kind == ',') {
                ast_node **lhs_list = NULL;
                token first_tok = eat_token(tok_stream);
                ast_node *first = ident_node(ast, first_tok.ident_string);
                da_push(lhs_list, first);

                while (peek_token(tok_stream).kind == ',') {
                    match_and_eat_token(tok_stream, ',');
                    token next_tok = eat_token(tok_stream);
                    ast_node *lhs = ident_node(ast, next_tok.ident_string);
                    da_push(lhs_list, lhs);
                }

                tok = peek_token(tok_stream);
                binop_kind assign_op = assignment_binop_from_token(tok);
                if (assign_op == BINOP_NONE) {
                    ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                    report_error_new(ast, tok_stream, err_node, "expected an assignment operator after the variables in the multi-assignment expression.");
                    return err_node;
                }
                match_and_eat_token(tok_stream, tok.kind);
                ast_node *rhs = parse_expression(ast, tok_stream, -9999);

                b32 matched = match_and_eat_token(tok_stream, ';');
                if (!matched) {
                    ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                    report_error_new(ast, tok_stream, err_node, "expected ';' after multi-assignment");
                    return err_node;
                }
                return multi_assign_node(ast, assign_op, lhs_list, rhs);
            }

            ast_node *expr = parse_expression(ast, tok_stream, -9999);
            b32 matched = match_and_eat_token(tok_stream, ';');
            if(expr->kind == NODE_KIND_ERROR) {
                return error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
            } else if(!matched) {
                ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
                report_error_new(ast, tok_stream, err_node, "expected ';' after expression");
                return err_node;
            } else {
                return expr;
            }
            break;
        case TOKEN_KIND_INT_LITERAL:
        case TOKEN_KIND_FLOAT_LITERAL:
        case TOKEN_KIND_STRING_LITERAL:
        case TOKEN_KIND_CHAR_LITERAL:
        case TOKEN_KIND_BOOL_LITERAL:
            while(1) {
                token tok = eat_token(tok_stream);
                if(tok.kind == ';') {
                    break;
                }
            }
            ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
            report_error_new(ast, tok_stream, err_node, "statement cannot start with a literal.");
            return err_node;
            break;
    }
    return NULL;
}

ast_node *parse_function_declaration(ast *ast, token_stream *tok_stream, ast_node *ident) {
    ast_node **params = parse_function_parameters(ast, tok_stream);
    ast_node *return_type = NULL;
    if(match_and_eat_token(tok_stream, TOKEN_KIND_RIGHT_ARROW)) {
        if(peek_token(tok_stream).kind == TOKEN_KIND_IDENTIFIER) {
            return_type = ident_node(ast, eat_token(tok_stream).ident_string);
        }
    }
    ast_node *block = parse_block(ast, tok_stream);
    return function_declaration_node(ast, ident, params, block, return_type);
}

ast_node *parse_declaration(ast *ast, token_stream *tok_stream) {
    token ident_tok = {0};
    if(!(peek_token(tok_stream).kind == TOKEN_KIND_IDENTIFIER)) {
        ast_node *err_node = error_node(ast, ident_tok, ERROR_KIND_PARSE_ERROR);
        report_error_new(ast, tok_stream, err_node, "Expected a name belonging to a function, union, enum, or struct declaration.");
        return err_node;
    } else {
        ident_tok = eat_token(tok_stream);
    }
    ast_node *ident = ident_node(ast, ident_tok.ident_string);
    match_and_eat_token(tok_stream, TOKEN_KIND_COLON_COLON);

    token tok = eat_token(tok_stream);
    if (tok.kind == '(') {
        return parse_function_declaration(ast, tok_stream, ident);
    } else if (tok.kind == TOKEN_KIND_STRUCT) {
        ast_node *block = parse_block(ast, tok_stream);
        return struct_declaration_node(ast, ident, block);
    } else if (tok.kind == TOKEN_KIND_ENUM) {
        ast_node *block = parse_block(ast, tok_stream);
        return enum_declaration_node(ast, ident, block);
    } else if (tok.kind == TOKEN_KIND_UNION) {
        ast_node *block = parse_block(ast, tok_stream);
        return union_declaration_node(ast, ident, block);
    } else {
        while(1) {
            token tok = eat_token(tok_stream);
            if(tok.kind == '}') {
                break;
            }
        }
        ast_node *err_node = error_node(ast, tok, ERROR_KIND_PARSE_ERROR);
        report_error_new(ast, tok_stream, err_node, "Error: Expected function parameters or 'union' or 'enum' or 'struct' after \"::\".");
        return err_node;
    }
    return NULL;
}

ast_node *parse_file(ast *ast, token_stream *tok_stream) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FILE;
    while(peek_token(tok_stream).kind != TOKEN_KIND_END_OF_STREAM) {
        ast_node *declaration = parse_declaration(ast, tok_stream);
        da_push(node->file.declarations, declaration);
    }
    return node;
}
