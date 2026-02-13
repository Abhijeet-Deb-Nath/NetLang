# NetLang Parser - Complete Phase Analysis

**Author:** Abhijeet Deb Nath  
**Course:** Compiler Design  
**File:** `src/parser/net_lang.y`

---

## Table of Contents

1. [Introduction to Parsing](#introduction-to-parsing)
2. [What is Bison?](#what-is-bison)
3. [File Structure Overview](#file-structure-overview)
4. [Prologue Section](#prologue-section)
5. [Bison Declarations](#bison-declarations)
6. [Grammar Rules](#grammar-rules)
7. [Semantic Actions](#semantic-actions)
8. [Error Handling](#error-handling)
9. [Main Function](#main-function)
10. [Complete Workflow](#complete-workflow)
11. [Example Parse Tree](#example-parse-tree)

---

## 1. Introduction to Parsing

### What is Parsing?

**Parsing** is the second phase of compilation that comes after lexical analysis.

```
Source Code → [LEXER] → Tokens → [PARSER] → Abstract Syntax Tree (AST)
```

**The parser's job:**
1. **Check syntax** - Is the token sequence grammatically correct?
2. **Build AST** - Create a tree structure representing the program's structure
3. **Prepare for semantic analysis** - Organize code for type checking and validation

**Example:**

```nlang
network MyNet {
    x1 = Conv2D(filters: 32) from input
}
```

**Tokens from Lexer:**
```
NETWORK IDENTIFIER("MyNet") LBRACE
IDENTIFIER("x1") ASSIGN CONV2D LPAREN
FILTERS COLON NUMBER(32) RPAREN
FROM INPUT
RBRACE
```

**Parser's job:** Verify this matches the grammar and build an AST:
```
Program
└── Network "MyNet"
    └── Assignment "x1"
        ├── Layer: Conv2D (filters=32)
        └── Source: input
```

---

## 2. What is Bison?

**Bison** is a **parser generator** - a tool that automatically creates a parser from a grammar specification.

### How Bison Works

```
Your Grammar          Bison              Generated Parser
(net_lang.y)    →   Generator    →      (net_lang.tab.c)
                                         (net_lang.tab.h)
```

**What you write:** Grammar rules in BNF-like notation  
**What Bison generates:** A complete LR parser in C code

### Why Use Bison?

1. **Automatic:** No need to manually code parsing logic
2. **Powerful:** Handles complex grammars with shift/reduce
3. **Standard:** Industry-standard tool (GNU project)
4. **Error handling:** Built-in error recovery mechanisms

---

## 3. File Structure Overview

The `.y` file has **three main sections** separated by `%%`:

```yacc
%{
    /* PROLOGUE: C code, includes, declarations */
%}

/* BISON DECLARATIONS: tokens, types, precedence */

%%

/* GRAMMAR RULES: syntax rules with semantic actions */

%%

/* EPILOGUE: Additional C code (error handling, main) */
```

### Our File Structure

```
Lines 1-32:    Prologue (C includes, function declarations)
Lines 33-99:   Bison declarations (%union, %token, %type)
Lines 100-564: Grammar rules (%%...%%)
Lines 565-631: Epilogue (yyerror, main function)
```

---

## 4. Prologue Section

### 4.1 Header Comment Block

```c
/*
 * NetLang Parser - Phase 2 & 3
 * Builds Abstract Syntax Tree from token stream
 * Performs semantic analysis and type checking
 */
```

**Purpose:** Documents the parser's role and functionality.

---

### 4.2 Include Directives

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../semantic/semantic.h"
```

**Why these includes?**

| Header | Purpose |
|--------|---------|
| `stdio.h` | `printf()`, `fprintf()`, file operations |
| `stdlib.h` | `malloc()`, `free()`, memory management |
| `string.h` | `strcmp()`, `strdup()`, string operations |
| `ast.h` | AST node creation and manipulation |
| `semantic.h` | Semantic analysis after parsing |

---

### 4.3 External Declarations

```c
extern int yylex(void);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;
```

**What are these?**

These are **variables and functions provided by Flex** (the lexer):

| Declaration | Type | Purpose |
|------------|------|---------|
| `yylex()` | Function | Lexer's main function - returns next token |
| `yylineno` | Variable | Current line number in source file |
| `yytext` | Variable | Text of current token |
| `yyin` | Variable | Input file pointer |

**Why `extern`?**
- These are **defined in another file** (`lex.yy.c`)
- `extern` tells the compiler "trust me, these exist elsewhere"
- The linker will connect them during compilation

---

### 4.4 Function Prototypes

```c
void yyerror(const char* s);
```

**Purpose:** Error reporting function called by Bison when syntax errors occur.

**Implemented later** in the epilogue section (line 565).

---

### 4.5 Global Variables

```c
ASTNode* ast_root = NULL;
```

**Purpose:** Stores the root of the Abstract Syntax Tree after successful parsing.

**Workflow:**
1. Starts as `NULL`
2. Parser builds AST during parsing
3. Root node assigned to `ast_root` at the end
4. Main function uses this to print and analyze the AST

---

## 5. Bison Declarations

### 5.1 Parser Configuration

```yacc
%define parse.error verbose
```

**What it does:** Enables **detailed error messages**.

**Without this:**
```
syntax error
```

**With this:**
```
syntax error, unexpected RBRACE, expecting IDENTIFIER or CONV2D
```

Much more helpful for debugging!

---

### 5.2 Location Tracking

```yacc
%locations
```

**Purpose:** Enables **source location tracking** for every token and rule.

**What it provides:**

Automatically creates the `YYLTYPE` structure:
```c
struct YYLTYPE {
    int first_line;   // Starting line
    int first_column; // Starting column
    int last_line;    // Ending line
    int last_column;  // Ending column
};
```

**Accessed via:** `@1`, `@2`, `@$` in semantic actions.

**Used for:** Error messages showing exact line/column of errors.

---

### 5.3 Semantic Value Types (`%union`)

```yacc
%union {
    int ival;
    float fval;
    char* sval;
    ASTNode* node;
    ASTList* list;
    ParameterList* params;
    ActivationType activation;
    NetworkBody* netbody;
}
```

**What is `%union`?**

This creates a **C union** called `YYSTYPE` that can hold different types of values:

```c
typedef union {
    int ival;           // Integer values
    float fval;         // Floating-point values
    char* sval;         // String values
    ASTNode* node;      // AST nodes
    ASTList* list;      // Lists of AST nodes
    // ...
} YYSTYPE;
```

**Why a union?**
- Different tokens/rules produce different types of values
- A union saves memory by sharing the same space
- Only **one** field is active at a time

**Example Usage:**

```yacc
%token <ival> NUMBER        // NUMBER tokens carry integer values
%token <sval> IDENTIFIER    // IDENTIFIER tokens carry string values
%type <node> layer_expr     // layer_expr rules produce AST nodes
```

---

### 5.4 Token Declarations

#### 5.4.1 Keywords (No Value)

```yacc
%token NETWORK MODULE RETURN FROM INPUT SHAPE WEIGHTS
```

**What this means:**
- These are **terminal symbols** (tokens from the lexer)
- They carry **no semantic value** (just their presence matters)
- Examples: `network`, `module`, `return`

---

#### 5.4.2 Tokens with Values

```yacc
%token <sval> RELU SIGMOID TANH SOFTMAX LINEAR
%token <ival> NUMBER
%token <fval> FLOAT_NUM
%token <sval> IDENTIFIER STRING_LIT
```

**What `<type>` means:**

The type in angle brackets tells Bison which field of the `%union` this token uses.

**Example breakdown:**

```yacc
%token <ival> NUMBER
```

**Translates to:**
- When lexer returns `NUMBER` token
- The token's value is in `yylval.ival`
- Parser can access it via `$1.ival` (or just `$1` since Bison knows the type)

**In the lexer (net_lang.l):**
```c
{INTEGER}  { 
    yylval.ival = atoi(yytext);  // Set the integer value
    return NUMBER;                // Return token code
}
```

**In the parser:**
```yacc
number:
    NUMBER {
        $$ = ast_number_int($1, @1.first_line);
        // $1 is the NUMBER's ival (integer value)
    }
```

---

### 5.5 Non-Terminal Types

```yacc
%type <node> program definition network_def module_def
%type <node> statement assignment return_stmt
%type <list> definition_list statement_list
%type <params> module_params param_list layer_params
```

**What are non-terminals?**

Non-terminals are **grammar rules** (not tokens). They represent syntactic structures.

**What `%type` does:**

Declares what type of value a grammar rule produces.

**Example:**

```yacc
%type <node> network_def
```

**Means:**
- The `network_def` rule produces an `ASTNode*`
- In semantic actions, `$$` (the result) is of type `ASTNode*`
- Other rules can use this value via `$1`, `$2`, etc.

---

### 5.6 Precedence Declarations

```yacc
%left FROM
```

**What is precedence?**

Determines how to resolve **ambiguous grammar conflicts**.

**Example ambiguity:**

```nlang
x1 = Conv2D(filters: 32) from input
```

Should this be:
1. `(x1 = Conv2D(filters: 32)) from input`  ❌
2. `x1 = (Conv2D(filters: 32) from input)`  ✓

**`%left FROM` declares:**
- `FROM` is **left-associative**
- Helps Bison make the right choice in shift/reduce conflicts

**Other precedence types:**
- `%left` - Left associative (e.g., `+`, `-`)
- `%right` - Right associative (e.g., `=`, `^`)
- `%nonassoc` - Non-associative (e.g., `<`, `>`)

---

## 6. Grammar Rules

Grammar rules define the **syntax** of NetLang. Each rule has:

1. **Non-terminal** (left side of `:`)
2. **Production** (right side of `:`)
3. **Semantic action** (C code in `{...}`)

### Format:

```yacc
non_terminal:
    production1 { /* action */ }
    | production2 { /* action */ }
    | production3 { /* action */ }
    ;
```

---

### 6.1 Start Symbol: `program`

```yacc
program:
    definition_list {
        ast_root = ast_program(@1.first_line);
        ast_root->data.program.definitions = $1;
        $$ = ast_root;
    }
    ;
```

**Breakdown:**

| Element | Meaning |
|---------|---------|
| `program:` | This is the **start symbol** (top-level rule) |
| `definition_list` | Must match a list of definitions |
| `{...}` | **Semantic action** - C code executed when rule matches |
| `ast_root = ...` | Create program node and store in global variable |
| `$1` | Value of `definition_list` (an `ASTList*`) |
| `$$` | Return value of this rule (an `ASTNode*`) |
| `@1.first_line` | Line number where `definition_list` starts |

**What this rule says:**
> "A complete NetLang program consists of a list of definitions (networks and modules). When we successfully parse this, create a program AST node and store it."

---

### 6.2 Definition Lists

```yacc
definition_list:
    definition {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | definition_list definition {
        ast_list_append($1, $2);
        $$ = $1;
    }
    ;
```

**This is a recursive rule** for building lists.

**Case 1: Single definition**
```yacc
definition {
    $$ = ast_list_new();      // Create new empty list
    ast_list_append($$, $1);  // Add the definition to it
}
```

**Case 2: Multiple definitions (recursive)**
```yacc
definition_list definition {
    ast_list_append($1, $2);  // Add new definition to existing list
    $$ = $1;                  // Return the extended list
}
```

**Example:**

Source code:
```nlang
network Net1 { ... }
network Net2 { ... }
module MyModule(...) { ... }
```

Parse sequence:
```
1. Parse "network Net1" → definition₁
2. Match: definition → definition_list₁ (list with 1 item)

3. Parse "network Net2" → definition₂
4. Match: definition_list₁ definition₂ → definition_list₂ (list with 2 items)

5. Parse "module MyModule" → definition₃
6. Match: definition_list₂ definition₃ → definition_list₃ (list with 3 items)
```

Final: `definition_list` contains [Net1, Net2, MyModule]

---

### 6.3 Network Definition

```yacc
network_def:
    NETWORK IDENTIFIER LBRACE network_body RBRACE {
        $$ = ast_node_new(NODE_NETWORK, @1.first_line);
        $$->data.network.name = $2;
        $$->data.network.input = $4->input;
        $$->data.network.weights = $4->weights;
        $$->data.network.statements = $4->statements;
        free($4);
    }
    ;
```

**Matches syntax:**
```nlang
network NetworkName {
    /* network body */
}
```

**Token/rule breakdown:**

| Position | Symbol | Type | What it is |
|----------|--------|------|------------|
| `$1` | `NETWORK` | Token | The keyword "network" |
| `$2` | `IDENTIFIER` | Token (`char*`) | Network name string |
| `$3` | `LBRACE` | Token | Opening `{` |
| `$4` | `network_body` | Rule (`NetworkBody*`) | Parsed body structure |
| `$5` | `RBRACE` | Token | Closing `}` |

**Semantic action explanation:**

```c
$$ = ast_node_new(NODE_NETWORK, @1.first_line);
```
- Create new AST node of type `NODE_NETWORK`
- `@1.first_line` = line number where `NETWORK` token appears

```c
$$->data.network.name = $2;
```
- Store the network name (from `IDENTIFIER` token)
- **Transfer ownership** of the string (no strdup needed here)

```c
$$->data.network.input = $4->input;
$$->data.network.weights = $4->weights;
$$->data.network.statements = $4->statements;
```
- Extract components from the parsed network body
- Store them in the network node

```c
free($4);
```
- Free the temporary `NetworkBody` struct
- **Important:** Only free the struct itself, NOT the pointers inside (they're now owned by the AST node)

---

### 6.4 Network Body

```yacc
network_body:
    /* empty */ {
        $$ = (NetworkBody*)ast_alloc(sizeof(NetworkBody));
        $$->input = NULL;
        $$->weights = NULL;
        $$->statements = ast_list_new();
    }
    | network_body input_decl {
        $1->input = $2;
        $$ = $1;
    }
    | network_body weights_decl {
        $1->weights = $2;
        $$ = $1;
    }
    | network_body statement {
        ast_list_append($1->statements, $2);
        $$ = $1;
    }
    ;
```

**This is a left-recursive rule** that builds up the network body incrementally.

**Base case:** Empty body
```c
/* empty */ {
    $$ = (NetworkBody*)ast_alloc(sizeof(NetworkBody));
    $$->input = NULL;
    $$->weights = NULL;
    $$->statements = ast_list_new();
}
```
Creates an empty `NetworkBody` structure.

**Recursive cases:** Add components one by one

```yacc
network_body input_decl      // Add input declaration
network_body weights_decl    // Add weights declaration
network_body statement       // Add statement (layer assignment)
```

**Example:**

Source:
```nlang
network MyNet {
    input(shape: [224, 224, 3])
    weights("model.weights")
    x1 = Conv2D(filters: 32) from input
    x2 = MaxPool(pool: [2, 2]) from x1
}
```

Parse sequence:
```
1. Start: network_body = empty { input=NULL, weights=NULL, statements=[] }

2. Parse "input(...)" → input_decl
   network_body + input_decl
   → network_body = { input=<node>, weights=NULL, statements=[] }

3. Parse "weights(...)" → weights_decl
   network_body + weights_decl
   → network_body = { input=<node>, weights=<node>, statements=[] }

4. Parse "x1 = Conv2D..." → statement
   network_body + statement
   → network_body = { input=<node>, weights=<node>, statements=[stmt1] }

5. Parse "x2 = MaxPool..." → statement
   network_body + statement
   → network_body = { input=<node>, weights=<node>, statements=[stmt1, stmt2] }
```

---

### 6.5 Module Definition

```yacc
module_def:
    MODULE IDENTIFIER LPAREN module_params RPAREN
    LBRACE statement_list return_stmt RBRACE {
        $$ = ast_module($2, $4, @1.first_line);
        $$->data.module.statements = $7;
        $$->data.module.return_stmt = $8;
        free($2);
    }
    ;
```

**Matches syntax:**
```nlang
module ResidualBlock(x, filters) {
    y = Conv2D(filters: filters) from x
    z = Conv2D(filters: filters) from y
    return Concat(x, z)
}
```

**Component mapping:**

| Position | Value | Example |
|----------|-------|---------|
| `$1` | `MODULE` | "module" keyword |
| `$2` | `IDENTIFIER` | "ResidualBlock" |
| `$3` | `LPAREN` | `(` |
| `$4` | `module_params` | [x, filters] |
| `$5` | `RPAREN` | `)` |
| `$6` | `LBRACE` | `{` |
| `$7` | `statement_list` | [y=Conv2D..., z=Conv2D...] |
| `$8` | `return_stmt` | return Concat(...) |
| `$9` | `RBRACE` | `}` |

---

### 6.6 Layer Expressions - Conv2D Example

```yacc
conv2d_layer:
    CONV2D LPAREN layer_params RPAREN {
        ASTNode* node = ast_node_new(NODE_CONV2D, @1.first_line);
        
        /* Set default values */
        node->data.conv2d.filters = 1;
        node->data.conv2d.kernel[0] = 3;
        node->data.conv2d.kernel[1] = 3;
        node->data.conv2d.stride = 1;
        node->data.conv2d.padding = 0;
        node->data.conv2d.activation = ACT_NONE;
        
        /* Process parameters from grammar */
        Parameter* p = $3->head;
        while (p) {
            if (strcmp(p->name, "filters") == 0 && p->value) {
                node->data.conv2d.filters = p->value->data.number.ival;
            } else if (strcmp(p->name, "kernel") == 0 && p->value) {
                ASTNode* arr = p->value;
                if (arr->type == NODE_ARRAY && arr->data.array.elements) {
                    ASTNode* e = arr->data.array.elements->head;
                    if (e) { 
                        node->data.conv2d.kernel[0] = e->data.number.ival;
                        e = e->next;
                    }
                    if (e) { 
                        node->data.conv2d.kernel[1] = e->data.number.ival;
                    }
                }
            } else if (strcmp(p->name, "activation") == 0 && p->value) {
                if (p->value->type == NODE_IDENTIFIER) {
                    node->data.conv2d.activation = 
                        activation_from_string(p->value->data.identifier.name);
                }
            }
            // ... more parameters
            p = p->next;
        }
        $$ = node;
    }
    ;
```

**What this does:**

1. **Create Conv2D node** with default parameter values
2. **Iterate through parameters** in the parameter list
3. **Extract values** based on parameter names
4. **Handle different types:**
   - Simple integers: `filters: 32`
   - Arrays: `kernel: [3, 3]`
   - Identifiers: `activation: relu`

**Example:**

Source:
```nlang
Conv2D(filters: 64, kernel: [5, 5], activation: relu)
```

Processing:
```c
1. Create NODE_CONV2D with defaults

2. Loop through parameters:
   - param[0]: name="filters", value=64
     → node->data.conv2d.filters = 64
   
   - param[1]: name="kernel", value=[5, 5]
     → node->data.conv2d.kernel[0] = 5
     → node->data.conv2d.kernel[1] = 5
   
   - param[2]: name="activation", value="relu"
     → node->data.conv2d.activation = ACT_RELU

3. Return completed node
```

---

### 6.7 Layer Parameters

```yacc
layer_param_list:
    IDENTIFIER COLON expr {
        $$ = param_list_new();
        param_list_add($$, $1, $3);
        free($1);
    }
    | FILTERS COLON expr {
        $$ = param_list_new();
        param_list_add($$, "filters", $3);
    }
    | layer_param_list COMMA FILTERS COLON expr {
        param_list_add($1, "filters", $5);
        $$ = $1;
    }
    // ... more parameter types
    ;
```

**Why so many rules?**

We need separate rules for:
1. **Keywords as parameter names:** `filters:`, `kernel:`, `activation:`
2. **Generic identifiers:** `custom_param:`, `my_value:`
3. **Lists:** Multiple parameters separated by commas

**Example matches:**

```nlang
filters: 32                    → FILTERS COLON expr
kernel: [3, 3]                 → KERNEL COLON expr
filters: 32, activation: relu  → layer_param_list COMMA ACTIVATION COLON expr
```

---

### 6.8 Expressions

```yacc
expr:
    number { $$ = $1; }
    | array { $$ = $1; }
    | identifier_expr { $$ = $1; }
    | activation_value {
        $$ = ast_identifier(activation_to_string($1), @1.first_line);
    }
    ;
```

**What counts as an expression?**

| Type | Example | AST Node Type |
|------|---------|--------------|
| Number | `32`, `0.5` | `NODE_NUMBER` |
| Array | `[224, 224, 3]` | `NODE_ARRAY` |
| Identifier | `x1`, `input` | `NODE_IDENTIFIER` |
| Activation | `relu`, `sigmoid` | `NODE_IDENTIFIER` (converted) |

---

### 6.9 Arrays

```yacc
array:
    LBRACKET array_elements RBRACKET {
        ASTNode* node = ast_array(@1.first_line);
        node->data.array.elements = $2;
        $$ = node;
    }
    ;

array_elements:
    number {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | array_elements COMMA number {
        ast_list_append($1, $3);
        $$ = $1;
    }
    ;
```

**How arrays are parsed:**

Source: `[224, 224, 3]`

```
1. Match LBRACKET: [
2. Parse array_elements:
   a. number: 224 → list=[224]
   b. COMMA number: 224 → list=[224, 224]
   c. COMMA number: 3 → list=[224, 224, 3]
3. Match RBRACKET: ]
4. Create array node with elements list
```

---

## 7. Semantic Actions

### What are Semantic Actions?

**Semantic actions** are C code blocks `{...}` that execute when a grammar rule matches.

**Purpose:**
1. **Build AST nodes** from matched tokens
2. **Propagate values** up the parse tree
3. **Perform computations** (e.g., extract parameter values)
4. **Memory management** (malloc/free)

---

### 7.1 Special Variables in Actions

| Variable | Type | Meaning |
|----------|------|---------|
| `$$` | Any | **Result** of this rule (return value) |
| `$1` | Any | Value of **first** symbol on right side |
| `$2` | Any | Value of **second** symbol |
| `$n` | Any | Value of **nth** symbol |
| `@$` | `YYLTYPE` | Location info for this rule |
| `@1` | `YYLTYPE` | Location of first symbol |
| `@n` | `YYLTYPE` | Location of nth symbol |

---

### 7.2 Example: Building an Assignment

```yacc
assignment:
    IDENTIFIER ASSIGN layer_expr from_clause {
        $$ = ast_assignment($1, $3, $4, @1.first_line);
        free($1);
    }
    ;
```

**What happens:**

Input: `x1 = Conv2D(...) from input`

```c
$1 = "x1"              // IDENTIFIER (char*)
$2 = (token)           // ASSIGN (not used in action)
$3 = <Conv2D node>     // layer_expr (ASTNode*)
$4 = <input node>      // from_clause (ASTNode*)

$$ = ast_assignment(
    "x1",              // Variable name
    <Conv2D node>,     // Layer expression
    <input node>,      // Source expression
    @1.first_line      // Line number of IDENTIFIER
);

free($1);              // Free the strdup'd string "x1"
```

**Result:** Assignment AST node connecting variable, layer, and source.

---

### 7.3 Example: Parameter Processing

```yacc
FILTERS COLON expr {
    $$ = param_list_new();
    param_list_add($$, "filters", $3);
}
```

**Breakdown:**

```c
$1 = (token FILTERS)   // The keyword "filters"
$2 = (token COLON)     // The ":"
$3 = <number node>     // expr (ASTNode* containing value)

$$ = param_list_new(); // Create new empty parameter list

param_list_add(
    $$,                // The list we just created
    "filters",         // Parameter name (string literal)
    $3                 // Parameter value (AST node)
);
```

**Result:** Parameter list containing one entry: `filters` → value

---

## 8. Error Handling

### 8.1 The `yyerror` Function

```c
void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    fprintf(stderr, "Near token: '%s'\n", yytext);
}
```

**When is this called?**

Automatically by Bison when a **syntax error** is detected.

**Example:**

Source (error):
```nlang
network MyNet {
    x1 = Conv2D(filters 32) from input
}
```
Missing `:` after `filters`

Output:
```
Parse error at line 2: syntax error, unexpected NUMBER, expecting COLON
Near token: '32'
```

**How it works:**

1. Parser encounters unexpected token
2. Bison calls `yyerror()` with error message
3. Function prints line number (`yylineno`) and nearby text (`yytext`)
4. Parser attempts error recovery or stops

---

### 8.2 Error Recovery

Bison has built-in error recovery, but this parser uses **simple error handling**:
- Print error message
- Stop parsing
- Return failure code

**Advanced error recovery** (not implemented here) would use the `error` token to skip bad sections and continue parsing.

---

## 9. Main Function

### 9.1 File Opening

```c
int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.nlang>\n", argv[0]);
        return 1;
    }
    
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
```

**Purpose:**
1. Check command-line arguments
2. Open source file
3. Set `yyin` (Flex reads from this)

---

### 9.2 Parsing

```c
printf("NetLang Parser v0.1\n");
printf("Parsing: %s\n", argv[1]);

int result = yyparse();

fclose(yyin);
```

**`yyparse()` return values:**
- `0` = Success (no syntax errors)
- `1` = Failure (syntax errors found)
- `2` = Out of memory

---

### 9.3 AST Processing

```c
if (result == 0 && ast_root) {
    printf("\n✓ Parse successful!\n\n");
    printf("========== AST DUMP ==========\n\n");
    ast_print(ast_root, 0);
```

**If parsing succeeds:**
1. Print success message
2. Dump the AST tree structure
3. Proceed to semantic analysis

---

### 9.4 Semantic Analysis

```c
SemanticResult sem_result = analyze_program(ast_root);

if (sem_result.is_valid) {
    printf("\n✓ Semantic analysis passed!\n");
    printf("  Warnings: %d\n", sem_result.warning_count);
    
    printf("\n========== SYMBOL TABLE ==========\n\n");
    scope_print(sem_result.global_scope, 0);
} else {
    fprintf(stderr, "\n✗ Semantic analysis failed!\n");
    fprintf(stderr, "  Errors: %d\n", sem_result.error_count);
}
```

**What happens:**
1. Call `analyze_program()` from semantic analyzer
2. Check if program is semantically valid
3. Print symbol table or error count
4. Clean up and exit

---

## 10. Complete Workflow

### 10.1 Build Process

```
1. SOURCE FILES
   ├── net_lang.l (Lexer spec)
   └── net_lang.y (Parser spec)
   
2. GENERATION PHASE
   ├── bison -d net_lang.y
   │   ├─→ net_lang.tab.c (Parser implementation)
   │   └─→ net_lang.tab.h (Token definitions, types)
   └── flex net_lang.l
       └─→ lex.yy.c (Lexer implementation)
   
3. COMPILATION PHASE
   gcc net_lang.tab.c lex.yy.c ast.c semantic.c ... -o parser
   └─→ parser.exe (Executable)
```

---

### 10.2 Runtime Process

```
1. User runs: parser.exe input.nlang

2. main() function:
   ├── Open input.nlang
   ├── Set yyin = file pointer
   └── Call yyparse()

3. yyparse() (Parser main loop):
   ├── Call yylex() → Get token
   ├── Check grammar rules
   ├── If match: Execute semantic action
   ├── Build AST incrementally
   └── Repeat until EOF or error

4. yylex() (Lexer):
   ├── Read characters from yyin
   ├── Match patterns
   ├── Set yylval (semantic value)
   ├── Set yylloc (location)
   └── Return token code

5. After parsing:
   ├── If success: ast_root contains complete AST
   ├── Print AST
   ├── Run semantic analysis
   └── Print results
```

---

### 10.3 Example Parse Trace

**Source code:**
```nlang
network SimpleNet {
    input(shape: [28, 28, 1])
    x1 = Dense(units: 10) from input
}
```

**Parse trace:**

```
Step 1: yyparse() starts
Step 2: yyparse() calls yylex()
Step 3: yylex() returns NETWORK

Step 4: yyparse() calls yylex()
Step 5: yylex() returns IDENTIFIER("SimpleNet"), sets yylval.sval = "SimpleNet"

Step 6: yyparse() calls yylex()
Step 7: yylex() returns LBRACE

Step 8: yyparse() matches: network_body → empty
        Action: Create empty NetworkBody

Step 9: yyparse() calls yylex()
Step 10: yylex() returns INPUT

Step 11: yyparse() calls yylex() multiple times...
         Matches: input(shape: [28, 28, 1])
         
Step 12: yyparse() reduces: input_decl
         Action: Create INPUT node with shape array

Step 13: yyparse() reduces: network_body input_decl
         Action: Add input to network body

Step 14: yyparse() calls yylex() multiple times...
         Matches: x1 = Dense(units: 10) from input
         
Step 15: yyparse() reduces: dense_layer
         Action: Create DENSE node with units=10

Step 16: yyparse() reduces: assignment
         Action: Create ASSIGNMENT node linking x1, Dense, input

Step 17: yyparse() reduces: network_body statement
         Action: Add assignment to network body statements

Step 18: yyparse() calls yylex()
Step 19: yylex() returns RBRACE

Step 20: yyparse() reduces: network_def
         Action: Create NETWORK node with all components

Step 21: yyparse() reduces: definition
         (network_def IS a definition)

Step 22: yyparse() reduces: definition_list
         Action: Create list containing the network

Step 23: yyparse() reduces: program
         Action: Create PROGRAM node, set ast_root

Step 24: yyparse() calls yylex()
Step 25: yylex() returns 0 (EOF)

Step 26: yyparse() returns 0 (success!)
```

---

## 11. Example Parse Tree

**Source:**
```nlang
network MiniNet {
    input(shape: [28, 28])
    x1 = Conv2D(filters: 16) from input
    x2 = Flatten() from x1
}
```

**AST Structure:**

```
Program (NODE_PROGRAM)
└── definitions: [1 network]
    └── Network "MiniNet" (NODE_NETWORK)
        ├── input: Input (NODE_INPUT)
        │   └── shape: Array [28, 28]
        │       ├── Number: 28
        │       └── Number: 28
        ├── weights: NULL
        └── statements: [2 statements]
            ├── Assignment "x1" (NODE_ASSIGNMENT)
            │   ├── layer: Conv2D (NODE_CONV2D)
            │   │   └── filters: 16
            │   └── source: Identifier "input"
            └── Assignment "x2" (NODE_ASSIGNMENT)
                ├── layer: Flatten (NODE_FLATTEN)
                └── source: Identifier "x1"
```

**Printed AST (from `ast_print()`):**

```
Program (line 1)
  Network 'MiniNet' (line 1)
    Input (line 2)
      Shape: [28, 28]
    Statements:
      Assignment 'x1' (line 3)
        Layer: Conv2D
          filters: 16
          kernel: [3, 3]
          stride: 1
          padding: 0
          activation: none
        Source: input
      Assignment 'x2' (line 4)
        Layer: Flatten
        Source: x1
```

---

## Summary

### Key Takeaways

1. **Bison generates a parser** from grammar rules
2. **Grammar rules** define NetLang syntax
3. **Semantic actions** build the AST
4. **`$$`, `$1`, `$2`** are used to pass values between rules
5. **`@1`, `@2`** provide location tracking for error messages
6. **`%union`** defines all possible value types
7. **Lexer and parser work together** through token codes and `yylval`

### The Parser's Role

```
Input: Token stream from lexer
Process: Match grammar rules, execute actions
Output: Abstract Syntax Tree (AST)
Next: Semantic analysis and code generation
```

### Files Generated by Bison

| File | Content |
|------|---------|
| `net_lang.tab.c` | Parser implementation (state machine, `yyparse()`) |
| `net_lang.tab.h` | Token codes, `YYSTYPE`, `YYLTYPE`, declarations |

### Study Tips

1. **Understand recursion** - Many grammar rules are recursive (lists, statements)
2. **Track `$$` and `$n`** - Know what values are flowing through rules
3. **Follow the AST** - Each action builds part of the final tree
4. **Read bottom-up** - Parser reduces from tokens up to start symbol
5. **Test incrementally** - Start with simple programs, add complexity

---

**End of Parser Phase Analysis**
