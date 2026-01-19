%{
/*
 * NetLang Lexer - Phase 1 (Flex Only)
 * Tokenizes NetLang DSL into categorized token stream
 * 
 * Tokens vs Values:
 * - TOKENS: Fixed language keywords/punctuation (network, Conv2D, {, :, etc.)
 * - VALUES: User-provided content (MNIST, 32, "relu", "weights.bin")
 * 
 * Author: Abhijeet Deb Nath
 * Course: Compiler Design
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Token types for output
typedef enum {
    // Keywords (language tokens)
    TOK_NETWORK, TOK_INPUT, TOK_SHAPE, TOK_WEIGHTS, TOK_LAYERS,
    TOK_CONV2D, TOK_DENSE, TOK_MAXPOOL, TOK_AVGPOOL, TOK_FLATTEN,
    
    // Values (user content)
    TOK_NUMBER,
    TOK_IDENTIFIER,     // Network names, parameter names
    TOK_STRING,         // Generic string literal
    TOK_STRING_ACT,     // Activation function string
    TOK_STRING_PATH,    // File path string
    
    // Punctuation (language tokens)
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_LPAREN, TOK_RPAREN,
    TOK_COLON, TOK_COMMA,
    
    // Special
    TOK_ERROR
} TokenType;

// Token names for human-readable output
const char* token_names[] = {
    "NETWORK", "INPUT", "SHAPE", "WEIGHTS", "LAYERS",
    "CONV2D", "DENSE", "MAXPOOL", "AVGPOOL", "FLATTEN",
    "NUMBER",
    "IDENTIFIER",
    "STRING",
    "STRING_ACT",
    "STRING_PATH",
    "LBRACE", "RBRACE",
    "LBRACKET", "RBRACKET",
    "LPAREN", "RPAREN",
    "COLON", "COMMA",
    "ERROR"
};

// Global variables
int line_number = 1;
int token_count = 0;
int error_count = 0;

// Function prototypes
void print_token(TokenType type, char* text);
int is_activation(char* str);
int is_filepath(char* str);
void print_summary();

%}

%option noyywrap
%option yylineno

/* Regular expressions for patterns */
DIGIT       [0-9]
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}*
ID          [a-zA-Z_][a-zA-Z0-9_]*
STRING      \"[^\"]*\"
WS          [ \t\r]
NEWLINE     \n
COMMENT     "//"[^\n]*

%%

 /* ========== KEYWORDS (Language Tokens) ========== */
 /* These are FIXED elements of NetLang grammar */
network     { print_token(TOK_NETWORK, yytext); }
input       { print_token(TOK_INPUT, yytext); }
shape       { print_token(TOK_SHAPE, yytext); }
weights     { print_token(TOK_WEIGHTS, yytext); }
layers      { print_token(TOK_LAYERS, yytext); }

Conv2D      { print_token(TOK_CONV2D, yytext); }
Dense       { print_token(TOK_DENSE, yytext); }
MaxPool     { print_token(TOK_MAXPOOL, yytext); }
AvgPool     { print_token(TOK_AVGPOOL, yytext); }
Flatten     { print_token(TOK_FLATTEN, yytext); }

 /* ========== NUMBERS (Values) ========== */
{INTEGER}   { print_token(TOK_NUMBER, yytext); }
{FLOAT}     { print_token(TOK_NUMBER, yytext); }

 /* ========== IDENTIFIERS (Values) ========== */
{ID}        { print_token(TOK_IDENTIFIER, yytext); }

 /* ========== STRING LITERALS (Values) ========== */
{STRING}    {
    char* text = yytext;
    char content[256];
    
    // Remove quotes for analysis
    strncpy(content, text + 1, strlen(text) - 2);
    content[strlen(text) - 2] = '\0';
    
    // Categorize the string
    if (is_activation(content)) {
        print_token(TOK_STRING_ACT, text);
    } else if (is_filepath(content)) {
        print_token(TOK_STRING_PATH, text);
    } else {
        print_token(TOK_STRING, text);
    }
}

 /* ========== PUNCTUATION (Language Tokens) ========== */
