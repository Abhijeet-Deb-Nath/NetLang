/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src\\parser\\net_lang.y"

/*
 * NetLang Parser - Phase 2
 * Builds Abstract Syntax Tree from token stream
 * 
 * Grammar for NetLang DSL:
 * - Networks: network Name { ... }
 * - Modules: module Name(params) { ... return expr }
 * - Layers: Conv2D, Dense, MaxPool, etc.
 * - Dataflow: Layer(...) from source
 * 
 * Author: Abhijeet Deb Nath
 * Course: Compiler Design
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"

/* Flex/Bison integration */
extern int yylex(void);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

void yyerror(const char* s);

/* Global AST root - populated after successful parse */
ASTNode* ast_root = NULL;


#line 104 "build\\net_lang.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "net_lang.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NETWORK = 3,                    /* NETWORK  */
  YYSYMBOL_MODULE = 4,                     /* MODULE  */
  YYSYMBOL_RETURN = 5,                     /* RETURN  */
  YYSYMBOL_FROM = 6,                       /* FROM  */
  YYSYMBOL_INPUT = 7,                      /* INPUT  */
  YYSYMBOL_SHAPE = 8,                      /* SHAPE  */
  YYSYMBOL_WEIGHTS = 9,                    /* WEIGHTS  */
  YYSYMBOL_CONV2D = 10,                    /* CONV2D  */
  YYSYMBOL_DENSE = 11,                     /* DENSE  */
  YYSYMBOL_MAXPOOL = 12,                   /* MAXPOOL  */
  YYSYMBOL_AVGPOOL = 13,                   /* AVGPOOL  */
  YYSYMBOL_FLATTEN = 14,                   /* FLATTEN  */
  YYSYMBOL_CONCAT = 15,                    /* CONCAT  */
  YYSYMBOL_BATCHNORM = 16,                 /* BATCHNORM  */
  YYSYMBOL_LAYERNORM = 17,                 /* LAYERNORM  */
  YYSYMBOL_FILTERS = 18,                   /* FILTERS  */
  YYSYMBOL_KERNEL = 19,                    /* KERNEL  */
  YYSYMBOL_ACTIVATION = 20,                /* ACTIVATION  */
  YYSYMBOL_STRIDE = 21,                    /* STRIDE  */
  YYSYMBOL_PADDING = 22,                   /* PADDING  */
  YYSYMBOL_POOL = 23,                      /* POOL  */
  YYSYMBOL_UNITS = 24,                     /* UNITS  */
  YYSYMBOL_RELU = 25,                      /* RELU  */
  YYSYMBOL_SIGMOID = 26,                   /* SIGMOID  */
  YYSYMBOL_TANH = 27,                      /* TANH  */
  YYSYMBOL_SOFTMAX = 28,                   /* SOFTMAX  */
  YYSYMBOL_LINEAR = 29,                    /* LINEAR  */
  YYSYMBOL_NUMBER = 30,                    /* NUMBER  */
  YYSYMBOL_FLOAT_NUM = 31,                 /* FLOAT_NUM  */
  YYSYMBOL_IDENTIFIER = 32,                /* IDENTIFIER  */
  YYSYMBOL_STRING_LIT = 33,                /* STRING_LIT  */
  YYSYMBOL_LBRACE = 34,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 35,                    /* RBRACE  */
  YYSYMBOL_LBRACKET = 36,                  /* LBRACKET  */
  YYSYMBOL_RBRACKET = 37,                  /* RBRACKET  */
  YYSYMBOL_LPAREN = 38,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 39,                    /* RPAREN  */
  YYSYMBOL_COLON = 40,                     /* COLON  */
  YYSYMBOL_COMMA = 41,                     /* COMMA  */
  YYSYMBOL_ASSIGN = 42,                    /* ASSIGN  */
  YYSYMBOL_YYACCEPT = 43,                  /* $accept  */
  YYSYMBOL_program = 44,                   /* program  */
  YYSYMBOL_definition_list = 45,           /* definition_list  */
  YYSYMBOL_definition = 46,                /* definition  */
  YYSYMBOL_network_def = 47,               /* network_def  */
  YYSYMBOL_network_body = 48,              /* network_body  */
  YYSYMBOL_module_def = 49,                /* module_def  */
  YYSYMBOL_module_params = 50,             /* module_params  */
  YYSYMBOL_param_list = 51,                /* param_list  */
  YYSYMBOL_input_decl = 52,                /* input_decl  */
  YYSYMBOL_weights_decl = 53,              /* weights_decl  */
  YYSYMBOL_statement_list = 54,            /* statement_list  */
  YYSYMBOL_statement = 55,                 /* statement  */
  YYSYMBOL_assignment = 56,                /* assignment  */
  YYSYMBOL_from_clause = 57,               /* from_clause  */
  YYSYMBOL_return_stmt = 58,               /* return_stmt  */
  YYSYMBOL_layer_expr = 59,                /* layer_expr  */
  YYSYMBOL_conv2d_layer = 60,              /* conv2d_layer  */
  YYSYMBOL_dense_layer = 61,               /* dense_layer  */
  YYSYMBOL_pool_layer = 62,                /* pool_layer  */
  YYSYMBOL_flatten_layer = 63,             /* flatten_layer  */
  YYSYMBOL_concat_layer = 64,              /* concat_layer  */
  YYSYMBOL_concat_args = 65,               /* concat_args  */
  YYSYMBOL_norm_layer = 66,                /* norm_layer  */
  YYSYMBOL_module_call = 67,               /* module_call  */
  YYSYMBOL_layer_params = 68,              /* layer_params  */
  YYSYMBOL_layer_param_list = 69,          /* layer_param_list  */
  YYSYMBOL_expr = 70,                      /* expr  */
  YYSYMBOL_identifier_expr = 71,           /* identifier_expr  */
  YYSYMBOL_number = 72,                    /* number  */
  YYSYMBOL_array = 73,                     /* array  */
  YYSYMBOL_array_elements = 74,            /* array_elements  */
  YYSYMBOL_activation_value = 75           /* activation_value  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   144

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  33
/* YYNRULES -- Number of rules.  */
#define YYNRULES  77
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  162

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   297


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    96,    96,   104,   108,   115,   116,   122,   133,   139,
     143,   147,   156,   165,   168,   174,   179,   189,   195,   204,
     207,   214,   218,   225,   231,   236,   246,   247,   248,   249,
     250,   251,   252,   258,   298,   321,   346,   376,   382,   390,
     394,   401,   404,   412,   424,   427,   433,   439,   443,   447,
     451,   455,   459,   463,   467,   472,   476,   480,   484,   488,
     492,   496,   505,   506,   507,   508,   515,   519,   525,   528,
     534,   542,   546,   553,   554,   555,   556,   557
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NETWORK", "MODULE",
  "RETURN", "FROM", "INPUT", "SHAPE", "WEIGHTS", "CONV2D", "DENSE",
  "MAXPOOL", "AVGPOOL", "FLATTEN", "CONCAT", "BATCHNORM", "LAYERNORM",
  "FILTERS", "KERNEL", "ACTIVATION", "STRIDE", "PADDING", "POOL", "UNITS",
  "RELU", "SIGMOID", "TANH", "SOFTMAX", "LINEAR", "NUMBER", "FLOAT_NUM",
  "IDENTIFIER", "STRING_LIT", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
  "LPAREN", "RPAREN", "COLON", "COMMA", "ASSIGN", "$accept", "program",
  "definition_list", "definition", "network_def", "network_body",
  "module_def", "module_params", "param_list", "input_decl",
  "weights_decl", "statement_list", "statement", "assignment",
  "from_clause", "return_stmt", "layer_expr", "conv2d_layer",
  "dense_layer", "pool_layer", "flatten_layer", "concat_layer",
  "concat_args", "norm_layer", "module_call", "layer_params",
  "layer_param_list", "expr", "identifier_expr", "number", "array",
  "array_elements", "activation_value", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-69)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      26,   -18,   -16,    18,    26,   -69,   -69,   -69,   -14,     3,
     -69,   -69,   -69,    11,    10,   -69,    17,    16,    20,    23,
      21,   -69,   -69,   -69,   -69,   -69,    28,    32,    57,    33,
      58,   -69,   -69,    27,    37,    39,    40,    41,    42,    43,
      68,    69,    70,    71,   104,   -69,   -69,   -69,   -69,   -69,
     -69,   -69,    -1,    75,   -69,    73,    73,    73,    73,    74,
       8,    76,    77,    73,     8,   -69,    -4,   -69,    79,    24,
      78,    81,    82,    83,    84,    85,    86,    87,    88,    90,
      89,    92,    93,    94,   -69,   -69,   -69,   -69,   -69,   -69,
     -69,   -69,   -69,   -69,   -31,   -69,   -69,   -69,   -69,   -69,
     -69,   -69,    95,   -69,   -69,   -69,   -69,   -69,   -28,   -69,
       8,     8,     8,     8,     8,     8,     8,     8,   -69,    80,
     -69,   -69,   -69,   -69,     8,   -69,   -69,    24,   -69,   -69,
     -69,   -69,   -69,   -69,   -69,   -69,    96,    97,    98,    99,
     100,   101,   102,   103,   -69,   -69,     8,     8,     8,     8,
       8,     8,     8,     8,   -69,   -69,   -69,   -69,   -69,   -69,
     -69,   -69
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     2,     3,     5,     6,     0,     0,
       1,     4,     8,    13,     0,    15,     0,    14,     0,     0,
       0,     7,     9,    10,    11,    21,     0,     0,     0,     0,
       0,    19,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,    27,    28,    29,    30,
      31,    32,     0,     0,    18,    44,    44,    44,    44,     0,
       0,     0,     0,    44,     0,    22,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      45,     0,     0,     0,    37,    67,    73,    74,    75,    76,
      77,    68,    69,    66,     0,    39,    64,    62,    63,    65,
      41,    42,     0,    23,    25,    24,    12,    71,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
      34,    35,    36,    38,     0,    43,    70,     0,    47,    48,
      49,    50,    51,    52,    53,    46,     0,     0,     0,     0,
       0,     0,     0,     0,    40,    72,     0,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,    58,    59,    60,
      61,    54
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -69,   -69,   -69,   114,   -69,   -69,   -69,   -69,   -69,   -69,
     -69,   -69,    67,   -69,   -69,   -69,   -69,   -69,   -69,   -69,
     -69,    54,   -69,   -69,   -69,   -51,   -69,   -64,   -69,   -68,
      91,   -69,   -69
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     3,     4,     5,     6,    14,     7,    16,    17,    22,
      23,    52,    24,    25,    65,    68,    44,    45,    46,    47,
      48,    49,    94,    50,    51,    79,    80,    95,    96,    97,
      98,   108,    99
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
     103,   107,   105,    85,    66,    81,    82,    83,   123,   126,
     124,    40,   102,   127,     8,    85,     9,    18,    10,    19,
      12,    86,    87,    88,    89,    90,    91,    92,    93,     1,
       2,    20,    69,    86,    87,    88,    89,    90,    91,    92,
      93,    13,    20,    15,    69,    21,   128,   129,   130,   131,
     132,   133,   134,   135,    91,    92,    26,    27,    28,   145,
     144,    29,    31,    30,    32,    33,    34,    53,    35,    36,
      37,    38,    39,    40,    41,    42,    54,    55,    56,    57,
      58,    59,   154,   155,   156,   157,   158,   159,   160,   161,
      43,    71,    72,    73,    74,    75,    76,    77,   136,   137,
     138,   139,   140,   141,   142,    78,    60,    61,    62,    63,
      64,    69,   143,    84,   106,   100,   101,   109,    11,    67,
     104,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   125,     0,   146,   147,   148,   149,
     150,   151,   152,   153,    70
};

