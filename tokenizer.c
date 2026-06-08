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

void eat_all_whitespaces(tokenizer *tokenizer) {
    while(tokenizer->at != tokenizer->end && is_whitespace(*tokenizer->at)) {
        tokenizer->at++;
    }
}

u8 eat_char(tokenizer *tokenizer) {
    u8 current_char = *tokenizer->at;
    if(tokenizer->at + 1 <= tokenizer->end) {
        tokenizer->at += 1;
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

void make_token(tokenizer *tokenizer, token_kind kind) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = kind;
}

void make_char_token(tokenizer *tokenizer, u8 *char_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_CHAR_LITERAL;
    if(token_len == 0) {
        tok->char_value = 0;
    } else if(token_len == 1) {
        tok->char_value = char_start[0];
    } else if(token_len == 2) {
        if(char_start[0] != '\\') {
            fatal_error("Error: Expected backslash in multi-character char literal");
        }
        switch(char_start[1]) {
            case 'a':
                tok->char_value = '\a';
                break;
            case 'b':
                tok->char_value = '\b';
                break;
            case 'f':
                tok->char_value = '\f';
                break;
            case 'n':
                tok->char_value = '\n';
                break;
            case 'r':
                tok->char_value = '\r';
                break;
            case 't':
                tok->char_value = '\t';
                break;
            case 'v':
                tok->char_value = '\v';
                break;
            case '\'':
                tok->char_value = '\'';
                break;
            case '\\':
                tok->char_value = '\\';
                break;
            case '0':
                tok->char_value = '\0';
                break;
            default:
                fatal_error("Error: unsupported escape character in char literal");
                break;
        }
    }
}

void make_ident(tokenizer *tokenizer, u8 *ident_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_IDENTIFIER;
    tok->ident_string.data = ident_start;
    tok->ident_string.length = token_len;
}

void make_string_token(tokenizer *tokenizer, u8 *string_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_STRING_LITERAL;
    tok->string_value.data = string_start;
    tok->string_value.length = token_len;
}

void make_int_token(tokenizer *tokenizer, u8 *int_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_INT_LITERAL;
    string8 str = {.data = int_start, .length = token_len};
    tok->integer_value = str_to_i64(str);
}

void make_float_token(tokenizer *tokenizer, u8 *float_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_FLOAT_LITERAL;
    string8 str = {.data = float_start, .length = token_len};
    tok->float_value = str_to_f64(str);
}

void make_bool_token(tokenizer *tokenizer, b32 true_or_false) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_BOOL_LITERAL;
    tok->bool_value = true_or_false;
}

