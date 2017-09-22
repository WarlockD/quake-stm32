#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 69 "grammar.y"

#include "ygrammer.h"



#line 18 "y.tab.c"
#define T_IDENTIFIER 257
#define T_TYPEDEF_NAME 258
#define T_DEFINE_NAME 259
#define T_AUTO 260
#define T_EXTERN 261
#define T_REGISTER 262
#define T_STATIC 263
#define T_TYPEDEF 264
#define T_INLINE 265
#define T_EXTENSION 266
#define T_CHAR 267
#define T_DOUBLE 268
#define T_FLOAT 269
#define T_INT 270
#define T_VOID 271
#define T_LONG 272
#define T_SHORT 273
#define T_SIGNED 274
#define T_UNSIGNED 275
#define T_ENUM 276
#define T_STRUCT 277
#define T_UNION 278
#define T_Bool 279
#define T_Complex 280
#define T_Imaginary 281
#define T_TYPE_QUALIFIER 282
#define T_BRACKETS 283
#define T_LBRACE 284
#define T_MATCHRBRACE 285
#define T_ELLIPSIS 286
#define T_INITIALIZER 287
#define T_STRING_LITERAL 288
#define T_ASM 289
#define T_ASMARG 290
#define T_VA_DCL 291
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   26,   26,   27,   27,   27,   27,   27,   27,
   27,   32,   30,   30,   28,   28,   35,   28,   33,   33,
   34,   34,   36,   36,   38,   39,   29,   40,   29,   37,
   37,   37,   41,   41,    1,    1,    2,    2,    2,    3,
    3,    3,    3,    3,    3,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    5,    5,    6,    6,    6,   19,   19,    8,    8,    9,
   42,    9,   31,   31,    7,    7,    7,   25,   23,   23,
   10,   10,   11,   11,   11,   11,   11,   20,   20,   21,
   21,   22,   22,   14,   14,   15,   15,   16,   16,   16,
   17,   17,   18,   18,   24,   24,   12,   12,   12,   13,
   13,   13,   13,   13,   13,   13,
};
short yylen[] = {                                         2,
    0,    1,    1,    2,    1,    1,    1,    1,    1,    2,
    2,    2,    3,    3,    2,    3,    0,    5,    2,    1,
    0,    1,    1,    3,    0,    0,    7,    0,    5,    0,
    1,    1,    1,    2,    1,    2,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    3,    2,    2,    1,    1,    1,    3,    1,
    0,    4,    2,    3,    3,    2,    2,    1,    1,    1,
    2,    1,    1,    3,    2,    4,    4,    2,    3,    0,
    1,    1,    2,    1,    3,    1,    3,    2,    2,    1,
    0,    1,    1,    3,    1,    2,    1,    2,    1,    3,
    2,    1,    4,    3,    3,    2,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,   79,    0,   62,   40,    0,   42,
   43,   20,   44,    0,   46,   47,   48,   49,   54,   50,
   51,   52,   53,   78,   66,   67,   55,   56,   57,   61,
    0,    7,    0,    0,   35,   37,   38,   39,   59,   60,
   28,    0,    0,    0,    0,  105,   83,    0,    0,    3,
    5,    6,    8,    9,    0,   10,   11,   80,    0,   92,
    0,    0,  106,    0,   19,   73,    0,   41,   45,  112,
   15,   36,    0,   68,    0,   99,    0,    0,    0,    0,
   85,    0,    0,    0,   64,    0,    0,   76,    4,   58,
    0,   84,   89,   93,    0,   14,   13,  116,    0,    0,
    0,    0,   96,   16,    0,   71,    0,    0,  111,    0,
   31,   33,    0,    0,    0,    0,    0,  103,   74,   12,
   63,   75,    0,    0,   98,  110,  115,    0,   69,    0,
    0,  114,    0,    0,   34,   86,   87,    0,   23,    0,
    0,   95,   97,   72,   26,  113,   29,  104,   18,    0,
    0,   24,   27,
};
short yydgoto[] = {                                      33,
   95,   35,   36,   37,   38,   39,   40,   73,   74,   41,
   42,   76,   77,  101,  102,  103,  116,  117,   44,   45,
   61,   62,   46,   47,   48,   49,   50,   51,   52,   53,
   54,   85,   55,  140,  123,  141,  113,  107,  151,   79,
  114,  130,
};
short yysindex[] = {                                     64,
   -2,   27, -201, -227,    0,    0,    0,    0, -261,    0,
    0,    0,    0, -230,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -239,    0,    0,  479,    0,    0,    0,    0,    0,    0,
    0,  -35, -225, -229,   57,    0,    0, -229,   64,    0,
    0,    0,    0,    0,  691,    0,    0,    0,   18,    0,
   20, -201,    0,  613,    0,    0,  309,    0,    0,    0,
    0,    0,  -34,    0,   15,    0,  -33,  174,  557,  641,
    0, -224, -207, -204,    0,  -35, -204,    0,    0,    0,
  691,    0,    0,    0,  506,    0,    0,    0,  343,   41,
   46,   42,    0,    0,   27,    0,  557,  532,    0,  -33,
    0,    0, -196,  666,   49,   50,   48,    0,    0,    0,
    0,    0,   27,   15,    0,    0,    0,  584,    0, -194,
 -188,    0,   66, -187,    0,    0,    0, -227,    0,   51,
   67,    0,    0,    0,    0,    0,    0,    0,    0,   27,
 -177,    0,    0,
};
short yyrindex[] = {                                    113,
    0,    0,  157,    0,    0,  -38,    0,    0,   97,    0,
    0,    0,    0,  130,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -175,    0,    0,    0,    0,    0,    0,
    0,  376,    0,    0,    0,    0,    0,    0,  116,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  169,  164,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  444,    0,  -28,   -8, -167,   77,
    0,    0,    0,   -3,    0,  410,   30,    0,    0,    0,
   61,    0,    0,    0,    0,    0,    0,    0,  -22,    0,
    0,   78,    0,    0,    0,    0, -167,    0,    0,    5,
    0,    0,    0, -163,    0,    0,   81,    0,    0,    0,
    0,    0,   65,  -27,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   68,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
short yygindex[] = {                                      0,
   45,  -16,    0,    0,   11,    0,    0,    0,   21,   10,
  -36,   63,   53,  -65,    0,    1,    0,    0,    0,  -14,
    0,    0,    4,    0,    0,    0,   79,  -53,    0,    0,
    0,  -24,    0,    0,    0,    0,   25,    0,    0,    0,
    0,    0,
};
#define YYTABLESIZE 973
short yytable[] = {                                      58,
   43,   58,   58,   58,   80,   58,  108,   63,   86,  105,
   96,   59,  109,   60,  115,  109,   70,   72,  100,   78,
   58,  100,   80,   88,  104,  112,   64,    5,   58,    5,
   58,   70,  107,   65,   65,  107,   65,   65,   65,   97,
   65,   86,  133,   75,   34,  108,   93,   84,  108,   43,
   66,   87,   78,  112,   83,   65,   57,    7,   92,  121,
  135,    3,  122,   82,    4,  119,    2,   77,    3,   77,
   77,   77,   94,   77,   72,  106,   59,  120,   72,   83,
   30,  126,   72,  118,   78,  128,  127,  134,   77,  136,
  137,  138,  144,   34,    4,  145,    2,  147,   17,   91,
   17,    4,   17,    2,  124,    3,  146,  153,  125,  149,
  150,   99,    1,  100,  124,    2,   30,  101,   94,   17,
   32,  102,   32,   21,   99,  129,   22,   89,  143,  100,
  110,  131,  139,    0,   41,    0,   41,    0,   41,    0,
    0,  148,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   99,    0,    0,   41,    0,    0,    0,  152,
    0,    0,    0,    0,    0,    0,    0,   45,    0,   45,
    0,   45,   99,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   45,    0,
    0,    0,    0,    0,   90,    0,   90,   90,   90,    0,
   90,   91,    0,   91,   91,   91,   88,   91,   88,   88,
    0,    4,   88,   67,    0,    0,    0,    0,   58,   58,
   58,   58,   58,   58,   58,   80,   58,   58,   58,   58,
   58,   58,   58,   58,   58,   58,   58,   58,   58,   58,
   58,   58,   58,   58,   58,   80,    0,   81,    0,  109,
   58,    0,   80,   65,   65,   65,   65,   65,   65,   65,
  109,   65,   65,   65,   65,   65,   65,   65,   65,   65,
   65,   65,   65,   65,   65,   65,   65,   65,   65,   65,
  107,    0,   56,    5,   58,   65,   77,   77,   77,   77,
   77,   77,   77,  108,   77,   77,   77,   77,   77,   77,
   77,   77,   77,   77,   77,   77,   77,   77,   77,   77,
   77,   77,   77,    5,   58,    0,    0,   17,   77,    1,
    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
   25,   26,   27,   28,   29,   30,    4,    0,   67,   98,
    3,    0,   31,   41,   41,   41,   41,   41,   41,   41,
    0,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
    4,    0,   67,    0,    3,   41,   45,   45,   45,   45,
   45,   45,   45,    0,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   90,   90,    0,   82,    0,   45,   82,
   91,   91,    0,    0,    0,   88,   88,    0,    0,    0,
    5,   58,    0,    0,   82,    0,   82,    0,    0,   90,
    0,    0,    0,    0,    0,   90,   91,    0,    0,    0,
   81,   88,   91,   81,    0,    0,   70,   88,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   81,    0,
   81,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   70,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   70,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    4,    0,   67,    0,
    3,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   71,    0,    0,
    0,    0,    0,    4,    0,    2,    0,    3,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   71,    5,    6,    7,    8,   68,
   10,   11,  132,   13,   69,   15,   16,   17,   18,   19,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   30,   70,    0,    0,    0,    0,    0,    0,    0,    5,
    6,    7,    8,   68,   10,   11,    0,   13,   69,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
   26,   27,   28,   29,   30,   70,    0,    0,    0,    0,
    0,    0,    0,   82,   82,   82,   82,   82,   82,   82,
   82,   82,   82,   82,   82,   82,   82,   82,   82,   82,
   82,   82,   82,   82,   82,   82,   82,   82,    0,   82,
    0,    0,    0,    0,   82,    0,   82,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,    0,   81,    0,    0,    0,    0,   81,    0,
   81,   25,   25,   25,   25,   25,   25,   25,   25,   25,
   25,   25,   25,   25,   25,   25,   25,   25,   25,   25,
   25,   25,   25,   25,   25,   25,    0,   25,    0,    0,
    0,    0,   98,    0,   25,    5,    6,    7,    8,   68,
   10,   11,    0,   13,   69,   15,   16,   17,   18,   19,
   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
   30,   70,    5,    6,    7,    8,   68,   10,   11,    0,
   13,   69,   15,   16,   17,   18,   19,   20,   21,   22,
   23,   24,   25,   26,   27,   28,   29,   30,    0,   90,
    7,    8,   68,   10,   11,    0,   13,   69,   15,   16,
   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
   27,   28,   29,   30,   90,    7,    8,   68,   10,   11,
   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
   22,   23,   24,   25,   26,   27,   28,   29,   30,    0,
    0,   90,    7,    8,   68,   10,   11,  111,   13,   69,
   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
   25,   26,   27,   28,   29,   30,    0,    0,    0,  142,
   90,    7,    8,   68,   10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
   26,   27,   28,   29,   30,    0,   83,    5,    6,    7,
    8,   68,   10,   11,    0,   13,   69,   15,   16,   17,
   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
   28,   29,   30,   90,    7,    8,   68,   10,   11,   12,
   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
   23,   24,   25,   26,   27,   28,   29,   30,   90,    7,
    8,   68,   10,   11,    0,   13,   69,   15,   16,   17,
   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
   28,   29,   30,
};
short yycheck[] = {                                      38,
    0,   40,   41,   42,   40,   44,   40,    4,   45,   44,
   64,    2,   41,    3,   80,   44,   44,   34,   41,   34,
   59,   44,   61,   48,   59,   79,  288,  257,  258,  257,
  258,   59,   41,  264,   38,   44,   40,   41,   42,   64,
   44,   78,  108,   34,    0,   41,   61,   44,   44,   49,
  290,   48,   67,  107,  284,   59,   59,  259,   41,   84,
  114,   42,   87,  289,   38,  290,   40,   38,   42,   40,
   41,   42,   62,   44,   91,   61,   67,  285,   95,  284,
  282,   41,   99,   80,   99,   44,   41,  284,   59,   41,
   41,   44,  287,   49,   38,  284,   40,  285,   38,   55,
   40,   38,   42,   40,   95,   42,   41,  285,   99,   59,
   44,   67,    0,  289,  105,    0,  284,   41,   41,   59,
  284,   41,   59,   59,   80,  105,   59,   49,  128,   67,
   78,  107,  123,   -1,   38,   -1,   40,   -1,   42,   -1,
   -1,  138,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  108,   -1,   -1,   59,   -1,   -1,   -1,  150,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   38,   -1,   40,
   -1,   42,  128,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   59,   -1,
   -1,   -1,   -1,   -1,   38,   -1,   40,   41,   42,   -1,
   44,   38,   -1,   40,   41,   42,   38,   44,   40,   41,
   -1,   38,   44,   40,   -1,   -1,   -1,   -1,  257,  258,
  259,  260,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,  283,   -1,  283,
  289,   -1,  291,  257,  258,  259,  260,  261,  262,  263,
  289,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  289,   -1,  285,  257,  258,  289,  257,  258,  259,  260,
  261,  262,  263,  289,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  257,  258,   -1,   -1,  257,  289,  256,
  257,  258,  259,  260,  261,  262,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,   38,   -1,   40,   41,
   42,   -1,  289,  257,  258,  259,  260,  261,  262,  263,
   -1,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
   38,   -1,   40,   -1,   42,  289,  257,  258,  259,  260,
  261,  262,  263,   -1,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  257,  258,   -1,   41,   -1,  289,   44,
  257,  258,   -1,   -1,   -1,  257,  258,   -1,   -1,   -1,
  257,  258,   -1,   -1,   59,   -1,   61,   -1,   -1,  283,
   -1,   -1,   -1,   -1,   -1,  289,  283,   -1,   -1,   -1,
   41,  283,  289,   44,   -1,   -1,  283,  289,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   59,   -1,
   61,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   44,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   59,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   38,   -1,   40,   -1,
   42,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   59,   -1,   -1,
   -1,   -1,   -1,   38,   -1,   40,   -1,   42,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   59,  257,  258,  259,  260,  261,
  262,  263,   41,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,
  258,  259,  260,  261,  262,  263,   -1,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  258,  259,  260,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,   -1,  284,
   -1,   -1,   -1,   -1,  289,   -1,  291,  258,  259,  260,
  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,   -1,  284,   -1,   -1,   -1,   -1,  289,   -1,
  291,  258,  259,  260,  261,  262,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,   -1,  284,   -1,   -1,
   -1,   -1,  289,   -1,  291,  257,  258,  259,  260,  261,
  262,  263,   -1,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  257,  258,  259,  260,  261,  262,  263,   -1,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,   -1,  258,
  259,  260,  261,  262,  263,   -1,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  258,  259,  260,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,   -1,
   -1,  258,  259,  260,  261,  262,  263,  291,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,   -1,   -1,   -1,  286,
  258,  259,  260,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,   -1,  284,  257,  258,  259,
  260,  261,  262,  263,   -1,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  258,  259,  260,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  258,  259,
  260,  261,  262,  263,   -1,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,
};
#define YYFINAL 33
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 291
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,"'&'",0,"'('","')'","'*'",0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,0,"';'",0,
"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"T_IDENTIFIER","T_TYPEDEF_NAME","T_DEFINE_NAME","T_AUTO","T_EXTERN",
"T_REGISTER","T_STATIC","T_TYPEDEF","T_INLINE","T_EXTENSION","T_CHAR",
"T_DOUBLE","T_FLOAT","T_INT","T_VOID","T_LONG","T_SHORT","T_SIGNED",
"T_UNSIGNED","T_ENUM","T_STRUCT","T_UNION","T_Bool","T_Complex","T_Imaginary",
"T_TYPE_QUALIFIER","T_BRACKETS","T_LBRACE","T_MATCHRBRACE","T_ELLIPSIS",
"T_INITIALIZER","T_STRING_LITERAL","T_ASM","T_ASMARG","T_VA_DCL",
};
char *yyrule[] = {
"$accept : program",
"program :",
"program : translation_unit",
"translation_unit : external_declaration",
"translation_unit : translation_unit external_declaration",
"external_declaration : declaration",
"external_declaration : function_definition",
"external_declaration : ';'",
"external_declaration : linkage_specification",
"external_declaration : asm_specifier",
"external_declaration : error T_MATCHRBRACE",
"external_declaration : error ';'",
"braces : T_LBRACE T_MATCHRBRACE",
"linkage_specification : T_EXTERN T_STRING_LITERAL braces",
"linkage_specification : T_EXTERN T_STRING_LITERAL declaration",
"declaration : decl_specifiers ';'",
"declaration : decl_specifiers init_declarator_list ';'",
"$$1 :",
"declaration : any_typedef decl_specifiers $$1 opt_declarator_list ';'",
"any_typedef : T_EXTENSION T_TYPEDEF",
"any_typedef : T_TYPEDEF",
"opt_declarator_list :",
"opt_declarator_list : declarator_list",
"declarator_list : declarator",
"declarator_list : declarator_list ',' declarator",
"$$2 :",
"$$3 :",
"function_definition : decl_specifiers declarator $$2 opt_declaration_list T_LBRACE $$3 T_MATCHRBRACE",
"$$4 :",
"function_definition : declarator $$4 opt_declaration_list T_LBRACE T_MATCHRBRACE",
"opt_declaration_list :",
"opt_declaration_list : T_VA_DCL",
"opt_declaration_list : declaration_list",
"declaration_list : declaration",
"declaration_list : declaration_list declaration",
"decl_specifiers : decl_specifier",
"decl_specifiers : decl_specifiers decl_specifier",
"decl_specifier : storage_class",
"decl_specifier : type_specifier",
"decl_specifier : type_qualifier",
"storage_class : T_AUTO",
"storage_class : T_EXTERN",
"storage_class : T_REGISTER",
"storage_class : T_STATIC",
"storage_class : T_INLINE",
"storage_class : T_EXTENSION",
"type_specifier : T_CHAR",
"type_specifier : T_DOUBLE",
"type_specifier : T_FLOAT",
"type_specifier : T_INT",
"type_specifier : T_LONG",
"type_specifier : T_SHORT",
"type_specifier : T_SIGNED",
"type_specifier : T_UNSIGNED",
"type_specifier : T_VOID",
"type_specifier : T_Bool",
"type_specifier : T_Complex",
"type_specifier : T_Imaginary",
"type_specifier : T_TYPEDEF_NAME",
"type_specifier : struct_or_union_specifier",
"type_specifier : enum_specifier",
"type_qualifier : T_TYPE_QUALIFIER",
"type_qualifier : T_DEFINE_NAME",
"struct_or_union_specifier : struct_or_union any_id braces",
"struct_or_union_specifier : struct_or_union braces",
"struct_or_union_specifier : struct_or_union any_id",
"struct_or_union : T_STRUCT",
"struct_or_union : T_UNION",
"init_declarator_list : init_declarator",
"init_declarator_list : init_declarator_list ',' init_declarator",
"init_declarator : declarator",
"$$5 :",
"init_declarator : declarator '=' $$5 T_INITIALIZER",
"asm_specifier : T_ASM T_ASMARG",
"asm_specifier : parameter_declaration T_ASM T_ASMARG",
"enum_specifier : enumeration any_id braces",
"enum_specifier : enumeration braces",
"enum_specifier : enumeration any_id",
"enumeration : T_ENUM",
"any_id : T_IDENTIFIER",
"any_id : T_TYPEDEF_NAME",
"declarator : pointer direct_declarator",
"declarator : direct_declarator",
"direct_declarator : identifier_or_ref",
"direct_declarator : '(' declarator ')'",
"direct_declarator : direct_declarator T_BRACKETS",
"direct_declarator : direct_declarator '(' parameter_type_list ')'",
"direct_declarator : direct_declarator '(' opt_identifier_list ')'",
"pointer : '*' opt_type_qualifiers",
"pointer : '*' opt_type_qualifiers pointer",
"opt_type_qualifiers :",
"opt_type_qualifiers : type_qualifier_list",
"type_qualifier_list : type_qualifier",
"type_qualifier_list : type_qualifier_list type_qualifier",
"parameter_type_list : parameter_list",
"parameter_type_list : parameter_list ',' T_ELLIPSIS",
"parameter_list : parameter_declaration",
"parameter_list : parameter_list ',' parameter_declaration",
"parameter_declaration : decl_specifiers declarator",
"parameter_declaration : decl_specifiers abs_declarator",
"parameter_declaration : decl_specifiers",
"opt_identifier_list :",
"opt_identifier_list : identifier_list",
"identifier_list : any_id",
"identifier_list : identifier_list ',' any_id",
"identifier_or_ref : any_id",
"identifier_or_ref : '&' any_id",
"abs_declarator : pointer",
"abs_declarator : pointer direct_abs_declarator",
"abs_declarator : direct_abs_declarator",
"direct_abs_declarator : '(' abs_declarator ')'",
"direct_abs_declarator : direct_abs_declarator T_BRACKETS",
"direct_abs_declarator : T_BRACKETS",
"direct_abs_declarator : direct_abs_declarator '(' parameter_type_list ')'",
"direct_abs_declarator : direct_abs_declarator '(' ')'",
"direct_abs_declarator : '(' parameter_type_list ')'",
"direct_abs_declarator : '(' ')'",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 772 "grammar.y"
extern char* yytext;

  void save_text_offset(void)
 {
	 (void)strcpy(yylval.text.text, yytext);
#if OPT_LINTLIBRARY
	 copy_typedef(yytext);
#endif
	 if (cur_file->convert) {
		 yylval.text.begin = ftell(cur_file->tmp_file);
		 fputs(yytext, cur_file->tmp_file);
	 }
	 else
		 yylval.text.begin = 0;
}

#line 532 "y.tab.c"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 10:
#line 94 "grammar.y"
{
	    yyerrok;
	}
break;
case 11:
#line 98 "grammar.y"
{
	    yyerrok;
	}
break;
case 13:
#line 109 "grammar.y"
{
	    /* Provide an empty action here so bison will not complain about
	     * incompatible types in the default action it normally would
	     * have generated.
	     */
	}
break;
case 14:
#line 116 "grammar.y"
{
	    /* empty */
	}
break;
case 15:
#line 123 "grammar.y"
{
#if OPT_LINTLIBRARY
	    if (types_out && want_typedef()) {
		gen_declarations(&yyvsp[-1].decl_spec, (DeclaratorList *)0);
		flush_varargs();
	    }
#endif
	    free_decl_spec(&yyvsp[-1].decl_spec);
	    end_typedef();
	}
break;
case 16:
#line 134 "grammar.y"
{
	    if (func_params != NULL) {
		set_param_types(func_params, &yyvsp[-2].decl_spec, &yyvsp[-1].decl_list);
	    } else {
		gen_declarations(&yyvsp[-2].decl_spec, &yyvsp[-1].decl_list);
#if OPT_LINTLIBRARY
		flush_varargs();
#endif
		free_decl_list(&yyvsp[-1].decl_list);
	    }
	    free_decl_spec(&yyvsp[-2].decl_spec);
	    end_typedef();
	}
break;
case 17:
#line 148 "grammar.y"
{
	    cur_decl_spec_flags = yyvsp[0].decl_spec.flags;
	    free_decl_spec(&yyvsp[0].decl_spec);
	}
break;
case 18:
#line 153 "grammar.y"
{
	    end_typedef();
	}
break;
case 19:
#line 160 "grammar.y"
{
	    begin_typedef();
	}
break;
case 20:
#line 164 "grammar.y"
{
	    begin_typedef();
	}
break;
case 23:
#line 176 "grammar.y"
{
	    int flags = cur_decl_spec_flags;

	    /* If the typedef is a pointer type, then reset the short type
	     * flags so it does not get promoted.
	     */
	    if (strcmp(yyvsp[0].declarator->text, yyvsp[0].declarator->name) != 0)
		flags &= ~(DS_CHAR | DS_SHORT | DS_FLOAT);
	    new_symbol(typedef_names, yyvsp[0].declarator->name, NULL, flags);
	    free_declarator(yyvsp[0].declarator);
	}
break;
case 24:
#line 188 "grammar.y"
{
	    int flags = cur_decl_spec_flags;

	    if (strcmp(yyvsp[0].declarator->text, yyvsp[0].declarator->name) != 0)
		flags &= ~(DS_CHAR | DS_SHORT | DS_FLOAT);
	    new_symbol(typedef_names, yyvsp[0].declarator->name, NULL, flags);
	    free_declarator(yyvsp[0].declarator);
	}
break;
case 25:
#line 200 "grammar.y"
{
	    check_untagged(&yyvsp[-1].decl_spec);
	    if (yyvsp[0].declarator->func_def == FUNC_NONE) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &(yyvsp[0].declarator->head->params);
	    func_params->begin_comment = cur_file->begin_comment;
	    func_params->end_comment = cur_file->end_comment;
	}
break;
case 26:
#line 211 "grammar.y"
{
	    /* If we're converting to K&R and we've got a nominally K&R
	     * function which has a parameter which is ANSI (i.e., a prototyped
	     * function pointer), then we must override the deciphered value of
	     * 'func_def' so that the parameter will be converted.
	     */
	    if (func_style == FUNC_TRADITIONAL
	     && haveAnsiParam()
	     && yyvsp[-3].declarator->head->func_def == func_style) {
		yyvsp[-3].declarator->head->func_def = FUNC_BOTH;
	    }

	    func_params = NULL;

	    if (cur_file->convert)
		gen_func_definition(&yyvsp[-4].decl_spec, yyvsp[-3].declarator);
	    gen_prototype(&yyvsp[-4].decl_spec, yyvsp[-3].declarator);
#if OPT_LINTLIBRARY
	    flush_varargs();
#endif
	    free_decl_spec(&yyvsp[-4].decl_spec);
	    free_declarator(yyvsp[-3].declarator);
	}
break;
case 28:
#line 236 "grammar.y"
{
	    if (yyvsp[0].declarator->func_def == FUNC_NONE) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &(yyvsp[0].declarator->head->params);
	    func_params->begin_comment = cur_file->begin_comment;
	    func_params->end_comment = cur_file->end_comment;
	}
break;
case 29:
#line 246 "grammar.y"
{
	    DeclSpec decl_spec;

	    func_params = NULL;

	    new_decl_spec(&decl_spec, dft_decl_spec(), yyvsp[-4].declarator->begin, DS_NONE);
	    if (cur_file->convert)
		gen_func_definition(&decl_spec, yyvsp[-4].declarator);
	    gen_prototype(&decl_spec, yyvsp[-4].declarator);
#if OPT_LINTLIBRARY
	    flush_varargs();
#endif
	    free_decl_spec(&decl_spec);
	    free_declarator(yyvsp[-4].declarator);
	}
break;
case 36:
#line 277 "grammar.y"
{
	    join_decl_specs(&yyval.decl_spec, &yyvsp[-1].decl_spec, &yyvsp[0].decl_spec);
	    free_decl_spec(&yyvsp[-1].decl_spec);
	    free_decl_spec(&yyvsp[0].decl_spec);
	}
break;
case 40:
#line 292 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 41:
#line 296 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_EXTERN);
	}
