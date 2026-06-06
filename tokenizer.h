#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "ds.h"

typedef enum token_kind {
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
    TOKEN_KIND_FALSE,
    TOKEN_KIND_TRUE,
}token_kind;

typedef struct token token;
struct token {
    token_kind kind;

    i32 l0, c0;
    i32 l1, c1;

    union {
        b64 bool_value;
        i64 integer_value;
        f64 float_value;
        u8  char_value;
        string8 string_value;
        string8 ident_string;
    };
};

typedef struct tokenizer tokenizer;
struct tokenizer {
    u8 *at;
    u8 *end;
    token *tokens;
    size_t token_count;
};

typedef struct token_stream token_stream;
struct token_stream {
    token *at;
    token *end;
};

token_stream tokenize(tokenizer *tokenizer);

#endif /* TOKENIZER_H */
