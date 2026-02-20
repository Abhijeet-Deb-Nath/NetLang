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
 * NetLang Parser - Phase 2 & 3
 * Builds Abstract Syntax Tree from token stream
 * Performs semantic analysis and type checking
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
#include "../semantic/semantic.h"

/* Flex/Bison integration */
extern int yylex(void);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

void yyerror(const char* s);

/* Global AST root - populated after successful parse */
ASTNode* ast_root = NULL;


#line 106 "build\\net_lang.tab.c"

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
  YYSYMBOL_module_list = 45,               /* module_list  */
  YYSYMBOL_network_def = 46,               /* network_def  */
  YYSYMBOL_network_body = 47,              /* network_body  */
  YYSYMBOL_module_def = 48,                /* module_def  */
  YYSYMBOL_module_params = 49,             /* module_params  */
  YYSYMBOL_param_list = 50,                /* param_list  */
  YYSYMBOL_input_decl = 51,                /* input_decl  */
  YYSYMBOL_weights_decl = 52,              /* weights_decl  */
  YYSYMBOL_statement_list = 53,            /* statement_list  */
  YYSYMBOL_statement_list_nonempty = 54,   /* statement_list_nonempty  */
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
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   184

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  33
/* YYNRULES -- Number of rules.  */
#define YYNRULES  74
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  207

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
       0,    99,    99,   106,   116,   120,   129,   140,   151,   160,
     163,   169,   174,   184,   190,   199,   202,   209,   213,   220,
     224,   231,   237,   247,   248,   249,   250,   251,   252,   253,
     259,   287,   303,   320,   342,   348,   356,   360,   367,   370,
     378,   390,   393,   399,   405,   409,   413,   417,   421,   425,
     429,   433,   438,   442,   446,   450,   454,   458,   462,   471,
     472,   473,   474,   481,   485,   491,   494,   500,   508,   512,
     519,   520,   521,   522,   523
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
  "module_list", "network_def", "network_body", "module_def",
  "module_params", "param_list", "input_decl", "weights_decl",
  "statement_list", "statement_list_nonempty", "statement", "assignment",
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

#define YYPACT_NINF (-171)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      29,   -16,   -14,    42,    29,  -171,  -171,     6,    35,  -171,
    -171,  -171,    67,    43,    38,    45,    68,  -171,    39,    51,
      73,  -171,    55,    62,    61,    64,    57,    65,    58,    62,
    -171,  -171,  -171,  -171,    63,    66,    -3,  -171,    -2,    16,
      69,  -171,    71,    72,    74,    75,    76,    77,    78,    79,
      80,    95,  -171,  -171,  -171,  -171,  -171,  -171,  -171,    -5,
    -171,    87,  -171,  -171,  -171,     4,  -171,    84,    99,    81,
      88,    85,    -5,    86,    89,    40,    -5,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,    16,    90,    91,    92,    93,  -171,   -24,  -171,
    -171,  -171,    94,    96,    97,    98,   100,   102,   103,   104,
     106,   105,  -171,  -171,    -5,    -5,    63,    63,  -171,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,  -171,    47,
     107,   108,   109,   110,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    83,   126,   139,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,   121,   122,   123,   124,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,    63,    10,    -5,    -5,   125,
     128,   127,   129,   144,  -171,   147,   149,   132,   133,   134,
      -5,    -5,    -5,   135,   136,   138,   156,  -171,  -171,   140,
      -5,   141,   159,   143,    10,   142,  -171
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     2,     4,     0,     0,     1,
       3,     5,     0,     9,     0,     0,     0,    11,     0,    10,
       0,     6,     0,     0,     0,     0,     0,     0,     0,     7,
      17,    19,    15,    12,     0,     0,     0,    18,     0,     0,
       0,    14,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,    24,    25,    26,    27,    28,    29,     0,
      16,     0,    65,    66,    68,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,    41,     0,    20,    64,    70,
      71,    72,    73,    74,    63,    22,    61,    59,    60,    62,
       8,    67,     0,     0,     0,     0,     0,    34,     0,    36,
      38,    39,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    42,    21,    69,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,    37,    44,    45,    46,    47,    48,
      49,    50,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,    51,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    32,    33,     0,
       0,     0,     0,     0,     0,     0,    30
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -171,  -171,  -171,   131,  -171,   180,  -171,  -171,  -171,  -171,
    -171,  -171,   -10,  -171,  -171,  -171,  -171,  -171,  -171,  -171,
    -171,  -171,  -171,  -171,  -171,  -171,  -171,   -71,  -171,   -35,
     -34,  -171,  -170
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     3,     4,     5,    15,     6,    18,    19,    16,    23,
      38,    29,    30,    31,    77,    61,    51,    52,    53,    54,
      55,    56,    98,    57,    58,   110,   111,    85,    86,    87,
      88,    65,    89
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      40,    99,    78,    59,    64,   112,   180,    42,    43,    44,
      45,    46,    47,    48,    49,   118,     7,   119,     8,    37,
      79,    80,    81,    82,    83,    62,    63,    84,    60,    50,
      28,    39,     1,     2,   205,    79,    80,    81,    82,    83,
      12,    91,     9,   130,   131,    92,    62,    63,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   113,   102,   103,
     104,   105,   106,   107,   108,   143,   144,   145,   146,   147,
     148,   149,   109,    13,    14,    17,    20,    22,    24,   150,
      21,    26,   132,   133,   167,   168,   169,   170,   171,   172,
     173,   174,    25,    27,    28,    32,    33,    34,    35,    39,
      36,    76,    93,   164,    95,    41,   181,   182,    66,    67,
      68,    96,    69,    70,    71,    72,    73,    74,    75,   193,
     194,   195,    90,    94,    97,   100,     0,     0,   101,   201,
     114,   115,   116,   117,   120,    10,   121,   122,   123,   163,
     124,   179,   125,   126,   127,   128,   129,   165,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     166,   175,   176,   177,   178,   187,   183,   184,   185,   188,
     186,   189,   190,   191,   192,   197,   196,   198,   199,   203,
     200,   206,   202,   204,    11
};

