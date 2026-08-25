#include <stdarg.h>

#include "parser.h"
#include "tokenizer.h"

typedef enum operator_pos {
    PREFIX,
    POSTFIX,
}operator_pos;

u32 peek_token_idx(tokenizer *tokenizer) {
    return tokenizer->current_token;
}

u32 peek_next_token_idx(tokenizer *tokenizer) {
    return tokenizer->current_token + 1;
}

u32 eat_token_idx(tokenizer *tokenizer) {
    u32 current_tok = tokenizer->current_token;
    tokenizer->current_token++;
    return current_tok;
}

token peek_token(tokenizer *tokenizer) {
    return tokenizer->tokens[tokenizer->current_token];
}

token peek_next_token(tokenizer *tokenizer) {
    return tokenizer->tokens[tokenizer->current_token + 1];
}

token eat_token(tokenizer *tokenizer) {
    token current_tok = tokenizer->tokens[tokenizer->current_token];
    tokenizer->current_token++;
    return current_tok;
}

b32 match_and_eat_token(tokenizer *tokenizer, token_kind kind) {
    token tok = peek_token(tokenizer);
    if(tok.kind == kind || tok.kind == TOKEN_KIND_ERROR) {
        eat_token(tokenizer);
        return true;
    }
    return false;
}

ast_node *error_node(ast *ast, error_kind kind, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_ERROR;
    node->error.kind = kind;
    node->token_idx = token_idx;
    return node;
}

ast_node *binop_node(ast *ast, binop_kind kind, ast_node *left, ast_node *right, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BINOP;
    node->binop.left = left;
    node->binop.right = right;
    node->binop.kind = kind;
    node->token_idx = token_idx;
    return node;
}

ast_node *break_node(ast *ast, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BREAK;
    node->token_idx = token_idx;
    return node;
}

ast_node *continue_node(ast *ast, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_CONTINUE;
    node->token_idx = token_idx;
    return node;
}

ast_node *return_node(ast *ast, ast_node *expression, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_RETURN;
    node->return_stmt.expression = expression;
    node->token_idx = token_idx;
    return node;
}

ast_node *if_node(ast *ast, ast_node *cond, ast_node *then_block, ast_node *else_part, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_IF;
    node->if_stmt.cond = cond;
    node->if_stmt.then_block = then_block;
    node->if_stmt.else_part = else_part;
    node->token_idx = token_idx;
    return node;
}

ast_node *for_node(ast *ast, ast_node *range, ast_node *block, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FOR;
    node->for_loop.range = range;
    node->for_loop.block = block;
    node->token_idx = token_idx;
    return node;
}

ast_node *while_node(ast *ast, ast_node *cond, ast_node *block, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_WHILE;
    node->while_loop.cond = cond;
    node->while_loop.block = block;
    node->token_idx = token_idx;
    return node;
}

ast_node *unary_node(ast *ast, unary_kind kind, ast_node *operand, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_UNARY;
    node->unary.kind = kind;
    node->unary.operand = operand;
    node->token_idx = token_idx;
    return node;
}

ast_node *function_call_node(ast *ast, ast_node *function_name, ast_node **params, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FUNCTION_CALL;
    node->function_call.callee = function_name;
    node->function_call.params = params;
    node->token_idx = token_idx;
    return node;
}

ast_node *ident_node(ast *ast, string8 value, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_IDENT;
    node->ident.name = value;
    node->token_idx = token_idx;
    return node;
}

ast_node *multi_assign_node(ast *ast, binop_kind kind, ast_node **left, ast_node *right, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_MULTI_ASSIGN;
    node->multi_assign.kind = kind;
    node->multi_assign.left = left;
    node->multi_assign.right = right;
    node->token_idx = token_idx;
    return node;
}

ast_node *struct_declaration_node(ast *ast, ast_node *ident, ast_node *block, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_STRUCT;
    node->struct_decl.ident = ident;
    node->struct_decl.block = block;
    node->token_idx = token_idx;
    return node;
}

