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
    tokenizer->start_of_current_line = tokenizer->at;
    while(1) {
        u8 ch = peek_char(tokenizer);
        if(!is_whitespace(ch)) {
            break;
        }
        eat_char(tokenizer);
        tokenizer->start_of_current_line++;
        if(ch == '\n') {
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

u8 parse_escape_char(u8 *escape_start) {
    if(escape_start[0] != '\\') {
        report_error("Error: Expected backslash in multi-character char literal\n");
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
            report_error("Error: Invalid escape character in char literal.\n");
            return 1;
            break;
    }
}

string8 token_kind_to_string(token_kind kind) {
    switch(kind) {
        case '+':                                return STR8_LIT("+");
        case '-':                                return STR8_LIT("-");
        case '/':                                return STR8_LIT("/");
        case '%':                                return STR8_LIT("%");
        case '*':                                return STR8_LIT("*");
        case '=':                                return STR8_LIT("=");
        case ':':                                return STR8_LIT(":");
        case '>':                                return STR8_LIT(">");
        case '<':                                return STR8_LIT("<");
        case '&':                                return STR8_LIT("&");
        case '^':                                return STR8_LIT("^");
        case '|':                                return STR8_LIT("|");
        case ';':                                return STR8_LIT(";");

        case TOKEN_KIND_PLUS_EQUAL:              return STR8_LIT("+=");
        case TOKEN_KIND_MINUS_EQUAL:             return STR8_LIT("-=");
        case TOKEN_KIND_DIVIDE_EQUAL:            return STR8_LIT("/=");
        case TOKEN_KIND_MOD_EQUAL:               return STR8_LIT("%=");
        case TOKEN_KIND_MULTIPLY_EQUAL:          return STR8_LIT("*=");
        case TOKEN_KIND_XOR_EQUAL:               return STR8_LIT("^=");
        case TOKEN_KIND_AND_EQUAL:               return STR8_LIT("&=");
        case TOKEN_KIND_OR_EQUAL:                return STR8_LIT("|=");
        case TOKEN_KIND_EQUAL_EQUAL:             return STR8_LIT("==");
        case TOKEN_KIND_NOT_EQUAL:               return STR8_LIT("!=");
        case TOKEN_KIND_COLON_EQUAL:             return STR8_LIT(":=");
        case TOKEN_KIND_DOT_DOT:                 return STR8_LIT("..");
        case TOKEN_KIND_LEFT_SHIFT_EQUAL:        return STR8_LIT("<<=");
        case TOKEN_KIND_RIGHT_SHIFT_EQUAL:       return STR8_LIT(">>=");
        case TOKEN_KIND_GREATER_THAN_EQUAL:      return STR8_LIT(">=");
        case TOKEN_KIND_LESS_THAN_EQUAL:         return STR8_LIT("<=");
        case TOKEN_KIND_RIGHT_SHIFT:             return STR8_LIT(">");
        case TOKEN_KIND_LEFT_SHIFT:              return STR8_LIT("<");
        case TOKEN_KIND_COLON_COLON:             return STR8_LIT("::");
        case TOKEN_KIND_RIGHT_ARROW:             return STR8_LIT("->");
        case TOKEN_KIND_AND_AND:                 return STR8_LIT("&&");
        case TOKEN_KIND_OR_OR:                   return STR8_LIT("||");
        case TOKEN_KIND_PLUS_PLUS:               return STR8_LIT("++");
        case TOKEN_KIND_MINUS_MINUS:             return STR8_LIT("--");
        case TOKEN_KIND_END_OF_STREAM:           return STR8_LIT("end_of_stream");
        case TOKEN_KIND_IF:                      return STR8_LIT("if");
        case TOKEN_KIND_ELSE:                    return STR8_LIT("else");
        case TOKEN_KIND_WHILE:                   return STR8_LIT("while");
        case TOKEN_KIND_FOR:                     return STR8_LIT("for");
        case TOKEN_KIND_UNION:                   return STR8_LIT("union");
        case TOKEN_KIND_STRUCT:                  return STR8_LIT("struct");
        case TOKEN_KIND_ENUM:                    return STR8_LIT("enum");
        case TOKEN_KIND_CONTINUE:                return STR8_LIT("continue");
        case TOKEN_KIND_BREAK:                   return STR8_LIT("break");
        case TOKEN_KIND_RETURN:                  return STR8_LIT("return");
        default:                                 return STR8_LIT("");
    }
    return STR8_LIT("");
}

string8 token_to_string(mem_arena *arena, token tok) {
    switch(tok.kind) {
        case TOKEN_KIND_IDENTIFIER:              return tok.ident_string;
        case TOKEN_KIND_INT_LITERAL:             return str_from_i64(arena, tok.integer_value);
        case TOKEN_KIND_FLOAT_LITERAL:           return str_from_f64(arena, tok.float_value);
        case TOKEN_KIND_CHAR_LITERAL:            return str_from_char(arena, tok.char_value);
        case TOKEN_KIND_BOOL_LITERAL:            return tok.bool_value ? STR8_LIT("true") : STR8_LIT("false");
        case TOKEN_KIND_STRING_LITERAL:          return tok.string_value;
        default:                                 return token_kind_to_string(tok.kind);
    }
    return STR8_LIT("");
}

void set_line_and_column_number_on_token(tokenizer *tokenizer, token *tok, u64 token_len) {
    tok->c0 = tokenizer->current_column_number;
    tok->c1 = tok->c0 + token_len;
    tok->l0 = tokenizer->current_line_number;
    tok->l1 = tokenizer->current_line_number;
    tok->start_of_line = tokenizer->start_of_current_line;
}

void make_token(tokenizer *tokenizer, token_kind kind) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = kind;
    // FIX: This is wrong. We should get the length of the token.
    // Maybe make a function that does that?
    u64 wrong_length = 1;
    set_line_and_column_number_on_token(tokenizer, tok, wrong_length);
}

void make_char_token(tokenizer *tokenizer, u8 *char_start, i8 token_len, b32 error) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = error ? TOKEN_KIND_ERROR : TOKEN_KIND_CHAR_LITERAL;
    if(token_len == 0) {
        tok->char_value = 0;
    } else if(token_len == 1) {
        tok->char_value = char_start[0];
    } else if(token_len == 2) {
        tok->char_value = parse_escape_char(char_start);
        if(tok->char_value == 1) {
            tok->kind = TOKEN_KIND_ERROR;
        }
    }

    set_line_and_column_number_on_token(tokenizer, tok, token_len);
}