static const yytype_int16 yycheck[] =
{
      34,    72,     7,     5,    39,    76,   176,    10,    11,    12,
      13,    14,    15,    16,    17,    39,    32,    41,    32,    29,
      25,    26,    27,    28,    29,    30,    31,    32,    38,    32,
      32,    36,     3,     4,   204,    25,    26,    27,    28,    29,
      34,    37,     0,   114,   115,    41,    30,    31,   119,   120,
     121,   122,   123,   124,   125,   126,   127,    92,    18,    19,
      20,    21,    22,    23,    24,    18,    19,    20,    21,    22,
      23,    24,    32,    38,     7,    32,    38,     9,    39,    32,
      35,     8,   116,   117,   155,   156,   157,   158,   159,   160,
     161,   162,    41,    38,    32,    34,    32,    40,    33,    36,
      42,     6,    18,    20,    23,    39,   177,   178,    39,    38,
      38,    23,    38,    38,    38,    38,    38,    38,    38,   190,
     191,   192,    35,    24,    39,    39,    -1,    -1,    39,   200,
      40,    40,    40,    40,    40,     4,    40,    40,    40,    19,
      40,   175,    40,    40,    40,    39,    41,    21,    41,    41,
      41,    41,    40,    40,    40,    40,    40,    40,    40,    40,
      21,    40,    40,    40,    40,    21,    41,    39,    41,    22,
      41,    22,    40,    40,    40,    39,    41,    39,    22,    20,
      40,    39,    41,    40,     4
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,    44,    45,    46,    48,    32,    32,     0,
      46,    48,    34,    38,     7,    47,    51,    32,    49,    50,
      38,    35,     9,    52,    39,    41,     8,    38,    32,    54,
      55,    56,    34,    32,    40,    33,    42,    55,    53,    36,
      73,    39,    10,    11,    12,    13,    14,    15,    16,    17,
      32,    59,    60,    61,    62,    63,    64,    66,    67,     5,
      55,    58,    30,    31,    72,    74,    39,    38,    38,    38,
      38,    38,    38,    38,    38,    38,     6,    57,     7,    25,
      26,    27,    28,    29,    32,    70,    71,    72,    73,    75,
      35,    37,    41,    18,    24,    23,    23,    39,    65,    70,
      39,    39,    18,    19,    20,    21,    22,    23,    24,    32,
      68,    69,    70,    72,    40,    40,    40,    40,    39,    41,
      40,    40,    40,    40,    40,    40,    40,    40,    39,    41,
      70,    70,    73,    73,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    18,    19,    20,    21,    22,    23,    24,
      32,    41,    41,    41,    41,    40,    40,    40,    40,    40,
      40,    40,    40,    19,    20,    21,    21,    70,    70,    70,
      70,    70,    70,    70,    70,    40,    40,    40,    40,    73,
      75,    70,    70,    41,    39,    41,    41,    21,    22,    22,
      40,    40,    40,    70,    70,    70,    41,    39,    39,    22,
      40,    70,    41,    20,    40,    75,    39
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    43,    44,    44,    45,    45,    46,    47,    48,    49,
      49,    50,    50,    51,    52,    53,    53,    54,    54,    55,
      56,    57,    58,    59,    59,    59,    59,    59,    59,    59,
      60,    61,    62,    62,    63,    64,    65,    65,    66,    66,
      67,    68,    68,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    70,
      70,    70,    70,    71,    71,    72,    72,    73,    74,    74,
      75,    75,    75,    75,    75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     5,     3,     9,     0,
       1,     1,     3,     6,     4,     0,     2,     1,     2,     1,
       4,     2,     2,     1,     1,     1,     1,     1,     1,     1,
      22,    10,    14,    14,     3,     4,     1,     3,     3,     3,
       4,     0,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     5,     5,     5,     5,     5,     5,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     1,     1
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
  case 2: /* program: network_def  */
#line 99 "src\\parser\\net_lang.y"
                {
        ast_root = ast_program((yylsp[0]).first_line);
        ASTList* defs = ast_list_new();
        ast_list_append(defs, (yyvsp[0].node));
        ast_root->data.program.definitions = defs;
        (yyval.node) = ast_root;
    }
#line 1687 "build\\net_lang.tab.c"
    break;

  case 3: /* program: module_list network_def  */
#line 106 "src\\parser\\net_lang.y"
                              {
        ast_root = ast_program((yylsp[-1]).first_line);
        ast_list_append((yyvsp[-1].list), (yyvsp[0].node));  /* Add network to end of module list */
        ast_root->data.program.definitions = (yyvsp[-1].list);
        (yyval.node) = ast_root;
    }
#line 1698 "build\\net_lang.tab.c"
    break;

  case 4: /* module_list: module_def  */
#line 116 "src\\parser\\net_lang.y"
               {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 1707 "build\\net_lang.tab.c"
    break;

  case 5: /* module_list: module_list module_def  */
#line 120 "src\\parser\\net_lang.y"
                             {
        ast_list_append((yyvsp[-1].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-1].list);
    }
#line 1716 "build\\net_lang.tab.c"
    break;

  case 6: /* network_def: NETWORK IDENTIFIER LBRACE network_body RBRACE  */
#line 129 "src\\parser\\net_lang.y"
                                                  {
        (yyval.node) = ast_node_new(NODE_NETWORK, (yylsp[-4]).first_line);
        (yyval.node)->data.network.name = (yyvsp[-3].sval);  /* Transfer ownership, don't free */
        (yyval.node)->data.network.input = (yyvsp[-1].netbody)->input;
        (yyval.node)->data.network.weights = (yyvsp[-1].netbody)->weights;
        (yyval.node)->data.network.statements = (yyvsp[-1].netbody)->statements;
        free((yyvsp[-1].netbody));  /* Free the temporary NetworkBody struct only */
    }
#line 1729 "build\\net_lang.tab.c"
    break;

  case 7: /* network_body: input_decl weights_decl statement_list_nonempty  */
#line 140 "src\\parser\\net_lang.y"
                                                    {
        (yyval.netbody) = (NetworkBody*)ast_alloc(sizeof(NetworkBody));
        (yyval.netbody)->input = (yyvsp[-2].node);
        (yyval.netbody)->weights = (yyvsp[-1].node);
        (yyval.netbody)->statements = (yyvsp[0].list);
    }
#line 1740 "build\\net_lang.tab.c"
    break;

  case 8: /* module_def: MODULE IDENTIFIER LPAREN module_params RPAREN LBRACE statement_list return_stmt RBRACE  */
#line 151 "src\\parser\\net_lang.y"
                                                                                           {
        (yyval.node) = ast_module((yyvsp[-7].sval), (yyvsp[-5].params), (yylsp[-8]).first_line);
        (yyval.node)->data.module.statements = (yyvsp[-2].list);
        (yyval.node)->data.module.return_stmt = (yyvsp[-1].node);
        free((yyvsp[-7].sval));
    }
#line 1751 "build\\net_lang.tab.c"
    break;

  case 9: /* module_params: %empty  */
#line 160 "src\\parser\\net_lang.y"
                {
        (yyval.params) = param_list_new();
    }
#line 1759 "build\\net_lang.tab.c"
    break;

  case 10: /* module_params: param_list  */
#line 163 "src\\parser\\net_lang.y"
                 {
        (yyval.params) = (yyvsp[0].params);
    }
#line 1767 "build\\net_lang.tab.c"
    break;

  case 11: /* param_list: IDENTIFIER  */
#line 169 "src\\parser\\net_lang.y"
               {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), (yyvsp[0].sval), NULL);
        free((yyvsp[0].sval));
    }