break;
case 42:
#line 300 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 43:
#line 304 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_STATIC);
	}
break;
case 44:
#line 308 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_INLINE);
	}
break;
case 45:
#line 312 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_JUNK);
	}
break;
case 46:
#line 319 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_CHAR);
	}
break;
case 47:
#line 323 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 48:
#line 327 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_FLOAT);
	}
break;
case 49:
#line 331 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 50:
#line 335 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 51:
#line 339 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_SHORT);
	}
break;
case 52:
#line 343 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 53:
#line 347 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 54:
#line 351 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 55:
#line 355 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_CHAR);
	}
break;
case 56:
#line 359 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 57:
#line 363 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 58:
#line 367 "grammar.y"
{
	    Symbol *s;
	    s = find_symbol(typedef_names, yyvsp[0].text.text);
	    if (s != NULL)
		new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, s->flags);
	}
break;
case 61:
#line 379 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, DS_NONE);
	}
break;
case 62:
#line 383 "grammar.y"
{
	    /* This rule allows the <pointer> nonterminal to scan #define
	     * names as if they were type modifiers.
	     */
	    Symbol *s;
	    s = find_symbol(define_names, yyvsp[0].text.text);
	    if (s != NULL)
		new_decl_spec(&yyval.decl_spec, yyvsp[0].text.text, yyvsp[0].text.begin, s->flags);
	}
