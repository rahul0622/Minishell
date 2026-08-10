/*
* ===================== * C LEXICAL ANALYZER * ========================== *
    File : main.c  
    Author : Rahul 
    Description : Entry point for the C Lexical Analyzer project.

    ------------------ * Project Description * --------------------------- 
    This project implements a lexical analyzer for C source files.
    The analyzer reads a C source file character by character and identifies 
    different types of tokens such as:
    1. Keywords 2. Identifiers 3. Integer constants 4. Floating-point constants 5. String literals 6. Character constants 
    7. Operators 8. Special symbols 9. Preprocessor directives 10. Lexical errors.
    The analyzer also: 
    - Tracks line numbers - Ignores whitespace - Handles single-line comments - Handles multi-line comments 
    - Recognizes multi-character operators - Detects unterminated strings, character constants and comments
    - Displays token statistics after lexical analysis

    ---------------------------- * Files in the Project * ------------------------
    main.c : Program entry point and command-line argument handling.
    lexer.c : Implementation of lexical analysis and token recognition.
    lexer.h : Token definitions, structures and function declarations.
    sample.c : Sample C source file used as input for testing.
    Makefile : Automates compilation, linking and cleanup.

    ----------------------- * Compilation Using Makefile * -------------------------
    Build the project:
    make  or  make lexical.exe 
    -------------------------------- * Execution * -------------------------------- 
    The lexical analyzer accepts the C source file as a command-line argument.
    ./lexical.exe sample.c 
*/
#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    FILE *fp;
    Token token;

    if(argc != 2)
    {
        printf("Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "r");

    if(fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    lexer_init(fp);

    printf("\n");
    printf("============================================================\n");
    printf("                  C LEXICAL ANALYZER\n");
    printf("============================================================\n");

    printf("%-18s %-30s %-8s\n",
           "TOKEN TYPE", "LEXEME", "LINE");

    printf("------------------------------------------------------------\n");

    while(1)
    {
        token = get_next_token();

        if(token.type == TOKEN_EOF)
        {
            break;
        }

        printf("%-18s %-30s %-8d\n",
               token_type_to_string(token.type),
               token.lexeme,
               token.line);
    }

    printf("------------------------------------------------------------\n");

    fclose(fp);

    print_statistics();

    return 0;
}