void make_ident(tokenizer *tokenizer, u8 *ident_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_IDENTIFIER;
    tok->ident_string.data = ident_start;
    tok->ident_string.length = token_len;
    set_line_and_column_number_on_token(tokenizer, tok, token_len);
}

void make_string_token(tokenizer *tokenizer, u8 *string_start, u64 token_len, b32 error) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = error ? TOKEN_KIND_ERROR : TOKEN_KIND_STRING_LITERAL;
    tok->string_value.data = string_start;
    tok->string_value.length = token_len;
    set_line_and_column_number_on_token(tokenizer, tok, token_len);
}

void make_int_token(tokenizer *tokenizer, u8 *int_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_INT_LITERAL;
    string8 str = {.data = int_start, .length = token_len};
    tok->integer_value = str_to_i64(str);
    set_line_and_column_number_on_token(tokenizer, tok, token_len);
}

void make_float_token(tokenizer *tokenizer, u8 *float_start, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_FLOAT_LITERAL;
    string8 str = {.data = float_start, .length = token_len};
    tok->float_value = str_to_f64(str);
    set_line_and_column_number_on_token(tokenizer, tok, token_len);
}

void make_bool_token(tokenizer *tokenizer, b32 true_or_false, u64 token_len) {
    token *tok = &tokenizer->tokens[tokenizer->token_count++];
    tok->kind = TOKEN_KIND_BOOL_LITERAL;
    tok->bool_value = true_or_false;
    set_line_and_column_number_on_token(tokenizer, tok, token_len);
}