token_stream tokenize(tokenizer *tokenizer) {
    u8 ch = 0;

    while(tokenizer->at < tokenizer->end) {
        eat_all_whitespaces(tokenizer);
        switch(eat_char(tokenizer)) {
            case '\0':
                // if we need to peek ahead more than one token in
                // the future, then we can put in a second end of
                // stream token so that peak_next_next token is
                // guarunteed to return the sentinel instead of going
                // past the end since peek_token() and friends do no bounds
                // checking for performance reasons.
                make_token(tokenizer, TOKEN_KIND_END_OF_STREAM);
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
                make_token(tokenizer, *(tokenizer->at - 1));
                break;
            case '\'': {
                u8 literal_length = 0;
                u8 *char_start = tokenizer->at;
                if(match(tokenizer, '\\')) {
                    if(is_printable(peek_char(tokenizer))) {
                        eat_char(tokenizer);
                        if(match(tokenizer, '\'')) {
                            literal_length = 2;
                        } else {
                            fatal_error("Error: missing closing quote to terminate char literal");
                        }
                    }
                } else if(is_printable(peek_char(tokenizer)) && peek_char(tokenizer) != '\'') {
                    eat_char(tokenizer);
                    if(match(tokenizer, '\'')) {
                        literal_length = 1;
                    } else {
                        fatal_error("Error: missing closing quote to terminate char literal");
                    }
                } else if(match(tokenizer, '\'')) {
                    literal_length = 0;
                } else {
                    fatal_error("Error: missing closing quote to terminate char literal");
                }
                make_char_token(tokenizer, char_start, literal_length);
                break;
            }
            case '"': /* TODO: Handle escaping inside of string literals */
                u8 *string_start = tokenizer->at;
                i32 string_len = 0;

                while(tokenizer->at < tokenizer->end && *tokenizer->at != '"') {
                    string_len++;
                    eat_char(tokenizer);
                }
                if(tokenizer->at >= tokenizer->end) {
                    fatal_error("Error: unterminated string literal");
                }

                eat_char(tokenizer);
                make_string_token(tokenizer, string_start, string_len);
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
                    ch = peek_char(tokenizer);
                    if(ch == '.' && !found_decimal) {
                        found_decimal = true;
                        is_float = true;
                    } else if(ch == '.' && found_decimal) {
                        break;
                    } else if(!is_number(ch)) {
                        break;
                    }
                    eat_char(tokenizer);
                }
                u64 num_len = tokenizer->at - start;
                if(is_float) {
                    make_float_token(tokenizer, start, num_len);
                } else {
                    make_int_token(tokenizer, start, num_len);
                }
                break;
            }
            case '>':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_GREATER_THAN_EQUAL);
                } else if (match(tokenizer, '>')) {
                    if(match(tokenizer, '=')) {
                        make_token(tokenizer, TOKEN_KIND_RIGHT_SHIFT_EQUAL);
                    } else {
                        make_token(tokenizer, TOKEN_KIND_RIGHT_SHIFT);
                    }
                } else {
                    make_token(tokenizer, '>');
                }
                break;
            case '<':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_LESS_THAN_EQUAL);
                } else if (match(tokenizer, '<')) {
                    if(match(tokenizer, '=')) {
                        make_token(tokenizer, TOKEN_KIND_LEFT_SHIFT_EQUAL);
                    } else {
                        make_token(tokenizer, TOKEN_KIND_LEFT_SHIFT);
                    }
                } else {
                    make_token(tokenizer, '<');
                }
                break;
            case '!':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_NOT_EQUAL);
                } else {
                    make_token(tokenizer, '!');
                }
                break;
            case '=':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_EQUAL_EQUAL);
                } else {
                    make_token(tokenizer, '=');
                }
                break;
            case '|':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_OR_EQUAL);
                } else if(match(tokenizer, '|')) {
                    make_token(tokenizer, TOKEN_KIND_OR_OR);
                } else {
                    make_token(tokenizer, '|');
                }
                break;
            case '^':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_XOR_EQUAL);
                } else {
                    make_token(tokenizer, '^');
                }
                break;
            case '&':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_AND_EQUAL);
                } else if(match(tokenizer, '&')) {
                    make_token(tokenizer, TOKEN_KIND_AND_AND);
                } else {
                    make_token(tokenizer, '&');
                }
                break;
            case '+':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_PLUS_EQUAL);
                } else if(match(tokenizer, '+')) {
                    make_token(tokenizer, TOKEN_KIND_PLUS_PLUS);
                } else {
                    make_token(tokenizer, '+');
                }
                break;
            case '-':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_MINUS_EQUAL);
                } else if(match(tokenizer, '>')) {
                    make_token(tokenizer, TOKEN_KIND_RIGHT_ARROW);
                } else if(match(tokenizer, '-')) {
                    make_token(tokenizer, TOKEN_KIND_MINUS_MINUS);
                } else {
                    make_token(tokenizer, '-');
                }
                break;
            case '/':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_DIVIDE_EQUAL);
                } else if(match(tokenizer, '/')) {
                    while(1) {
                        ch = eat_char(tokenizer);
                        if(ch == '\n' || ch == 0) {
                            break;
                        }
                    }
                } else {
                    make_token(tokenizer, '/');
                }
                break;
            case '%':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_MOD_EQUAL);
                } else {
                    make_token(tokenizer, '%');
                }
                break;
            case '*':
                if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_MULTIPLY_EQUAL);
                } else {
                    make_token(tokenizer, '*');
                }
                break;
            case ':':
                if(match(tokenizer, ':')) {
                    make_token(tokenizer, TOKEN_KIND_COLON_COLON);
                } else if(match(tokenizer, '=')) {
                    make_token(tokenizer, TOKEN_KIND_COLON_EQUAL);
                } else {
                    make_token(tokenizer, ':');
                }
                break;
            case '.':
                if(match(tokenizer, '.')) {
                    make_token(tokenizer, TOKEN_KIND_DOT_DOT);
                } else {
                    make_token(tokenizer, '.');
                }
                break;
            default: {
                u8 *ident_start = tokenizer->at - 1;
                if(is_alpha(*ident_start)) { /* we found a potential ident */
                    i32 ident_len = 1;
                    while(tokenizer->at < tokenizer->end) {
                        ch = peek_char(tokenizer);
                        if(!(is_alphanumeric(ch) || ch == '_')) {
                            break;
                        }
                        ident_len++;
                        eat_char(tokenizer);
                    }
                    string8 tok = {.data = ident_start, .length = ident_len};
                    token_kind kind = TOKEN_KIND_IDENTIFIER;
                    switch(ident_len) {
                        case 2:
                            if(str_are_strings_equal(tok, STR8_LIT("if"))) {
                                kind = TOKEN_KIND_IF;
                            }
                            break;
                        case 3:
                            if(str_are_strings_equal(tok, STR8_LIT("for"))) {
                                kind = TOKEN_KIND_FOR;
                            }
                            break;
                        case 4:
                            if(str_are_strings_equal(tok, STR8_LIT("else"))) {
                                kind = TOKEN_KIND_ELSE;
                            } else if(str_are_strings_equal(tok, STR8_LIT("enum"))) {
                                kind = TOKEN_KIND_ENUM;
                            } else if(str_are_strings_equal(tok, STR8_LIT("true"))) {
                                kind = TOKEN_KIND_TRUE;
                            }
                            break;
                        case 5:
                            if(str_are_strings_equal(tok, STR8_LIT("false"))) {
                                kind = TOKEN_KIND_FALSE;
                            } else if(str_are_strings_equal(tok, STR8_LIT("break"))) {
                                kind = TOKEN_KIND_BREAK;
                            } else if(str_are_strings_equal(tok, STR8_LIT("union"))) {
                                kind = TOKEN_KIND_UNION;
                            } else if(str_are_strings_equal(tok, STR8_LIT("while"))) {
                                kind = TOKEN_KIND_WHILE;
                            }
                            break;
                        case 6:
                            if(str_are_strings_equal(tok, STR8_LIT("struct"))) {
                                kind = TOKEN_KIND_STRUCT;
                            } else if(str_are_strings_equal(tok, STR8_LIT("return"))) {
                                kind = TOKEN_KIND_RETURN;
                            }
                            break;
                        case 8:
                            if(str_are_strings_equal(tok, STR8_LIT("continue"))) {
                                kind = TOKEN_KIND_CONTINUE;
                            }
                            break;
                    }
                    if(kind != TOKEN_KIND_IDENTIFIER) {
                        make_token(tokenizer, kind);
                    } else {
                        make_ident(tokenizer, ident_start, ident_len);
                    }
                } else {
                    printf("Error: Unexpected character:");
                }
                break;
            }
        }
    }

    token_stream stream = {0};
    stream.at = tokenizer->tokens;
    stream.end = tokenizer->tokens + tokenizer->token_count;
    return stream;
}
