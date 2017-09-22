#ifndef _YGRAMMER_H_
#define _YGRAMMER_H_

#include "symbol.h"
#include "semantic.h"
#include "y.tab.h"

#define YYMAXDEPTH 150

extern int yylex(void);

/* declaration specifier attributes for the typedef statement currently being
* scanned
*/
extern int cur_decl_spec_flags;

/* pointer to parameter list for the current function definition */
extern ParameterList *func_params;

/* A parser semantic action sets this pointer to the current declarator in
* a function parameter declaration in order to catch any comments following
* the parameter declaration on the same line.  If the lexer scans a comment
* and <cur_declarator> is not NULL, then the comment is attached to the
* declarator.  To ignore subsequent comments, the lexer sets this to NULL
* after scanning a comment or end of line.
*/
extern Declarator *cur_declarator;

/* temporary string buffer */
extern char *temp_buf;
extern size_t temp_len;

/* table of typedef names */
extern SymbolTable *typedef_names;

/* table of define names */
extern SymbolTable *define_names;

/* table of type qualifiers */
extern SymbolTable *type_qualifiers;

/* information about the current input file */
typedef struct {
	char *base_name;		/* base input file name */
	char *file_name;		/* current file name */
	size_t len_file_name;	/* ...its allocated size */
	FILE *file;			/* input file */
	unsigned line_num;		/* current line number in input file */
	FILE *tmp_file;		/* temporary file */
	long begin_comment;		/* tmp file offset after last written ) or ; */
	long end_comment;		/* tmp file offset after last comment */
	boolean convert;		/* if TRUE, convert function definitions */
	boolean changed;		/* TRUE if conversion done in this file */
} IncludeStack;

static IncludeStack *cur_file;	/* current input file */

//#include "yyerror.c"

 int haveAnsiParam(void);

/* Flags to enable us to find if a procedure returns a value.
*/
 extern int return_val;		/* nonzero on BRACES iff return-expression found */
extern int returned_at;			/* marker for token-number to set 'return_val' */

#if OPT_LINTLIBRARY
static const char *
dft_decl_spec(void)
{
	return (lintLibrary() && !return_val) ? "void" : "int";
}

#else
#define dft_decl_spec() "int"
#endif

int haveAnsiParam(void);
void save_text_offset(void);
void need_temp(size_t need);

// lex stuff



extern char *varargs_str;		/* save printflike/scanflike text */
extern int varargs_num;		/* code to save "VARARGS" */
extern int debug_trace;		/* true if we trace token-level stuff */
extern char base_file[BUFSIZ];		/* top-level file name */

extern int asm_level;		/* parenthesis level for "asm" parsing */
extern int save_cpp;		/* true if cpp-text within curly braces */
extern int in_cpp;		/* true while we are within cpp-text */
extern int curly;		/* number of curly brace nesting levels */
extern int ly_count;		/* number of occurances of %% */
extern unsigned inc_limit;	/* stack size */
extern int inc_depth;		/* include nesting level */
extern IncludeStack *inc_stack;	/* stack of included files */
extern SymbolTable *included_files;	/* files already included */



int LexInput();
int type_of_name(char *name);

#if defined(apollo) || !OPT_LINTLIBRARY
void absorb_special(void);
#endif
#if OPT_LINTLIBRARY
void gcc_attribute(void);
#endif
void update_line_num(void);
void save_text(void);

void get_quoted(void);
void get_comment(void);
void get_cpp_directive(int copy);
void parsing_file_name(unsigned need);
void do_include(char *f);
void include_file(char *name, int convert);
void put_file(FILE *outf);
void put_quoted(int c);

void startCpp(int level);
void finishCpp(void);
void  yaccError(const char* s);
#define yyerror(s) yaccError(s)
#if OPT_LINTLIBRARY
extern int decipher_comment(char *keyword, int len);
#endif





#endif