break;
case 63:
#line 396 "grammar.y"
{
	    char *s;
	    if ((s = implied_typedef()) == 0) {
		need_temp(2 + strlen(yyvsp[-2].text.text) + strlen(yyvsp[-1].text.text));
	        (void)sprintf(s = temp_buf, "%s %s", yyvsp[-2].text.text, yyvsp[-1].text.text);
	    }
	    new_decl_spec(&yyval.decl_spec, s, yyvsp[-2].text.begin, DS_NONE);
	}
break;
case 64:
#line 405 "grammar.y"
{
	    char *s;
	    if ((s = implied_typedef()) == 0) {
		need_temp(4 + strlen(yyvsp[-1].text.text));
		(void)sprintf(s = temp_buf, "%s {}", yyvsp[-1].text.text);
	    }
	    new_decl_spec(&yyval.decl_spec, s, yyvsp[-1].text.begin, DS_NONE);
	}
break;
case 65:
#line 414 "grammar.y"
{
	    need_temp(2 + strlen(yyvsp[-1].text.text) + strlen(yyvsp[0].text.text));
	    (void)sprintf(temp_buf, "%s %s", yyvsp[-1].text.text, yyvsp[0].text.text);
	    new_decl_spec(&yyval.decl_spec, temp_buf, yyvsp[-1].text.begin, DS_NONE);
	}
