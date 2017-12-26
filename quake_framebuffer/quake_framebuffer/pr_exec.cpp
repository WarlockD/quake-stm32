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
#include "icommon.h"
#include <iomanip>
using namespace std::chrono;


/*

*/


int		pr_argc; // used ONLY for error checking


static const char *pr_opnames[] =
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
		os << val->string;
		break;
	case etype_t::ev_entity:
		os << "entity_" << vm.NUM_FOR_EDICT(val->edict);
		break;
	case etype_t::ev_function:
		os << val->function->name() << "()";
	break;
	case etype_t::ev_field:
		os << '.' << val->field->name();
	break;
	case etype_t::ev_void:
		os << "void";
		break;
	case etype_t::ev_float:
		os << std::setw(5) << std::setprecision(1) << val->_float;
		break;
	case etype_t::ev_vector:
		os << "{ " << val->vector[0] << ", " << val->vector[1] << ", " << val->vector[2] << "}";
		break;
	case etype_t::ev_pointer:
		os << "pointer_" << std::hex << std::setfill('0') << std::setw(8) << val->_int;
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
static bool IsOffsetLocal(int ofs) {
	decltype(vm.pr_xfunction) f = vm.pr_xfunction;
	const uint16_t locals_start = f->parm_start;
	const uint16_t locals_end = f->parm_start + f->locals;;
	return locals_start >= ofs && locals_end < locals_end;
}
static std::string_view GetFieldOffsetName(int ofs) {
	const ddef_t *def = vm.ED_FieldAtOfs(ofs);
	assert(def);
	return def->name();
}
static const char* GetGlobalOffsetName(uint16_t ofs) {
	static char buffer[512];// not thread frendly of course
	const ddef_t *def = vm.ED_GlobalAtOfs(ofs);
	// no idea of the name, is it local?
	decltype(vm.pr_xfunction) f = vm.pr_xfunction;
	const uint16_t locals_start = f->parm_start;
	const uint16_t locals_end = f->parm_start + f->locals;;
	const char* name = "?NAME?";
	if (!def) {
		if (OFS_NULL == ofs) name = "NULL";
		else if (ofs < OFS_PARM0) name = "RETURN";
		else if (ofs < OFS_PARM1) name = "PARM0";
		else if (ofs < OFS_PARM2) name = "PARM1";
		else if (ofs < OFS_PARM3) name = "PARM2";
		else if (ofs < OFS_PARM4) name = "PARM3";
		else if (ofs < OFS_PARM5) name = "PARM4";
		else if (ofs < OFS_PARM6) name = "PARM5";
		else if (ofs < OFS_PARM7) name = "PARM6";
		else if (ofs < RESERVED_OFS) name = "PARM7";
	}
	else {
		name = def->name().c_str();
	}

	if (ofs >= locals_start && ofs < locals_end) {
			// we ARE
		Q_sprintf(buffer, "(%i)%s_LOCAL%i", ofs, ofs - locals_start);
	}
	else {
		Q_sprintf(buffer, "(%i)%s", ofs, name);
	}
	return buffer;
}
void quake::PR_GlobalString::text_output(std::ostream& os)  const  {
	quake::fixed_string_stream<512> line;

	const eval_t	*val = (eval_t *)&vm.pr_globals[ofs];
	const ddef_t	*def = vm.ED_GlobalAtOfs(ofs);
	const char* field_name = GetGlobalOffsetName(ofs);
	line << '<' << ofs << '(' << field_name << ')';
	if (def) {
		if (def->type == etype_t::ev_field)
			line << PR_ValueString(def->type, (eval_t*)val) << '>';
		else
			line << " : " << PR_ValueString(def->type, (eval_t*)val) << '>';
	}
	os << std::setw(20) << line.rdbuf() << ' ';
}

void quake::PR_GlobalStringNoContents::text_output(std::ostream& os)  const  {
	quake::fixed_string_stream<128> line;
	const ddef_t	*def = vm.ED_GlobalAtOfs(ofs);
	const char* field_name = GetGlobalOffsetName(ofs);
	line << '<' << ofs << '(' << field_name << ')';

	os << std::setw(20) << line.rdbuf() << ' ';
}


/*
============
PR_StackTrace
============
*/
void PR_StackTrace (std::ostream& os)
{
	assert(0); // BEEEP FIX
#if 0
	if (pr_depth == 0) {
		os << "<NO STACK>" << std::endl;
		return;
	}
	
	pr_stack[pr_depth].f = vm.pr_xfunction;
	for (int i=pr_depth ; i>=0 ; i--)
	{
		dfunction_t	*f = pr_stack[i].f;
		if (!f)
			os << "<NO FUNCTION>" << std::endl;
		else
			os << std::setw(12) << (const char*)(vm.pr_strings + f->s_file) << " : " << (const char*)(vm.pr_strings + f->s_name) << std::endl;
	}
#endif
}