ast_node *union_declaration_node(ast *ast, ast_node *ident, ast_node *block, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_UNION;
    node->union_decl.ident = ident;
    node->union_decl.block = block;
    node->token_idx = token_idx;
    return node;
}

ast_node *enum_declaration_node(ast *ast, ast_node *ident, ast_node *block, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_ENUM;
    node->enum_decl.ident = ident;
    node->enum_decl.block = block;
    node->token_idx = token_idx;
    return node;
}

ast_node *function_declaration_node(ast *ast, ast_node *function_name, ast_node **params, ast_node *block, ast_node *return_type, u32 token_idx) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FUNCTION_DECLARATION;
    node->func_decl.function_name = function_name;
    node->func_decl.params = params;
    node->func_decl.block = block;
    node->func_decl.return_type = return_type;
    node->token_idx = token_idx;
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

void report_parse_error(ast *ast, tokenizer *tokenizer, u32 token_idx, const char *fmt, ...) {
    source_location location = get_source_location_from_token(tokenizer, token_idx);

    printf("%s:%d:%d ", ast->file_name, location.l0, location.c0);
    printf("Error: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    print_line(location.start_of_line);
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


void skip_statement_starting_with_literal(tokenizer *tokenizer) {
    //TODO: fix this shit.
    assert(0);
    u32 current_line_number = tokenizer->current_line_number;
    while(1) {
        token tok = eat_token(tokenizer);
        u32 new_line_number = 0;
        if(tok.kind == ';' || tok.kind == TOKEN_KIND_END_OF_STREAM || current_line_number != new_line_number) {
            break;
        }
        new_line_number = tokenizer->current_line_number;
    }
}

ast_node *parse_prefix(ast *ast, tokenizer *tokenizer) {
    u32 token_idx = eat_token_idx(tokenizer);
    token tok = tokenizer->tokens[token_idx];
    switch(tok.kind) {
        case TOKEN_KIND_FLOAT_LITERAL:
            return node_float_lit(ast, get_float_value_from_token(tokenizer, tok));
            break;
        case TOKEN_KIND_CHAR_LITERAL:
            return node_char_lit(ast, get_char_value_from_token(tokenizer, tok));
            break;
        case TOKEN_KIND_STRING_LITERAL:
            return node_string_lit(ast, get_ident_or_string_literal_from_token(tokenizer, tok));
            break;
        case TOKEN_KIND_BOOL_LITERAL:
            return node_bool_lit(ast, get_bool_value_from_token(tokenizer, tok));
            break;
        case TOKEN_KIND_INT_LITERAL:
            return node_int_lit(ast, get_int_value_from_token(tokenizer, tok));
            break;
        case '(': // for parenthesized expressions.
            ast_node *expr = parse_expression(ast, tokenizer, -9999);
            b32 matched = match_and_eat_token(tokenizer, ')');

            if(expr->kind == NODE_KIND_ERROR && expr->error.kind == ERROR_KIND_LEX_ERROR) {
                return expr;
            } else if(!matched) {
                ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, err_node->token_idx, "Expected closing parenthesis to terminate parenthesized expression.");
                return err_node;
            } else {
                return expr;
            }
            break;
        case '+':
        case '-':
        case '~':
        case '!':
        case TOKEN_KIND_MINUS_MINUS:
        case TOKEN_KIND_PLUS_PLUS:
            ast_node *operand = parse_prefix(ast, tokenizer);
            unary_kind kind = unary_from_token(tok, PREFIX);
            return unary_node(ast, kind, operand, token_idx);
            break;
        case TOKEN_KIND_IDENTIFIER:
            return ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, tok), token_idx);
            break;
        case TOKEN_KIND_ERROR:
            return error_node(ast, ERROR_KIND_LEX_ERROR, token_idx);
            break;
        default:
            mem_arena *scratch = arena_get_scratch();
            string8 token_string = token_to_string(scratch, tokenizer, tok);
            printf("unhandled token kind in %s(), %.*s", __func__, (int)token_string.length, token_string.data);
            return error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            break;
    }
}

// 1 + 2 * 3 + 4 + 5