#line 1777 "build\\net_lang.tab.c"
    break;

  case 12: /* param_list: param_list COMMA IDENTIFIER  */
#line 174 "src\\parser\\net_lang.y"
                                  {
        param_list_add((yyvsp[-2].params), (yyvsp[0].sval), NULL);
        (yyval.params) = (yyvsp[-2].params);
        free((yyvsp[0].sval));
    }
#line 1787 "build\\net_lang.tab.c"
    break;

  case 13: /* input_decl: INPUT LPAREN SHAPE COLON array RPAREN  */
#line 184 "src\\parser\\net_lang.y"
                                          {
        (yyval.node) = ast_input((yyvsp[-1].node), (yylsp[-5]).first_line);
    }
#line 1795 "build\\net_lang.tab.c"
    break;

  case 14: /* weights_decl: WEIGHTS LPAREN STRING_LIT RPAREN  */
#line 190 "src\\parser\\net_lang.y"
                                     {
        (yyval.node) = ast_weights((yyvsp[-1].sval), (yylsp[-3]).first_line);
        free((yyvsp[-1].sval));
    }
#line 1804 "build\\net_lang.tab.c"
    break;

  case 15: /* statement_list: %empty  */
#line 199 "src\\parser\\net_lang.y"
                {
        (yyval.list) = ast_list_new();
    }
