#include "tokenizer.h"

b32 is_whitespace(unsigned char ch) {
    return ((ch == ' ')  ||
            (ch == '\t') ||
            (ch == '\r') ||
            (ch == '\n') ||
            (ch == '\v') ||
            (ch == '\f'));
}

b32 is_number(unsigned char ch) {
    return ((ch >= '0') && (ch <= '9'));
}

b32 is_alpha(unsigned char ch) {
    return (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')));
}

b32 is_lower(unsigned char ch) {
    return ((ch >= 'a') && (ch <= 'z'));
}

b32 is_upper(unsigned char ch) {
    return ((ch >= 'A') && (ch <= 'Z'));
}

b32 is_alphanumeric(unsigned char ch) {
    return(is_number(ch) || is_alpha(ch));
}

void eat_all_whitespaces(tokenizer *tokenizer) {
    while(tokenizer->at != tokenizer->end && is_whitespace(*tokenizer->at)) {
        tokenizer->at++;
    }
}

unsigned char eat_char(tokenizer *tokenizer, size_t num_chars_to_eat) {
    unsigned char current_char = *tokenizer->at;
    if(tokenizer->at + num_chars_to_eat <= tokenizer->end) {
        tokenizer->at += num_chars_to_eat;
        return current_char;
    }
    printf("Error: %s(): Tried to eat past end of file. Returning 0.\n", __func__);
    return 0;
}

unsigned char peek_char(const tokenizer *tokenizer, size_t lookahead) {
    if(tokenizer->at + lookahead != tokenizer->end) {
        return *(tokenizer->at + lookahead);
    }
    printf("Error: %s(): Tried to peek past end of file. Returning 0.\n", __func__);
    return 0;
}

void make_token(tokenizer *tokenizer, token_kind kind, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = kind;
    eat_char(tokenizer, token_len);
}

void make_char_token(tokenizer *tokenizer, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_CHAR_LITERAL;
    if(token_len == 0) {
        tok->char_value = 0;
    } else if(token_len == 1) {
        tok->char_value = tokenizer->at[0];
    } else if(token_len == 2) {
        switch(tokenizer->at[1]) {
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
            default:
                fatal_error("Error: unsupported escape character in char literal");
                break;
        }
    }
    eat_char(tokenizer, token_len);
}

void make_ident(tokenizer *tokenizer, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_IDENTIFIER;
    tok->ident_string.data = tokenizer->at;
    tok->ident_string.length = token_len;
    eat_char(tokenizer, token_len);
}

void make_string_token(tokenizer *tokenizer, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_STRING_LITERAL;
    tok->string_value.data = tokenizer->at;
    tok->string_value.length = token_len;
    eat_char(tokenizer, token_len);
}

void make_int_token(tokenizer *tokenizer, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_INT_LITERAL;
    string8 str = {.data = tokenizer->at, .length = token_len};
    tok->integer_value = str_to_i64(str);
    eat_char(tokenizer, token_len);
}

void make_float_token(tokenizer *tokenizer, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_FLOAT_LITERAL;
    string8 str = {.data = tokenizer->at, .length = token_len};
    tok->float_value = str_to_f64(str);
    eat_char(tokenizer, token_len);
}

void make_bool_token(tokenizer *tokenizer, u64 token_len, b32 true_or_false) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_BOOL_LITERAL;
    tok->bool_value = true_or_false;
    eat_char(tokenizer, token_len);
}