token_stream tokenize(tokenizer *tokenizer) {
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
                i8 literal_length = 0;
                b32 error = 0;
                u8 *char_start = tokenizer->at;
                if(match(tokenizer, '\\')) {
                    if(is_printable(peek_char(tokenizer))) {
                        eat_char(tokenizer);
                        if(match(tokenizer, '\'')) {
                            literal_length = 2;
                        } else {
                            report_error("Error: missing closing quote to terminate char literal\n");
                            printf("%.*s\n", (int)tokenizer->current_column_number, tokenizer->at - tokenizer->current_column_number);
                            error = 1;
                        }
                    }
                } else if(is_printable(peek_char(tokenizer)) && peek_char(tokenizer) != '\'') {
                    eat_char(tokenizer);
                    if(match(tokenizer, '\'')) {
                        literal_length = 1;
                    } else {
                        report_error("Error: missing closing quote to terminate char literal\n");
                        printf("%.*s\n", (int)tokenizer->current_column_number, tokenizer->at - tokenizer->current_column_number);
                        error = 1;
                    }
                } else if(match(tokenizer, '\'')) {
                    literal_length = 0;
                } else {
                    report_error("Error: missing closing quote to terminate char literal\n");
                    printf("%.*s\n", (int)tokenizer->current_column_number, tokenizer->at - tokenizer->current_column_number);
                    error = 1;
                }
                make_char_token(tokenizer, char_start, literal_length, error);
                break;
            }
            case '"': /* TODO: Handle escaping inside of string literals */
                u8 *string_start = tokenizer->at;
                u64 string_len = 0;
                b32 error = 0;

                while(tokenizer->at < tokenizer->end && *tokenizer->at != '"') {
                    string_len++;
                    eat_char(tokenizer);
                }
                if(tokenizer->at >= tokenizer->end) {
                    report_error("Error: unterminated string literal\n");
                    error = 1;
                }

                eat_char(tokenizer);
                make_string_token(tokenizer, string_start, string_len, error);
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
                            if(str_are_strings_equal(tok, STR8_LIT("if"))) {
                                make_token(tokenizer, TOKEN_KIND_IF);
                            } else {
                                make_ident(tokenizer, ident_start, ident_len);
                            }
                            break;
                        case 3:
                            if(str_are_strings_equal(tok, STR8_LIT("for"))) {
                                make_token(tokenizer, TOKEN_KIND_FOR);
                            } else {
                                make_ident(tokenizer, ident_start, ident_len);
                            }
                            break;
                        case 4:
                            if(str_are_strings_equal(tok, STR8_LIT("else"))) {
                                make_token(tokenizer, TOKEN_KIND_ELSE);
                            } else if(str_are_strings_equal(tok, STR8_LIT("enum"))) {
                                make_token(tokenizer, TOKEN_KIND_ENUM);
                            } else if(str_are_strings_equal(tok, STR8_LIT("true"))) {
                                make_bool_token(tokenizer, true, ident_len);
                            } else {
                                make_ident(tokenizer, ident_start, ident_len);
                            }
                            break;
                        case 5:
                            if(str_are_strings_equal(tok, STR8_LIT("false"))) {
                                make_bool_token(tokenizer, false, ident_len);
                            } else if(str_are_strings_equal(tok, STR8_LIT("break"))) {
                                make_token(tokenizer, TOKEN_KIND_BREAK);
                            } else if(str_are_strings_equal(tok, STR8_LIT("union"))) {
                                make_token(tokenizer, TOKEN_KIND_UNION);
                            } else if(str_are_strings_equal(tok, STR8_LIT("while"))) {
                                make_token(tokenizer, TOKEN_KIND_WHILE);
                            } else {
                                make_ident(tokenizer, ident_start, ident_len);
                            }
                            break;
                        case 6:
                            if(str_are_strings_equal(tok, STR8_LIT("struct"))) {
                                make_token(tokenizer, TOKEN_KIND_STRUCT);
                            } else if(str_are_strings_equal(tok, STR8_LIT("return"))) {
                                make_token(tokenizer, TOKEN_KIND_RETURN);
                            } else {
                                make_ident(tokenizer, ident_start, ident_len);
                            }
                            break;
                        case 8:
                            if(str_are_strings_equal(tok, STR8_LIT("continue"))) {
                                make_token(tokenizer, TOKEN_KIND_CONTINUE);
                            } else {
                                make_ident(tokenizer, ident_start, ident_len);
                            }
                            break;
                        default:
                            make_ident(tokenizer, ident_start, ident_len);
                            break;
                    }
                } else {
                    report_error("Error: Unexpected character:\n");
                }
                break;
            }
        }
    }

    token_stream stream = {0};
    stream.at = tokenizer->tokens;
    stream.file = tokenizer->file_path;
    return stream;
}
