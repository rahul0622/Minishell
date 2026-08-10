#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

/*----------------------------------------------------------
 * C Keywords
 *----------------------------------------------------------*/

static const char *keywords[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum",
                                "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return",
                                "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void",
                                "volatile", "while", "_Bool", "_Complex", "_Imaginary"};

#define KEYWORD_COUNT (sizeof(keywords) / sizeof(keywords[0]))

/*----------------------------------------------------------
 * Global lexer data
 *----------------------------------------------------------*/

static FILE *source_file = NULL;
static int current_line = 1;
static TokenStats stats = {0};

/*----------------------------------------------------------
 * Helper: Check whether string is a keyword
 *----------------------------------------------------------*/

static int is_keyword(const char *word)
{
    int i;

    for(i = 0; i < KEYWORD_COUNT; i++)
    {
        if(strcmp(word, keywords[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

/*----------------------------------------------------------
 * Helper: Add character to lexeme safely
 *----------------------------------------------------------*/

static void append_char(char *lexeme, int *index, int ch)
{
    if (*index < MAX_LEXEME - 1)
    {
        lexeme[*index] = (char)ch;
        (*index)++;
        lexeme[*index] = '\0';
    }
}

/*----------------------------------------------------------
 * Initialize lexer
 *----------------------------------------------------------*/

void lexer_init(FILE *fp)
{
    source_file = fp;
    current_line = 1;

    memset(&stats, 0, sizeof(stats));
}

/*----------------------------------------------------------
 * Token type to string
 *----------------------------------------------------------*/

const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
        case TOKEN_KEYWORD:
            return "KEYWORD";

        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";

        case TOKEN_INTEGER:
            return "INTEGER";

        case TOKEN_FLOAT:
            return "FLOAT";

        case TOKEN_STRING:
            return "STRING";

        case TOKEN_CHARACTER:
            return "CHARACTER";

        case TOKEN_OPERATOR:
            return "OPERATOR";

        case TOKEN_SPECIAL_SYMBOL:
            return "SPECIAL_SYMBOL";

        case TOKEN_PREPROCESSOR:
            return "PREPROCESSOR";

        case TOKEN_UNKNOWN:
            return "UNKNOWN";

        case TOKEN_ERROR:
            return "ERROR";

        case TOKEN_EOF:
            return "EOF";

        default:
            return "UNKNOWN";
    }
}

/*----------------------------------------------------------
 * Read identifier / keyword
 *----------------------------------------------------------*/

static Token read_identifier(int first_char)
{
    Token token;
    int ch;
    int index = 0;

    token.line = current_line;

    append_char(token.lexeme, &index, first_char);

    while ((ch = fgetc(source_file)) != EOF)
    {
        if (isalnum(ch) || ch == '_')
        {
            append_char(token.lexeme, &index, ch);
        }
        else
        {
            ungetc(ch, source_file);
            break;
        }
    }

    if (is_keyword(token.lexeme))
    {
        token.type = TOKEN_KEYWORD;
        stats.keywords++;
    }
    else
    {
        token.type = TOKEN_IDENTIFIER;
        stats.identifiers++;
    }

    return token;
}

/*----------------------------------------------------------
 * Read number
 *
 * Handles:
 * 10
 * 1234
 * 10.25
 * .25
 * 10e5
 * 10.25e-2
 * 0xFF
 * 0x1A
 *----------------------------------------------------------*/

static Token read_number(int first_char)
{
    Token token;
    int ch;
    int index = 0;
    int is_float = 0;

    token.line = current_line;

    append_char(token.lexeme, &index, first_char);

    /*
     * Hexadecimal integer
     */
    if (first_char == '0')
    {
        ch = fgetc(source_file);

        if (ch == 'x' || ch == 'X')
        {
            append_char(token.lexeme, &index, ch);

            while ((ch = fgetc(source_file)) != EOF)
            {
                if (isxdigit(ch))
                {
                    append_char(token.lexeme, &index, ch);
                }
                else
                {
                    ungetc(ch, source_file);
                    break;
                }
            }

            token.type = TOKEN_INTEGER;
            stats.integers++;

            return token;
        }

        if (ch != EOF)
        {
            ungetc(ch, source_file);
        }
    }

    /*
     * Decimal / floating point
     */
    while ((ch = fgetc(source_file)) != EOF)
    {
        if (isdigit(ch))
        {
            append_char(token.lexeme, &index, ch);
        }
        else
        {
            ungetc(ch, source_file);
            break;
        }
    }

    ch = fgetc(source_file);

    if (ch == '.')
    {
        is_float = 1;
        append_char(token.lexeme, &index, ch);

        while ((ch = fgetc(source_file)) != EOF)
        {
            if (isdigit(ch))
            {
                append_char(token.lexeme, &index, ch);
            }
            else
            {
                ungetc(ch, source_file);
                break;
            }
        }
    }
    else if (ch != EOF)
    {
        ungetc(ch, source_file);
    }

    /*
     * Exponent
     */
    ch = fgetc(source_file);

    if (ch == 'e' || ch == 'E')
    {
        int next;

        is_float = 1;
        append_char(token.lexeme, &index, ch);

        next = fgetc(source_file);

        if (next == '+' || next == '-')
        {
            append_char(token.lexeme, &index, next);
            next = fgetc(source_file);
        }

        if (!isdigit(next))
        {
            token.type = TOKEN_ERROR;
            stats.errors++;

            if (next != EOF)
            {
                ungetc(next, source_file);
            }

            return token;
        }

        append_char(token.lexeme, &index, next);

        while ((ch = fgetc(source_file)) != EOF)
        {
            if (isdigit(ch))
            {
                append_char(token.lexeme, &index, ch);
            }
            else
            {
                ungetc(ch, source_file);
                break;
            }
        }
    }
    else if (ch != EOF)
    {
        ungetc(ch, source_file);
    }

    if (is_float)
    {
        token.type = TOKEN_FLOAT;
        stats.floats++;
    }
    else
    {
        token.type = TOKEN_INTEGER;
        stats.integers++;
    }

    return token;
}

/*----------------------------------------------------------
 * Read string literal
 *----------------------------------------------------------*/

static Token read_string(void)
{
    Token token;
    int ch;
    int index = 0;
    int start_line = current_line;

    token.line = start_line;

    append_char(token.lexeme, &index, '"');

    while ((ch = fgetc(source_file)) != EOF)
    {
        append_char(token.lexeme, &index, ch);

        if (ch == '\\')
        {
            /*
             * Escaped character
             */
            ch = fgetc(source_file);

            if (ch == EOF)
            {
                break;
            }

            append_char(token.lexeme, &index, ch);
        }
        else if (ch == '"')
        {
            token.type = TOKEN_STRING;
            stats.strings++;

            return token;
        }
        else if (ch == '\n')
        {
            current_line++;

            token.type = TOKEN_ERROR;
            stats.errors++;

            return token;
        }
    }

    token.type = TOKEN_ERROR;
    stats.errors++;

    return token;
}

/*----------------------------------------------------------
 * Read character constant
 *----------------------------------------------------------*/

static Token read_character(void)
{
    Token token;
    int ch;
    int index = 0;
    int start_line = current_line;

    token.line = start_line;

    append_char(token.lexeme, &index, '\'');

    while ((ch = fgetc(source_file)) != EOF)
    {
        append_char(token.lexeme, &index, ch);

        if (ch == '\\')
        {
            ch = fgetc(source_file);

            if (ch == EOF)
            {
                break;
            }

            append_char(token.lexeme, &index, ch);
        }
        else if (ch == '\'')
        {
            token.type = TOKEN_CHARACTER;
            stats.characters++;

            return token;
        }
        else if (ch == '\n')
        {
            current_line++;

            token.type = TOKEN_ERROR;
            stats.errors++;

            return token;
        }
    }

    token.type = TOKEN_ERROR;
    stats.errors++;

    return token;
}

/*----------------------------------------------------------
 * Read preprocessor directive
 *
 * Example:
 *
 * #include <stdio.h>
 * #define MAX 100
 *----------------------------------------------------------*/

static Token read_preprocessor(void)
{
    Token token;
    int ch;
    int index = 0;

    token.type = TOKEN_PREPROCESSOR;
    token.line = current_line;

    append_char(token.lexeme, &index, '#');

    while ((ch = fgetc(source_file)) != EOF)
    {
        if (ch == '\n')
        {
            current_line++;
            break;
        }

        append_char(token.lexeme, &index, ch);
    }

    stats.preprocessors++;

    return token;
}

/*----------------------------------------------------------
 * Read operator
 *----------------------------------------------------------*/

static Token read_operator(int first_char)
{
    Token token;
    int ch;
    int index = 0;

    token.type = TOKEN_OPERATOR;
    token.line = current_line;

    append_char(token.lexeme, &index, first_char);

    ch = fgetc(source_file);

    switch (first_char)
    {
        case '+':
            if (ch == '+' || ch == '=')
            {
                append_char(token.lexeme, &index, ch);
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, source_file);
            }
            break;

        case '-':
            if (ch == '-' || ch == '=' || ch == '>')
            {
                append_char(token.lexeme, &index, ch);
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, source_file);
            }
            break;

        case '*':
        case '%':
        case '^':
        case '!':
        case '=':
        case '<':
        case '>':
        case '&':
        case '|':
            if (ch == '=')
            {
                append_char(token.lexeme, &index, ch);
            }
            else if (first_char == '!' && ch == '=')
            {
                append_char(token.lexeme, &index, ch);
            }
            else if (first_char == '=' && ch == '=')
            {
                append_char(token.lexeme, &index, ch);
            }
            else if (first_char == '<' && ch == '<')
            {
                append_char(token.lexeme, &index, ch);

                ch = fgetc(source_file);

                if (ch == '=')
                {
                    append_char(token.lexeme, &index, ch);
                }
                else if (ch != EOF)
                {
                    ungetc(ch, source_file);
                }
            }
            else if (first_char == '>' && ch == '>')
            {
                append_char(token.lexeme, &index, ch);

                ch = fgetc(source_file);

                if (ch == '=')
                {
                    append_char(token.lexeme, &index, ch);
                }
                else if (ch != EOF)
                {
                    ungetc(ch, source_file);
                }
            }
            else if (first_char == '&' && ch == '&')
            {
                append_char(token.lexeme, &index, ch);
            }
            else if (first_char == '|' && ch == '|')
            {
                append_char(token.lexeme, &index, ch);
            }
            else if (ch != EOF)
            {
                ungetc(ch, source_file);
            }
            break;

        case '/':
            if (ch == '=')
            {
                append_char(token.lexeme, &index, ch);
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, source_file);
            }
            break;

        default:
            if (ch != EOF)
                ungetc(ch, source_file);
            break;
    }

    stats.operators++;

    return token;
}

