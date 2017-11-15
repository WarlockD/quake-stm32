/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#include <iomanip>
using namespace std::chrono;


/*

*/



prstack_t	pr_stack[PR_MAX_STACK_DEPTH];
int			pr_depth;


int			localstack[PR_LOCALSTACK_SIZE];
int			localstack_used;


qboolean	pr_trace;
dfunction_t	*pr_xfunction;
int			pr_xstatement;


int		pr_argc;
ddef_t *ED_GlobalAtOfs(int ofs);
ddef_t *ED_FieldAtOfs(int ofs);

char *pr_opnames[] =
{
"DONE",

"MUL_F",
"MUL_V", 
"MUL_FV",
"MUL_VF",
 
"DIV",

"ADD_F",
"ADD_V", 
  
"SUB_F",
"SUB_V",

"EQ_F",
"EQ_V",
"EQ_S", 
"EQ_E",
"EQ_FNC",
 
"NE_F",
"NE_V", 
"NE_S",
"NE_E", 
"NE_FNC",
 
"LE",
"GE",
"LT",
"GT", 

"INDIRECT",
"INDIRECT",
"INDIRECT", 
"INDIRECT", 
"INDIRECT",
"INDIRECT", 

"ADDRESS", 

"STORE_F",
"STORE_V",
"STORE_S",
"STORE_ENT",
"STORE_FLD",
"STORE_FNC",

"STOREP_F",
"STOREP_V",
"STOREP_S",
"STOREP_ENT",
"STOREP_FLD",
"STOREP_FNC",

"RETURN",
  
"NOT_F",
"NOT_V",
"NOT_S", 
"NOT_ENT", 
"NOT_FNC", 
  
"IF",
"IFNOT",
  
"CALL0",
"CALL1",
"CALL2",
"CALL3",
"CALL4",
"CALL5",
"CALL6",
"CALL7",
"CALL8",
  
"STATE",
  
"GOTO", 
  
"AND",
"OR", 

"BITAND",
"BITOR"
};
// hack, fix this latter, check buffer size etc
static void PadString(char* s, const size_t count) {
	size_t i = Q_strlen(s);
	s += i;
	for (s; i < count; i++) *s++ = ' ';
	*s++ = ' ';
	*s++ = '\0';
}

void quake::PR_ValueString::text_output(std::ostream& os) const {
	switch (type)
	{
	case etype_t::ev_string:
		os << (const char*)(pr_strings + val->string);
		break;
	case etype_t::ev_entity:
		os << "entity " << NUM_FOR_EDICT(PROG_TO_EDICT(val->edict));
		break;
	case etype_t::ev_function:
	{
		dfunction_t	*f = pr_functions + val->function;
		os << (const char*)(pr_strings + f->s_name) << "()";
	}
	break;
	case etype_t::ev_field:
	{
		ddef_t *def = ED_FieldAtOfs(val->_int);
		os << '.' << (const char*)(pr_strings + def->s_name);
	}
	break;
	case etype_t::ev_void:
		os << "void";
		break;
	case etype_t::ev_float:
		os << std::setw(5) << std::setprecision(1) << val->_float;
		break;
	case etype_t::ev_vector:
		os << std::setw(5) << std::setprecision(1) << val->vector[0] << ' ' << val->vector[1] << ' ' << val->vector[2];
		break;
	case etype_t::ev_pointer:
		os << "pointer";
		break;
	default:
		os << "bad type " << (int)((etype_t)type);
		break;
	}
}


/*
============
PR_GlobalString

Returns a string with a description and the contents of a global,
padded to 20 field width
============
*/

void quake::PR_GlobalString::text_output(std::ostream& os)  const  {
	quake::fixed_string_stream<64> line;

	void	*val = (void *)&pr_globals[ofs];
	ddef_t	*def = ED_GlobalAtOfs(ofs);
	if (!def)
		line << ofs << "(???)";
	else
		line << ofs << '(' << (const char*)(pr_strings + def->s_name) << ')' << PR_ValueString(def->type, (eval_t*)val);
	os << std::setw(20) << line.rdbuf();
}

void quake::PR_GlobalStringNoContents::text_output(std::ostream& os)  const  {
	quake::fixed_string_stream<128> line;

	ddef_t	*def = ED_GlobalAtOfs(ofs);
	if (!def)
		line << ofs << "(???)";
	else
		line << ofs << '(' << (const char*)(pr_strings + def->s_name) << ')';
	os << std::setw(20) << line.rdbuf();
}


//=============================================================================

