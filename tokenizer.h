#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "ds.h"
#define token_buf_size 512

typedef enum token_kind {
    // The token kind for single-character tokens is just the ascii value of that token.
    // because of that, we don't have to define those tokens here.
    TOKEN_KIND_IDENTIFIER = 256,

    TOKEN_KIND_PLUS_EQUAL = 257, /* += */
    TOKEN_KIND_MINUS_EQUAL,
    TOKEN_KIND_DIVIDE_EQUAL,
    TOKEN_KIND_MOD_EQUAL,
    TOKEN_KIND_MULTIPLY_EQUAL,

    TOKEN_KIND_XOR_EQUAL, /* ^= */
    TOKEN_KIND_AND_EQUAL,
    TOKEN_KIND_OR_EQUAL,

    TOKEN_KIND_EQUAL_EQUAL,
    TOKEN_KIND_NOT_EQUAL,

    TOKEN_KIND_COLON_EQUAL,
    TOKEN_KIND_DOT_DOT,

    TOKEN_KIND_LEFT_SHIFT_EQUAL,
    TOKEN_KIND_RIGHT_SHIFT_EQUAL,
    TOKEN_KIND_GREATER_THAN_EQUAL,
    TOKEN_KIND_LESS_THAN_EQUAL,

    TOKEN_KIND_RIGHT_SHIFT,
    TOKEN_KIND_LEFT_SHIFT,

    TOKEN_KIND_COLON_COLON,
    TOKEN_KIND_RIGHT_ARROW,

    TOKEN_KIND_AND_AND,
    TOKEN_KIND_OR_OR,

    TOKEN_KIND_PLUS_PLUS,
    TOKEN_KIND_MINUS_MINUS,
    TOKEN_KIND_INT_LITERAL,
    TOKEN_KIND_FLOAT_LITERAL,
    TOKEN_KIND_CHAR_LITERAL,
    TOKEN_KIND_BOOL_LITERAL,
    TOKEN_KIND_STRING_LITERAL,
    TOKEN_KIND_END_OF_STREAM,

    TOKEN_KIND_IF,
    TOKEN_KIND_ELSE,
    TOKEN_KIND_WHILE,
    TOKEN_KIND_FOR,
    TOKEN_KIND_UNION,
    TOKEN_KIND_STRUCT,
    TOKEN_KIND_ENUM,
    TOKEN_KIND_CONTINUE,
    TOKEN_KIND_BREAK,
    TOKEN_KIND_RETURN,
    TOKEN_KIND_ERROR,
}token_kind;

typedef struct source_location source_location;
struct source_location {
    u32 l0, c0;
    u32 l1, c1;
    u8 *start_of_line;
};

typedef struct token token;
struct token {
    u32 pos; // position of the start of the token in the file.
    token_kind kind;
};

typedef struct tokenizer tokenizer;
struct tokenizer {
    u8 *file;
    u8 *at;
    u8 *end;
    token *tokens;
    u32 current_token;
    u32 token_count;

    u32 current_line_number;
    u32 current_column_number;
    u8 *start_of_current_line;
    u8 *file_path;
    u32 error_count;
};

void tokenize(tokenizer *tokenizer);
string8 token_to_string(tokenizer *tokenizer, token tok);
void tok_report_error(tokenizer *tokenizer, const char *fmt, ...);
u32 get_token_len(tokenizer *tokenizer, token tok);

// NOTE: These functions are not to be called inside the lexer. They are meant to
// be used in the parser and/or typechecking phases.
source_location get_source_location_from_token(tokenizer *tokenizer, u32 token_idx);
u8 get_char_value_from_token(tokenizer *tokenizer, token tok);
i64 get_int_value_from_token(tokenizer *tokenizer, token tok);
f64 get_float_value_from_token(tokenizer *tokenizer, token tok);
b32 get_bool_value_from_token(tokenizer *tokenizer, token tok);
string8 get_ident_or_string_literal_from_token(tokenizer *tokenizer, token tok);



u32 get_identifier_len(tokenizer *tokenizer, token tok);
u32 get_string_literal_len(tokenizer *tokenizer, token tok);
u32 get_int_literal_len(tokenizer *tokenizer, token tok);
u32 get_float_literal_len(tokenizer *tokenizer, token tok);
u32 get_char_literal_len(tokenizer *tokenizer, token tok);
u32 get_bool_literal_len(tokenizer *tokenizer, token tok);

#endif /* TOKENIZER_H */
