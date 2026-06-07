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

token eat_token(token_stream *stream, token_kind kind) {
    token current_tok = *stream->at;
    if(stream->at + 1 < stream->end) {
        if(current_tok.kind == kind) {
            stream->at += 1;
            return current_tok;
        } else {
            // NOTE: This is trash, do something better.
            // print the actual kind as text.
            fatal_error("Error: expected %d token", kind);
        }
    }
    printf("Error: %s(): Tried to eat past end of file\n", __func__);
    return current_tok;
}

ast_node *binop_node(ast *ast, u8 kind, ast_node *left, ast_node *right) {
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

ast_node *ident_node(ast *ast, token_stream *tok_stream) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_IDENT;
    node->ident.name = eat_token(tok_stream, TOKEN_KIND_IDENTIFIER).string_value;
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

ast_node *function_declaration_node(ast *ast, ast_node *ident, ast_node **params, ast_node *block, ast_node *return_type) {
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FUNCTION_DECLARATION;
    node->func_decl.callee = ident;
    node->func_decl.params = params;
    node->func_decl.block = block;
    node->func_decl.return_type = return_type;
    return node;
}

i32 get_operator_precedence(token tok) {
    i32 prec = 0;
    switch(tok.kind) {
        case ',':
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

ast_node *parse_int_lit(ast *ast, token_stream *tok_stream) {
    token tok = eat_token(tok_stream, TOKEN_KIND_INT_LITERAL);

    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_INT_LIT;
    node->int_lit.value = tok.integer_value;
    return node;
}

ast_node *parse_float_lit(ast *ast, token_stream *tok_stream) {
    token tok = eat_token(tok_stream, TOKEN_KIND_FLOAT_LITERAL);

    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_FLOAT_LIT;
    node->float_lit.value = tok.float_value;
    return node;
}

ast_node *parse_char_lit(ast *ast, token_stream *tok_stream) {
    token tok = eat_token(tok_stream, TOKEN_KIND_CHAR_LITERAL);

    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_CHAR_LIT;
    node->char_lit.value = tok.char_value;
    return node;
}

ast_node *parse_string_lit(ast *ast, token_stream *tok_stream) {
    token tok = eat_token(tok_stream, TOKEN_KIND_STRING_LITERAL);

    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_STRING_LIT;
    node->str_lit.value = tok.string_value;
    return node;
}

ast_node *parse_bool_lit(ast *ast, token_stream *tok_stream) {
    token tok = eat_token(tok_stream, TOKEN_KIND_BOOL_LITERAL);

    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BOOL_LIT;
    node->int_lit.value = tok.bool_value;
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
    token tok = peek_token(tok_stream);
    switch(tok.kind) {
        case TOKEN_KIND_FLOAT_LITERAL:
            return parse_float_lit(ast, tok_stream);
            break;
        case TOKEN_KIND_CHAR_LITERAL:
            return parse_char_lit(ast, tok_stream);
            break;
        case TOKEN_KIND_STRING_LITERAL:
            return parse_string_lit(ast, tok_stream);
            break;
        case TOKEN_KIND_BOOL_LITERAL:
            return parse_bool_lit(ast, tok_stream);
            break;
        case TOKEN_KIND_INT_LITERAL:
            return parse_int_lit(ast, tok_stream);
            break;
        case '(': // for parenthesized expressions.
            eat_token(tok_stream, '(');
            ast_node *expr = parse_expression(ast, tok_stream, -9999);
            token eaten = eat_token(tok_stream, ')');
            if(eaten.kind != ')') {
                fatal_error("Error: Expected closing parenthesis to terminate parenthesized expression.");
            }
            return expr;
            break;
        case '+':
        case '-':
        case '~':
        case '!':
        case TOKEN_KIND_MINUS_MINUS:
        case TOKEN_KIND_PLUS_PLUS:
            eat_token(tok_stream, TOKEN_KIND_PLUS_PLUS);
            ast_node *operand = parse_prefix(ast, tok_stream);
            unary_kind kind = unary_from_token(tok, PREFIX);
            return unary_node(ast, kind, operand);
            break;
        case TOKEN_KIND_IDENTIFIER:
            return ident_node(ast, tok_stream);
            break;
    }
    return NULL;
}

// 6 * 2 + 4
// we see 6, so we call parse_expression(ast, tok_stream, -9999)
// we consume the 6, check the next token.
// Next token is a binary operator (*), with a prec of 400.
// 400 > -9999, so call parse_expression(ast, tok_stream, -9999).
// we see a 2 and consume it, check the next token.
// Next token is a binary operator (+).
// Prec of + is 200.
// 200 < 400, so DO NOT RECURSE. return 6 * 2.
/*
 left =    *
          / \
         6   2

we are in the first call, second iteration of the loop
we call parse_infix_and_postfix() in parse_expression().
which calls parse expression.
we see +.
prec of + is 200
200 > -9999.
RECURSE.
we see 4, no tokens left, return + 4

op = +
left = *
      / \
     6   2
right = 4

Becomes:

      +
     / \
    *   4
   / \
  6   2

*/
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
    eat_token(tok_stream, '(');
    ast_node **params = NULL;
    while(1) {
        token tok = peek_token(tok_stream);
        if(tok.kind == ')') {
            break;
        }
        switch(tok.kind) {
            case ',':
                eat_token(tok_stream, ',');
                break;
            default:
                ast_node *param = parse_expression(ast, tok_stream, -9999);
                da_push(params, param);
                break;
        }
    }
    eat_token(tok_stream, ')');
    return params;
}

binop_kind binop_from_token(token tok) {
    binop_kind kind = BINOP_NONE;
    switch(tok.kind) {
        case '+':
            kind = BINOP_ADD;
            break;
        case '-':
            kind = BINOP_SUB;
            break;
        case '/':
            kind = BINOP_DIV;
            break;
        case '%':
            kind = BINOP_MOD;
            break;
        case '*':
            kind = BINOP_MUL;
            break;
        case '=':
            kind = BINOP_ASSIGN;
            break;
        case ':':
            kind = BINOP_COLON;
            break;
        case TOKEN_KIND_DOT_DOT:
            kind = BINOP_RANGE;
            break;
        case TOKEN_KIND_EQUAL_EQUAL:
            kind = BINOP_EQUALS;
            break;
        case TOKEN_KIND_PLUS_EQUAL:
            kind = BINOP_ADD_ASSIGN;
            break;
        case TOKEN_KIND_MINUS_EQUAL:
            kind = BINOP_MINUS_ASSIGN;
            break;
        case TOKEN_KIND_DIVIDE_EQUAL:
            kind = BINOP_DIV_ASSIGN;
            break;
        case TOKEN_KIND_MOD_EQUAL:
            kind = BINOP_MOD_ASSIGN;
            break;
        case TOKEN_KIND_MULTIPLY_EQUAL:
            kind = BINOP_MUL_ASSIGN;
            break;
        case TOKEN_KIND_XOR_EQUAL:
            kind = BINOP_XOR_ASSIGN;
            break;
        case TOKEN_KIND_AND_EQUAL:
            kind = BINOP_AND_ASSIGN;
            break;
        case TOKEN_KIND_OR_EQUAL:
            kind = BINOP_OR_ASSIGN;
            break;
        case TOKEN_KIND_NOT_EQUAL:
            kind = BINOP_LOGICAL_NOT;
            break;
        case TOKEN_KIND_COLON_EQUAL:
            kind = BINOP_COLON_EQUAL;
            break;
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:
            kind = BINOP_LEFT_SHIFT_ASSIGN;
            break;
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL:
            kind = BINOP_RIGHT_SHIFT_ASSIGN;
            break;
        case TOKEN_KIND_GREATER_THAN_EQUAL:
            kind = BINOP_GREATER_THAN_ASSIGN;
            break;
        case TOKEN_KIND_LESS_THAN_EQUAL:
            kind = BINOP_LESS_THAN_ASSIGN;
            break;
        case TOKEN_KIND_RIGHT_SHIFT:
            kind = BINOP_RIGHT_SHIFT;
            break;
        case TOKEN_KIND_LEFT_SHIFT:
            kind = BINOP_LEFT_SHIFT;
            break;
        case TOKEN_KIND_COLON_COLON:
            kind = BINOP_COLON_COLON;
            break;
        case TOKEN_KIND_AND_AND:
            kind = BINOP_LOGICAL_AND;
            break;
        case TOKEN_KIND_OR_OR:
            kind = BINOP_LOGICAL_OR;
            break;
        default:
            kind = BINOP_NONE;
    }
    return kind;
}

ast_node *parse_infix_and_postfix(ast *ast, token_stream *tok_stream, i32 prec, ast_node *left) {
    token tok = peek_token(tok_stream);
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
        case ',':
        case TOKEN_KIND_AND_AND:
        case TOKEN_KIND_OR_OR:

        case TOKEN_KIND_NOT_EQUAL:
        case TOKEN_KIND_EQUAL_EQUAL:
        case TOKEN_KIND_GREATER_THAN_EQUAL:
        case TOKEN_KIND_LESS_THAN_EQUAL:
        case TOKEN_KIND_RIGHT_SHIFT:
        case TOKEN_KIND_LEFT_SHIFT: {
            eat_token(tok_stream, tok.kind);  // consume the operator
            ast_node *right = parse_expression(ast, tok_stream, prec + 1); // +1 for left-assoc
            binop_kind kind = binop_from_token(tok);
            return binop_node(ast, kind, left, right);
            break;
        }
        case '[': {
            eat_token(tok_stream, '[');
            ast_node *index = parse_expression(ast, tok_stream, -9999);
            eat_token(tok_stream, ']');
            return binop_node(ast, BINOP_ARRAY_SUBSCRIPT, left, index);
            break;
        }
        case '.': {
            eat_token(tok_stream, '.');
            ast_node *right = parse_prefix(ast, tok_stream);
            return binop_node(ast, BINOP_MEMBER_ACCESS, left, right);
            break;
        }
        case TOKEN_KIND_DOT_DOT: {
            ast_node *left = parse_expression(ast, tok_stream, -9999);
            eat_token(tok_stream, TOKEN_KIND_DOT_DOT);
            ast_node *right = parse_expression(ast, tok_stream, -9999);
            return binop_node(ast, BINOP_RANGE, left, right);
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
            eat_token(tok_stream, tok.kind);  // consume the operator
            ast_node *right = parse_expression(ast, tok_stream, prec - 1); // -1 for right-assoc
            binop_kind kind = binop_from_token(tok);
            return binop_node(ast, kind, left, right);
            break;
        }
        case '(':
            ast_node **params = parse_function_parameters(ast, tok_stream);
            return function_call_node(ast, left, params);
            break;
    }
    return NULL;
}

ast_node *parse_block(ast *ast, token_stream *tok_stream) {
    eat_token(tok_stream, '{');
    ast_node *node = &ast->nodes[ast->node_count++];
    node->kind = NODE_KIND_BLOCK;
    node->block.statements = NULL;
    while (peek_token(tok_stream).kind != '}') {
        ast_node *stmt = parse_statement(ast, tok_stream);
        da_push(node->block.statements, stmt);
    }
    eat_token(tok_stream, '}'); // }
    return node;
}

ast_node *parse_if(ast *ast, token_stream *tok_stream) {
    eat_token(tok_stream, TOKEN_KIND_IF); // if
    ast_node *cond = parse_expression(ast, tok_stream, -9999);
    ast_node *then_block = parse_block(ast, tok_stream);
    ast_node *else_part = parse_else_or_else_if(ast, tok_stream);
    return if_node(ast, cond, then_block, else_part);
}

ast_node *parse_else_or_else_if(ast *ast, token_stream *tok_stream) {
    token tok = peek_token(tok_stream);
    ast_node *node = NULL;
    if(tok.kind == TOKEN_KIND_ELSE) {
        eat_token(tok_stream, TOKEN_KIND_ELSE); // else
        tok = peek_token(tok_stream);
        if(tok.kind == TOKEN_KIND_IF) {
            eat_token(tok_stream, TOKEN_KIND_IF); // if
            node = parse_if(ast, tok_stream);
        } else if(tok.kind == '{') {
            node = parse_block(ast, tok_stream);
        } else {
            fatal_error("Error: expected curly brace after \"else if\"");
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
                fatal_error("Error: expected curly brace after \"if\"");
            }
            break;
        }
        case TOKEN_KIND_WHILE: {
            eat_token(tok_stream, TOKEN_KIND_WHILE);
            ast_node *cond = parse_expression(ast, tok_stream, -9999);
            ast_node *block = parse_block(ast, tok_stream);
            return while_node(ast, cond, block);
            break;
        }
        case TOKEN_KIND_FOR: {
            eat_token(tok_stream, TOKEN_KIND_FOR);
            ast_node *range_or_count = parse_expression(ast, tok_stream, -9999);
            ast_node *block = parse_block(ast, tok_stream);
            return for_node(ast, range_or_count, block);
            break;
        }
        case TOKEN_KIND_CONTINUE: {
            eat_token(tok_stream, TOKEN_KIND_CONTINUE);
            tok = eat_token(tok_stream, ';');
            if(tok.kind != ';') {
                fatal_error("Error: Expected semicolon after continue statement.");
            }
            return continue_node(ast);
            break;
        }
        case TOKEN_KIND_BREAK: {
            eat_token(tok_stream, TOKEN_KIND_BREAK);
            tok = eat_token(tok_stream, ';');
            if(tok.kind != ';') {
                fatal_error("Error: Expected semicolon after break statement.");
            }
            return break_node(ast);
            break;
        }
        case TOKEN_KIND_RETURN: {
            eat_token(tok_stream, TOKEN_KIND_RETURN);
            ast_node *expression = parse_expression(ast, tok_stream, -9999);
            tok = eat_token(tok_stream, ';');
            if(tok.kind != ';') {
                fatal_error("Error: Expected semicolon after return statement.");
            }
            return return_node(ast, expression);
            break;
        }
        case '{': {
            return parse_block(ast, tok_stream);
            break;
        }
        case TOKEN_KIND_IDENTIFIER:
            ast_node *expr = parse_expression(ast, tok_stream, -9999);
            tok = eat_token(tok_stream, ';');
            if (tok.kind != ';') {
                fatal_error("Error: Expected semicolon after expression statement.");
            }
            return expr;
            break;
        case TOKEN_KIND_INT_LITERAL:
        case TOKEN_KIND_FLOAT_LITERAL:
        case TOKEN_KIND_STRING_LITERAL:
        case TOKEN_KIND_CHAR_LITERAL:
        case TOKEN_KIND_BOOL_LITERAL:
            fatal_error("Error: statement cannot start with a literal.");
            return NULL;
            break;
    }
    return NULL;
}