static const yytype_int16 yycheck[] =
{
      64,    69,    66,     7,     5,    56,    57,    58,    39,    37,
      41,    15,    63,    41,    32,     7,    32,     7,     0,     9,
      34,    25,    26,    27,    28,    29,    30,    31,    32,     3,
       4,    32,    36,    25,    26,    27,    28,    29,    30,    31,
      32,    38,    32,    32,    36,    35,   110,   111,   112,   113,
     114,   115,   116,   117,    30,    31,    39,    41,    38,   127,
     124,    38,    34,    42,    32,     8,    33,    40,    10,    11,
      12,    13,    14,    15,    16,    17,    39,    38,    38,    38,
      38,    38,   146,   147,   148,   149,   150,   151,   152,   153,
      32,    18,    19,    20,    21,    22,    23,    24,    18,    19,
      20,    21,    22,    23,    24,    32,    38,    38,    38,    38,
       6,    36,    32,    39,    35,    39,    39,    39,     4,    52,
      66,    40,    40,    40,    40,    40,    40,    40,    40,    39,
      41,    39,    39,    39,    39,    -1,    40,    40,    40,    40,
      40,    40,    40,    40,    53
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,    44,    45,    46,    47,    49,    32,    32,
       0,    46,    34,    38,    48,    32,    50,    51,     7,     9,
      32,    35,    52,    53,    55,    56,    39,    41,    38,    38,
      42,    34,    32,     8,    33,    10,    11,    12,    13,    14,
      15,    16,    17,    32,    59,    60,    61,    62,    63,    64,
      66,    67,    54,    40,    39,    38,    38,    38,    38,    38,
      38,    38,    38,    38,     6,    57,     5,    55,    58,    36,
      73,    18,    19,    20,    21,    22,    23,    24,    32,    68,
      69,    68,    68,    68,    39,     7,    25,    26,    27,    28,
      29,    30,    31,    32,    65,    70,    71,    72,    73,    75,
      39,    39,    68,    70,    64,    70,    35,    72,    74,    39,
      40,    40,    40,    40,    40,    40,    40,    40,    39,    41,
      39,    39,    39,    39,    41,    39,    37,    41,    70,    70,
      70,    70,    70,    70,    70,    70,    18,    19,    20,    21,
      22,    23,    24,    32,    70,    72,    40,    40,    40,    40,
      40,    40,    40,    40,    70,    70,    70,    70,    70,    70,
      70,    70
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    43,    44,    45,    45,    46,    46,    47,    48,    48,
      48,    48,    49,    50,    50,    51,    51,    52,    53,    54,
      54,    55,    56,    57,    58,    58,    59,    59,    59,    59,
      59,    59,    59,    60,    61,    62,    62,    63,    64,    65,
      65,    66,    66,    67,    68,    68,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    70,    70,    70,    70,    71,    71,    72,    72,
      73,    74,    74,    75,    75,    75,    75,    75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     5,     0,     2,
       2,     2,     9,     0,     1,     1,     3,     6,     4,     0,
       2,     1,     4,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     4,     4,     4,     4,     3,     4,     1,
       3,     3,     3,     4,     0,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     5,     5,     5,     5,     5,
       5,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: definition_list  */
#line 96 "src\\parser\\net_lang.y"
                    {
        ast_root = ast_program((yylsp[0]).first_line);
        ast_root->data.program.definitions = (yyvsp[0].list);
        (yyval.node) = ast_root;
    }
#line 1663 "build\\net_lang.tab.c"
    break;

  case 3: /* definition_list: definition  */
#line 104 "src\\parser\\net_lang.y"
               {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 1672 "build\\net_lang.tab.c"
    break;

  case 4: /* definition_list: definition_list definition  */
#line 108 "src\\parser\\net_lang.y"
                                 {
        ast_list_append((yyvsp[-1].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-1].list);
    }
#line 1681 "build\\net_lang.tab.c"
    break;

  case 5: /* definition: network_def  */
#line 115 "src\\parser\\net_lang.y"
                { (yyval.node) = (yyvsp[0].node); }
#line 1687 "build\\net_lang.tab.c"
    break;

  case 6: /* definition: module_def  */
#line 116 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1693 "build\\net_lang.tab.c"
    break;

  case 7: /* network_def: NETWORK IDENTIFIER LBRACE network_body RBRACE  */
#line 122 "src\\parser\\net_lang.y"
                                                  {
        (yyval.node) = ast_node_new(NODE_NETWORK, (yylsp[-4]).first_line);
        (yyval.node)->data.network.name = (yyvsp[-3].sval);  /* Transfer ownership, don't free */
        (yyval.node)->data.network.input = (yyvsp[-1].netbody)->input;
        (yyval.node)->data.network.weights = (yyvsp[-1].netbody)->weights;
        (yyval.node)->data.network.statements = (yyvsp[-1].netbody)->statements;
        free((yyvsp[-1].netbody));  /* Free the temporary NetworkBody struct only */
    }
#line 1706 "build\\net_lang.tab.c"
    break;

  case 8: /* network_body: %empty  */
#line 133 "src\\parser\\net_lang.y"
                {
        (yyval.netbody) = (NetworkBody*)ast_alloc(sizeof(NetworkBody));
        (yyval.netbody)->input = NULL;
        (yyval.netbody)->weights = NULL;
        (yyval.netbody)->statements = ast_list_new();
    }
#line 1717 "build\\net_lang.tab.c"
    break;

  case 9: /* network_body: network_body input_decl  */
#line 139 "src\\parser\\net_lang.y"
                              {
        (yyvsp[-1].netbody)->input = (yyvsp[0].node);
        (yyval.netbody) = (yyvsp[-1].netbody);
    }
#line 1726 "build\\net_lang.tab.c"
    break;

  case 10: /* network_body: network_body weights_decl  */
#line 143 "src\\parser\\net_lang.y"
                                {
        (yyvsp[-1].netbody)->weights = (yyvsp[0].node);
        (yyval.netbody) = (yyvsp[-1].netbody);
    }
#line 1735 "build\\net_lang.tab.c"
    break;

  case 11: /* network_body: network_body statement  */
#line 147 "src\\parser\\net_lang.y"
                             {
        ast_list_append((yyvsp[-1].netbody)->statements, (yyvsp[0].node));
        (yyval.netbody) = (yyvsp[-1].netbody);
    }
#line 1744 "build\\net_lang.tab.c"
    break;

  case 12: /* module_def: MODULE IDENTIFIER LPAREN module_params RPAREN LBRACE statement_list return_stmt RBRACE  */
#line 156 "src\\parser\\net_lang.y"
                                                                                           {
        (yyval.node) = ast_module((yyvsp[-7].sval), (yyvsp[-5].params), (yylsp[-8]).first_line);
        (yyval.node)->data.module.statements = (yyvsp[-2].list);
        (yyval.node)->data.module.return_stmt = (yyvsp[-1].node);
        free((yyvsp[-7].sval));
    }
#line 1755 "build\\net_lang.tab.c"
    break;

  case 13: /* module_params: %empty  */
#line 165 "src\\parser\\net_lang.y"
                {
        (yyval.params) = param_list_new();
    }
#line 1763 "build\\net_lang.tab.c"
    break;

  case 14: /* module_params: param_list  */
#line 168 "src\\parser\\net_lang.y"
                 {
        (yyval.params) = (yyvsp[0].params);
    }
#line 1771 "build\\net_lang.tab.c"
    break;

  case 15: /* param_list: IDENTIFIER  */
#line 174 "src\\parser\\net_lang.y"
               {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), (yyvsp[0].sval), NULL);
        free((yyvsp[0].sval));
    }