#line 1812 "build\\net_lang.tab.c"
    break;

  case 16: /* statement_list: statement_list statement  */
#line 202 "src\\parser\\net_lang.y"
                               {
        ast_list_append((yyvsp[-1].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-1].list);
    }
#line 1821 "build\\net_lang.tab.c"
    break;

  case 17: /* statement_list_nonempty: statement  */
#line 209 "src\\parser\\net_lang.y"
              {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 1830 "build\\net_lang.tab.c"
    break;

  case 18: /* statement_list_nonempty: statement_list_nonempty statement  */
#line 213 "src\\parser\\net_lang.y"
                                        {
        ast_list_append((yyvsp[-1].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-1].list);
    }
#line 1839 "build\\net_lang.tab.c"
    break;

  case 19: /* statement: assignment  */
#line 220 "src\\parser\\net_lang.y"
               { (yyval.node) = (yyvsp[0].node); }
#line 1845 "build\\net_lang.tab.c"
    break;

  case 20: /* assignment: IDENTIFIER ASSIGN layer_expr from_clause  */
#line 224 "src\\parser\\net_lang.y"
                                             {
        (yyval.node) = ast_assignment((yyvsp[-3].sval), (yyvsp[-1].node), (yyvsp[0].node), (yylsp[-3]).first_line);
        free((yyvsp[-3].sval));
    }
#line 1854 "build\\net_lang.tab.c"
    break;

  case 21: /* from_clause: FROM expr  */
#line 231 "src\\parser\\net_lang.y"
              {
        (yyval.node) = (yyvsp[0].node);
    }
#line 1862 "build\\net_lang.tab.c"
    break;

  case 22: /* return_stmt: RETURN expr  */
#line 237 "src\\parser\\net_lang.y"
                {
        ASTNode* node = ast_node_new(NODE_RETURN, (yylsp[-1]).first_line);
        node->data.return_stmt.value = (yyvsp[0].node);
        (yyval.node) = node;
    }
#line 1872 "build\\net_lang.tab.c"
    break;

  case 23: /* layer_expr: conv2d_layer  */
#line 247 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1878 "build\\net_lang.tab.c"
    break;

  case 24: /* layer_expr: dense_layer  */
#line 248 "src\\parser\\net_lang.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1884 "build\\net_lang.tab.c"
    break;

  case 25: /* layer_expr: pool_layer  */
#line 249 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1890 "build\\net_lang.tab.c"
    break;

  case 26: /* layer_expr: flatten_layer  */
#line 250 "src\\parser\\net_lang.y"
                    { (yyval.node) = (yyvsp[0].node); }
#line 1896 "build\\net_lang.tab.c"
    break;

  case 27: /* layer_expr: concat_layer  */
#line 251 "src\\parser\\net_lang.y"
                   { (yyval.node) = (yyvsp[0].node); }
#line 1902 "build\\net_lang.tab.c"
    break;

  case 28: /* layer_expr: norm_layer  */
#line 252 "src\\parser\\net_lang.y"
                 { (yyval.node) = (yyvsp[0].node); }
#line 1908 "build\\net_lang.tab.c"
    break;

  case 29: /* layer_expr: module_call  */
#line 253 "src\\parser\\net_lang.y"
                  { (yyval.node) = (yyvsp[0].node); }
#line 1914 "build\\net_lang.tab.c"
    break;

  case 30: /* conv2d_layer: CONV2D LPAREN FILTERS COLON expr COMMA KERNEL COLON array COMMA STRIDE COLON expr COMMA PADDING COLON expr COMMA ACTIVATION COLON activation_value RPAREN  */
#line 265 "src\\parser\\net_lang.y"
           {
        ASTNode* node = ast_node_new(NODE_CONV2D, (yylsp[-21]).first_line);
        
        /* Strict positional assignment */
        node->data.conv2d.filters = (yyvsp[-17].node)->data.number.ival;
        
        /* Parse kernel array */
        ASTNode* e = (yyvsp[-13].node)->data.array.elements->head;
        if (e) { node->data.conv2d.kernel[0] = e->data.number.ival; e = e->next; }
        if (e) { node->data.conv2d.kernel[1] = e->data.number.ival; }
        
        node->data.conv2d.stride = (yyvsp[-9].node)->data.number.ival;
        node->data.conv2d.padding = (yyvsp[-5].node)->data.number.ival;
        node->data.conv2d.activation = (yyvsp[-1].activation);
        
        (yyval.node) = node;
    }
#line 1936 "build\\net_lang.tab.c"
    break;

  case 31: /* dense_layer: DENSE LPAREN UNITS COLON expr COMMA ACTIVATION COLON activation_value RPAREN  */
#line 290 "src\\parser\\net_lang.y"
           {
        ASTNode* node = ast_node_new(NODE_DENSE, (yylsp[-9]).first_line);
        
        node->data.dense.units = (yyvsp[-5].node)->data.number.ival;
        node->data.dense.activation = (yyvsp[-1].activation);
        
        (yyval.node) = node;
    }
#line 1949 "build\\net_lang.tab.c"
    break;

  case 32: /* pool_layer: MAXPOOL LPAREN POOL COLON array COMMA STRIDE COLON expr COMMA PADDING COLON expr RPAREN  */
#line 307 "src\\parser\\net_lang.y"
           {
        ASTNode* node = ast_node_new(NODE_MAXPOOL, (yylsp[-13]).first_line);
        
        /* Parse pool array */
        ASTNode* e = (yyvsp[-9].node)->data.array.elements->head;
        if (e) { node->data.pooling.pool[0] = e->data.number.ival; e = e->next; }
        if (e) { node->data.pooling.pool[1] = e->data.number.ival; }
        
        node->data.pooling.stride = (yyvsp[-5].node)->data.number.ival;
        node->data.pooling.padding = (yyvsp[-1].node)->data.number.ival;
        
        (yyval.node) = node;
    }
#line 1967 "build\\net_lang.tab.c"
    break;

  case 33: /* pool_layer: AVGPOOL LPAREN POOL COLON array COMMA STRIDE COLON expr COMMA PADDING COLON expr RPAREN  */
#line 324 "src\\parser\\net_lang.y"
           {
        ASTNode* node = ast_node_new(NODE_AVGPOOL, (yylsp[-13]).first_line);
        
        /* Parse pool array */
        ASTNode* e = (yyvsp[-9].node)->data.array.elements->head;
        if (e) { node->data.pooling.pool[0] = e->data.number.ival; e = e->next; }
        if (e) { node->data.pooling.pool[1] = e->data.number.ival; }
        
        node->data.pooling.stride = (yyvsp[-5].node)->data.number.ival;
        node->data.pooling.padding = (yyvsp[-1].node)->data.number.ival;
        
        (yyval.node) = node;
    }
#line 1985 "build\\net_lang.tab.c"
    break;

  case 34: /* flatten_layer: FLATTEN LPAREN RPAREN  */
#line 342 "src\\parser\\net_lang.y"
                          {
        (yyval.node) = ast_node_new(NODE_FLATTEN, (yylsp[-2]).first_line);
    }
#line 1993 "build\\net_lang.tab.c"
    break;

  case 35: /* concat_layer: CONCAT LPAREN concat_args RPAREN  */
#line 348 "src\\parser\\net_lang.y"
                                     {
        ASTNode* node = ast_node_new(NODE_CONCAT, (yylsp[-3]).first_line);
        node->data.concat.inputs = (yyvsp[-1].list);
        (yyval.node) = node;
    }
#line 2003 "build\\net_lang.tab.c"
    break;

  case 36: /* concat_args: expr  */
#line 356 "src\\parser\\net_lang.y"
         {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 2012 "build\\net_lang.tab.c"
    break;

  case 37: /* concat_args: concat_args COMMA expr  */
#line 360 "src\\parser\\net_lang.y"
                             {
        ast_list_append((yyvsp[-2].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-2].list);
    }
#line 2021 "build\\net_lang.tab.c"
    break;

  case 38: /* norm_layer: BATCHNORM LPAREN RPAREN  */
#line 367 "src\\parser\\net_lang.y"
                            {
        (yyval.node) = ast_node_new(NODE_BATCHNORM, (yylsp[-2]).first_line);
    }
#line 2029 "build\\net_lang.tab.c"
    break;

  case 39: /* norm_layer: LAYERNORM LPAREN RPAREN  */
#line 370 "src\\parser\\net_lang.y"
                              {
        (yyval.node) = ast_node_new(NODE_LAYERNORM, (yylsp[-2]).first_line);
    }
#line 2037 "build\\net_lang.tab.c"
    break;

  case 40: /* module_call: IDENTIFIER LPAREN layer_params RPAREN  */
#line 378 "src\\parser\\net_lang.y"
                                          {
        ASTNode* node = ast_node_new(NODE_MODULE_CALL, (yylsp[-3]).first_line);
        node->data.module_call.module_name = strdup((yyvsp[-3].sval));
        node->data.module_call.args = (yyvsp[-1].params);
        free((yyvsp[-3].sval));
        (yyval.node) = node;
    }
#line 2049 "build\\net_lang.tab.c"
    break;

  case 41: /* layer_params: %empty  */
#line 390 "src\\parser\\net_lang.y"
                {
        (yyval.params) = param_list_new();
    }
#line 2057 "build\\net_lang.tab.c"
    break;

  case 42: /* layer_params: layer_param_list  */
#line 393 "src\\parser\\net_lang.y"
                       {
        (yyval.params) = (yyvsp[0].params);
    }
#line 2065 "build\\net_lang.tab.c"
    break;

  case 43: /* layer_param_list: IDENTIFIER COLON expr  */
#line 399 "src\\parser\\net_lang.y"
                          {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), (yyvsp[-2].sval), (yyvsp[0].node));
        free((yyvsp[-2].sval));
    }