ast_node *parse_expression(ast *ast, tokenizer *tokenizer, int min_prec) {
    ast_node *left = parse_prefix(ast, tokenizer);
    while (1) {
        token next_token = peek_token(tokenizer);
        i32 prec = get_operator_precedence(next_token);
        // prec == 0 means that the token is not an operator or we hit end of stream.
        if (prec == 0 || prec < min_prec) {
            break;
        }

        left = parse_infix_and_postfix(ast, tokenizer, prec, left);
    }
    return left;
}

ast_node **parse_function_parameters(ast *ast, tokenizer *tokenizer) {
    ast_node **params = NULL;
    while(1) {
        token tok = peek_token(tokenizer);
        if(tok.kind == ')') {
            break;
        }
        switch(tok.kind) {
            case ',':
                eat_token(tokenizer);
                break;
            default:
                ast_node *param = parse_expression(ast, tokenizer, -9999);
                da_push(params, param);
                break;
        }
    }
    match_and_eat_token(tokenizer, ')');
    return params;
}

ast_node *parse_infix_and_postfix(ast *ast, tokenizer *tokenizer, i32 prec, ast_node *left) {
    u32 token_idx = eat_token_idx(tokenizer);
    token tok = tokenizer->tokens[token_idx];
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
            ast_node *right = parse_expression(ast, tokenizer, prec + 1); // +1 for left-assoc
            binop_kind kind = binop_from_token(tok);
            return binop_node(ast, kind, left, right, token_idx);
            break;
        }
        case '[': {
            ast_node *index = parse_expression(ast, tokenizer, -9999);
            match_and_eat_token(tokenizer, ']');
            return binop_node(ast, BINOP_ARRAY_SUBSCRIPT, left, index, token_idx);
            break;
        }
        case '.': {
            ast_node *right = parse_prefix(ast, tokenizer);
            return binop_node(ast, BINOP_MEMBER_ACCESS, left, right, token_idx);
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
            ast_node *right = parse_expression(ast, tokenizer, prec - 1); // -1 for right-assoc
            binop_kind kind = binop_from_token(tok);
            mem_arena *arena = arena_get_scratch();
            string8 str = token_to_string(arena, tokenizer, tok);
            const char *cstr = str_to_cstr(arena, str);

            if(right->kind == NODE_KIND_ERROR && right->error.kind == ERROR_KIND_LEX_ERROR) {
                return right;
            } else if(right->kind == NODE_KIND_ERROR && right->error.kind == ERROR_KIND_PARSE_ERROR) {
                report_parse_error(ast, tokenizer, right->token_idx, "expected expression after '%s'", cstr);
                return right;
            } else {
                return binop_node(ast, kind, left, right, token_idx);
            }
            break;
        }
        case '(':
            ast_node **params = parse_function_parameters(ast, tokenizer);
            return function_call_node(ast, left, params, token_idx);
            break;
        case TOKEN_KIND_ERROR:
            return error_node(ast, ERROR_KIND_LEX_ERROR, token_idx);
            break;
        default:
            return error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            break;
    }
}

ast_node *parse_block(ast *ast, tokenizer *tokenizer) {
    match_and_eat_token(tokenizer, '{');
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BLOCK;
    node->block.statements = NULL;
    while (peek_token(tokenizer).kind != '}' &&
           peek_token(tokenizer).kind != TOKEN_KIND_END_OF_STREAM) {
        ast_node *stmt = parse_statement(ast, tokenizer);
        da_push(node->block.statements, stmt);
    }
    match_and_eat_token(tokenizer, '}');
    return node;
}

ast_node *parse_if(ast *ast, tokenizer *tokenizer, u32 token_idx) {
    match_and_eat_token(tokenizer, TOKEN_KIND_IF);
    ast_node *cond = parse_expression(ast, tokenizer, -9999);
    ast_node *then_block = parse_block(ast, tokenizer);
    ast_node *else_part = parse_else_or_else_if(ast, tokenizer);
    return if_node(ast, cond, then_block, else_part, token_idx);
}