#line 1781 "build\\net_lang.tab.c"
    break;

  case 16: /* param_list: param_list COMMA IDENTIFIER  */
#line 179 "src\\parser\\net_lang.y"
                                  {
        param_list_add((yyvsp[-2].params), (yyvsp[0].sval), NULL);
        (yyval.params) = (yyvsp[-2].params);
        free((yyvsp[0].sval));
    }
#line 1791 "build\\net_lang.tab.c"
    break;

  case 17: /* input_decl: INPUT LPAREN SHAPE COLON array RPAREN  */
#line 189 "src\\parser\\net_lang.y"
                                          {
        (yyval.node) = ast_input((yyvsp[-1].node), (yylsp[-5]).first_line);
    }
#line 1799 "build\\net_lang.tab.c"
    break;

  case 18: /* weights_decl: WEIGHTS LPAREN STRING_LIT RPAREN  */
#line 195 "src\\parser\\net_lang.y"
                                     {
        (yyval.node) = ast_weights((yyvsp[-1].sval), (yylsp[-3]).first_line);
        free((yyvsp[-1].sval));
    }
#line 1808 "build\\net_lang.tab.c"
    break;

  case 19: /* statement_list: %empty  */
#line 204 "src\\parser\\net_lang.y"
                {
        (yyval.list) = ast_list_new();
    }
