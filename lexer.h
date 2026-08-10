#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_LEXEME 256

typedef enum
{
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHARACTER,
    TOKEN_OPERATOR,
    TOKEN_SPECIAL_SYMBOL,
    TOKEN_PREPROCESSOR,
    TOKEN_UNKNOWN,
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    char lexeme[MAX_LEXEME];
    int line;
} Token;

typedef struct
{
    int keywords;
    int identifiers;
    int integers;
    int floats;
    int strings;
    int characters;
    int operators;
    int special_symbols;
    int preprocessors;
    int errors;
} TokenStats;

void lexer_init(FILE *fp);
Token get_next_token(void);
const char *token_type_to_string(TokenType type);
void print_statistics(void);

#endif