/*----------------------------------------------------------
 * Check if character is an operator
 *----------------------------------------------------------*/

static int is_operator_start(int ch)
{
    switch (ch)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '=':
        case '!':
        case '<':
        case '>':
        case '&':
        case '|':
        case '^':
        case '~':
            return 1;

        default:
            return 0;
    }
}

/*----------------------------------------------------------
 * Check special symbol
 *----------------------------------------------------------*/

static int is_special_symbol(int ch)
{
    switch (ch)
    {
        case ';':
        case ',':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ':':
        case '?':
        case '.':
        case '\\':
            return 1;

        default:
            return 0;
    }
}

/*----------------------------------------------------------
 * Get next token
 *----------------------------------------------------------*/

Token get_next_token(void)
{
    Token token;
    int ch;
    int next;

    memset(&token, 0, sizeof(token));

    while ((ch = fgetc(source_file)) != EOF)
    {
        /*
         * Ignore spaces
         */
        if (ch == ' ' || ch == '\t' || ch == '\r')
        {
            continue;
        }

        /*
         * New line
         */
        if (ch == '\n')
        {
            current_line++;
            continue;
        }

        /*
         * Preprocessor
         */
        if (ch == '#')
        {
            return read_preprocessor();
        }

        /*
         * Identifier / keyword
         */
        if (isalpha(ch) || ch == '_')
        {
            return read_identifier(ch);
        }

        /*
         * Number
         */
        if (isdigit(ch))
        {
            return read_number(ch);
        }

        /*
         * Floating point starting with '.'
         *
         * Example:
         * .25
         */
        if (ch == '.')
        {
            next = fgetc(source_file);

            if (isdigit(next))
            {
                ungetc(next, source_file);
                return read_number(ch);
            }

            if (next != EOF)
            {
                ungetc(next, source_file);
            }

            token.type = TOKEN_SPECIAL_SYMBOL;
            token.lexeme[0] = '.';
            token.lexeme[1] = '\0';
            token.line = current_line;

            stats.special_symbols++;

            return token;
        }

        /*
         * String
         */
        if (ch == '"')
        {
            return read_string();
        }

        /*
         * Character
         */
        if (ch == '\'')
        {
            return read_character();
        }

        /*
         * Comments
         */
        if (ch == '/')
        {
            next = fgetc(source_file);

            /*
             * Single line comment
             */
            if (next == '/')
            {
                while ((ch = fgetc(source_file)) != EOF)
                {
                    if (ch == '\n')
                    {
                        current_line++;
                        break;
                    }
                }

                continue;
            }

            /*
             * Multi-line comment
             */
            if (next == '*')
            {
                int previous = 0;
                int closed = 0;

                while ((ch = fgetc(source_file)) != EOF)
                {
                    if (ch == '\n')
                    {
                        current_line++;
                    }

                    if (previous == '*' && ch == '/')
                    {
                        closed = 1;
                        break;
                    }

                    previous = ch;
                }

                if (!closed)
                {
                    token.type = TOKEN_ERROR;
                    strcpy(token.lexeme, "Unterminated comment");
                    token.line = current_line;

                    stats.errors++;

                    return token;
                }

                continue;
            }

            /*
             * It is division operator
             */
            if (next != EOF)
            {
                ungetc(next, source_file);
            }

            return read_operator('/');
        }

        /*
         * Operators
         */
        if (is_operator_start(ch))
        {
            return read_operator(ch);
        }

        /*
         * Special symbols
         */
        if (is_special_symbol(ch))
        {
            token.type = TOKEN_SPECIAL_SYMBOL;
            token.lexeme[0] = (char)ch;
            token.lexeme[1] = '\0';
            token.line = current_line;

            stats.special_symbols++;

            return token;
        }

        /*
         * Unknown character
         */
        token.type = TOKEN_UNKNOWN;
        token.lexeme[0] = (char)ch;
        token.lexeme[1] = '\0';
        token.line = current_line;

        stats.errors++;

        return token;
    }

    token.type = TOKEN_EOF;
    strcpy(token.lexeme, "EOF");
    token.line = current_line;

    return token;
}

/*----------------------------------------------------------
 * Print statistics
 *----------------------------------------------------------*/

void print_statistics(void)
{
    printf("\n");
    printf("========================================\n");
    printf("           TOKEN STATISTICS\n");
    printf("========================================\n");

    printf("Keywords          : %d\n", stats.keywords);
    printf("Identifiers       : %d\n", stats.identifiers);
    printf("Integer constants : %d\n", stats.integers);
    printf("Float constants   : %d\n", stats.floats);
    printf("String literals   : %d\n", stats.strings);
    printf("Character const.  : %d\n", stats.characters);
    printf("Operators         : %d\n", stats.operators);
    printf("Special symbols   : %d\n", stats.special_symbols);
    printf("Preprocessor      : %d\n", stats.preprocessors);
    printf("Errors            : %d\n", stats.errors);

    printf("========================================\n");
}