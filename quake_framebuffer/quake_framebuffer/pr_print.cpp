#include "icommon.h"
#include "pr_print.h"


namespace quake {
	namespace decomp {

		int DecompileFileCtr = 0;
		static std::vector<std::string> DecompileProfiles;

		constexpr static const char *type_names[8] =
		{
			"void",
			"string",
			"float",
			"vector",
			"entity",
			"ev_field",
			"void()",
			"ev_pointer"
		};

		extern def_t def_void;
		extern def_t def_string;
		extern def_t def_float;
		extern def_t def_vector;
		extern def_t def_entity;;
		extern def_t def_field;
		extern def_t def_function;
		extern def_t def_pointer;
		type_t type_void = { etype_t::ev_void, &def_void ,1 };
		type_t type_string = { etype_t::ev_string, &def_string ,1 };
		type_t type_float = { etype_t::ev_float, &def_float,1 };
		type_t type_vector = { etype_t::ev_vector, &def_vector,3 };
		type_t type_entity = { etype_t::ev_entity, &def_entity ,1};
		type_t type_field = { etype_t::ev_field, &def_field,1 };
		type_t type_function = { etype_t::ev_function, &def_function, 1, NULL, &type_void };

		/* type_function is a void() function used for state defs */
		type_t type_pointer = { etype_t::ev_pointer, &def_pointer,1 };

		type_t type_floatfield = { etype_t::ev_field, &def_field, 1, NULL, &type_float };

		int type_size[8] = { 1, 1, 1, 3, 1, 1, 1, 1 };

		def_t def_void = { &type_void, "temp" };
		def_t def_string =  { &type_string, "temp" };
		def_t def_float = { &type_float, "temp" };
		def_t def_vector ={ &type_vector, "temp" };
		def_t def_entity ={ &type_entity, "temp" };
		def_t def_field ={ &type_field, "temp" };
		def_t def_function ={ &type_function, "temp" };
		def_t def_pointer ={ &type_pointer, "temp" };

		def_t def_ret, def_parms[MAX_PARMS];

		def_t *def_for_type[8] = { &def_void, &def_string, &def_float, &def_vector, &def_entity, &def_field, &def_function, &def_pointer };


		struct opcode_t {
			const char *name;
			const char *opname;
			//        float           priority;
			int priority;
			bool right_associative;
			def_t *type_a, *type_b, *type_c;
		} ;

