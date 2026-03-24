/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_BUILD_NET_LANG_TAB_H_INCLUDED
# define YY_YY_BUILD_NET_LANG_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NETWORK = 258,                 /* NETWORK  */
    MODULE = 259,                  /* MODULE  */
    RETURN = 260,                  /* RETURN  */
    FROM = 261,                    /* FROM  */
    INPUT = 262,                   /* INPUT  */
    SHAPE = 263,                   /* SHAPE  */
    WEIGHTS = 264,                 /* WEIGHTS  */
    CONV2D = 265,                  /* CONV2D  */
    DENSE = 266,                   /* DENSE  */
    MAXPOOL = 267,                 /* MAXPOOL  */
    AVGPOOL = 268,                 /* AVGPOOL  */
    FLATTEN = 269,                 /* FLATTEN  */
    ADD = 270,                     /* ADD  */
    CONCAT = 271,                  /* CONCAT  */
    BATCHNORM = 272,               /* BATCHNORM  */
    LAYERNORM = 273,               /* LAYERNORM  */
    FILTERS = 274,                 /* FILTERS  */
    KERNEL = 275,                  /* KERNEL  */
    ACTIVATION = 276,              /* ACTIVATION  */
    STRIDE = 277,                  /* STRIDE  */
    PADDING = 278,                 /* PADDING  */
    POOL = 279,                    /* POOL  */
    UNITS = 280,                   /* UNITS  */
    RELU = 281,                    /* RELU  */
    SIGMOID = 282,                 /* SIGMOID  */
    TANH = 283,                    /* TANH  */
    SOFTMAX = 284,                 /* SOFTMAX  */
    LINEAR = 285,                  /* LINEAR  */
    NUMBER = 286,                  /* NUMBER  */
    FLOAT_NUM = 287,               /* FLOAT_NUM  */
    IDENTIFIER = 288,              /* IDENTIFIER  */
    STRING_LIT = 289,              /* STRING_LIT  */
    LBRACE = 290,                  /* LBRACE  */
    RBRACE = 291,                  /* RBRACE  */
    LBRACKET = 292,                /* LBRACKET  */
    RBRACKET = 293,                /* RBRACKET  */
    LPAREN = 294,                  /* LPAREN  */
    RPAREN = 295,                  /* RPAREN  */
    COLON = 296,                   /* COLON  */
    COMMA = 297,                   /* COMMA  */
    ASSIGN = 298                   /* ASSIGN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 97 "src\\parser\\net_lang.y"

    int ival;
    float fval;
    char* sval;
    ASTNode* node;
    ASTList* list;
    ParameterList* params;
    ActivationType activation;
    NetworkBody* netbody;

#line 118 "build\\net_lang.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_BUILD_NET_LANG_TAB_H_INCLUDED  */