#line 1816 "build\\net_lang.tab.c"
    break;

  case 20: /* statement_list: statement_list statement  */
#line 207 "src\\parser\\net_lang.y"
                               {
        ast_list_append((yyvsp[-1].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-1].list);
    }
#line 1825 "build\\net_lang.tab.c"
    break;

  case 21: /* statement: assignment  */
#line 214 "src\\parser\\net_lang.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1831 "build\\net_lang.tab.c"
    break;

  case 22: /* assignment: IDENTIFIER ASSIGN layer_expr from_clause  */
#line 218 "src\\parser\\net_lang.y"
                                             {
        (yyval.node) = ast_assignment((yyvsp[-3].sval), (yyvsp[-1].node), (yyvsp[0].node), (yylsp[-3]).first_line);
        free((yyvsp[-3].sval));
    }
#line 1840 "build\\net_lang.tab.c"
    break;

  case 23: /* from_clause: FROM expr  */
#line 225 "src\\parser\\net_lang.y"
              {
        (yyval.node) = (yyvsp[0].node);
    }
#line 1848 "build\\net_lang.tab.c"
    break;

  case 24: /* return_stmt: RETURN expr  */
#line 231 "src\\parser\\net_lang.y"
                {
        ASTNode* node = ast_node_new(NODE_RETURN, (yylsp[-1]).first_line);
        node->data.return_stmt.value = (yyvsp[0].node);
        (yyval.node) = node;
    }