break;
case 66:
#line 423 "grammar.y"
{
	    imply_typedef(yyval.text.text);
	}
break;
case 67:
#line 427 "grammar.y"
{
	    imply_typedef(yyval.text.text);
	}
break;
case 68:
#line 434 "grammar.y"
{
	    new_decl_list(&yyval.decl_list, yyvsp[0].declarator);
	}
break;
case 69:
#line 438 "grammar.y"
{
	    add_decl_list(&yyval.decl_list, &yyvsp[-2].decl_list, yyvsp[0].declarator);
	}
break;
case 70:
#line 445 "grammar.y"
{
	    if (yyvsp[0].declarator->func_def != FUNC_NONE && func_params == NULL &&
		func_style == FUNC_TRADITIONAL && cur_file->convert) {
		gen_func_declarator(yyvsp[0].declarator);
		fputs(cur_text(), cur_file->tmp_file);
	    }
	    cur_declarator = yyval.declarator;
	}
break;
case 71:
#line 454 "grammar.y"
{
	    if (yyvsp[-1].declarator->func_def != FUNC_NONE && func_params == NULL &&
		func_style == FUNC_TRADITIONAL && cur_file->convert) {
		gen_func_declarator(yyvsp[-1].declarator);
		fputs(" =", cur_file->tmp_file);
	    }
	}