ast_node *parse_else_or_else_if(ast *ast, tokenizer *tokenizer) {
    u32 token_idx = peek_token_idx(tokenizer);
    token tok = tokenizer->tokens[token_idx];
    ast_node *node = NULL;
    if(match_and_eat_token(tokenizer, TOKEN_KIND_ELSE)) {
        tok = peek_token(tokenizer);
        if(tok.kind == TOKEN_KIND_IF) {
            node = parse_if(ast, tokenizer, token_idx);
        } else if(tok.kind == '{') {
            node = parse_block(ast, tokenizer);
        } else {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "expected '{' after else if statement");
            return err_node;
        }
    } else {
        node = NULL;
    }
    return node;
}

ast_node *parse_statement(ast *ast, tokenizer *tokenizer) {
    u32 token_idx = peek_token_idx(tokenizer);
    token tok = tokenizer->tokens[token_idx];
    switch(tok.kind) {
        case TOKEN_KIND_IF: {
            tok = peek_next_token(tokenizer);
            if(tok.kind == TOKEN_KIND_IDENTIFIER || tok.kind == TOKEN_KIND_INT_LITERAL) {
                return parse_if(ast, tokenizer, token_idx);
            } else {
                ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, err_node->token_idx, "expected '{' after if statement");
                return err_node;
            }
            break;
        }
        case TOKEN_KIND_WHILE: {
            eat_token(tokenizer);
            ast_node *cond = parse_expression(ast, tokenizer, -9999);
            ast_node *block = parse_block(ast, tokenizer);
            return while_node(ast, cond, block, token_idx);
            break;
        }
        case TOKEN_KIND_FOR: {
            eat_token(tokenizer);
            ast_node *range_or_count = parse_expression(ast, tokenizer, -9999);
            ast_node *block = parse_block(ast, tokenizer);
            return for_node(ast, range_or_count, block, token_idx);
            break;
        }
        case TOKEN_KIND_CONTINUE: {
            eat_token(tokenizer);
            b32 matched = match_and_eat_token(tokenizer, ';');
            if(!matched) {
                ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, err_node->token_idx, "expected ';' after continue statement");
                return err_node;
            }
            return continue_node(ast, token_idx);
            break;
        }
        case TOKEN_KIND_BREAK: {
            eat_token(tokenizer);
            b32 matched = match_and_eat_token(tokenizer, ';');
            if(!matched) {
                ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, err_node->token_idx, "expected ';' after break statement");
                return err_node;
            }
            return break_node(ast, token_idx);
            break;
        }
        case TOKEN_KIND_RETURN: {
            eat_token(tokenizer);
            ast_node *expression = parse_expression(ast, tokenizer, -9999);
            b32 matched = match_and_eat_token(tokenizer, ';');
            if(!matched) {
                ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, err_node->token_idx, "expected ';' after return statement");
                return err_node;
            }
            return return_node(ast, expression, token_idx);
            break;
        }
        case '{': {
            return parse_block(ast, tokenizer);
            break;
        }
        case TOKEN_KIND_IDENTIFIER:
            tok = peek_next_token(tokenizer);
            if (tok.kind == ',') {
                ast_node **lhs_list = NULL;
                token first_tok = eat_token(tokenizer);
                ast_node *first = ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, first_tok), token_idx);
                da_push(lhs_list, first);

                while (peek_token(tokenizer).kind == ',') {
                    match_and_eat_token(tokenizer, ',');
                    token next_tok = eat_token(tokenizer);
                    ast_node *lhs = ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, next_tok), token_idx);
                    da_push(lhs_list, lhs);
                }

                tok = peek_token(tokenizer);
                binop_kind assign_op = assignment_binop_from_token(tok);
                if (assign_op == BINOP_NONE) {
                    ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                    report_parse_error(ast, tokenizer, err_node->token_idx, "expected an assignment operator after the variables in the multi-assignment expression.");
                    return err_node;
                }
                match_and_eat_token(tokenizer, tok.kind);
                ast_node *rhs = parse_expression(ast, tokenizer, -9999);

                b32 matched = match_and_eat_token(tokenizer, ';');
                if (!matched) {
                    ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                    report_parse_error(ast, tokenizer, err_node->token_idx, "expected ';' after multi-assignment");
                    return err_node;
                }
                return multi_assign_node(ast, assign_op, lhs_list, rhs, token_idx);
            }

            ast_node *expr = parse_expression(ast, tokenizer, -9999);
            b32 matched = match_and_eat_token(tokenizer, ';');
            if(expr->kind == NODE_KIND_ERROR) {
                return expr;
            } else if(!matched) {
                expr = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, expr->token_idx, "expected ';' after expression");
                return expr;
            } else {
                return expr;
            }
            break;
        case TOKEN_KIND_INT_LITERAL:
        case TOKEN_KIND_FLOAT_LITERAL:
        case TOKEN_KIND_STRING_LITERAL:
        case TOKEN_KIND_CHAR_LITERAL:
        case TOKEN_KIND_BOOL_LITERAL:
            skip_statement_starting_with_literal(tokenizer);
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "statement cannot start with a literal.");
            return err_node;
            break;
        default:
            printf("unhandled token kind in %s()", __func__);
            break;
    }
    return NULL;
}