#line 1858 "build\\net_lang.tab.c"
    break;

  case 25: /* return_stmt: RETURN concat_layer  */
#line 236 "src\\parser\\net_lang.y"
                          {
        ASTNode* node = ast_node_new(NODE_RETURN, (yylsp[-1]).first_line);
        node->data.return_stmt.value = (yyvsp[0].node);
        (yyval.node) = node;
    }
#line 1868 "build\\net_lang.tab.c"
    break;

  case 26: /* layer_expr: conv2d_layer  */
#line 246 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1874 "build\\net_lang.tab.c"
    break;

  case 27: /* layer_expr: dense_layer  */
#line 247 "src\\parser\\net_lang.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1880 "build\\net_lang.tab.c"
    break;

  case 28: /* layer_expr: pool_layer  */
#line 248 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1886 "build\\net_lang.tab.c"
    break;

  case 29: /* layer_expr: flatten_layer  */
#line 249 "src\\parser\\net_lang.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 1892 "build\\net_lang.tab.c"
    break;

  case 30: /* layer_expr: concat_layer  */
#line 250 "src\\parser\\net_lang.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 1898 "build\\net_lang.tab.c"
    break;

  case 31: /* layer_expr: norm_layer  */
#line 251 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1904 "build\\net_lang.tab.c"
    break;

  case 32: /* layer_expr: module_call  */
#line 252 "src\\parser\\net_lang.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1910 "build\\net_lang.tab.c"
    break;

  case 33: /* conv2d_layer: CONV2D LPAREN layer_params RPAREN  */
#line 258 "src\\parser\\net_lang.y"
                                      {
        ASTNode* node = ast_node_new(NODE_CONV2D, (yylsp[-3]).first_line);
        /* Default values */
        node->data.conv2d.filters = 1;
        node->data.conv2d.kernel[0] = 3;
        node->data.conv2d.kernel[1] = 3;
        node->data.conv2d.stride = 1;
        node->data.conv2d.padding = 0;
        node->data.conv2d.activation = ACT_NONE;
        
        /* Process parameters */
        Parameter* p = (yyvsp[-1].params)->head;
        while (p) {
            if (strcmp(p->name, "filters") == 0 && p->value) {
                node->data.conv2d.filters = p->value->data.number.ival;
            } else if (strcmp(p->name, "kernel") == 0 && p->value) {
                ASTNode* arr = p->value;
                if (arr->type == NODE_ARRAY && arr->data.array.elements) {
                    ASTNode* e = arr->data.array.elements->head;
                    if (e) { node->data.conv2d.kernel[0] = e->data.number.ival; e = e->next; }
                    if (e) { node->data.conv2d.kernel[1] = e->data.number.ival; }
                }
            } else if (strcmp(p->name, "stride") == 0 && p->value) {
                node->data.conv2d.stride = p->value->data.number.ival;
            } else if (strcmp(p->name, "padding") == 0 && p->value) {
                node->data.conv2d.padding = p->value->data.number.ival;
            } else if (strcmp(p->name, "activation") == 0 && p->value) {
                if (p->value->type == NODE_IDENTIFIER) {
                    node->data.conv2d.activation = activation_from_string(p->value->data.identifier.name);
                }
            }
            p = p->next;
        }
        (yyval.node) = node;
    }
#line 1950 "build\\net_lang.tab.c"
    break;

  case 34: /* dense_layer: DENSE LPAREN layer_params RPAREN  */
