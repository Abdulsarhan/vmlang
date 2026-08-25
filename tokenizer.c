#include "tokenizer.h"

b32 is_whitespace(u8 ch) {
    return ((ch == ' ')  ||
            (ch == '\t') ||
            (ch == '\r') ||
            (ch == '\n') ||
            (ch == '\v') ||
            (ch == '\f'));
}

b32 is_number(u8 ch) {
    return ((ch >= '0') && (ch <= '9'));
}

b32 is_alpha(u8 ch) {
    return (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')));
}

b32 is_lower(u8 ch) {
    return ((ch >= 'a') && (ch <= 'z'));
}

b32 is_upper(u8 ch) {
    return ((ch >= 'A') && (ch <= 'Z'));
}

b32 is_alphanumeric(u8 ch) {
    return(is_number(ch) || is_alpha(ch));
}

b32 is_printable(u8 ch) {
    return ((ch >= 32) && (ch <= 127));
}

u8 eat_char(tokenizer *tokenizer) {
    u8 current_char = *tokenizer->at;
    if(tokenizer->at + 1 <= tokenizer->end) {
        tokenizer->at += 1;
        tokenizer->current_column_number++;
        return current_char;
    }
    printf("Error: %s(): Tried to eat past end of file. Returning 0.\n", __func__);
    return 0;
}

u8 peek_char(const tokenizer *tokenizer) {
    if(tokenizer->at != tokenizer->end) {
        return *(tokenizer->at);
    }
    printf("Error: %s(): Tried to peek past end of file. Returning 0.\n", __func__);
    return 0;
}

void eat_all_whitespaces(tokenizer *tokenizer) {
    while(1) {
        u8 ch = peek_char(tokenizer);
        if(!is_whitespace(ch)) {
            break;
        }
        eat_char(tokenizer);
        if(ch == '\n') {
            tokenizer->start_of_current_line += tokenizer->current_column_number;
            tokenizer->current_column_number = 0;
            tokenizer->current_line_number++;
        }
    }
}

b32 match(tokenizer *tokenizer, u8 expected) {
    if(peek_char(tokenizer) == expected) {
        eat_char(tokenizer);
        return true;
    }
    return false;
}

u8 peek_next_char(const tokenizer *tokenizer) {
    if(tokenizer->at + 1 != tokenizer->end) {
        return *(tokenizer->at + 1);
    }
    printf("Error: %s(): Tried to peek past end of file. Returning 0.\n", __func__);
    return 0;
}

u8 parse_escape_char(tokenizer *tokenizer, u8 *escape_start) {
    if(escape_start[0] != '\\') {
        tok_report_error(tokenizer, "Error: Expected backslash in multi-character char literal\n");
        return 1;
    }
    switch(escape_start[1]) {
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case 'v':  return '\v';
        case '\'': return '\'';
        case '\\': return '\\';
        case '0':  return '\0';
        default:
            tok_report_error(tokenizer, "Error: Invalid escape character in char literal.\n");
            return 1;
            break;
    }
}

string8 token_kind_to_string(token_kind kind) {
    switch(kind) {
        case '+':                                return s("+");
        case '-':                                return s("-");
        case '/':                                return s("/");
        case '%':                                return s("%");
        case '*':                                return s("*");
        case '=':                                return s("=");
        case ':':                                return s(":");
        case '>':                                return s(">");
        case '<':                                return s("<");
        case '&':                                return s("&");
        case '^':                                return s("^");
        case '|':                                return s("|");
        case ';':                                return s(";");

        case TOKEN_KIND_PLUS_EQUAL:              return s("+=");
        case TOKEN_KIND_MINUS_EQUAL:             return s("-=");
        case TOKEN_KIND_DIVIDE_EQUAL:            return s("/=");
        case TOKEN_KIND_MOD_EQUAL:               return s("%=");
        case TOKEN_KIND_MULTIPLY_EQUAL:          return s("*=");
        case TOKEN_KIND_XOR_EQUAL:               return s("^=");
        case TOKEN_KIND_AND_EQUAL:               return s("&=");
        case TOKEN_KIND_OR_EQUAL:                return s("|=");
        case TOKEN_KIND_EQUAL_EQUAL:             return s("==");
        case TOKEN_KIND_NOT_EQUAL:               return s("!=");
        case TOKEN_KIND_COLON_EQUAL:             return s(":=");
        case TOKEN_KIND_DOT_DOT:                 return s("..");
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:        return s("<<=");
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL:       return s(">>=");
        case TOKEN_KIND_GREATER_THAN_EQUAL:      return s(">=");
        case TOKEN_KIND_LESS_THAN_EQUAL:         return s("<=");
        case TOKEN_KIND_RIGHT_SHIFT:             return s(">");
        case TOKEN_KIND_LEFT_SHIFT:              return s("<");
        case TOKEN_KIND_COLON_COLON:             return s("::");
        case TOKEN_KIND_RIGHT_ARROW:             return s("->");
        case TOKEN_KIND_AND_AND:                 return s("&&");
        case TOKEN_KIND_OR_OR:                   return s("||");
        case TOKEN_KIND_PLUS_PLUS:               return s("++");
        case TOKEN_KIND_MINUS_MINUS:             return s("--");
        case TOKEN_KIND_END_OF_STREAM:           return s("end_of_stream");
        case TOKEN_KIND_IF:                      return s("if");
        case TOKEN_KIND_ELSE:                    return s("else");
        case TOKEN_KIND_WHILE:                   return s("while");
        case TOKEN_KIND_FOR:                     return s("for");
        case TOKEN_KIND_UNION:                   return s("union");
        case TOKEN_KIND_STRUCT:                  return s("struct");
        case TOKEN_KIND_ENUM:                    return s("enum");
        case TOKEN_KIND_CONTINUE:                return s("continue");
        case TOKEN_KIND_BREAK:                   return s("break");
        case TOKEN_KIND_RETURN:                  return s("return");
        default:                                 return s("");
    }
    return s("");
}

string8 token_to_string(mem_arena *arena, tokenizer *tokenizer, token tok) {
    switch(tok.kind) {
        case TOKEN_KIND_IDENTIFIER:              return get_ident_or_string_literal_from_token(tokenizer, tok);
        case TOKEN_KIND_INT_LITERAL:             return (string8){.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
        case TOKEN_KIND_FLOAT_LITERAL:           return (string8){.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
        case TOKEN_KIND_CHAR_LITERAL:            return (string8){.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
        case TOKEN_KIND_BOOL_LITERAL:            return get_bool_value_from_token(tokenizer, tok) ? s("true") : s("false");
        case TOKEN_KIND_STRING_LITERAL:          return get_ident_or_string_literal_from_token(tokenizer, tok);
        default:                                 return token_kind_to_string(tok.kind);
    }
    return s("");
}

void tok_report_error(tokenizer *tokenizer, const char *fmt, ...) {
    printf("%s:%d:%d ", tokenizer->file_path, tokenizer->current_line_number, tokenizer->current_column_number);
    printf("Error: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    print_line(tokenizer->start_of_current_line);
    tokenizer->error_count++;
    printf("\n");
}

void make_token(tokenizer *tokenizer, u8 *at, token_kind kind) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->pos = at - tokenizer->file;
    tok->kind = kind;
}

u8 get_char_value_from_token(tokenizer *tokenizer, token tok) {
    u32 token_len = get_token_len(tokenizer, tok);

    u8 *char_start = &tokenizer->file[tok.pos + 1];

    u8 char_value = 0;
    if(token_len < 3) {
        // Because malformed char literals are checked for
        // in the tokenizer, this codepath will never be hit because
        // malformed char literals have the token kind of TOKEN_KIND_ERROR
        // which means that the parser will never call this function on the malformed
        // char literal token.
        unreachable;
        char_value = 0;
    } else if(token_len == 3) {
        char_value = *char_start;
    } else if(token_len == 4) {
        char_value = parse_escape_char(tokenizer, char_start);
        if(char_value == 1) {
            //TODO: handle this error path.
            assert(0);
        }
    }
    return char_value;
}

i64 get_int_value_from_token(tokenizer *tokenizer, token tok) {
    string8 str = {.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
    return str_to_i64(str);
}

f64 get_float_value_from_token(tokenizer *tokenizer, token tok) {
    string8 str = {.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
    return str_to_f64(str);
}

b32 get_bool_value_from_token(tokenizer *tokenizer, token tok) {
    string8 str = {.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
    if(str_match(str, s("false"))) {
        return true;
    } else if(str_match(str, s("true"))) {
        return false;
    } else {
        assert(0);
    }
}

string8 get_ident_or_string_literal_from_token(tokenizer *tokenizer, token tok) {
    string8 str = {.data = &tokenizer->file[tok.pos], .length = get_token_len(tokenizer, tok)};
    return str;

}

typedef struct line line;
struct line {
    u32 line_number;
    u8 *start_of_line;
};

line get_token_line_number(tokenizer *tokenizer, token tok) {
    u8 *at = tokenizer->file;
    u8 *token_pos = &tokenizer->file[tok.pos];

    u32 current_line_number = 1;
    while((at < tokenizer->end) && (at != token_pos)) {
        if(*at == '\n') {
            current_line_number++;
        }
        at++;
    }
    line l;
    l.line_number = current_line_number;
    l.start_of_line = at;
    return l;
}

u32 get_token_len(tokenizer *tokenizer, token tok) {
    u8 *token_pos = &tokenizer->file[tok.pos];
    u8 *orig = token_pos;

    while((token_pos < tokenizer->end) && !is_whitespace(*token_pos)) {
        token_pos++;
    }
    if(tok.kind == TOKEN_KIND_CHAR_LITERAL || tok.kind == TOKEN_KIND_STRING_LITERAL) {
        token_pos--; // to avoid including the closing quote in the length.
    }
    return token_pos - orig;
}

source_location get_source_location_from_token(tokenizer *tokenizer, u32 token_idx) {
    source_location location;
    token tok = tokenizer->tokens[token_idx];
    line l = get_token_line_number(tokenizer, tok);
    location.l0 = l.line_number;
    location.l1 = l.line_number + get_token_len(tokenizer, tok);
    location.c0 = tokenizer->current_column_number;
    location.c1 = 0;
    location.start_of_line = l.start_of_line;
    return location;
}

void tokenize(tokenizer *tokenizer) {
    while((tokenizer->at < tokenizer->end)) {
        eat_all_whitespaces(tokenizer);
        switch(eat_char(tokenizer)) {
            case '\0':
                // if we need to peek ahead more than one token in
                // the future, then we can put in a second end of
                // stream token so that peak_next_next token is
                // guarunteed to return the sentinel instead of going
                // past the end since peek_token() and friends do no bounds
                // checking for performance reasons.
                make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_END_OF_STREAM);
                break;
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '#':
            case '~':
            case ';':
            case ',':
                make_token(tokenizer, tokenizer->at - 1, *(tokenizer->at - 1));
                break;
            case '\'': {
                // this is where we detect malformed char literals.
                // if a char literal is malformed, we make an error token.
                b32 error = false;
                u8 *checkpoint = tokenizer->at;
                if(match(tokenizer, '\\')) {
                    if(is_printable(peek_char(tokenizer))) {
                        eat_char(tokenizer);
                        if(!match(tokenizer, '\'')) {
                            tok_report_error(tokenizer, "Error: missing closing quote to terminate char literal");
                            error = true;
                        }
                    }
                } else if(is_printable(peek_char(tokenizer)) && peek_char(tokenizer) != '\'') {
                    eat_char(tokenizer);
                    if(!match(tokenizer, '\'')) {
                        tok_report_error(tokenizer, "Error: missing closing quote to terminate char literal");
                        error = true;
                    }
                } else {
                    tok_report_error(tokenizer, "Error: missing closing quote to terminate char literal");
                        error = true;
                }
                // char literal tokens start AFTER the first quote. We can bring back the quote
                // to print this token if we need to, but as far as the parser and type checker
                // are concerned, we don't need the quotes.
                make_token(tokenizer, checkpoint, error ? TOKEN_KIND_ERROR : TOKEN_KIND_CHAR_LITERAL);
                break;
            }
            case '"': /* TODO: Handle escaping inside of string literals */
                // this is where we detect malformed string literals.
                // if a string literal is malformed, we make an error token.
                u8 *string_start = tokenizer->at;
                b32 error = false;

                while(tokenizer->at < tokenizer->end && *tokenizer->at != '"') {
                    eat_char(tokenizer);
                }
                if(tokenizer->at >= tokenizer->end) {
                    tok_report_error(tokenizer, "Error: unterminated string literal\n");
                    error = true;
                }

                eat_char(tokenizer);
                // string literal tokens start AFTER the first quote. We can bring back the quote
                // to print this token if we need to, but as far as the parser and type checker
                // are concerned, we don't need the quotes.
                make_token(tokenizer, string_start, error ? TOKEN_KIND_ERROR : TOKEN_KIND_STRING_LITERAL);
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                u8 *start = tokenizer->at - 1;  // the digit already eaten by the switch
                b32 found_decimal = false;
                b32 is_float = false;
                while(tokenizer->at < tokenizer->end) {
                    u8 ch = peek_char(tokenizer);
                    if(ch == '.' && !found_decimal) {
                        // Don't consume the dot if the next char is also a dot (range operator)
                        if(peek_next_char(tokenizer) == '.') {
                            break;
                        }
                        found_decimal = true;
                        is_float = true;
                    } else if(ch == '.' && found_decimal) {
                        break;
                    } else if(!is_number(ch)) {
                        break;
                    }
                    eat_char(tokenizer);
                }
                if(is_float) {
                    make_token(tokenizer, start, TOKEN_KIND_FLOAT_LITERAL);
                } else {
                    make_token(tokenizer, start, TOKEN_KIND_INT_LITERAL);
                }
                break;
            }
            case '>':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at -1, TOKEN_KIND_GREATER_THAN_EQUAL);
                } else if (match(tokenizer, '>')) {
                    if(match(tokenizer, '=')) {
                        make_token(tokenizer, tokenizer->at -1, TOKEN_KIND_RIGHT_SHIFT_EQUAL);
                    } else {
                        make_token(tokenizer, tokenizer->at -1, TOKEN_KIND_RIGHT_SHIFT);
                    }
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '>');
                }
                break;
            case '<':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_LESS_THAN_EQUAL);
                } else if (match(tokenizer, '<')) {
                    if(match(tokenizer, '=')) {
                        make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_LEFT_SHIFT_EQUAL);
                    } else {
                        make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_LEFT_SHIFT);
                    }
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '<');
                }
                break;
            case '!':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_NOT_EQUAL);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '!');
                }
                break;
            case '=':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_EQUAL_EQUAL);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '=');
                }
                break;
            case '|':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_OR_EQUAL);
                } else if(match(tokenizer, '|')) {
                    make_token(tokenizer, tokenizer->at -1, TOKEN_KIND_OR_OR);
                } else {
                    make_token(tokenizer, tokenizer->at -1, '|');
                }
                break;
            case '^':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_XOR_EQUAL);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '^');
                }
                break;
            case '&':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_AND_EQUAL);
                } else if(match(tokenizer, '&')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_AND_AND);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '&');
                }
                break;
            case '+':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_PLUS_EQUAL);
                } else if(match(tokenizer, '+')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_PLUS_PLUS);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '+');
                }
                break;
            case '-':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_MINUS_EQUAL);
                } else if(match(tokenizer, '>')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_RIGHT_ARROW);
                } else if(match(tokenizer, '-')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_MINUS_MINUS);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '-');
                }
                break;
            case '/':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_DIVIDE_EQUAL);
                } else if(match(tokenizer, '/')) {
                    while(1) {
                        u8 ch = eat_char(tokenizer);
                        if(ch == '\n' || ch == 0) {
                            break;
                        }
                    }
                } else if(match(tokenizer, '*')) {
                    while(1) {
                        u8 ch = eat_char(tokenizer);
                        if(ch == '*') {
                            if(match(tokenizer, '/')) {
                                break;
                            }
                        } else if(ch == 0) {
                            break;
                        }
                    }
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '/');
                }
                break;
            case '%':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_MOD_EQUAL);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '%');
                }
                break;
            case '*':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_MULTIPLY_EQUAL);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '*');
                }
                break;
            case ':':
                if(match(tokenizer, ':')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_COLON_COLON);
                } else if(match(tokenizer, '=')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_COLON_EQUAL);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, ':');
                }
                break;
            case '.':
                if(match(tokenizer, '.')) {
                    make_token(tokenizer, tokenizer->at - 1, TOKEN_KIND_DOT_DOT);
                } else {
                    make_token(tokenizer, tokenizer->at - 1, '.');
                }
                break;
            default: {
                u8 *ident_start = tokenizer->at - 1;
                if(is_alpha(*ident_start)) { /* we found a potential ident */
                    i32 ident_len = 1;
                    while(tokenizer->at < tokenizer->end) {
                        u8 ch = peek_char(tokenizer);
                        if(!(is_alphanumeric(ch) || ch == '_')) {
                            break;
                        }
                        ident_len++;
                        eat_char(tokenizer);
                    }
                    string8 tok = {.data = ident_start, .length = ident_len};
                    switch(ident_len) {
                        case 2:
                            if(str_match(tok, s("if"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IF);
                            } else {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            }
                            break;
                        case 3:
                            if(str_match(tok, s("for"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_FOR);
                            } else {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            }
                            break;
                        case 4:
                            if(str_match(tok, s("else"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_ELSE);
                            } else if(str_match(tok, s("enum"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_ENUM);
                            } else if(str_match(tok, s("true"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_BOOL_LITERAL);
                            } else {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            }
                            break;
                        case 5:
                            if(str_match(tok, s("false"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_BOOL_LITERAL);
                            } else if(str_match(tok, s("break"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_BREAK);
                            } else if(str_match(tok, s("union"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_UNION);
                            } else if(str_match(tok, s("while"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_WHILE);
                            } else {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            }
                            break;
                        case 6:
                            if(str_match(tok, s("struct"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_STRUCT);
                            } else if(str_match(tok, s("return"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_RETURN);
                            } else {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            }
                            break;
                        case 8:
                            if(str_match(tok, s("continue"))) {
                                make_token(tokenizer, ident_start, TOKEN_KIND_CONTINUE);
                            } else {
                                make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            }
                            break;
                        default:
                            make_token(tokenizer, ident_start, TOKEN_KIND_IDENTIFIER);
                            break;
                    }
                } else {
                    tok_report_error(tokenizer, "Error: Unexpected character:\n");
                }
                break;
            }
        }
    }
}