		opcode_t pr_opcodes[] =
		{
			{ "<DONE>", "DONE", -1, false, &def_entity, &def_field, &def_void },

			{ "*", "MUL_F", 2, false, &def_float, &def_float, &def_float },
			{ "*", "MUL_V", 2, false, &def_vector, &def_vector, &def_float },
			{ "*", "MUL_FV", 2, false, &def_float, &def_vector, &def_vector },
			{ "*", "MUL_VF", 2, false, &def_vector, &def_float, &def_vector },

			{ "/", "DIV", 2, false, &def_float, &def_float, &def_float },

			{ "+", "ADD_F", 3, false, &def_float, &def_float, &def_float },
			{ "+", "ADD_V", 3, false, &def_vector, &def_vector, &def_vector },

			{ "-", "SUB_F", 3, false, &def_float, &def_float, &def_float },
			{ "-", "SUB_V", 3, false, &def_vector, &def_vector, &def_vector },

			{ "==", "EQ_F", 4, false, &def_float, &def_float, &def_float },
			{ "==", "EQ_V", 4, false, &def_vector, &def_vector, &def_float },
			{ "==", "EQ_S", 4, false, &def_string, &def_string, &def_float },
			{ "==", "EQ_E", 4, false, &def_entity, &def_entity, &def_float },
			{ "==", "EQ_FNC", 4, false, &def_function, &def_function, &def_float },

			{ "!=", "NE_F", 4, false, &def_float, &def_float, &def_float },
			{ "!=", "NE_V", 4, false, &def_vector, &def_vector, &def_float },
			{ "!=", "NE_S", 4, false, &def_string, &def_string, &def_float },
			{ "!=", "NE_E", 4, false, &def_entity, &def_entity, &def_float },
			{ "!=", "NE_FNC", 4, false, &def_function, &def_function, &def_float },

			{ "<=", "LE", 4, false, &def_float, &def_float, &def_float },
			{ ">=", "GE", 4, false, &def_float, &def_float, &def_float },
			{ "<", "LT", 4, false, &def_float, &def_float, &def_float },
			{ ">", "GT", 4, false, &def_float, &def_float, &def_float },

			{ ".", "INDIRECT", 1, false, &def_entity, &def_field, &def_float },
			{ ".", "INDIRECT", 1, false, &def_entity, &def_field, &def_vector },
			{ ".", "INDIRECT", 1, false, &def_entity, &def_field, &def_string },
			{ ".", "INDIRECT", 1, false, &def_entity, &def_field, &def_entity },
			{ ".", "INDIRECT", 1, false, &def_entity, &def_field, &def_field },
			{ ".", "INDIRECT", 1, false, &def_entity, &def_field, &def_function },

			{ ".", "ADDRESS", 1, false, &def_entity, &def_field, &def_pointer },

			{ "=", "STORE_F", 5, true, &def_float, &def_float, &def_float },
			{ "=", "STORE_V", 5, true, &def_vector, &def_vector, &def_vector },
			{ "=", "STORE_S", 5, true, &def_string, &def_string, &def_string },
			{ "=", "STORE_ENT", 5, true, &def_entity, &def_entity, &def_entity },
			{ "=", "STORE_FLD", 5, true, &def_field, &def_field, &def_field },
			{ "=", "STORE_FNC", 5, true, &def_function, &def_function, &def_function },

			{ "=", "STOREP_F", 5, true, &def_pointer, &def_float, &def_float },
			{ "=", "STOREP_V", 5, true, &def_pointer, &def_vector, &def_vector },
			{ "=", "STOREP_S", 5, true, &def_pointer, &def_string, &def_string },
			{ "=", "STOREP_ENT", 5, true, &def_pointer, &def_entity, &def_entity },
			{ "=", "STOREP_FLD", 5, true, &def_pointer, &def_field, &def_field },
			{ "=", "STOREP_FNC", 5, true, &def_pointer, &def_function, &def_function },

			{ "<RETURN>", "RETURN", -1, false, &def_void, &def_void, &def_void },

			{ "!", "NOT_F", -1, false, &def_float, &def_void, &def_float },
			{ "!", "NOT_V", -1, false, &def_vector, &def_void, &def_float },
			{ "!", "NOT_S", -1, false, &def_vector, &def_void, &def_float },
			{ "!", "NOT_ENT", -1, false, &def_entity, &def_void, &def_float },
			{ "!", "NOT_FNC", -1, false, &def_function, &def_void, &def_float },

			{ "<IF>", "IF", -1, false, &def_float, &def_float, &def_void },
			{ "<IFNOT>", "IFNOT", -1, false, &def_float, &def_float, &def_void },

			/* calls returns REG_RETURN */
			{ "<CALL0>", "CALL0", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL1>", "CALL1", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL2>", "CALL2", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL3>", "CALL3", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL4>", "CALL4", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL5>", "CALL5", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL6>", "CALL6", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL7>", "CALL7", -1, false, &def_function, &def_void, &def_void },
			{ "<CALL8>", "CALL8", -1, false, &def_function, &def_void, &def_void },

			{ "<STATE>", "STATE", -1, false, &def_float, &def_float, &def_void },

			{ "<GOTO>", "GOTO", -1, false, &def_float, &def_void, &def_void },

			{ "&&", "AND", 6, false, &def_float, &def_float, &def_float },
			{ "||", "OR", 6, false, &def_float, &def_float, &def_float },

			{ "&", "BITAND", 2, false, &def_float, &def_float, &def_float },
			{ "|", "BITOR", 2, false, &def_float, &def_float, &def_float },

			{ NULL }
		};

		constexpr static const char *builtins[79] =
		{

			NULL,
			"void (vector ang)",
			"void (entity e, vector o)",
			"void (entity e, string m)",
			"void (entity e, vector min, vector max)",
			NULL,
			"void ()",
			"float ()",
			"void (entity e, float chan, string samp, float vol, float atten)",
			"vector (vector v)",
			"void (string e)",
			"void (string e)",
			"float (vector v)",
			"float (vector v)",
			"entity ()",
			"void (entity e)",
			"void (vector v1, vector v2, float nomonsters, entity forent)",
			"entity ()",
			"entity (entity start, .string fld, string match)",
			"string (string s)",
			"string (string s)",
			"void (entity client, string s)",
			"entity (vector org, float rad)",
			"void (string s)",
			"void (entity client, string s)",
			"void (string s)",
			"string (float f)",
			"string (vector v)",
			"void ()",
			"void ()",
			"void ()",
			"void (entity e)",
			"float (float yaw, float dist)",
			NULL,
			"float (float yaw, float dist)",
			"void (float style, string value)",
			"float (float v)",
			"float (float v)",
			"float (float v)",
			NULL,
			"float (entity e)",
			"float (vector v)",
			NULL,
			"float (float f)",
			"vector (entity e, float speed)",
			"float (string s)",
			"void (string s)",
			"entity (entity e)",
			"void (vector o, vector d, float color, float count)",
			"void ()",
			NULL,
			"vector (vector v)",
			"void (float to, float f)",
			"void (float to, float f)",
			"void (float to, float f)",
			"void (float to, float f)",
			"void (float to, float f)",
			"void (float to, float f)",
			"void (float to, string s)",
			"void (float to, entity s)",
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			"void (float step)",
			"string (string s)",
			"void (entity e)",
			"void (string s)",
			NULL,
			"void (string var, string val)",
			"void (entity client, string s)",
			"void (vector pos, string samp, float vol, float atten)",
			"string (string s)",
			"string (string s)",
			"string (string s)",
			"void (entity e)"

		};

		std::string DecompileValueString(etype_t type, void *val);
		const ddef_t *DecompileGetParameter(gofs_t ofs);
		std::string DecompilePrintParameter(ddef_t * def);


		template<typename T>
		static std::string DecompileString(const T&string)
		{
			std::string ret;
			ret += '"';
			for (char s : string) {
				switch (s) {
				case '\n': 	ret += "\\n"; break;
				case '\"': 	ret += "\\\""; break;
				default:
					ret += s;
				}
			}
			ret += '"';
			return ret;
		}

		void DecompileCalcProfiles(void)
		{

			const dstatement_t *ds, *rds;

			unsigned short dom;
			DecompileProfiles.clear();

			for (size_t i = 1; i < vm.progs->numfunctions; i++) {
				std::string fname;
				const dfunction_t *df = vm.pr_functions + i;


				if (df->first_statement <= 0) {
					fname = builtins[-df->first_statement] + ' ';
					fname.append(vm.pr_functions[i].name());
				}
				else {
					ds = vm.pr_statements + df->first_statement;
					rds = NULL;

					/*
					* find a return statement, to determine the result type
					*/

					while (1) {

						dom = (ds->op) % 100;
						if (!dom)
							break;
						if (dom == OP_RETURN) {
							rds = ds;
							/*		    break; */
						}
						ds++;
					}

					/*
					* print the return type
					*/

					if ((rds != NULL) && (rds->a != 0)) {

						auto par = DecompileGetParameter(rds->a);

						if (par) {
							fname = type_names[static_cast<int>(par->type)] + ' ';
						}
						else {
							fname = "float /* Warning: Could not determine return type */ ";
						}

					}
					else {
						fname = "\nvoid ";
					}
					fname += '(';

					/*
					* determine overall parameter size
					*/
					size_t ps;
					for (size_t j = 0, ps = 0; j < df->numparms; j++)
						ps += df->parm_size[j];

					if (ps > 0) {

						for (size_t j = df->parm_start; j < (df->parm_start) + ps; j++) {
							auto par = DecompileGetParameter(j);

							if (!par)
								Sys_Error("Sys_Error - No parameter names with offset %i.", j);

							if (par->type == etype_t::ev_vector)
								j += 2;

							if (j < (df->parm_start) + ps - 1) {
								fname += DecompilePrintParameter(par) + ' ';
							}
							else {
								fname += DecompilePrintParameter(par);
							}
							//fname += line;
						}

					}
					fname += ')';
					fname += vm.pr_functions[i].name();
				}
				DecompileProfiles.emplace_back(std::move(fname));
			}
		}

		std::string DecompileGlobal(gofs_t ofs, const def_t * req_t)
		{
			int i;
			char *res;
			char found = 0;

			std::string line;
			for (size_t i = 0; i < vm.progs->numglobaldefs; i++) {
				const ddef_t *def = &vm.pr_globaldefs[i];

				if (def->ofs == ofs) {
					/*
					printf("DecompileGlobal - Found %i at %i.\n", ofs, (int)def);
					*/
					if (def->name() == "IMMEDIATE")
						line = DecompileValueString(def->type, &vm.pr_globals[def->ofs]);
					else {
						line = def->name().c_str();
						if (def->type == etype_t::ev_vector && req_t == &def_float)
							line += "_x";

					}
					break;
				}
			}
			return line;
		}

		gofs_t DecompileScaleIndex(const dfunction_t * df, gofs_t ofs)
		{
			return ofs > RESERVED_OFS ? (ofs - df->parm_start) + RESERVED_OFS : ofs;
		}

#define MAX_NO_LOCAL_IMMEDIATES 4096 /* dln: increased, not enough! */

		const std::string& DecompileImmediate(const dfunction_t * df, gofs_t ofs, int fun, std::string_view n)
		{
			static std::map<gofs_t, std::string> IMMEDIATES;
			static std::string empty("");
			/*
			* free 'em all
			*/
			if (fun == 0) {
				/*
				printf("DecompileImmediate - Initializing function environment.\n");
				*/
				IMMEDIATES.clear();
				return empty;
			}
			gofs_t nofs = DecompileScaleIndex(df, ofs);
			/*
			printf("DecompileImmediate - Index scale: %i -> %i.\n", ofs, nofs);
			*/

			/*
			* check consistency
			*/
			if ((nofs <= 0) || (nofs > MAX_NO_LOCAL_IMMEDIATES - 1))
				Sys_Error("Fatal Sys_Error - Index (%i) out of bounds.\n", nofs);

			/*
			* insert at nofs
			*/
			if (fun == 1) {
				assert(!n.empty());
				IMMEDIATES[nofs] = n;
				/*
				printf("DecompileImmediate - Putting \"%s\" at index %i.\n", new, nofs);
				*/
				return empty;
			}
			/*
			* get from nofs
			*/
			if (fun == 2) {
				auto it = IMMEDIATES.find(nofs);
				assert(it != IMMEDIATES.end());
				return it->second;
				/*
				printf("DecompileImmediate - Reading \"%s\" at index %i.\n", IMMEDIATES[nofs], nofs);
				*/
				//Sys_Error("Sys_Error - %i not defined.", nofs);
			}
			return empty;
		}

		std::string DecompileGet(const dfunction_t * df, gofs_t ofs, const def_t * req_t)
		{
			auto arg1 = DecompileGlobal(ofs, req_t);

			if (arg1.empty())
				arg1 = DecompileImmediate(df, ofs, 2);

			/*
			if (arg1)
			printf("DecompileGet - found \"%s\".\n", arg1);
			*/

			return arg1;
		}


		void DecompileDecompileStatement(std::ostream& os, const dfunction_t * df,  dstatement_t * s, Indent& indent)
		{

			uint16_t dom = s->op;
			uint16_t  doc = dom / 10000;
			uint16_t ifc = (dom % 10000) / 100;

			/*
			* use program flow information
			*/
			for (uint16_t  i = 0; i < ifc; i++) {
				indent--;
				os << std::endl << indent << ')' << std::endl;
			}
			for (uint16_t i = 0; i < doc; i++) {
				os << indent << "do {" << std::endl << std::endl;
				indent++;
			}

			/*
			* remove all program flow information
			*/
			s->op %= 100;

			def_t *typ1 = pr_opcodes[s->op].type_a;
			def_t *typ2 = pr_opcodes[s->op].type_b;
			def_t *typ3 = pr_opcodes[s->op].type_c;

			/*
			* printf("DecompileDecompileStatement - decompiling %i (%i):\n",(int)(s - statements),dom);
			* DecompilePrintStatement (s);
			*/
			/*
			* states are handled at top level
			*/

			if (s->op == OP_DONE || s->op == OP_STATE) {

			}
			else if (s->op == OP_RETURN) {
				os << indent << "return ";
				if (s->a) os << "( " << DecompileGet(df, s->a, typ1) << " )";
				os << ';' << std::endl;
			}
			else if ((OP_MUL_F <= s->op && s->op <= OP_SUB_V) ||
				(OP_EQ_F <= s->op && s->op <= OP_GT) ||
				(OP_AND <= s->op && s->op <= OP_BITOR)) {

				auto arg1 = DecompileGet(df, s->a, typ1);
				auto arg2 = DecompileGet(df, s->b, typ2);
				auto arg3 = DecompileGlobal(s->c, typ3);

				if (!arg3.empty()) {
					os << indent << arg3 << " = " << arg1 << ' ' << pr_opcodes[s->op].name << ' ' << arg2 << ';' << std::endl;
				}
				else {
					// humm so is imediate kind of like a stack? huh
					std::stringstream ss;
					ss << '(' << arg1 << ' ' << pr_opcodes[s->op].name << ' ' << arg2 << ')';
					DecompileImmediate(df, s->c, 1, ss.str());
				}

			}
			else if (OP_LOAD_F <= s->op && s->op <= OP_ADDRESS) {
				auto arg1 = DecompileGet(df, s->a, typ1);
				auto arg2 = DecompileGet(df, s->b, typ2);
				auto arg3 = DecompileGlobal(s->c, typ3);

				if (!arg3.empty()) {
					os << indent << arg3 << " = " << arg1 << '.' << arg2 << ';' << std::endl;
				}
				else {
					// humm so is imediate kind of like a stack? huh
					std::stringstream ss;
					ss << '(' << arg1 << '.' << arg2 << ')';
					DecompileImmediate(df, s->c, 1, ss.str());

				}
			}
			else if (OP_STORE_F <= s->op && s->op <= OP_STORE_FNC) {

				auto arg1 = DecompileGet(df, s->a, typ1);
				auto arg3 = DecompileGlobal(s->b, typ2);
				if (!arg3.empty()) {
					os << indent << arg3 << " = " << arg1 << ';' << std::endl;
				}
				else {
					// humm so is imediate kind of like a stack? huh
					DecompileImmediate(df, s->b, 1, arg1);

				}
			}
			else if (OP_STOREP_F <= s->op && s->op <= OP_STOREP_FNC) {

				auto arg1 = DecompileGet(df, s->a, typ1);
				auto arg2 = DecompileGet(df, s->b, typ2);
				os << indent << arg2 << " = " << arg1 << ';' << std::endl;
			}
			else if (OP_NOT_F <= s->op && s->op <= OP_NOT_FNC) {
				auto arg1 = DecompileGet(df, s->a, typ1);
				arg1.insert(arg1.begin(), '!');
				DecompileImmediate(df, s->c, 1, arg1);

			}
			else if (OP_CALL0 <= s->op && s->op <= OP_CALL8) {
				auto nargs = s->op - OP_CALL0;

				std::string  fnam = DecompileGet(df, s->a, NULL);
				std::string line = fnam + '(';

				for (size_t i = 0; i < nargs; i++) {

					typ1 = NULL;

					size_t j = 4 + 3 * i;


					auto arg1 = DecompileGet(df, j, typ1);
					line += arg1;

#ifndef DONT_USE_DIRTY_TRICKS
					if (fnam == "WriteCoord") {
						if (arg1 == "org" || arg1 == "trace_endpos" || arg1 == "p1" || arg1 == "p2" || arg1 == "o") {
							line += "_x";
						}
					}
#endif
					if (i < nargs - 1) line += ',';
				}
				line += ')';
				DecompileImmediate(df, 1, 1, line);

				/*
				* if ( ( ( (s+1)->a != 1) && ( (s+1)->b != 1) &&
				* ( (s+2)->a != 1) && ( (s+2)->b != 1) ) ||
				* ( ((s+1)->op) % 100 == OP_CALL0 ) ) {
				* DecompileIndent(*indent);
				* fprintf(Decompileofile,"%s;\n",line);
				* }
				*/

				if ((((s + 1)->a != 1) && ((s + 1)->b != 1) &&
					((s + 2)->a != 1) && ((s + 2)->b != 1)) ||
					((((s + 1)->op) % 100 == OP_CALL0) && ((((s + 2)->a != 1)) || ((s + 2)->b != 1)))) {
					os << indent << line << ';' << std::endl;
				}
			}
			else if (s->op == OP_IF || s->op == OP_IFNOT) {

				auto arg1 = DecompileGet(df, s->a, NULL);
				auto arg2 = DecompileGlobal(s->a, NULL);

				if (s->op == OP_IFNOT) {

					if (s->b < 1)
						Sys_Error("Found a negative IFNOT jump.");

					/*
					* get instruction right before the target
					*/
					const dstatement_t *t = s + s->b - 1;
					uint16_t tom = t->op % 100;

					if (tom != OP_GOTO) {

						/*
						* pure if
						*/
						os << indent << "if ( " << arg1 << " ) {" << std::endl << std::endl;
#if 0
						fprintf(Decompileofile, "if ( %s ) { /*1*/\n\n", arg1);
#endif
						++indent;
					}
					else {

						if (t->a > 0) {
							/*
							* ite
							*/
#if 0
							fprintf(Decompileofile, "if ( %s ) { /*2*/\n\n", arg1);
#endif
							os << indent << "if ( " << arg1 << " ) {" << std::endl << std::endl;
							++indent;
						}
						else {

#if 0
							if ((((t->a + s->b) == 0) ||
								((arg2 != NULL) && ((t->a + s->b) == 1)))) {
								/*
								* while
								*/
								DecompileIndent(*indent);
								fprintf(Decompileofile, "while ( %s ) {\n\n", arg1);
								(*indent)++;
							}
							else {
								/*
								* pure if
								*/
								DecompileIndent(*indent);
								/*
								* fprintf(Decompileofile,"if ( %s ) { //3\n\n",arg1);
								*/
								fprintf(Decompileofile, "if ( %s ) {\n\n", arg1);
								(*indent)++;
							}
#endif

							if ((t->a + s->b) > 1) {
								/*
								* pure if
								*/
								/*
								fprintf(Decompileofile, "if ( %s ) { //3\n\n", arg1);
								*/
								os << indent << "if ( " << arg1 << " ) {" << std::endl << std::endl;
								++indent;
							}
							else {

								int dum = 1;
								for (const dstatement_t *k = t + (t->a); k < s; k++) {
									tom = k->op % 100;
									if (tom == OP_GOTO || tom == OP_IF || tom == OP_IFNOT)
										dum = 0;
								}
								if (dum) {	/*
											* while
											*/
									os << indent << "while ( " << arg1 << " ) {" << std::endl << std::endl;
									indent++;
								}
								else {
									/*
									* pure if
									*/

									/*
									fprintf(Decompileofile, "if ( %s ) { //3\n\n", arg1);
									*/
									os << indent << "if ( " << arg1 << " ) {" << std::endl << std::endl;
									indent++;
								}
							}
						}
					}

				}
				else {
					/*
					* do ... while
					*/

					--indent;
					os << std::endl << indent << "} while ( " << arg1 << " );" << std::endl;
				}
			}
			else if (s->op == OP_GOTO) {

				if (s->a > 0) {
					/*
					* else
					*/
					--indent;
					os << std::endl << indent << "} else {" << std::endl << std::endl;
					++indent;
				}
				else {
					/*
					* while
					*/
					--indent;
					os << std::endl << indent << '}' << std::endl;
				}
			}
			else {
				os << std::endl << "/* Warning: UNKNOWN COMMAND */" << std::endl;
			}

			/*
			printf("DecompileDecompileStatement - Current line is \"%s\"\n", line);
			*/

			return;
		}

		void DecompileDecompileFunction(std::ostream& os, const dfunction_t * df)
		{
			Indent indent;

			/*
			* Initialize
			*/
			DecompileImmediate(df, 0, 0);

			indent = 1;

			auto ds = vm.pr_statements + df->first_statement;
			while (1) {
				dstatement_t ts = *ds;
				DecompileDecompileStatement(os,df, &ts, indent);
				if (!ds->op)
					break;
				ds++;
			}

			if (indent.count() != 1)
				os << "/* Warning : Indentiation structure corrupt */" << std::endl;

		}


		std::string  DecompileValueString(etype_t type, void *val)
		{
			static char line[1024];

			line[0] = '\0';

			switch (type) {
			case etype_t::ev_string:
				return DecompileString(std::string_view((const char*)val));
			case etype_t::ev_void:
				return "void";
				break;
			case etype_t::ev_float:
				return '"' + std::to_string(*(float *)val) + '"';
				break;
			case etype_t::ev_vector:
				return 
					'"' + std::to_string(((float *)val)[0]) +
					' ' + std::to_string(((float *)val)[1]) +
					' ' + std::to_string(((float *)val)[2]) + '"';
				break;
			default:
				return "bad type " + std::to_string((int)type);
				break;
			}

			return line;
		}

		std::string  DecompilePrintParameter(const ddef_t * def)
		{
			std::string line;

			if (def->name() ==  "IMMEDIATE")
				line = DecompileValueString(def->type, &vm.pr_globals[def->ofs]);
			else {
				line = type_names[static_cast<int>(def->type)] + ' ';
				line.append(def->name());
			}
				
			return line;
		}


		const ddef_t * DecompileGetParameter(gofs_t ofs)
		{
			return vm.ED_GlobalAtOfs(ofs);
		}

		void DecompileFunction(std::ostream& os, cstring_t name)
		{
			std::string line;

			const dfunction_t * df = vm.ED_FindFunction(name);

			if (df == nullptr)
				Sys_Error("No function named \"%s\"", name);
			const dfunction_t * dfpred = df - 1;
			size_t findex = df - vm.pr_functions;
			/*
			* Check ''local globals''
			*/
			size_t ps;
			for (size_t j = 0, ps = 0; j < dfpred->numparms; j++)
				ps += dfpred->parm_size[j];

			size_t start = dfpred->parm_start + dfpred->locals + ps;
			if (dfpred->first_statement < 0 && df->first_statement > 0) start -= 1;
			if (start == 0) start = 1;

			size_t end = df->parm_start;

			for (size_t j = start; j < end; j++) {

				const ddef_t* par = DecompileGetParameter(j);

				if (par) {
					const ddef_t* type_hack = par;
					
					if (static_cast<uint16_t>(par->type) & (1 << 15)) {
						ddef_t* tmp = (ddef_t*)_alloca(sizeof(ddef_t));
						std::memcpy(tmp, par, sizeof(ddef_t));
						tmp->type = static_cast<etype_t>(static_cast<uint16_t>(par->type) - (1 << 15));
						type_hack = tmp;
					}
					else type_hack = par;


					if (type_hack->type == etype_t::ev_function) {

						if (type_hack->name() == "IMMEDIATE" && name == "IMMEDIATE") {
							os << DecompileProfiles[vm.ED_FindFunctionIndex(type_hack->name())] << std::endl;
						}
					}


					else if (type_hack->type != etype_t::ev_pointer)
						if (type_hack->name() == "IMMEDIATE") {
							if (par->type == etype_t::ev_field) {

								auto ef = vm.ED_FindField(type_hack->name());

								if (!ef)
									Sys_Error("Could not locate a field named \"%s\"",  par->name().c_str());

								if (ef->type == etype_t::ev_vector)
									j += 3;

#ifndef DONT_USE_DIRTY_TRICKS
								if ((ef->type == etype_t::ev_function) && ef->name() == "th_pain") {
									os << ".void(entity attacker, float damage) th_pain;" << std::endl;
								}
								else
#endif
									os << '.' << type_names[static_cast<int>(ef->type)] << ' ' << ef->name() << ';' << std::endl;
							}
							else {

								if (par->type == etype_t::ev_vector)
									j += 2;

								if (par->type == etype_t::ev_entity || par->type == etype_t::ev_void) {
									os << type_names[static_cast<int>(par->type)] << ' ' << par->name();
								}
								else {
									if (par->name().size() > 1 &&
										isupper(par->name()[0]) &&
										(isupper(par->name()[1]) || par->name()[1] == '_')) {
										os << type_names[static_cast<int>(par->type)] << ' ' << par->name() << "    = " << DecompileValueString(par->type, &vm.pr_globals[par->ofs]) << ';' << std::endl;
									}
									else
										os << type_names[static_cast<int>(par->type)] << ' ' << par->name() << " /* = " << DecompileValueString(par->type, &vm.pr_globals[par->ofs]) << " */;" << std::endl;
								}
							}
						}
				}
			}
			/*
			* Check ''local globals''
			*/

			if (df->first_statement <= 0) {
				os << DecompileProfiles[findex] << " = #" - df->first_statement << ';' << std::endl;
				return;
			}

			const dstatement_t	*ds = vm.pr_statements + df->first_statement;

			while (1) {
				dstatement_t ts;
				uint16_t tom;
				uint16_t dom = (ds->op) % 100;

				if (!dom)
					break;

				else if (dom == OP_GOTO) {
					/*
					* check for i-t-e
					*/

					if (ds->a > 0) {
						ts = *(ds + ds->a);
						ts.op += 100;	/*
										* mark the end of a if/ite construct
										*/
					}
				}
				else if (dom == OP_IFNOT) {
					/*
					* check for pure if
					*/

					ts = *(ds + ds->b);
					tom = (ds - 1)->op % 100;

					if (tom != OP_GOTO)
						ts.op += 100;	/*
										* mark the end of a if/ite construct
										*/
					else if ((ds - 1)->a < 0) {

						/*
						arg2 = DecompileGlobal(ds->a, NULL);
						if (!((((ts - 1)->a + ds->b) == 0) ||
						((arg2 != NULL) && (((ts - 1)->a + ds->b) == 1))))
						(ts - 1)->op += 100;
						if (arg2)
						free(arg2);
						*/

						if (((ds - 1)->a + ds->b) > 1) {
							/*
							* pure if
							*/
							ts.op += 100;	/*
											* mark the end of a if/ite construct
											*/
						}
						else {

							auto dum = 1;
							for (auto k = (ds - 1) + ((ds - 1)->a); k < ds; k++) {
								tom = k->op % 100;
								if (tom == OP_GOTO || tom == OP_IF || tom == OP_IFNOT)
									dum = 0;
							}
							if (!dum) {
								/*
								* pure if
								*/
								ts.op += 100;	/*
												* mark the end of a if/ite construct
												*/
							}
						}
					}
				}
				else if (dom == OP_IF) {
					ts = *(ds + ds->b);
					ts.op += 10000;	/*
										* mark the start of a do construct
										*/
				}
				ds++;
			}

			/*
			* print the prototype
			*/
			os << DecompileProfiles[findex];

			/*
			* handle state functions
			*/

			ds = vm.pr_statements + df->first_statement;

			if (ds->op == OP_STATE) {

				auto par = DecompileGetParameter(ds->a);
				if (!par)
					Sys_Error("Sys_Error - Can't determine frame number.");

				auto arg2 = DecompileGet(df, ds->b, NULL);
				if (arg2.empty())
					Sys_Error("Sys_Error - No state parameter with offset %i.", ds->b);

				os << " = [ " << DecompileValueString(par->type, &vm.pr_globals[par->ofs]) << ", " << arg2 << " ]";

			}
			else {
				os << " =";
			}
			os << std::endl << std::endl;
			/*
			fprintf(Decompileprofile, "%s", DecompileProfiles[findex]);
			fprintf(Decompileprofile, ") %s;\n", name);
			*/

			/*
			* calculate the parameter size
			*/

			for (uint16_t j = 0, ps = 0; j < df->numparms; j++)
				ps += df->parm_size[j];

			/*
			* print the locals
			*/

			if (df->locals > 0) {

				if ((df->parm_start) + df->locals - 1 >= (df->parm_start) + ps) {

					for (auto i = df->parm_start + ps; i < (df->parm_start) + df->locals; i++) {

						auto par = DecompileGetParameter(i);

						if (!par) {
							os << "   /* Warning: No local name with offset %i */" << std::endl;
						}
						else {

							if (par->type == etype_t::ev_function)
								os << "   /* Warning: Fields and functions must be global */" << std::endl;
							else
								os << "   local " << DecompilePrintParameter(par) << ';' << std::endl;
							if (par->type == etype_t::ev_vector)
								i += 2;
						}
					}
					os << std::endl;
				}
			}
			/*
			* do the hard work
			*/

			DecompileDecompileFunction(os, df);
			os << std::endl << "};" << std::endl;
		}


		std::string DecompileGlobalString(gofs_t ofs,bool with_contents)
		{
			const ddef_t *def = vm.ED_GlobalAtOfs(ofs);
			std::string line;
			line.reserve(20);
			line = std::to_string(ofs);
			if (def == nullptr) {
				line += "(???)";
			}
			else {
				line += '(';
				if (with_contents) {
					if (def->name() != "IMMEDIATE") {
						line.append(def->name());
						line += ':';
					}
					line.append(PR_ValueString(def->type, (eval_t*)&vm.pr_globals[ofs]).to_string());
					line += ')';
				}
				else {
					line += '(';
					line.append(def->name());
					line += ')';
				}
			}
			if (line.size() < 16) line.append(16 - line.size(), ' ');
			line += ' ';
			return line;
		}
		std::string DecompileGlobalStringNoContents(gofs_t ofs) { return DecompileGlobalString(ofs, false); }
		void DecompilePrintStatement(std::ostream& os, const dstatement_t * s)
		{
			std::stringstream line;
			line << std::setw(4) << (int)(s - vm.pr_statements) << " : " << pr_opcodes[s->op].opname;
			os << std::setw(10) << line.str();
			line.clear();

			if (s->op == OP_IF || s->op == OP_IFNOT)
				os << DecompileGlobalString(s->a) << "branch " << s->b;
			else if (s->op == OP_GOTO) {
				os <<  "branch " << s->a;
			}
			else if ((unsigned)(s->op - OP_STORE_F) < 6) {
				os << DecompileGlobalString(s->a) << DecompileGlobalStringNoContents(s->b);
			}
			else {
				if (s->a)
					os << DecompileGlobalString(s->a);
				if (s->b)
					os << DecompileGlobalString(s->b);
				if (s->c)
					os << DecompileGlobalString(s->c);
			}
			os << std::endl;
		}
#if 0
		void DecompilePrintFunction(std::ostream& os, const  std::string_view name)
		{
			int i;
			dstatement_t *ds;
			dfunction_t *df;

			for (i = 0; i < numfunctions; i++)
				if (!strcmp(name, strings + functions[i].s_name))
					break;
			if (i == numfunctions)
				Sys_Error("No function names \"%s\"", name);
			df = functions + i;

			printf("Statements for %s:\n", name);
			ds = statements + df->first_statement;
			while (1) {
				DecompilePrintStatement(ds);

				if (!ds->op)
					break;
				ds++;
			}
		}
#endif

	}
}