#line 298 "src\\parser\\net_lang.y"
                                     {
        ASTNode* node = ast_node_new(NODE_DENSE, (yylsp[-3]).first_line);
        node->data.dense.units = 1;
        node->data.dense.activation = ACT_NONE;
        
        Parameter* p = (yyvsp[-1].params)->head;
        while (p) {
            if (strcmp(p->name, "units") == 0 && p->value) {
                node->data.dense.units = p->value->data.number.ival;
            } else if (strcmp(p->name, "activation") == 0 && p->value) {
                if (p->value->type == NODE_IDENTIFIER) {
                    node->data.dense.activation = activation_from_string(p->value->data.identifier.name);
                }
            }
            p = p->next;
        }
        (yyval.node) = node;
    }
#line 1973 "build\\net_lang.tab.c"
    break;

  case 35: /* pool_layer: MAXPOOL LPAREN layer_params RPAREN  */
#line 321 "src\\parser\\net_lang.y"
                                       {
        ASTNode* node = ast_node_new(NODE_MAXPOOL, (yylsp[-3]).first_line);
        node->data.pooling.pool[0] = 2;
        node->data.pooling.pool[1] = 2;
        node->data.pooling.stride = 0;  /* 0 means same as pool size */
        node->data.pooling.padding = 0;
        
        Parameter* p = (yyvsp[-1].params)->head;
        while (p) {
            if (strcmp(p->name, "pool") == 0 && p->value) {
                ASTNode* arr = p->value;
                if (arr->type == NODE_ARRAY && arr->data.array.elements) {
                    ASTNode* e = arr->data.array.elements->head;
                    if (e) { node->data.pooling.pool[0] = e->data.number.ival; e = e->next; }
                    if (e) { node->data.pooling.pool[1] = e->data.number.ival; }
                }
            } else if (strcmp(p->name, "stride") == 0 && p->value) {
                node->data.pooling.stride = p->value->data.number.ival;
            } else if (strcmp(p->name, "padding") == 0 && p->value) {
                node->data.pooling.padding = p->value->data.number.ival;
            }
            p = p->next;
        }
        (yyval.node) = node;
    }
#line 2003 "build\\net_lang.tab.c"
    break;

  case 36: /* pool_layer: AVGPOOL LPAREN layer_params RPAREN  */
#line 346 "src\\parser\\net_lang.y"
                                         {
        ASTNode* node = ast_node_new(NODE_AVGPOOL, (yylsp[-3]).first_line);
        node->data.pooling.pool[0] = 2;
        node->data.pooling.pool[1] = 2;
        node->data.pooling.stride = 0;
        node->data.pooling.padding = 0;
        
        Parameter* p = (yyvsp[-1].params)->head;
        while (p) {
            if (strcmp(p->name, "pool") == 0 && p->value) {
                ASTNode* arr = p->value;
                if (arr->type == NODE_ARRAY && arr->data.array.elements) {
                    ASTNode* e = arr->data.array.elements->head;
                    if (e) { node->data.pooling.pool[0] = e->data.number.ival; e = e->next; }
                    if (e) { node->data.pooling.pool[1] = e->data.number.ival; }
                }
            } else if (strcmp(p->name, "stride") == 0 && p->value) {
                node->data.pooling.stride = p->value->data.number.ival;
            } else if (strcmp(p->name, "padding") == 0 && p->value) {
                node->data.pooling.padding = p->value->data.number.ival;
            }
            p = p->next;
        }
        (yyval.node) = node;
    }
#line 2033 "build\\net_lang.tab.c"
    break;

  case 37: /* flatten_layer: FLATTEN LPAREN RPAREN  */
#line 376 "src\\parser\\net_lang.y"
                          {
        (yyval.node) = ast_node_new(NODE_FLATTEN, (yylsp[-2]).first_line);
    }
#line 2041 "build\\net_lang.tab.c"
    break;

  case 38: /* concat_layer: CONCAT LPAREN concat_args RPAREN  */
#line 382 "src\\parser\\net_lang.y"
                                     {
        ASTNode* node = ast_node_new(NODE_CONCAT, (yylsp[-3]).first_line);
        node->data.concat.inputs = (yyvsp[-1].list);
        (yyval.node) = node;
    }
#line 2051 "build\\net_lang.tab.c"
    break;

  case 39: /* concat_args: expr  */
