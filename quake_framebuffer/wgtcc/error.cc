#include "error.h"

#include "ast.h"
#include "token.h"

#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <Windows.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


extern std::string program;


void Error(const char* format, ...) {
  fprintf(stderr,
          "%s: " ANSI_COLOR_RED "error: " ANSI_COLOR_RESET,
          program.c_str());
  
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  
  fprintf(stderr, "\n");

  exit(-1);
}


static void VError(const SourceLocation& loc,
                   const char* format,
                   va_list args) {
	std::stringstream ss;
	//ANSI_COLOR_RED "error: " ANSI_COLOR_RESET
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), format, args);
	ss << loc.filename_->c_str() << ':' << loc.line_ << ':' << loc.column_ << ": ";
	ss << "error: ";
	ss << buffer << "\n    ";


  bool sawNoSpace = false;
  int nspaces = 0;
  for (auto p = loc.lineBegin_; *p != '\n' && *p != 0; p++) {
    if (!sawNoSpace && (*p == ' ' || *p == '\t')) {
      ++nspaces;
    } else {
      sawNoSpace = true;
	  ss << (char)*p;
    }
  }
  
  ss  << "\n    ";
  for (unsigned i = 1; i + nspaces < loc.column_; ++i)
	  ss << ' ';
  ss  << "^\n";
//  fprintf(stderr, ANSI_COLOR_GREEN "^\n");
  OutputDebugStringA(ss.str().c_str());
  fputs(ss.str().c_str(), stderr);

  exit(-1);	
}



void Error(const SourceLocation& loc, const char* format, ...) {
  va_list args;
  va_start(args, format);
  VError(loc, format, args);
  va_end(args);
}


void Error(const Token* tok, const char* format, ...) {
  va_list args;
  va_start(args, format);
  VError(tok->loc_, format, args);
  va_end(args);
}


void Error(const Expr* expr, const char* format, ...) {
  va_list args;
  va_start(args, format);
  VError(expr->Tok()->loc_, format, args);
  va_end(args);
}