#line 2075 "build\\net_lang.tab.c"
    break;

  case 44: /* layer_param_list: FILTERS COLON expr  */
#line 405 "src\\parser\\net_lang.y"
                         {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "filters", (yyvsp[0].node));
    }
#line 2084 "build\\net_lang.tab.c"
    break;

  case 45: /* layer_param_list: KERNEL COLON expr  */
#line 409 "src\\parser\\net_lang.y"
                        {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "kernel", (yyvsp[0].node));
    }
#line 2093 "build\\net_lang.tab.c"
    break;

  case 46: /* layer_param_list: ACTIVATION COLON expr  */
#line 413 "src\\parser\\net_lang.y"
                            {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "activation", (yyvsp[0].node));
    }
#line 2102 "build\\net_lang.tab.c"
    break;

  case 47: /* layer_param_list: STRIDE COLON expr  */
#line 417 "src\\parser\\net_lang.y"
                        {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "stride", (yyvsp[0].node));
    }
#line 2111 "build\\net_lang.tab.c"
    break;

  case 48: /* layer_param_list: PADDING COLON expr  */
#line 421 "src\\parser\\net_lang.y"
                         {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "padding", (yyvsp[0].node));
    }
#line 2120 "build\\net_lang.tab.c"
    break;

  case 49: /* layer_param_list: POOL COLON expr  */
