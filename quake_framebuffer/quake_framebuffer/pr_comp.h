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
#ifndef _QUAKE_PR_COMP_H_
#define _QUAKE_PR_COMP_H_
// this file is shared by quake and qcc
#include <type_traits>

typedef int	func_t;
typedef int	string_t;

//#define	DEF_SAVEGLOBAL	(1<<15)
static constexpr uint16_t def_saveglobal = (1 << 15);
enum class etype_t : uint16_t {ev_void=0, ev_string, ev_float, ev_vector, ev_entity, ev_field, ev_function, ev_pointer, ev_saveglobal = def_saveglobal } ;

class idType {
public:
	using underlying_type = std::underlying_type<etype_t>::type;
	static constexpr size_t	type_size[8] = { 1, sizeof(string_t) / 4,1,3,1,1,sizeof(func_t) / 4, sizeof(void *) / 4 };
	static constexpr const char*	type_string[8] = { "void","string","float","vector","entity","field","function","pointer" };
	constexpr static inline etype_t ClearSaveGlobal(etype_t t) {
		return static_cast<etype_t>(static_cast<underlying_type>(t) & ~def_saveglobal);
	}
	constexpr static inline etype_t SetSaveGlobal(etype_t t) {
		return static_cast<etype_t>(static_cast<underlying_type>(t) | def_saveglobal);
	}
	constexpr static inline bool isSetSaveGlobal(etype_t t) {
		return static_cast<underlying_type>(t) & def_saveglobal;
	}
	constexpr static inline size_t eTypeSize(etype_t t) {
		return type_size[static_cast<underlying_type>(t) & ~def_saveglobal];
	}
	constexpr static inline const char* eTypeString(etype_t t) {
		return type_string[static_cast<underlying_type>(t) & ~def_saveglobal];
	}
	constexpr idType(etype_t t = etype_t::ev_void) : _type(t) {}
	constexpr idType(etype_t t,bool save_global) : _type(save_global ? SetSaveGlobal(t) : ClearSaveGlobal(t)) {}
	constexpr size_t size() const { return eTypeSize(_type); }
	constexpr const char* string() const { return eTypeString(_type); }
	constexpr bool saveglobal() const { return isSetSaveGlobal(_type); }
	constexpr operator etype_t() const { return ClearSaveGlobal(_type); }
private:
	etype_t _type;
};




#define	OFS_NULL		0
#define	OFS_RETURN		1
#define	OFS_PARM0		4		// leave 3 ofs for each parm to hold vectors
#define	OFS_PARM1		7
#define	OFS_PARM2		10
#define	OFS_PARM3		13
#define	OFS_PARM4		16
#define	OFS_PARM5		19
#define	OFS_PARM6		22
#define	OFS_PARM7		25
#define	RESERVED_OFS	28


enum {
	OP_DONE,
	OP_MUL_F,
	OP_MUL_V,
	OP_MUL_FV,
	OP_MUL_VF,
	OP_DIV_F,
	OP_ADD_F,
	OP_ADD_V,
	OP_SUB_F,
	OP_SUB_V,
	
	OP_EQ_F,
	OP_EQ_V,
	OP_EQ_S,
	OP_EQ_E,
	OP_EQ_FNC,
	
	OP_NE_F,
	OP_NE_V,
	OP_NE_S,
	OP_NE_E,
	OP_NE_FNC,
	
	OP_LE,
	OP_GE,
	OP_LT,
	OP_GT,

	OP_LOAD_F,
	OP_LOAD_V,
	OP_LOAD_S,
	OP_LOAD_ENT,
	OP_LOAD_FLD,
	OP_LOAD_FNC,

	OP_ADDRESS,

	OP_STORE_F,
	OP_STORE_V,
	OP_STORE_S,
	OP_STORE_ENT,
	OP_STORE_FLD,
	OP_STORE_FNC,

	OP_STOREP_F,
	OP_STOREP_V,
	OP_STOREP_S,
	OP_STOREP_ENT,
	OP_STOREP_FLD,
	OP_STOREP_FNC,

	OP_RETURN,
	OP_NOT_F,
	OP_NOT_V,
	OP_NOT_S,
	OP_NOT_ENT,
	OP_NOT_FNC,
	OP_IF,
	OP_IFNOT,
	OP_CALL0,
	OP_CALL1,
	OP_CALL2,
	OP_CALL3,
	OP_CALL4,
	OP_CALL5,
	OP_CALL6,
	OP_CALL7,
	OP_CALL8,
	OP_STATE,
	OP_GOTO,
	OP_AND,
	OP_OR,
	
	OP_BITAND,
	OP_BITOR
};


struct dstatement_t
{
	unsigned short	op;
	short	a,b,c;
} ;

struct ddef_t
{
	union {
		unsigned short	itype;		// if DEF_SAVEGLOBGAL bit is set
									// the variable needs to be saved in savegames
		etype_t type;
	};
	unsigned short	ofs;
	int			s_name;
} ;


#define	MAX_PARMS	8

struct dfunction_t
{
	int		first_statement;	// negative numbers are builtins
	int		parm_start;
	int		locals;				// total ints of parms + locals
	
	int		profile;		// runtime
	
	int		s_name;
	int		s_file;			// source file defined in
	
	int		numparms;
	byte	parm_size[MAX_PARMS];
} ;


#define	PROG_VERSION	6
struct dprograms_t
{
	int		version;
	int		crc;			// check of header file
	
	int		ofs_statements;
	int		numstatements;	// statement 0 is an error

	int		ofs_globaldefs;
	int		numglobaldefs;
	
	int		ofs_fielddefs;
	int		numfielddefs;
	
	int		ofs_functions;
	int		numfunctions;	// function 0 is an empty
	
	int		ofs_strings;
	int		numstrings;		// first string is a null string

	int		ofs_globals;
	int		numglobals;
	
	int		entityfields;

	dfunction_t * GetFunction(int offs)  { return reinterpret_cast<dfunction_t*>(reinterpret_cast<byte*>(this) + ofs_functions); }
	quake::string_view GetString(int offs)  { return (reinterpret_cast<const char*>(this) + ofs_strings); }
	ddef_t* GetGlobalDef(int offs)  { return reinterpret_cast<ddef_t*>(reinterpret_cast<byte*>(this) + ofs_globaldefs); }
	ddef_t* GetFieldDefs(int offs)  { return reinterpret_cast<ddef_t*>(reinterpret_cast<byte*>(this) + ofs_fielddefs); }
	//globalvars_t* GetGlobalVars(int offs)  { return reinterpret_cast<globalvars_t*>(reinterpret_cast<byte*>(this) + ofs_globals); }
	//dstatement_t* GetStatements(int offs)  { return reinterpret_cast<dstatement_t*>(reinterpret_cast<byte*>(this) + ofs_statements); }

	// debugging, going to be used for lookup
	static UMap<quake::string_view, ddef_t*> globals;
	static UMap<quake::string_view, ddef_t*> fields;
	static UMap<quake::string_view, dfunction_t*> functions;
} ;

#endif