break;
case 74:
#line 467 "grammar.y"
{
	    free_parameter(yyvsp[-2].parameter);
	}
break;
case 75:
#line 474 "grammar.y"
{
	    char *s;
	    if ((s = implied_typedef()) == 0) {
		need_temp(6 + strlen(yyvsp[-1].text.text));
		(void)sprintf(s = temp_buf, "enum %s", yyvsp[-1].text.text);
	    }
	    new_decl_spec(&yyval.decl_spec, s, yyvsp[-2].text.begin, DS_NONE);
	}
break;
case 76:
#line 483 "grammar.y"
{
	    char *s;
	    if ((s = implied_typedef()) == 0) {
		need_temp(4 + strlen(yyvsp[-1].text.text));
		(void)sprintf(s = temp_buf, "%s {}", yyvsp[-1].text.text);
	    }
	    new_decl_spec(&yyval.decl_spec, s, yyvsp[-1].text.begin, DS_NONE);
	}
break;
case 77:
#line 492 "grammar.y"
{
	    need_temp(6 + strlen(yyvsp[0].text.text));
	    (void)sprintf(temp_buf, "enum %s", yyvsp[0].text.text);
	    new_decl_spec(&yyval.decl_spec, temp_buf, yyvsp[-1].text.begin, DS_NONE);
	}