/*
============
PR_Profile_f

============
*/
void PR_Profile_f(cmd_source_t source, const StringArgs& args)
{
	int			num;		
	const dfunction_t	*best;
	do
	{
		int max = 0;
		best = nullptr;
		for (int i=0 ; i<vm.progs->numfunctions ; i++)
		{
			const dfunction_t	*f = &vm.pr_functions[i];
			if (f->profile > max){
				max = f->profile;
				best = f;
			}
		}
		if (best)
		{
			if (num < 10)
				quake::con << std::setw(7) << best->profile << ' ' << best->name() << std::endl;
			num++;
			const_cast<dfunction_t	*>(best)->profile = 0;
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
		assert(0); // beep
		//quake::con << *(vm.pr_statements + vm.pr_xstatement);
		PR_StackTrace(quake::con);
		quake::con << text; 

		//pr_depth = 0;		// dump the stack so host_error can shutdown functions
		Host_Error("Program error");
	}
};

void PR_RunError(const char* message) {
	pr_run_error_buffer_t _buf;
	std::ostream ss(&_buf);
	//quake::con << *(vm.pr_statements + vm.pr_xstatement);
	PR_StackTrace(quake::con);
	quake::con << message << std::endl;

	//pr_depth = 0;		// dump the stack so host_error can shutdown functions
	Host_Error("Program error");
}
/*
============================================================================
PR_ExecuteProgram

The interpretation main loop
============================================================================
*/





#include "pr_print.h"
int dfunction_t::call(edict_t* self, edict_t* other) const {
	std::swap(vm.pr_global_struct->self, self);
	std::swap(vm.pr_global_struct->other, other);
	int s = call();
	std::swap(vm.pr_global_struct->self, self);
	std::swap(vm.pr_global_struct->other, other);
	return s;
}



int dfunction_t::call() const {
	static constexpr bool pr_trace = true; // some start ups?
	static quake::decomp::Indent debug_ident;
	static size_t debug_depth = 0U;
	static size_t debug_runaway = 0U;
	// don't need to test this

	auto f = this;
	const prstack_t last_f = { vm.pr_xstatement, vm.pr_xfunction };
	vm.pr_xfunction = f;
	size_t s = vm.pr_xstatement = f->first_statement;

	const size_t size_of_locals_in_bytes = (parm_start + locals) * sizeof(uint32_t);
	uint32_t* local_stack = nullptr;
	if (size_of_locals_in_bytes > 0) {
		local_stack= (uint32_t*)_alloca(size_of_locals_in_bytes);
		std::memcpy(local_stack, vm.pr_globals, size_of_locals_in_bytes);
	}


	for (size_t i = 0, o = (size_t)f->parm_start; i<(size_t)f->numparms; i++)
	{
		for (size_t j = 0; j< (size_t )f->parm_size[i]; j++, o++)
		{
			((uint32_t *)vm.pr_globals)[o] = ((uint32_t *)vm.pr_globals)[OFS_PARM0 + i * 3 + j];
		}
	}

	if (debug_depth) debug_runaway = 10000; // start runway checking
	else debug_depth++;
	// we are now IN the function, run the vm!
	if (pr_trace) {
		quake::decomp::DecompileImmediate(vm.pr_xfunction, 0, 0);
	}
	// vm, run till we are done
	while (true) {
		while (1)
		{
			auto st = &vm.pr_statements[s++]; // get the statement
			auto a = (eval_t *)&vm.pr_globals[st->a];
			auto b = (eval_t *)&vm.pr_globals[st->b];
			auto c = (eval_t *)&vm.pr_globals[st->c];

			if (!--debug_runaway)
				PR_RunError("runaway loop error");

			vm.pr_xfunction->profile++;
			vm.pr_xstatement = s;

			if (pr_trace) {
				dstatement_t _st = *st;
				DecompileDecompileStatement(quake::debug, vm.pr_xfunction, &_st, debug_ident);
				//quake::debug << *st;
			}



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
				c->_float = !a->string.empty();
				break;
			case OP_NOT_FNC:
				c->_float = a->function == nullptr;
				break;
			case OP_NOT_ENT:
				c->_float = a->edict == nullptr;
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
				c->_float = a->string == b->string ? 1.0f : 0.0f;
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
				c->_float = a->string != b->string ? 1.0f : 0.0f;
				break;
			case OP_NE_E:
				c->_float = a->_int != b->_int;
				break;
			case OP_NE_FNC:
				c->_float = a->function != b->function;
				break;

				//==================
			case OP_STORE_F:
				b->_float = a->_float;
				break;
			case OP_STORE_ENT:
				b->edict = a->edict;
				break;
			case OP_STORE_FLD:
				b->field = a->field;
				break;
			case OP_STORE_S:
				b->string = a->string;
				break;
			case OP_STORE_FNC:
				b->function = a->function;
				break;
			case OP_STORE_V:
				b->vector[0] = a->vector[0];
				b->vector[1] = a->vector[1];
				b->vector[2] = a->vector[2];
				break;
				// special case.  We are storying a field into an edict but we have to figure out what field it is
				// from a raw byte pointer, witch is stupid
				// the original code takes the raw byte offsets
				// might end up going though all the statements and "fix" them
			case OP_STOREP_F:
			{
				//= (eval_t *)((byte *)sv.worldedict + b->_int);
				//ptr->_int = a->_int;
				auto e = vm.PROG_TO_EDICT_AND_FIELD(b->_int); // no type checking yet though
				e.second->_float = a->_float;
			}

			case OP_STOREP_ENT:
			{
				auto e = vm.PROG_TO_EDICT_AND_FIELD(b->_int); // no type checking yet though
				e.second->edict = a->edict;
			}
			break;
			case OP_STOREP_FLD:		// integers
			{
				auto e = vm.PROG_TO_EDICT_AND_FIELD(b->_int); // no type checking yet though
				e.second->field = a->field;
			}
			break;
			case OP_STOREP_S:
			{
				auto e = vm.PROG_TO_EDICT_AND_FIELD(b->_int); // no type checking yet though
				e.second->string = a->string;
			}
			break;
			case OP_STOREP_FNC:		// pointers
			{
				auto e = vm.PROG_TO_EDICT_AND_FIELD(b->_int); // no type checking yet though
				e.second->function = a->function;
			}
			break;
			case OP_STOREP_V:
			{
				auto e = vm.PROG_TO_EDICT_AND_FIELD(b->_int); // no type checking yet though
				e.second->vector[0] = a->vector[0];
				e.second->vector[1] = a->vector[1];
				e.second->vector[2] = a->vector[2];
			}
			case OP_ADDRESS:
				assert(a->edict);
#ifdef PARANOID
				vm.NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
				if (a->edict == sv.worldedict && sv.state == ss_active)
					PR_RunError("assignment to world entity");
				c->_int = (byte *)((int *)&a->edict->v + b->_int) - (byte *)sv.worldedict;
				// so... mabye op address is used for the storep?  if thats the case I might be able to trace it
				break;

			case OP_LOAD_F:
			{
				a = (eval_t *)((int *)&a->edict->v + b->_int);
				c->_float = a->_float;
			}
			break;
			case OP_LOAD_FLD:
			{
				a = (eval_t *)((int *)&a->edict->v + b->_int);
				c->field = a->field;
			}
			break;
			case OP_LOAD_ENT:
			{
				a = (eval_t *)((int *)&a->edict->v + b->_int);
				c->edict = a->edict;
			}
			break;
			case OP_LOAD_S:
			{
				a = (eval_t *)((int *)&a->edict->v + b->_int);
				c->string = a->string;
			}
			break;
			case OP_LOAD_FNC:
			{
				a = (eval_t *)((int *)&a->edict->v + b->_int);
				c->function = a->function;
			}
			break;
			case OP_LOAD_V:
			{
				a = (eval_t *)((int *)&a->edict->v + b->_int);
				c->vector[0] = a->vector[0];
				c->vector[1] = a->vector[1];
				c->vector[2] = a->vector[2];
			}
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
			{
				pr_argc = st->op - OP_CALL0; // error checking
				if (!a->function)
					PR_RunError("NULL function");

				auto newf = a->function;
				size_t findex = newf - vm.pr_functions;
				if (!findex || findex >= vm.progs->numfunctions) {
					if (vm.pr_global_struct->self)
						vm.pr_global_struct->self->Print();
					Host_Error("PR_ExecuteProgram: NULL function");
				}

				if (newf->first_statement < 0)
				{	// negative statements are built in functions
					int i = -newf->first_statement;
					if (i >= pr_numbuiltins)
						PR_RunError("Bad builtin call number");
					pr_builtins[i]();
					break;
				}
				s = newf->call();
			}
			break;

			case OP_DONE:
			case OP_RETURN:
				vm.pr_globals[OFS_RETURN] = vm.pr_globals[st->a];
				vm.pr_globals[OFS_RETURN + 1] = vm.pr_globals[st->a + 1];
				vm.pr_globals[OFS_RETURN + 2] = vm.pr_globals[st->a + 2];

				// this is where we leave
				if (debug_depth <= 0)
					Sys_Error("prog stack underflow");
				// copy the locals back
				// we should fix this so we can detect locals and we have the vm go there
				// but the memory is allocated because of qgcc.  meh I have to look more at that 
				// compiler
				if (size_of_locals_in_bytes > 0)
					std::memcpy((((int *)vm.pr_globals)+vm.pr_xfunction->parm_start), local_stack, size_of_locals_in_bytes);
				// restore locals from the stack

				// up stack
				debug_depth--;
				vm.pr_xfunction = last_f.f;

				return last_f.s;

			case OP_STATE:
			{
				assert(vm.pr_global_struct->self);
				auto ed = vm.pr_global_struct->self;
#ifdef FPS_20
				ed->v.nextthink = vm.pr_global_struct->time + 0.05;
#else
				ed->v.nextthink = vm.pr_global_struct->time + static_cast<float>(idTime(100ms));
#endif
				if (a->_float != ed->v.frame) ed->v.frame = a->_float;
				ed->v.think = b->function;
			}


			break;

			default: {
				pr_run_error_buffer_t buf;
				std::ostream ss(&buf);
				ss << "Bad opcode " << st->op << std::endl;
				assert(0);
			}
			}

		}
	}
}

