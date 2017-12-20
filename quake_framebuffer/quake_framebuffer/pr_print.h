#pragma once

#include "quakedef.h"

// decompiler from  
// https://github.com/lbsmith/ProQCC
// just simple c++ conversions

namespace quake {
	namespace decomp {


		using gofs_t = ptrdiff_t; // global offset?
		struct type_t;
		struct def_t {
			type_t *type;
			char *name;
			def_t *next, *prev;
			gofs_t ofs;
			def_t *scope;	// function the var was defined in, or NULL

			int initialized;		// 1 when a declaration included "= immediate"

		};
		struct type_t {
			etype_t type;
			def_t *def;		// a def that points to this type
			int word_size;

			type_t *next;
			// function types are more complex
			type_t *aux_type;	// return type or field type

			int num_parms;		// -1 = variable args

			type_t *parm_types[MAX_PARMS];	// only [num_parms] allocated

		};

		class Indent {
			size_t _count;
		public:

			constexpr Indent() noexcept : _count(0U) {}
			constexpr Indent(size_t count) noexcept: _count(count) {}
			constexpr int count() const noexcept { return _count; }
			void indent(std::ostream& os) const {
				size_t c = _count;
				while(c--) os << ' ';
			}
			Indent& operator++() { ++_count; return *this; }
			Indent operator++(int) { Indent tmp(*this); ++_count; return tmp; }
			Indent& operator--() { --_count; return *this; }
			Indent operator--(int) { Indent tmp(*this); --_count; return tmp; }

			Indent& operator+=(int c) { _count += c; return *this; }
			Indent& operator-=(int c) { _count -= c; return *this; }
		};
		inline std::ostream& operator<<(std::ostream& os, const Indent& i) { i.indent(os); return os; }

		void DecompileDecompileStatement(std::ostream& os,  const dfunction_t * df, dstatement_t * s, Indent& indent);
		void DecompileDecompileFunction(std::ostream& os, const dfunction_t * df);
		void DecompileFunction(std::ostream& os, cstring_t name);
		std::string  DecompilePrintParameter(const ddef_t * def);

		std::string  DecompileValueString(etype_t type, void *val);
		std::string DecompileGlobal(gofs_t ofs, const def_t * req_t);
		gofs_t DecompileScaleIndex(const dfunction_t * df, gofs_t ofs);
		const std::string& DecompileImmediate(const dfunction_t * df, gofs_t ofs, int fun, std::string_view n= std::string_view());
		void DecompileDecompileFunction(std::ostream& os, const dfunction_t * df);
		std::string  DecompileValueString(etype_t type, void *val);
		std::string  DecompilePrintParameter(const ddef_t * def);
		const ddef_t * DecompileGetParameter(gofs_t ofs);
		void DecompileFunction(std::ostream& os, cstring_t name);
		std::string DecompileGlobalString(gofs_t ofs, bool with_contents=true);

		std::string DecompileGet(const dfunction_t * df, gofs_t ofs, const def_t * req_t);
	};
};