ast_node *parse_function_declaration(ast *ast, tokenizer *tokenizer, ast_node *ident) {
    ast_node **params = parse_function_parameters(ast, tokenizer);
    ast_node *return_type = NULL;
    if(match_and_eat_token(tokenizer, TOKEN_KIND_RIGHT_ARROW)) {
        if(peek_token(tokenizer).kind == TOKEN_KIND_IDENTIFIER) {
            u32 return_tok_idx = eat_token_idx(tokenizer);
            token return_tok = tokenizer->tokens[return_tok_idx];
            return_type = ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, return_tok), return_tok_idx);
        }
    }
    ast_node *block = parse_block(ast, tokenizer);
    return function_declaration_node(ast, ident, params, block, return_type, ident->token_idx);
}

ast_node *parse_toplevel_statement(ast *ast, tokenizer *tokenizer, u32 token_idx) {
    token tok = tokenizer->tokens[token_idx];
    switch(tok.kind) {
        case TOKEN_KIND_IF: {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have an if statement in the global scope.");
            return err_node;
            break;
        }
        case TOKEN_KIND_WHILE: {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have a while loop in the global scope.");
            return err_node;
            break;
        }
        case TOKEN_KIND_FOR: {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have a for loop in the global scope.");
            return err_node;
            break;
        }
        case TOKEN_KIND_CONTINUE: {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have a continue in the global scope. It has to be in a switch statement that's in a function.");
            return err_node;
            break;
        }
        case TOKEN_KIND_BREAK: {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have a break in the global scope. It has to be in a switch statement that's in a function.");
            return err_node;
            break;
        }
        case TOKEN_KIND_RETURN: {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have a return in the global scope. It has to be in a function.");
            return err_node;
            break;
        }
        case '{': {
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Cannot have a block in the global scope.");
            return err_node;
            break;
        }
        case TOKEN_KIND_IDENTIFIER:
            tok = peek_next_token(tokenizer);
            if (tok.kind == ',') {
                ast_node **lhs_list = NULL;
                u32 first_tok_idx = eat_token_idx(tokenizer);
                token first_tok = tokenizer->tokens[first_tok_idx];
                ast_node *first = ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, first_tok), first_tok_idx);
                da_push(lhs_list, first);

                while (peek_token(tokenizer).kind == ',') {
                    match_and_eat_token(tokenizer, ',');
                    u32 next_tok_idx = eat_token_idx(tokenizer);
                    token next_tok = tokenizer->tokens[next_tok_idx];
                    ast_node *lhs = ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, next_tok), next_tok_idx);
                    da_push(lhs_list, lhs);
                }

                tok = peek_token(tokenizer);
                binop_kind assign_op = assignment_binop_from_token(tok);
                if (assign_op == BINOP_NONE) {
                    ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                    report_parse_error(ast, tokenizer, err_node->token_idx, "expected an assignment operator after the variables in the multi-assignment expression.");
                    return err_node;
                }
                match_and_eat_token(tokenizer, tok.kind);
                ast_node *rhs = parse_expression(ast, tokenizer, -9999);

                b32 matched = match_and_eat_token(tokenizer, ';');
                if (!matched) {
                    ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                    report_parse_error(ast, tokenizer, err_node->token_idx, "expected ';' after multi-assignment");
                    return err_node;
                }
                return multi_assign_node(ast, assign_op, lhs_list, rhs, token_idx);
            }

            ast_node *expr = parse_expression(ast, tokenizer, -9999);
            b32 matched = match_and_eat_token(tokenizer, ';');
            if(expr->kind == NODE_KIND_ERROR) {
                return expr;
            } else if(!matched) {
                expr = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
                report_parse_error(ast, tokenizer, expr->token_idx, "expected ';' after expression");
                return expr;
            } else {
                return expr;
            }
            break;
        case TOKEN_KIND_INT_LITERAL:
        case TOKEN_KIND_FLOAT_LITERAL:
        case TOKEN_KIND_STRING_LITERAL:
        case TOKEN_KIND_CHAR_LITERAL:
        case TOKEN_KIND_BOOL_LITERAL: {
            skip_statement_starting_with_literal(tokenizer);
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "statement cannot start with a literal.");
            return err_node;
        } break;
        default:
            mem_arena *scratch = arena_get_scratch();
            string8 token_string = token_to_string(scratch, tokenizer, tok);
            printf("unhandled token kind in %s(): %.*s", __func__, (int)token_string.length, token_string.data);
            break;
    }
    return NULL;
}