break;
case 78:
#line 501 "grammar.y"
{
	    imply_typedef("enum");
	    yyval.text = yyvsp[0].text;
	}
break;
case 81:
#line 514 "grammar.y"
{
	    yyval.declarator = yyvsp[0].declarator;

	    need_temp(1 + strlen(yyvsp[-1].text.text) + strlen(yyvsp[0].declarator->text));
	    (void)sprintf(temp_buf, "%s%s", yyvsp[-1].text.text, yyval.declarator->text);

	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(temp_buf);
	    yyval.declarator->begin = yyvsp[-1].text.begin;
	    yyval.declarator->pointer = TRUE;
	}
break;
case 83:
#line 530 "grammar.y"
{
	    yyval.declarator = new_declarator(yyvsp[0].text.text, yyvsp[0].text.text, yyvsp[0].text.begin);
	}
break;
case 84:
#line 534 "grammar.y"
{
	    yyval.declarator = yyvsp[-1].declarator;

	    need_temp(2 + strlen(yyvsp[-1].declarator->text));
	    (void)sprintf(temp_buf, "(%s)", yyval.declarator->text);

	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(temp_buf);
	    yyval.declarator->begin = yyvsp[-2].text.begin;
	}
break;
case 85:
#line 545 "grammar.y"
{
	    yyval.declarator = yyvsp[-1].declarator;

	    need_temp(1 + strlen(yyvsp[-1].declarator->text) + strlen(yyvsp[0].text.text));
	    (void)sprintf(temp_buf, "%s%s", yyval.declarator->text, yyvsp[0].text.text);

	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(temp_buf);
	}