#line 425 "src\\parser\\net_lang.y"
                      {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "pool", (yyvsp[0].node));
    }
#line 2129 "build\\net_lang.tab.c"
    break;

  case 50: /* layer_param_list: UNITS COLON expr  */
#line 429 "src\\parser\\net_lang.y"
                       {
        (yyval.params) = param_list_new();
        param_list_add((yyval.params), "units", (yyvsp[0].node));
    }
#line 2138 "build\\net_lang.tab.c"
    break;

  case 51: /* layer_param_list: layer_param_list COMMA IDENTIFIER COLON expr  */
#line 433 "src\\parser\\net_lang.y"
                                                   {
        param_list_add((yyvsp[-4].params), (yyvsp[-2].sval), (yyvsp[0].node));
        free((yyvsp[-2].sval));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2148 "build\\net_lang.tab.c"
    break;

  case 52: /* layer_param_list: layer_param_list COMMA FILTERS COLON expr  */
#line 438 "src\\parser\\net_lang.y"
                                                {
        param_list_add((yyvsp[-4].params), "filters", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2157 "build\\net_lang.tab.c"
    break;

  case 53: /* layer_param_list: layer_param_list COMMA KERNEL COLON expr  */
#line 442 "src\\parser\\net_lang.y"
                                               {
        param_list_add((yyvsp[-4].params), "kernel", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2166 "build\\net_lang.tab.c"
    break;

  case 54: /* layer_param_list: layer_param_list COMMA ACTIVATION COLON expr  */
#line 446 "src\\parser\\net_lang.y"
                                                   {
        param_list_add((yyvsp[-4].params), "activation", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2175 "build\\net_lang.tab.c"
    break;

  case 55: /* layer_param_list: layer_param_list COMMA STRIDE COLON expr  */
#line 450 "src\\parser\\net_lang.y"
                                               {
        param_list_add((yyvsp[-4].params), "stride", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2184 "build\\net_lang.tab.c"
    break;

  case 56: /* layer_param_list: layer_param_list COMMA PADDING COLON expr  */
#line 454 "src\\parser\\net_lang.y"
                                                {
        param_list_add((yyvsp[-4].params), "padding", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2193 "build\\net_lang.tab.c"
    break;

  case 57: /* layer_param_list: layer_param_list COMMA POOL COLON expr  */
#line 458 "src\\parser\\net_lang.y"
                                             {
        param_list_add((yyvsp[-4].params), "pool", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2202 "build\\net_lang.tab.c"
    break;

  case 58: /* layer_param_list: layer_param_list COMMA UNITS COLON expr  */
#line 462 "src\\parser\\net_lang.y"
                                              {
        param_list_add((yyvsp[-4].params), "units", (yyvsp[0].node));
        (yyval.params) = (yyvsp[-4].params);
    }
#line 2211 "build\\net_lang.tab.c"
    break;

  case 59: /* expr: number  */
#line 471 "src\\parser\\net_lang.y"
           { (yyval.node) = (yyvsp[0].node); }
#line 2217 "build\\net_lang.tab.c"
    break;

  case 60: /* expr: array  */
#line 472 "src\\parser\\net_lang.y"
            { (yyval.node) = (yyvsp[0].node); }
#line 2223 "build\\net_lang.tab.c"
    break;

  case 61: /* expr: identifier_expr  */
#line 473 "src\\parser\\net_lang.y"
                      { (yyval.node) = (yyvsp[0].node); }
#line 2229 "build\\net_lang.tab.c"
    break;

  case 62: /* expr: activation_value  */
#line 474 "src\\parser\\net_lang.y"
                       {
        /* Convert activation token to identifier node */
        (yyval.node) = ast_identifier(activation_to_string((yyvsp[0].activation)), (yylsp[0]).first_line);
    }
#line 2238 "build\\net_lang.tab.c"
    break;

  case 63: /* identifier_expr: IDENTIFIER  */
#line 481 "src\\parser\\net_lang.y"
               {
        (yyval.node) = ast_identifier((yyvsp[0].sval), (yylsp[0]).first_line);
        free((yyvsp[0].sval));
    }
#line 2247 "build\\net_lang.tab.c"
    break;

  case 64: /* identifier_expr: INPUT  */
#line 485 "src\\parser\\net_lang.y"
            {
        (yyval.node) = ast_identifier("input", (yylsp[0]).first_line);
    }
#line 2255 "build\\net_lang.tab.c"
    break;

  case 65: /* number: NUMBER  */
#line 491 "src\\parser\\net_lang.y"
           {
        (yyval.node) = ast_number_int((yyvsp[0].ival), (yylsp[0]).first_line);
    }
#line 2263 "build\\net_lang.tab.c"
    break;

  case 66: /* number: FLOAT_NUM  */
#line 494 "src\\parser\\net_lang.y"
                {
        (yyval.node) = ast_number_float((yyvsp[0].fval), (yylsp[0]).first_line);
    }
#line 2271 "build\\net_lang.tab.c"
    break;

  case 67: /* array: LBRACKET array_elements RBRACKET  */
#line 500 "src\\parser\\net_lang.y"
                                     {
        ASTNode* node = ast_array((yylsp[-2]).first_line);
        node->data.array.elements = (yyvsp[-1].list);
        (yyval.node) = node;
    }
#line 2281 "build\\net_lang.tab.c"
    break;

  case 68: /* array_elements: number  */
#line 508 "src\\parser\\net_lang.y"
           {
        (yyval.list) = ast_list_new();
        ast_list_append((yyval.list), (yyvsp[0].node));
    }
#line 2290 "build\\net_lang.tab.c"
    break;

  case 69: /* array_elements: array_elements COMMA number  */
#line 512 "src\\parser\\net_lang.y"
                                  {
        ast_list_append((yyvsp[-2].list), (yyvsp[0].node));
        (yyval.list) = (yyvsp[-2].list);
    }
#line 2299 "build\\net_lang.tab.c"
    break;

  case 70: /* activation_value: RELU  */
#line 519 "src\\parser\\net_lang.y"
         { (yyval.activation) = ACT_RELU; }
#line 2305 "build\\net_lang.tab.c"
    break;

  case 71: /* activation_value: SIGMOID  */
#line 520 "src\\parser\\net_lang.y"
              { (yyval.activation) = ACT_SIGMOID; }
#line 2311 "build\\net_lang.tab.c"
    break;

  case 72: /* activation_value: TANH  */
#line 521 "src\\parser\\net_lang.y"
           { (yyval.activation) = ACT_TANH; }
#line 2317 "build\\net_lang.tab.c"
    break;

  case 73: /* activation_value: SOFTMAX  */
#line 522 "src\\parser\\net_lang.y"
              { (yyval.activation) = ACT_SOFTMAX; }
#line 2323 "build\\net_lang.tab.c"
    break;

  case 74: /* activation_value: LINEAR  */
#line 523 "src\\parser\\net_lang.y"
             { (yyval.activation) = ACT_LINEAR; }
#line 2329 "build\\net_lang.tab.c"
    break;


#line 2333 "build\\net_lang.tab.c"

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

#line 526 "src\\parser\\net_lang.y"


/* ========== ERROR HANDLING ========== */

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    fprintf(stderr, "Near token: '%s'\n", yytext);
}

/* ========== MAIN FUNCTION ========== */

#ifndef NETLANG_NO_PARSER_MAIN
/* Test main - only compiled when not using external main.c */

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
        
        printf("\n========================================\n");
        printf("========== SEMANTIC ANALYSIS ==========\n");
        printf("========================================\n\n");
        
        SemanticResult sem_result = analyze_program(ast_root);
        
        if (sem_result.is_valid) {
            printf("\n✓ Semantic analysis passed!\n");
            printf("  Warnings: %d\n", sem_result.warning_count);
            
            printf("\n========== SYMBOL TABLE ==========\n\n");
            scope_print(sem_result.global_scope, 0);
        } else {
            fprintf(stderr, "\n✗ Semantic analysis failed!\n");
            fprintf(stderr, "  Errors: %d\n", sem_result.error_count);
            fprintf(stderr, "  Warnings: %d\n", sem_result.warning_count);
            
            /* Cleanup */
            scope_destroy(sem_result.global_scope);
            ast_free(ast_root);
            return 1;
        }
        
        /* Cleanup */
        scope_destroy(sem_result.global_scope);
        ast_free(ast_root);
    } else {
        fprintf(stderr, "\n✗ Parse failed with errors.\n");
        return 1;
    }
    
    return 0;
}

#endif /* NETLANG_NO_PARSER_MAIN */