#line 390 "src\\parser\\net_lang.y"
         {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 2060 "build\\net_lang.tab.c"
    break;

  case 40: /* concat_args: concat_args COMMA expr  */
#line 394 "src\\parser\\net_lang.y"
                             {
        ast_list_append((yyvsp[-2].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-2].list);
    }
#line 2069 "build\\net_lang.tab.c"
    break;

  case 41: /* norm_layer: BATCHNORM LPAREN RPAREN  */
#line 401 "src\\parser\\net_lang.y"
                            {
        (yyval.node) = ast_node_new(NODE_BATCHNORM, (yylsp[-2]).first_line);
    }
#line 2077 "build\\net_lang.tab.c"
    break;

  case 42: /* norm_layer: LAYERNORM LPAREN RPAREN  */
#line 404 "src\\parser\\net_lang.y"
                              {
        (yyval.node) = ast_node_new(NODE_LAYERNORM, (yylsp[-2]).first_line);
    }
#line 2085 "build\\net_lang.tab.c"
    break;

  case 43: /* module_call: IDENTIFIER LPAREN layer_params RPAREN  */
#line 412 "src\\parser\\net_lang.y"
                                          {
        ASTNode* node = ast_node_new(NODE_MODULE_CALL, (yylsp[-3]).first_line);
        node->data.module_call.module_name = strdup((yyvsp[-3].sval));
        node->data.module_call.args = (yyvsp[-1].params);
        free((yyvsp[-3].sval));
        (yyval.node) = node;
    }
#line 2097 "build\\net_lang.tab.c"
    break;

  case 44: /* layer_params: %empty  */
#line 424 "src\\parser\\net_lang.y"
                {
        (yyval.params) = param_list_new();
    }
#line 2105 "build\\net_lang.tab.c"
    break;

  case 45: /* layer_params: layer_param_list  */
#line 427 "src\\parser\\net_lang.y"
                       {
        (yyval.params) = (yyvsp[0].params);
    }
#line 2113 "build\\net_lang.tab.c"
    break;

  case 46: /* layer_param_list: IDENTIFIER COLON expr  */
#line 433 "src\\parser\\net_lang.y"
                          {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), (yyvsp[-2].sval), (yyvsp[0].node));
        free((yyvsp[-2].sval));
    }
#line 2123 "build\\net_lang.tab.c"
    break;

  case 47: /* layer_param_list: FILTERS COLON expr  */
#line 439 "src\\parser\\net_lang.y"
                         {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "filters", (yyvsp[0].node));
    }
#line 2132 "build\\net_lang.tab.c"
    break;

  case 48: /* layer_param_list: KERNEL COLON expr  */
#line 443 "src\\parser\\net_lang.y"
                        {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "kernel", (yyvsp[0].node));
    }
#line 2141 "build\\net_lang.tab.c"
    break;

  case 49: /* layer_param_list: ACTIVATION COLON expr  */
#line 447 "src\\parser\\net_lang.y"
                            {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "activation", (yyvsp[0].node));
    }
#line 2150 "build\\net_lang.tab.c"
    break;

  case 50: /* layer_param_list: STRIDE COLON expr  */
#line 451 "src\\parser\\net_lang.y"
                        {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "stride", (yyvsp[0].node));
    }
#line 2159 "build\\net_lang.tab.c"
    break;

  case 51: /* layer_param_list: PADDING COLON expr  */
#line 455 "src\\parser\\net_lang.y"
                         {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "padding", (yyvsp[0].node));
    }
#line 2168 "build\\net_lang.tab.c"
    break;

  case 52: /* layer_param_list: POOL COLON expr  */
#line 459 "src\\parser\\net_lang.y"
                      {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "pool", (yyvsp[0].node));
    }
#line 2177 "build\\net_lang.tab.c"
    break;

  case 53: /* layer_param_list: UNITS COLON expr  */
#line 463 "src\\parser\\net_lang.y"
                       {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "units", (yyvsp[0].node));
    }
#line 2186 "build\\net_lang.tab.c"
    break;

  case 54: /* layer_param_list: layer_param_list COMMA IDENTIFIER COLON expr  */