break;
case 86:
#line 555 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", yyvsp[-3].declarator->name, yyvsp[-3].declarator->begin);
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = yyvsp[-3].declarator;
	    yyval.declarator->head = (yyvsp[-3].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-3].declarator->head;
	    yyval.declarator->func_def = FUNC_ANSI;
	}
break;
case 87:
#line 563 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", yyvsp[-3].declarator->name, yyvsp[-3].declarator->begin);
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = yyvsp[-3].declarator;
	    yyval.declarator->head = (yyvsp[-3].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-3].declarator->head;
	    yyval.declarator->func_def = FUNC_TRADITIONAL;
	}
break;
case 88:
#line 574 "grammar.y"
{
	    need_temp(2 + strlen(yyvsp[0].text.text));
	    (void)sprintf(yyval.text.text, "*%s", yyvsp[0].text.text);
	    yyval.text.begin = yyvsp[-1].text.begin;
	}
break;
case 89:
#line 580 "grammar.y"
{
	    need_temp(2 + strlen(yyvsp[-1].text.text) + strlen(yyvsp[0].text.text));
	    (void)sprintf(yyval.text.text, "*%s%s", yyvsp[-1].text.text, yyvsp[0].text.text);
	    yyval.text.begin = yyvsp[-2].text.begin;
	}
break;
case 90:
#line 589 "grammar.y"
{
	    strcpy(yyval.text.text, "");
	    yyval.text.begin = 0L;
	}
break;
case 92:
#line 598 "grammar.y"
{
	    need_temp(2 + strlen(yyvsp[0].decl_spec.text));
	    (void)sprintf(yyval.text.text, "%s ", yyvsp[0].decl_spec.text);
	    yyval.text.begin = yyvsp[0].decl_spec.begin;
	    free(yyvsp[0].decl_spec.text);
	}
break;
case 93:
#line 605 "grammar.y"
{
	    need_temp(1 + strlen(yyvsp[-1].text.text) + strlen(yyvsp[0].decl_spec.text));
	    (void)sprintf(yyval.text.text, "%s%s ", yyvsp[-1].text.text, yyvsp[0].decl_spec.text);
	    yyval.text.begin = yyvsp[-1].text.begin;
	    free(yyvsp[0].decl_spec.text);
	}