/*
=================
PR_PrintStatement
=================
*/
std::ostream& operator<<(std::ostream& os, const dstatement_t& s) {

	if ((unsigned)s.op < sizeof(pr_opnames) / sizeof(pr_opnames[0]))
		os << std::setw(10) << std::left << pr_opnames[s.op] << ' ';

	if (s.op == OP_IF || s.op == OP_IFNOT)
		os << quake::PR_GlobalString(s.a) << " branch " << s.b;
	else if (s.op == OP_GOTO)
		os << "branch " << s.a;
	else if ((unsigned)(s.op - OP_STORE_F) < 6)
		os << quake::PR_GlobalString(s.a) << ' ' << quake::PR_GlobalStringNoContents(s.b);
	else
	{
		if (s.a) os << quake::PR_GlobalString(s.a);
		if (s.b) {
			if (s.a) os << ' ';
			os << quake::PR_GlobalString(s.b);
		}
		if (s.c) {
			if (s.a || s.b) os << ' ';
			os << quake::PR_GlobalStringNoContents(s.c);
		}
	}
	os << std::endl;
	return os;
}

/*
============
PR_StackTrace
============
*/
void PR_StackTrace (std::ostream& os)
{
	if (pr_depth == 0) {
		os << "<NO STACK>" << std::endl;
		return;
	}
	
	pr_stack[pr_depth].f = pr_xfunction;
	for (int i=pr_depth ; i>=0 ; i--)
	{
		dfunction_t	*f = pr_stack[i].f;
		if (!f)
			os << "<NO FUNCTION>" << std::endl;
		else
			os << std::setw(12) << (const char*)(pr_strings + f->s_file) << " : " << (const char*)(pr_strings + f->s_name) << std::endl;
	}
}


/*
============
PR_Profile_f

============
*/
void PR_Profile_f(cmd_source_t source, size_t argc, const quake::string_view argv[])
{
	dfunction_t	*f, *best;
	int			max;
	int			num;
	int			i;
	
	num = 0;	
	do
	{
		max = 0;
		best = NULL;
		for (i=0 ; i<progs->numfunctions ; i++)
		{
			f = &pr_functions[i];
			if (f->profile > max)
			{
				max = f->profile;
				best = f;
			}
		}
		if (best)
		{
			if (num < 10)
				quake::con << std::setw(7) << best->profile << ' ' << (const char*)(pr_strings + best->s_name) << std::endl;
			num++;
			best->profile = 0;
		}
	} while (best);
}

// pr run error
/*
============
PR_RunError

Aborts the currently executing function
============
*/

class pr_run_error_buffer_t : public quake::quake_console_buffer {
public:
	pr_run_error_buffer_t() : quake::quake_console_buffer() {}
protected:

	// Inherited via quake_console_buffer
	void text_out(const char * text, size_t size) override final
	{
		quake::con << *(pr_statements + pr_xstatement);
		PR_StackTrace(quake::con);
		quake::con << text; 

		pr_depth = 0;		// dump the stack so host_error can shutdown functions
		Host_Error("Program error");
	}
};

void PR_RunError(const char* message) {
	pr_run_error_buffer_t _buf;
	std::ostream ss(&_buf);
	quake::con << *(pr_statements + pr_xstatement);
	PR_StackTrace(quake::con);
	quake::con << message << std::endl;

	pr_depth = 0;		// dump the stack so host_error can shutdown functions
	Host_Error("Program error");
}
/*
============================================================================
PR_ExecuteProgram

The interpretation main loop
============================================================================
*/

/*
====================
PR_EnterFunction

Returns the new program statement counter
====================
*/
int PR_EnterFunction (dfunction_t *f)
{
	int		i, j, c, o;

	pr_stack[pr_depth].s = pr_xstatement;
	pr_stack[pr_depth].f = pr_xfunction;	
	pr_depth++;
	if (pr_depth >= PR_MAX_STACK_DEPTH)
		PR_RunError ("stack overflow");

// save off any locals that the new function steps on
	c = f->locals;
	if (localstack_used + c > PR_LOCALSTACK_SIZE) 
		PR_RunError("PR_ExecuteProgram: locals stack overflow");


	for (i=0 ; i < c ; i++)
		localstack[localstack_used+i] = ((int *)pr_globals)[f->parm_start + i];
	localstack_used += c;

// copy parameters
	o = f->parm_start;
	for (i=0 ; i<f->numparms ; i++)
	{
		for (j=0 ; j<f->parm_size[i] ; j++)
		{
			((int *)pr_globals)[o] = ((int *)pr_globals)[OFS_PARM0+i*3+j];
			o++;
		}
	}

	pr_xfunction = f;
	return f->first_statement - 1;	// offset the s++
}