ast_node *parse_function_declaration(ast *ast, token_stream *tok_stream, ast_node *ident) {
    ast_node **params = parse_function_parameters(ast, tok_stream);
    ast_node *return_type = NULL;
    if(peek_token(tok_stream).kind == TOKEN_KIND_RIGHT_ARROW) {
        eat_token(tok_stream, TOKEN_KIND_RIGHT_ARROW);
        if(peek_token(tok_stream).kind == TOKEN_KIND_IDENTIFIER) {
            return_type = ident_node(ast, tok_stream);
        }
    }
    ast_node *block = parse_block(ast, tok_stream);
    return function_declaration_node(ast, ident, params, block, return_type);
}

ast_node *parse_declaration_of_struct_or_union_or_enum(ast *ast, token_stream *tok_stream, ast_node *ident) {
    token tok = peek_token(tok_stream);
    if(tok.kind == TOKEN_KIND_STRUCT) {
        tok = eat_token(tok_stream, TOKEN_KIND_STRUCT);
    } else if(tok.kind == TOKEN_KIND_ENUM) {
        tok = eat_token(tok_stream, TOKEN_KIND_ENUM);
    } else if(tok.kind == TOKEN_KIND_UNION) {
        tok = eat_token(tok_stream, TOKEN_KIND_UNION);
    } else {
        fatal_error("we did a fucky wucky\n");
    }
    ast_node *block = parse_block(ast, tok_stream);
    // this assumes that the calling code has verified that the next token
    // is either a struct, enum or union.
    switch(tok.kind) {
        case TOKEN_KIND_STRUCT:
            return struct_declaration_node(ast, ident, block);
            break;
        case TOKEN_KIND_ENUM:
            return enum_declaration_node(ast, ident, block);
            break;
        case TOKEN_KIND_UNION:
            return union_declaration_node(ast, ident, block);
            break;
    }
    return NULL;
}

ast_node *parse_declaration(ast *ast, token_stream *tok_stream) {
    if (peek_token(tok_stream).kind != TOKEN_KIND_IDENTIFIER) {
        fatal_error("Error: Expected a name belonging to a function, union, enum, or struct declaration.");
    }
    ast_node *ident = ident_node(ast, tok_stream);
    eat_token(tok_stream, TOKEN_KIND_COLON_COLON);

    token tok = peek_token(tok_stream);
    if (tok.kind == '(') {
        return parse_function_declaration(ast, tok_stream, ident);
    } else if (tok.kind == TOKEN_KIND_STRUCT || tok.kind == TOKEN_KIND_UNION || tok.kind == TOKEN_KIND_ENUM) {
        return parse_declaration_of_struct_or_union_or_enum(ast, tok_stream, ident);
    } else {
        fatal_error("Error: Expected declaration of a function, union, enum, or struct after \"::\".");
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