"{"         { print_token(TOK_LBRACE, yytext); }
"}"         { print_token(TOK_RBRACE, yytext); }
"["         { print_token(TOK_LBRACKET, yytext); }
"]"         { print_token(TOK_RBRACKET, yytext); }
"("         { print_token(TOK_LPAREN, yytext); }
")"         { print_token(TOK_RPAREN, yytext); }
":"         { print_token(TOK_COLON, yytext); }
","         { print_token(TOK_COMMA, yytext); }

 /* ========== WHITESPACE & COMMENTS ========== */
{WS}+       { /* Ignore whitespace */ }
{NEWLINE}   { line_number++; }
{COMMENT}   { /* Ignore comments */ }

 /* ========== ERROR HANDLING ========== */
.           {
    error_count++;
    fprintf(stderr, "Line %d: ERROR - Unexpected character '%s'\n", line_number, yytext);
    print_token(TOK_ERROR, yytext);
}

%%

/* ========== HELPER FUNCTIONS ========== */

void print_token(TokenType type, char* text) {
    fprintf(yyout, "%-15s %-20s (Line %d)\n", 
           token_names[type], 
           text, 
           line_number);
    token_count++;
}

int is_activation(char* str) {
    return (strcmp(str, "relu") == 0 ||
            strcmp(str, "sigmoid") == 0 ||
            strcmp(str, "tanh") == 0 ||
            strcmp(str, "softmax") == 0 ||
            strcmp(str, "linear") == 0);
}

int is_filepath(char* str) {
    // Check if string looks like a file path
    return (strstr(str, ".bin") != NULL ||
            strstr(str, ".h5") != NULL ||
            strstr(str, ".weights") != NULL ||
            strstr(str, "/") != NULL ||
            strstr(str, "\\") != NULL ||
            strstr(str, ".") != NULL);
}

void print_summary() {
    fprintf(yyout, "\n============================================================\n");
    fprintf(yyout, "LEXICAL ANALYSIS SUMMARY\n");
    fprintf(yyout, "============================================================\n");
    fprintf(yyout, "Total lines processed:  %d\n", line_number);
    fprintf(yyout, "Total tokens found:     %d\n", token_count);
    fprintf(yyout, "Total errors detected:  %d\n", error_count);
    fprintf(yyout, "============================================================\n");
}

int main(int argc, char** argv) {
    FILE *outfile = stdout;
    char output_filename[256];
    
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
            return 1;
        }
        
        // Create output filename (e.g., demo.nlang -> demo_tokens.txt)
        snprintf(output_filename, sizeof(output_filename), "%s_tokens.txt", argv[1]);
        outfile = fopen(output_filename, "w");
        if (!outfile) {
            fprintf(stderr, "Error: Cannot create output file\n");
            fclose(yyin);
            return 1;
        }
        
        fprintf(outfile, "============================================================\n");
        fprintf(outfile, "NETLANG LEXICAL ANALYZER\n");
        fprintf(outfile, "============================================================\n\n");
        fprintf(outfile, "Processing file: %s\n\n", argv[1]);
    } else {
        fprintf(outfile, "============================================================\n");
        fprintf(outfile, "NETLANG LEXICAL ANALYZER\n");
        fprintf(outfile, "============================================================\n\n");
        fprintf(outfile, "Processing standard input...\n\n");
    }
    
    fprintf(outfile, "%-15s %-20s %s\n", "TOKEN TYPE", "VALUE", "LINE");
    fprintf(outfile, "%-15s %-20s %s\n", "----------", "-----", "----");
    
    // Redirect output to file
    yyout = outfile;
    
    // Run the lexer
    yylex();
    
    // Print summary
    print_summary();
    
    if (argc > 1) {
        fclose(yyin);
        if (outfile != stdout) {
            fclose(outfile);
            printf("\n✓ Lexical analysis complete!\n");
            printf("Output written to: %s\n", output_filename);
        }
    }
    
    return (error_count > 0) ? 1 : 0;
}