break;
case 95:
#line 616 "grammar.y"
{
	    add_ident_list(&yyval.param_list, &yyvsp[-2].param_list, "...");
	}
break;
case 96:
#line 623 "grammar.y"
{
	    new_param_list(&yyval.param_list, yyvsp[0].parameter);
	}
break;
case 97:
#line 627 "grammar.y"
{
	    add_param_list(&yyval.param_list, &yyvsp[-2].param_list, yyvsp[0].parameter);
	}
break;
case 98:
#line 634 "grammar.y"
{
	    check_untagged(&yyvsp[-1].decl_spec);
	    yyval.parameter = new_parameter(&yyvsp[-1].decl_spec, yyvsp[0].declarator);
	}
break;
case 99:
#line 639 "grammar.y"
{
	    check_untagged(&yyvsp[-1].decl_spec);
	    yyval.parameter = new_parameter(&yyvsp[-1].decl_spec, yyvsp[0].declarator);
	}
break;
case 100:
#line 644 "grammar.y"
{
	    check_untagged(&yyvsp[0].decl_spec);
	    yyval.parameter = new_parameter(&yyvsp[0].decl_spec, (Declarator *)0);
	}
break;
case 101:
#line 652 "grammar.y"
{
	    new_ident_list(&yyval.param_list);
	}
break;
case 103:
#line 660 "grammar.y"
{
	    new_ident_list(&yyval.param_list);
	    add_ident_list(&yyval.param_list, &yyval.param_list, yyvsp[0].text.text);
	}
break;
case 104:
#line 665 "grammar.y"
{
	    add_ident_list(&yyval.param_list, &yyvsp[-2].param_list, yyvsp[0].text.text);
	}
break;
case 105:
#line 672 "grammar.y"
{
	    yyval.text = yyvsp[0].text;
	}
break;
case 106:
#line 676 "grammar.y"
{
	    need_temp(2 + strlen(yyvsp[0].text.text));
#if OPT_LINTLIBRARY
	    if (lintLibrary()) { /* Lint doesn't grok C++ ref variables */
		yyval.text = yyvsp[0].text;
	    } else
#endif
		(void)sprintf(yyval.text.text, "&%s", yyvsp[0].text.text);
	    yyval.text.begin = yyvsp[-1].text.begin;
	}
break;
case 107:
#line 690 "grammar.y"
{
	    yyval.declarator = new_declarator(yyvsp[0].text.text, "", yyvsp[0].text.begin);
	}
break;
case 108:
#line 694 "grammar.y"
{
	    yyval.declarator = yyvsp[0].declarator;

	    need_temp(1 + strlen(yyvsp[-1].text.text) + strlen(yyvsp[0].declarator->text));
	    (void)sprintf(temp_buf, "%s%s", yyvsp[-1].text.text, yyval.declarator->text);

	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(temp_buf);
	    yyval.declarator->begin = yyvsp[-1].text.begin;
	}
break;
case 110:
#line 709 "grammar.y"
{
	    yyval.declarator = yyvsp[-1].declarator;

	    need_temp(3 + strlen(yyvsp[-1].declarator->text));
	    (void)sprintf(temp_buf, "(%s)", yyval.declarator->text);

	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(temp_buf);
	    yyval.declarator->begin = yyvsp[-2].text.begin;
	}
break;
case 111:
#line 720 "grammar.y"
{
	    yyval.declarator = yyvsp[-1].declarator;

	    need_temp(1 + strlen(yyvsp[-1].declarator->text) + strlen(yyvsp[0].text.text));
	    (void)sprintf(temp_buf, "%s%s", yyval.declarator->text, yyvsp[0].text.text);

	    free(yyval.declarator->text);
	    yyval.declarator->text = xstrdup(temp_buf);
	}
break;
case 112:
#line 730 "grammar.y"
{
	    yyval.declarator = new_declarator(yyvsp[0].text.text, "", yyvsp[0].text.begin);
	}
break;
case 113:
#line 734 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", "", yyvsp[-3].declarator->begin);
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = yyvsp[-3].declarator;
	    yyval.declarator->head = (yyvsp[-3].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-3].declarator->head;
	    yyval.declarator->func_def = FUNC_ANSI;
	}
break;
case 114:
#line 742 "grammar.y"
{
	    yyval.declarator = new_declarator("%s()", "", yyvsp[-2].declarator->begin);
	    yyval.declarator->func_stack = yyvsp[-2].declarator;
	    yyval.declarator->head = (yyvsp[-2].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-2].declarator->head;
	    yyval.declarator->func_def = FUNC_ANSI;
	}
break;
case 115:
#line 749 "grammar.y"
{
	    Declarator *d;

	    d = new_declarator("", "", yyvsp[-2].text.begin);
	    yyval.declarator = new_declarator("%s()", "", yyvsp[-2].text.begin);
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = d;
	    yyval.declarator->head = yyval.declarator;
	    yyval.declarator->func_def = FUNC_ANSI;
	}
break;
case 116:
#line 760 "grammar.y"
{
	    Declarator *d;

	    d = new_declarator("", "", yyvsp[-1].text.begin);
	    yyval.declarator = new_declarator("%s()", "", yyvsp[-1].text.begin);
	    yyval.declarator->func_stack = d;
	    yyval.declarator->head = yyval.declarator;
	    yyval.declarator->func_def = FUNC_ANSI;
	}
break;
#line 1386 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
