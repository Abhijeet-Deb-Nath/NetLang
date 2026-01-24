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
    TOK_NETWORK, TOK_MODULE, TOK_INPUT, TOK_SHAPE, TOK_WEIGHTS, TOK_LAYERS,
    TOK_RETURN, TOK_FROM,
    TOK_CONV2D, TOK_DENSE, TOK_MAXPOOL, TOK_AVGPOOL, TOK_FLATTEN, TOK_CONCAT,
    TOK_BATCHNORM, TOK_LAYERNORM,
    
    // Parameter keywords
    TOK_FILTERS, TOK_KERNEL, TOK_ACTIVATION, TOK_STRIDE, TOK_PADDING,
    TOK_POOL, TOK_UNITS,
    
    // Activation functions (tokens)
    TOK_RELU, TOK_SIGMOID, TOK_TANH, TOK_SOFTMAX, TOK_LINEAR,
    
    // Values (user content)
    TOK_NUMBER,
    TOK_IDENTIFIER,     // Network names, custom parameter names
    TOK_STRING_PATH,    // File path string
    
    // Punctuation (language tokens)
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_LPAREN, TOK_RPAREN,
    TOK_COLON, TOK_COMMA, TOK_ASSIGN,
    
    // Special
    TOK_ERROR
} TokenType;

// Token names for human-readable output
const char* token_names[] = {
    "NETWORK", "MODULE", "INPUT", "SHAPE", "WEIGHTS", "LAYERS",
    "RETURN", "FROM",
    "CONV2D", "DENSE", "MAXPOOL", "AVGPOOL", "FLATTEN", "CONCAT",
    "BATCHNORM", "LAYERNORM",
    "FILTERS", "KERNEL", "ACTIVATION", "STRIDE", "PADDING",
    "POOL", "UNITS",
    "RELU", "SIGMOID", "TANH", "SOFTMAX", "LINEAR",
    "NUMBER",
    "IDENTIFIER",
    "STRING_PATH",
    "LBRACE", "RBRACE",
    "LBRACKET", "RBRACKET",
    "LPAREN", "RPAREN",
    "COLON", "COMMA", "ASSIGN",
    "ERROR"
};

// Global variables
int line_number = 1;
int token_count = 0;
int error_count = 0;

// Function prototypes
void print_token(TokenType type, char* text);
void print_summary();

%}

%option noyywrap
%option yylineno

/* Start conditions for multi-line comments */
%x COMMENT

/* Regular expressions for patterns */
DIGIT       [0-9]
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}*
ID          [a-zA-Z_][a-zA-Z0-9_]*
STRING      \"[^\"]*\"
WS          [ \t\r]
NEWLINE     \n
SLCOMMENT   "//"[^\n]*

%%

 /* ========== KEYWORDS (Language Tokens) ========== */
 /* These are FIXED elements of NetLang grammar */
network     { print_token(TOK_NETWORK, yytext); }
module      { print_token(TOK_MODULE, yytext); }
return      { print_token(TOK_RETURN, yytext); }
from        { print_token(TOK_FROM, yytext); }
input       { print_token(TOK_INPUT, yytext); }
shape       { print_token(TOK_SHAPE, yytext); }
weights     { print_token(TOK_WEIGHTS, yytext); }
layers      { print_token(TOK_LAYERS, yytext); }

Conv2D      { print_token(TOK_CONV2D, yytext); }
Dense       { print_token(TOK_DENSE, yytext); }
MaxPool     { print_token(TOK_MAXPOOL, yytext); }
AvgPool     { print_token(TOK_AVGPOOL, yytext); }
Flatten     { print_token(TOK_FLATTEN, yytext); }
Concat      { print_token(TOK_CONCAT, yytext); }
BatchNorm   { print_token(TOK_BATCHNORM, yytext); }
LayerNorm   { print_token(TOK_LAYERNORM, yytext); }

 /* ========== PARAMETER KEYWORDS ========== */
filters     { print_token(TOK_FILTERS, yytext); }
kernel      { print_token(TOK_KERNEL, yytext); }
activation  { print_token(TOK_ACTIVATION, yytext); }
stride      { print_token(TOK_STRIDE, yytext); }
padding     { print_token(TOK_PADDING, yytext); }
pool        { print_token(TOK_POOL, yytext); }
units       { print_token(TOK_UNITS, yytext); }

 /* ========== ACTIVATION FUNCTIONS ========== */
relu        { print_token(TOK_RELU, yytext); }
sigmoid     { print_token(TOK_SIGMOID, yytext); }
tanh        { print_token(TOK_TANH, yytext); }
softmax     { print_token(TOK_SOFTMAX, yytext); }
linear      { print_token(TOK_LINEAR, yytext); }

 /* ========== NUMBERS (Values) ========== */
{INTEGER}   { 
    /* yylval.ival = atoi(yytext); */  /* Uncomment when integrating with parser */
    print_token(TOK_NUMBER, yytext); 
}
{FLOAT}     { 
    /* yylval.fval = atof(yytext); */  /* Uncomment when integrating with parser */
    print_token(TOK_NUMBER, yytext); 
}

 /* ========== IDENTIFIERS (Values) ========== */
{ID}        { 
    /* yylval.sval = strdup(yytext); */  /* Uncomment when integrating with parser */
    print_token(TOK_IDENTIFIER, yytext); 
}

 /* ========== STRING LITERALS (File Paths) ========== */
{STRING}    { 
    /* yylval.sval = strdup(yytext); */  /* Uncomment when integrating with parser */
    print_token(TOK_STRING_PATH, yytext); 
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
"="         { print_token(TOK_ASSIGN, yytext); }

 /* ========== WHITESPACE & COMMENTS ========== */
{WS}+       { /* Ignore whitespace */ }
{NEWLINE}   { line_number++; }
{SLCOMMENT} { /* Ignore single-line comments */ }

"/*"        { BEGIN(COMMENT); /* Start multi-line comment */ }
<COMMENT>"*/" { BEGIN(INITIAL); /* End multi-line comment */ }
<COMMENT>\n { line_number++; /* Count lines in comments */ }
<COMMENT>.  { /* Ignore comment content */ }

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