ast_node *parse_declaration(ast *ast, tokenizer *tokenizer) {
    // NOTE: we eagerly eat tokens in this function to avoid doing too much lookahead.
    // Which means passing some of the things that we eagerly consume into the functions
    // that parse the thing that we want to parse.
    u32 token_idx = peek_token_idx(tokenizer);
    token ident_tok = tokenizer->tokens[token_idx];
    if(!(ident_tok.kind == TOKEN_KIND_IDENTIFIER)) {
        ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
        report_parse_error(ast, tokenizer, err_node->token_idx, "Expected a name belonging to a toplevel statement, function, union, enum, or struct declaration.");
        return err_node;
    } else {
        ident_tok = eat_token(tokenizer);
    }
    ast_node *ident = ident_node(ast, get_ident_or_string_literal_from_token(tokenizer, ident_tok), token_idx);
    if(match_and_eat_token(tokenizer, TOKEN_KIND_COLON_COLON)) {
        token tok = eat_token(tokenizer);
        if (tok.kind == '(') {
            return parse_function_declaration(ast, tokenizer, ident);
        } else if (tok.kind == TOKEN_KIND_STRUCT) {
            ast_node *block = parse_block(ast, tokenizer);
            return struct_declaration_node(ast, ident, block, token_idx);
        } else if (tok.kind == TOKEN_KIND_ENUM) {
            ast_node *block = parse_block(ast, tokenizer);
            return enum_declaration_node(ast, ident, block, token_idx);
        } else if (tok.kind == TOKEN_KIND_UNION) {
            ast_node *block = parse_block(ast, tokenizer);
            return union_declaration_node(ast, ident, block, token_idx);
        } else {
            while(1) {
                token tok = eat_token(tokenizer);
                if(tok.kind == '}') {
                    break;
                }
            }
            ast_node *err_node = error_node(ast, ERROR_KIND_PARSE_ERROR, token_idx);
            report_parse_error(ast, tokenizer, err_node->token_idx, "Error: Expected function parameters or 'union' or 'enum' or 'struct' after \"::\".");
            return err_node;
        }
    } else {
        return parse_toplevel_statement(ast, tokenizer, token_idx);
    }
    return NULL;
}

ast_node *parse_file(ast *ast, tokenizer *tokenizer) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FILE;
    while(peek_token(tokenizer).kind != TOKEN_KIND_END_OF_STREAM) {
        ast_node *declaration = parse_declaration(ast, tokenizer);
        da_push(node->file.declarations, declaration);
    }

    // we use the scratch arena for some things in the parser.
    // this is a good point to reset it.
    mem_arena *scratch = arena_get_scratch();
    arena_clear(scratch);
    return node;
}