token_stream tokenize(tokenizer *tokenizer) {
    unsigned char ch = 0;

    while(tokenizer->at < tokenizer->end) {
        eat_all_whitespaces(tokenizer);
        switch(peek_char(tokenizer, 0)) {
            case 0:
                // if we need to peek ahead more than one token in
                // the future, then we can put in a second end of
                // stream token so that peak_next_next token is
                // guarunteed to return the sentinel instead of going
                // past the end since peek_token() and friends do no bounds
                // checking for performance reasons.
                make_token(tokenizer, TOKEN_KIND_END_OF_STREAM, 0);
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
                make_token(tokenizer, *tokenizer->at, 1);
                break;
            case '\'': {
                u8 literal_length = 0;
                ch = peek_char(tokenizer, 1);
                if(ch == '\\') { // if we get a backslash
                    if(peek_char(tokenizer, 3) == '\'') { // if we get a quote
                        literal_length = 2;
                    } else {
                        fatal_error("Error: character literal contains multiple characters!");
                    }
                } else if (ch == '\'') { // this is meant to handle empty char literals.
                    literal_length = 0;
                } else {
                    if(peek_char(tokenizer, 2) == '\'') {
                        literal_length = 1;
                    } else {
                        fatal_error("Error: character literal contains multiple characters!");
                    }
                }
                eat_char(tokenizer, 1);
                make_char_token(tokenizer, literal_length);
                eat_char(tokenizer, 1);
                break;
            }
            case '"': /* TODO: Handle escaping inside of string literals */
                i32 string_len = 0;
                eat_char(tokenizer, 1);
                while(peek_char(tokenizer, string_len) != '"') {
                    string_len++;
                }
                make_string_token(tokenizer, string_len);
                eat_char(tokenizer, 1);
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
                i32 num_len = 0;
                b32 found_decimal = false;
                b32 is_float = false;
                while(is_number(peek_char(tokenizer, num_len)) || peek_char(tokenizer, num_len) == '.') {
                    ch = peek_char(tokenizer, num_len);
                    if(ch == '.' && found_decimal) {
                        num_len--;
                        is_float = false;
                        break;
                    }
                    if(ch == '.') {
                        found_decimal = true;
                        is_float = true;
                    }
                    num_len++;
                }
                if(is_float) {
                    make_float_token(tokenizer, num_len);
                } else {
                    make_int_token(tokenizer, num_len);
                }
                break;
            }
            case '>':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_GREATER_THAN_EQUAL, 2);
                } else if (ch == '>') {
                    ch = peek_char(tokenizer, 2);
                    if(ch == '=') {
                        make_token(tokenizer, TOKEN_KIND_RIGHT_SHIFT_EQUAL, 3);
                    } else {
                        make_token(tokenizer, TOKEN_KIND_RIGHT_SHIFT, 2);
                    }
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '<':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_LESS_THAN_EQUAL, 2);
                } else if (ch == '<') {
                    ch = peek_char(tokenizer, 2);
                    if(ch == '=') {
                        make_token(tokenizer, TOKEN_KIND_LEFT_SHIFT_EQUAL, 3);
                    } else {
                        make_token(tokenizer, TOKEN_KIND_LEFT_SHIFT, 2);
                    }
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '!':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_NOT_EQUAL, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '=':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_EQUAL_EQUAL, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '|':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_OR_EQUAL, 2);
                } else if(ch == '|') {
                    make_token(tokenizer, TOKEN_KIND_OR_OR, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '^':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_OR_EQUAL, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '&':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_AND_EQUAL, 2);
                } else if(ch == '&') {
                    make_token(tokenizer, TOKEN_KIND_AND_AND, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '+':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_PLUS_EQUAL, 2);
                } else if(ch == '+') {
                    make_token(tokenizer, TOKEN_KIND_PLUS_PLUS, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '-':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_MINUS_EQUAL, 2);
                } else if(ch == '>') {
                    make_token(tokenizer, TOKEN_KIND_RIGHT_ARROW, 2);
                } else if(ch == '-') {
                    make_token(tokenizer, TOKEN_KIND_MINUS_MINUS, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '/':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_DIVIDE_EQUAL, 2);
                } else if(ch == '/') {
                    while(1) {
                        ch = eat_char(tokenizer, 1);
                        if(ch == '\n' || ch == 0) {
                            break;
                        }
                    }
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '%':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_MOD_EQUAL, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '*':
                ch = peek_char(tokenizer, 1);
                if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_MULTIPLY_EQUAL, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case ':':
                ch = peek_char(tokenizer, 1);
                if(ch == ':') {
                    make_token(tokenizer, TOKEN_KIND_COLON_COLON, 2);
                } else if(ch == '=') {
                    make_token(tokenizer, TOKEN_KIND_COLON_EQUAL, 2);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            case '.':
                ch = peek_char(tokenizer, 1);
                if(ch == '.') {
                    make_token(tokenizer, TOKEN_KIND_DOT_DOT, 1);
                } else {
                    make_token(tokenizer, *tokenizer->at, 1);
                }
                break;
            default: {
                if(is_alpha(peek_char(tokenizer, 0))) { /* we found a potential ident */
                    i32 ident_len = 0;
                    ch = peek_char(tokenizer, ident_len);
                    while(!is_whitespace(ch)) {
                        ident_len++;
                        ch = peek_char(tokenizer, ident_len);
                        if(!(is_alphanumeric(ch) || ch == '_')) {
                            break;
                        }
                    }
                    string8 tok = {.data = tokenizer->at, .length = ident_len};
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
                        make_token(tokenizer, kind, ident_len);
                    } else {
                        make_ident(tokenizer, ident_len);
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