/*
====================
PR_LeaveFunction
====================
*/
int PR_LeaveFunction (void)
{
	int		i, c;

	if (pr_depth <= 0)
		Sys_Error ("prog stack underflow");

// restore locals from the stack
	c = pr_xfunction->locals;
	localstack_used -= c;
	if (localstack_used < 0)
		PR_RunError ("PR_ExecuteProgram: locals stack underflow");

	for (i=0 ; i < c ; i++)
		((int *)pr_globals)[pr_xfunction->parm_start + i] = localstack[localstack_used+i];

// up stack
	pr_depth--;
	pr_xfunction = pr_stack[pr_depth].f;
	return pr_stack[pr_depth].s;
}


/*
====================
PR_ExecuteProgram
====================
*/
void PR_ExecuteProgram(func_t fnum)
{
	eval_t	*a, *b, *c;
	int			s;
	dstatement_t	*st;
	dfunction_t	*f, *newf;
	int		runaway;
	int		i;
	edict_t	*ed;
	int		exitdepth;
	eval_t	*ptr;

	if (!fnum || fnum >= progs->numfunctions)
	{
		if (pr_global_struct->self)
			PROG_TO_EDICT(pr_global_struct->self)->Print();
		Host_Error("PR_ExecuteProgram: NULL function");
	}

	f = &pr_functions[fnum];

	runaway = 100000;
	pr_trace = false;

	// make a stack frame
	exitdepth = pr_depth;

	s = PR_EnterFunction(f);

	while (1)
	{
		s++;	// next statement

		st = &pr_statements[s];
		a = (eval_t *)&pr_globals[st->a];
		b = (eval_t *)&pr_globals[st->b];
		c = (eval_t *)&pr_globals[st->c];

		if (!--runaway)
			PR_RunError("runaway loop error");

		pr_xfunction->profile++;
		pr_xstatement = s;

		if (pr_trace) quake::con << *st;

		switch (st->op)
		{
		case OP_ADD_F:
			c->_float = a->_float + b->_float;
			break;
		case OP_ADD_V:
			c->vector[0] = a->vector[0] + b->vector[0];
			c->vector[1] = a->vector[1] + b->vector[1];
			c->vector[2] = a->vector[2] + b->vector[2];
			break;

		case OP_SUB_F:
			c->_float = a->_float - b->_float;
			break;
		case OP_SUB_V:
			c->vector[0] = a->vector[0] - b->vector[0];
			c->vector[1] = a->vector[1] - b->vector[1];
			c->vector[2] = a->vector[2] - b->vector[2];
			break;

		case OP_MUL_F:
			c->_float = a->_float * b->_float;
			break;
		case OP_MUL_V:
			c->_float = a->vector[0] * b->vector[0]
				+ a->vector[1] * b->vector[1]
				+ a->vector[2] * b->vector[2];
			break;
		case OP_MUL_FV:
			c->vector[0] = a->_float * b->vector[0];
			c->vector[1] = a->_float * b->vector[1];
			c->vector[2] = a->_float * b->vector[2];
			break;
		case OP_MUL_VF:
			c->vector[0] = b->_float * a->vector[0];
			c->vector[1] = b->_float * a->vector[1];
			c->vector[2] = b->_float * a->vector[2];
			break;

		case OP_DIV_F:
			c->_float = a->_float / b->_float;
			break;

		case OP_BITAND:
			c->_float = (float)((int)a->_float & (int)b->_float);
			break;

		case OP_BITOR:
			c->_float = (float)((int)a->_float | (int)b->_float);
			break;


		case OP_GE:
			c->_float = a->_float >= b->_float;
			break;
		case OP_LE:
			c->_float = a->_float <= b->_float;
			break;
		case OP_GT:
			c->_float = a->_float > b->_float;
			break;
		case OP_LT:
			c->_float = a->_float < b->_float;
			break;
		case OP_AND:
			c->_float = a->_float && b->_float;
			break;
		case OP_OR:
			c->_float = a->_float || b->_float;
			break;

		case OP_NOT_F:
			c->_float = !a->_float;
			break;
		case OP_NOT_V:
			c->_float = !a->vector[0] && !a->vector[1] && !a->vector[2];
			break;
		case OP_NOT_S:
			c->_float = !a->string || !pr_strings[a->string];
			break;
		case OP_NOT_FNC:
			c->_float = !a->function;
			break;
		case OP_NOT_ENT:
			c->_float = (PROG_TO_EDICT(a->edict) == sv.edicts);
			break;

		case OP_EQ_F:
			c->_float = a->_float == b->_float;
			break;
		case OP_EQ_V:
			c->_float = (a->vector[0] == b->vector[0]) &&
				(a->vector[1] == b->vector[1]) &&
				(a->vector[2] == b->vector[2]);
			break;
		case OP_EQ_S:
			c->_float = !strcmp(pr_strings + a->string, pr_strings + b->string);
			break;
		case OP_EQ_E:
			c->_float = a->_int == b->_int;
			break;
		case OP_EQ_FNC:
			c->_float = a->function == b->function;
			break;


		case OP_NE_F:
			c->_float = a->_float != b->_float;
			break;
		case OP_NE_V:
			c->_float = (a->vector[0] != b->vector[0]) ||
				(a->vector[1] != b->vector[1]) ||
				(a->vector[2] != b->vector[2]);
			break;
		case OP_NE_S:
			c->_float = (float)strcmp(pr_strings + a->string, pr_strings + b->string);
			break;
		case OP_NE_E:
			c->_float = a->_int != b->_int;
			break;
		case OP_NE_FNC:
			c->_float = a->function != b->function;
			break;

			//==================
		case OP_STORE_F:
		case OP_STORE_ENT:
		case OP_STORE_FLD:		// integers
		case OP_STORE_S:
		case OP_STORE_FNC:		// pointers
			b->_int = a->_int;
			break;
		case OP_STORE_V:
			b->vector[0] = a->vector[0];
			b->vector[1] = a->vector[1];
			b->vector[2] = a->vector[2];
			break;

		case OP_STOREP_F:
		case OP_STOREP_ENT:
		case OP_STOREP_FLD:		// integers
		case OP_STOREP_S:
		case OP_STOREP_FNC:		// pointers
			ptr = (eval_t *)((byte *)sv.edicts + b->_int);
			ptr->_int = a->_int;
			break;
		case OP_STOREP_V:
			ptr = (eval_t *)((byte *)sv.edicts + b->_int);
			ptr->vector[0] = a->vector[0];
			ptr->vector[1] = a->vector[1];
			ptr->vector[2] = a->vector[2];
			break;

		case OP_ADDRESS:
			ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
			NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
			if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
				PR_RunError("assignment to world entity");
			c->_int = (byte *)((int *)&ed->v + b->_int) - (byte *)sv.edicts;
			break;

		case OP_LOAD_F:
		case OP_LOAD_FLD:
		case OP_LOAD_ENT:
		case OP_LOAD_S:
		case OP_LOAD_FNC:
			ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
			NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
			a = (eval_t *)((int *)&ed->v + b->_int);
			c->_int = a->_int;
			break;

		case OP_LOAD_V:
			ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
			NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
			a = (eval_t *)((int *)&ed->v + b->_int);
			c->vector[0] = a->vector[0];
			c->vector[1] = a->vector[1];
			c->vector[2] = a->vector[2];
			break;

			//==================

		case OP_IFNOT:
			if (!a->_int)
				s += st->b - 1;	// offset the s++
			break;

		case OP_IF:
			if (a->_int)
				s += st->b - 1;	// offset the s++
			break;

		case OP_GOTO:
			s += st->a - 1;	// offset the s++
			break;

		case OP_CALL0:
		case OP_CALL1:
		case OP_CALL2:
		case OP_CALL3:
		case OP_CALL4:
		case OP_CALL5:
		case OP_CALL6:
		case OP_CALL7:
		case OP_CALL8:
			pr_argc = st->op - OP_CALL0;
			if (!a->function)
				PR_RunError("NULL function");

			newf = &pr_functions[a->function];

			if (newf->first_statement < 0)
			{	// negative statements are built in functions
				i = -newf->first_statement;
				if (i >= pr_numbuiltins)
					PR_RunError("Bad builtin call number");
				pr_builtins[i]();
				break;
			}

			s = PR_EnterFunction(newf);
			break;

		case OP_DONE:
		case OP_RETURN:
			pr_globals[OFS_RETURN] = pr_globals[st->a];
			pr_globals[OFS_RETURN + 1] = pr_globals[st->a + 1];
			pr_globals[OFS_RETURN + 2] = pr_globals[st->a + 2];

			s = PR_LeaveFunction();
			if (pr_depth == exitdepth)
				return;		// all done
			break;

		case OP_STATE:
			ed = PROG_TO_EDICT(pr_global_struct->self);
#ifdef FPS_20
			ed->v.nextthink = pr_global_struct->time + 0.05;
#else
			ed->v.nextthink = pr_global_struct->time + 100ms;
#endif
			if (a->_float != ed->v.frame)
			{
				ed->v.frame = a->_float;
			}
			ed->v.think = b->function;
			break;

		default: {
			pr_run_error_buffer_t buf;
			std::ostream ss(&buf);
			ss << "Bad opcode " << st->op << std::endl;
		}
		}

	}
}