#line 467 "src\\parser\\net_lang.y"
                                                   {
        param_list_add((yyvsp[-4].params), (yyvsp[-2].sval), (yyvsp[0].node));
        free((yyvsp[-2].sval));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2196 "build\\net_lang.tab.c"
    break;

  case 55: /* layer_param_list: layer_param_list COMMA FILTERS COLON expr  */
#line 472 "src\\parser\\net_lang.y"
                                                {
        param_list_add((yyvsp[-4].params), "filters", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2205 "build\\net_lang.tab.c"
    break;

  case 56: /* layer_param_list: layer_param_list COMMA KERNEL COLON expr  */
#line 476 "src\\parser\\net_lang.y"
                                               {
        param_list_add((yyvsp[-4].params), "kernel", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2214 "build\\net_lang.tab.c"
    break;

  case 57: /* layer_param_list: layer_param_list COMMA ACTIVATION COLON expr  */
#line 480 "src\\parser\\net_lang.y"
                                                   {
        param_list_add((yyvsp[-4].params), "activation", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2223 "build\\net_lang.tab.c"
    break;

  case 58: /* layer_param_list: layer_param_list COMMA STRIDE COLON expr  */
#line 484 "src\\parser\\net_lang.y"
                                               {
        param_list_add((yyvsp[-4].params), "stride", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2232 "build\\net_lang.tab.c"
    break;

  case 59: /* layer_param_list: layer_param_list COMMA PADDING COLON expr  */
#line 488 "src\\parser\\net_lang.y"
                                                {
        param_list_add((yyvsp[-4].params), "padding", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2241 "build\\net_lang.tab.c"
    break;

  case 60: /* layer_param_list: layer_param_list COMMA POOL COLON expr  */
#line 492 "src\\parser\\net_lang.y"
                                             {
        param_list_add((yyvsp[-4].params), "pool", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2250 "build\\net_lang.tab.c"
    break;

  case 61: /* layer_param_list: layer_param_list COMMA UNITS COLON expr  */
#line 496 "src\\parser\\net_lang.y"
                                              {
        param_list_add((yyvsp[-4].params), "units", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2259 "build\\net_lang.tab.c"
    break;

  case 62: /* expr: number  */
#line 505 "src\\parser\\net_lang.y"
           { (yyval.node) = (yyvsp[0].node); }
#line 2265 "build\\net_lang.tab.c"
    break;

  case 63: /* expr: array  */
#line 506 "src\\parser\\net_lang.y"
            { (yyval.node) = (yyvsp[0].node); }
#line 2271 "build\\net_lang.tab.c"
    break;

  case 64: /* expr: identifier_expr  */
#line 507 "src\\parser\\net_lang.y"
                      { (yyval.node) = (yyvsp[0].node); }
#line 2277 "build\\net_lang.tab.c"
    break;

  case 65: /* expr: activation_value  */
#line 508 "src\\parser\\net_lang.y"
                       {
        /* Convert activation token to identifier node */
        (yyval.node) = ast_identifier(activation_to_string((yyvsp[0].activation)), (yylsp[0]).first_line);
    }
#line 2286 "build\\net_lang.tab.c"
    break;

  case 66: /* identifier_expr: IDENTIFIER  */
#line 515 "src\\parser\\net_lang.y"
               {
        (yyval.node) = ast_identifier((yyvsp[0].sval), (yylsp[0]).first_line);
        free((yyvsp[0].sval));
    }
#line 2295 "build\\net_lang.tab.c"
    break;

  case 67: /* identifier_expr: INPUT  */
#line 519 "src\\parser\\net_lang.y"
            {
        (yyval.node) = ast_identifier("input", (yylsp[0]).first_line);
    }
#line 2303 "build\\net_lang.tab.c"
    break;

  case 68: /* number: NUMBER  */
#line 525 "src\\parser\\net_lang.y"
           {
        (yyval.node) = ast_number_int((yyvsp[0].ival), (yylsp[0]).first_line);
    }
#line 2311 "build\\net_lang.tab.c"
    break;

  case 69: /* number: FLOAT_NUM  */
#line 528 "src\\parser\\net_lang.y"
                {
        (yyval.node) = ast_number_float((yyvsp[0].fval), (yylsp[0]).first_line);
    }
#line 2319 "build\\net_lang.tab.c"
    break;

  case 70: /* array: LBRACKET array_elements RBRACKET  */
#line 534 "src\\parser\\net_lang.y"
                                     {
        ASTNode* node = ast_array((yylsp[-2]).first_line);
        node->data.array.elements = (yyvsp[-1].list);
        (yyval.node) = node;
    }
#line 2329 "build\\net_lang.tab.c"
    break;

  case 71: /* array_elements: number  */
#line 542 "src\\parser\\net_lang.y"
           {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 2338 "build\\net_lang.tab.c"
    break;

  case 72: /* array_elements: array_elements COMMA number  */
#line 546 "src\\parser\\net_lang.y"
                                  {
        ast_list_append((yyvsp[-2].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-2].list);
    }
#line 2347 "build\\net_lang.tab.c"
    break;

  case 73: /* activation_value: RELU  */
#line 553 "src\\parser\\net_lang.y"
         { (yyval.activation) = ACT_RELU; free((yyvsp[0].sval)); }
#line 2353 "build\\net_lang.tab.c"
    break;

  case 74: /* activation_value: SIGMOID  */
#line 554 "src\\parser\\net_lang.y"
              { (yyval.activation) = ACT_SIGMOID; free((yyvsp[0].sval)); }
#line 2359 "build\\net_lang.tab.c"
    break;

  case 75: /* activation_value: TANH  */
#line 555 "src\\parser\\net_lang.y"
           { (yyval.activation) = ACT_TANH; free((yyvsp[0].sval)); }
#line 2365 "build\\net_lang.tab.c"
    break;

  case 76: /* activation_value: SOFTMAX  */
#line 556 "src\\parser\\net_lang.y"
              { (yyval.activation) = ACT_SOFTMAX; free((yyvsp[0].sval)); }
#line 2371 "build\\net_lang.tab.c"
    break;

  case 77: /* activation_value: LINEAR  */
#line 557 "src\\parser\\net_lang.y"
             { (yyval.activation) = ACT_LINEAR; free((yyvsp[0].sval)); }
#line 2377 "build\\net_lang.tab.c"
    break;


#line 2381 "build\\net_lang.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 560 "src\\parser\\net_lang.y"


/* ========== ERROR HANDLING ========== */

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    fprintf(stderr, "Near token: '%s'\n", yytext);
}

/* ========== MAIN FUNCTION ========== */

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
    
    printf("NetLang Parser v0.1\n");
    printf("Parsing: %s\n", argv[1]);
    printf("========================================\n\n");
    
    int result = yyparse();
    
    fclose(yyin);
    
    if (result == 0 && ast_root) {
        printf("\n✓ Parse successful!\n\n");
        printf("========== AST DUMP ==========\n\n");
        ast_print(ast_root, 0);
        
        /* Cleanup */
        ast_free(ast_root);
    } else {
        fprintf(stderr, "\n✗ Parse failed with errors.\n");
        return 1;
    }
    
    return 0;
}
