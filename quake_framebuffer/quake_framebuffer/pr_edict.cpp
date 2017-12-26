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
// sv_edict.c -- entity dictionary

/*
	Some side notes on the original Quake.  I havn't looked at Quake2 yet but they do ALOT of string copies.  First its copyied to com_token, THEN to another buff
	THEN another buffer for vector_t for the Q_atof.  Thats just WAY to much because ware are directly moving it to the dictionary
	So I made a class c alled std::string_view that contains the raw data from the file (poisiton size) for a token with inbuilt "to float" function so we don't have to do all
	that crazy copying.

	Just watch it as if the file is unloaded all the std::string_view's suddenly become invalid
*/
#include "icommon.h"

using namespace std::chrono;


pr_system_t vm;
#if 0
dprograms_t		*progs;
dfunction_t		*pr_functions;
char			*pr_strings;
ddef_t			*pr_fielddefs;
ddef_t			*pr_globaldefs;
dstatement_t	*pr_statements;
globalvars_t	*vm.pr_global_struct;
float			*pr_globals;			// same as vm.pr_global_struct
int				pr_edict_size;	// in bytes


#endif

// mainly for debugging

// not sure where to put this



qboolean	ED_ParseEpair(void *base, const ddef_t *key, const std::string_view& value);
qboolean	ED_ParseEpair(edict_t *base, const ddef_t *key, const std::string_view& value);

cvar_t<float> nomonsters = { 0.0f} ;
cvar_t<float> gamecfg = { 0.0f} ;
cvar_t<float> scratch1 = { 0.0f} ;
cvar_t<float> scratch2 = { 0.0f} ;
cvar_t<float> scratch3 = { 0.0f} ;
cvar_t<float> scratch4 = { 0.0f} ;
cvar_t<float> savedgamecfg = { 0.0f, true} ;
cvar_t<float> saved1 = { 0.0f, true} ;
cvar_t<float> saved2 = { 0.0f, true} ;
cvar_t<float> saved3 = { 0.0f, true} ;
cvar_t<float> saved4 = { 0.0f, true} ;


edict_t::edict_t(int num, int offset) : num(num) , offset(offset),free(false), vars(vm._field_info,&v) {
	ClearFields();
	set("origin", vec3_origin);
	set("angles", vec3_origin);
	set("nextthink", -1.0f);
}
edict_t::~edict_t() {
	if (area.islinked()) area.remove<&edict_t::area>();
}
void edict_t::ClearFields() {
	std::memset(&v, 0, vm.progs->entityfields * sizeof(float));
	set("origin", vec3_origin);
	set("angles", vec3_origin);
	set("nextthink", -1.0f);
}

/*
=================
ED_Alloc

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
void pr_system_t::ED_HulkAllocEdicts(size_t count) {

	const size_t total_size = pr_edict_size;
	const size_t var_size = sizeof(entvars_t);
	const size_t bentityfields = progs->entityfields * 4;


	const size_t size_of_entvars = bentityfields - var_size;
	const size_t edict_value_size = pr_edict_size - sizeof(edict_t) + var_size;
	const size_t field_size = progs->entityfields * sizeof(uint32_t);


	const size_t sizeof_edict = sizeof(edict_t) + progs->entityfields * 4 - sizeof(entvars_t);
	const size_t allinged_sizeof_edict = (sizeof_edict + (sizeof(float) - 1)) & size_t(-int(sizeof(float)));
	const size_t total_size_of_all_edicts = allinged_sizeof_edict * count;

	assert(pr_edict_size == allinged_sizeof_edict);
	free_pool.clear();
	used_pool.clear();
	reserved_pool.clear();
	pr_edicts.clear();

	pr_edicts.reserve(count);
	pr_edicts.clear();
	
	char* ptr = (char*)Hunk_AllocName(total_size_of_all_edicts, "edicts");
	for (size_t i = 0; i < count; i++) {
		ptrdiff_t offset = (i * allinged_sizeof_edict);
		edict_t* e = new(ptr + offset) edict_t(i, offset);
		e->free = true;
		pr_edicts.emplace_back(e);
		free_pool.emplace_back(e);
#if 0
		block_header_t* b = reinterpret_cast<block_header_t*>(ptr + (i * pr_edict_size));
		new(b) block_header_t;

		free_pool.push_back(b);
#endif

	}
}
edict_t *pr_system_t::ED_Alloc(bool reserve, edict_t* e)
{
	if (e == nullptr) {
		if (free_pool.empty()) { // long search for one thats free
			std::copy_if(
				std::make_move_iterator(used_pool.begin()),
				std::make_move_iterator(used_pool.end()),
				std::back_inserter(free_pool),
				[](const edict_t* e) { return e->free; }
			);
			assert(!free_pool.empty()); // if we are still empty, something went wrong
		}
		e = free_pool.front();
		free_pool.pop_front();
	}
	else {
		assert(e->free); // make sure we are free on suggestions
	}
	e->free = false;
	if (!reserve)
		used_pool.insert(e);
	else
		reserved_pool.insert(e);
	return e;
}

/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void pr_system_t::ED_Free(edict_t *e)
{
	if (!e->free) {
		used_pool.erase(e);
		e->free = true;
		e->ClearFields();
		e->freetime = sv.time;
		free_pool.emplace_back(e);
		//std::destroy_at(e);
	}
}







/*
============
PR_UglyValueString

Returns a string describing *data in a type specific manner
Easier to parse than PR_ValueString
=============
*/
class PR_UglyValueString : public quake::stream_output {
public:
	PR_UglyValueString(idType type, eval_t *val) : type(type), val(val) {}
	void text_output(std::ostream& os) const override final {


		switch (type)
		{
		case etype_t::ev_string:
			os << val->string.c_str();
			break;
		case etype_t::ev_entity:
			os << (int)(vm.NUM_FOR_EDICT(val->edict));
			break;
		case etype_t::ev_function:
			os << val->function->name();
		break;
		case etype_t::ev_field: 
			os << val->field->name();
		break;
		case etype_t::ev_void:
			os << "void";
			break;
		case etype_t::ev_float:
			os << val->_float;
			break;
		case etype_t::ev_vector:
			os << val->vector[0] << ' ' << val->vector[1] << ' ' << val->vector[2];
			break;
		default:
			os << "bad type " << (int)(etype_t)type;
			break;
		}
	}
private:
	idType type;
	eval_t *val;
};




/*
=============
ED_Print

For debugging
=============
*/

void edict_t::Print()
{

	auto ed = this;

	quake::con << std::endl;
	quake::con << "EDICT " << vm.NUM_FOR_EDICT(ed) << ':' << std::endl;
	assert(0);
#if 0
	ddef_t	*d;
	int		*v;
	char	*name;
	int j;
	for (int i=1 ; i<vm.progs->numfielddefs ; i++)
	{
		d = &vm.pr_fielddefs[i];
		name = vm.pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars
			
		v = (int *)((char *)&ed->v + d->ofs*4);
		eval_t* eval = (eval_t *)v;
	// if the value is still all 0, skip the field
		idType type = d->type;
		size_t type_size = type.size();

		for (j=0 ; j<type_size; j++)
			if (v[j])
				break;
		if (j == type_size)
			continue;
	
		quake::con << std::left << std::setw(15) << name << quake::PR_ValueString(d->type, (eval_t *)v) << std::endl;	
	}
#endif
}

/*
=============
ED_Write

For savegames
=============
*/
void ED_Write(std::ostream& f, edict_t *ed)
{

	assert(0);
#if 0
	ddef_t	*d;
	int		*v;
	size_t j;
	char	*name;
	idType	type;
	f << '{' << std::endl;

	for (size_t i = 1; i < (size_t)vm.progs->numfielddefs; i++)
	{
		d = &vm.pr_fielddefs[i];
		name = vm.pr_strings + d->s_name;
		if (name[strlen(name) - 2] == '_')
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs * 4);

		// if the value is still all 0, skip the field

		type = d->type;
		for (j = 0; j < type.size(); j++)
			if (v[j])
				break;
		if (j == type.size())
			continue;
		f << '"' << name << "\" \"" << PR_UglyValueString(d->type, (eval_t *)v) << '"' << std::endl;
	}

	f << ')' << std::endl;
#endif
}

void ED_PrintNum (int ent)
{
	vm.EDICT_NUM(ent)->Print ();
}

/*
=============
ED_PrintEdicts

For debugging, prints all the entities in the current server
=============
*/
void ED_PrintEdicts()
{

	Con_Printf ("%i entities\n", vm.used_pool.size());
	for (auto e : vm) {
		assert(0);
		//ED_PrintNum(i);
	}

}

/*
=============
ED_PrintEdict_f

For debugging, prints a single edicy
=============
*/
void ED_PrintEdict_f(cmd_source_t source, const StringArgs& args)
{
	size_t i = (size_t)Q_atoi (args[1]);
	if (i >= vm.used_pool.size())
	{
		Con_Printf("Bad edict number\n");
		return;
	}
	assert(0);
	ED_PrintNum (i);
}

/*
=============
ED_Count

For debugging
=============
*/
void ED_Count(cmd_source_t source, const StringArgs& args)
{
	const edict_t	*ent;
	int		active, models, solid, step;

	active = models = solid = step = 0;
	for (const auto& it : vm) {
		ent = it;
		active++;
		if (ent->v.solid)
			solid++;
		if (!ent->v.model.empty()) {
			models++;
			//assert(ent.vars->find("model") != ent.vars.end());

		}

		if (ent->v.movetype == MOVETYPE_STEP)
			step++;
	}

	Con_Printf ("num_edicts:%3i\n", vm.used_edicts());
	Con_Printf ("active    :%3i\n", active);
	Con_Printf ("view      :%3i\n", models);
	Con_Printf ("touch     :%3i\n", solid);
	Con_Printf ("step      :%3i\n", step);

}

/*
==============================================================================

					ARCHIVING GLOBALS

FIXME: need to tag constants, doesn't really work
==============================================================================
*/

/*
=============
ED_WriteGlobals
=============
*/
void ED_WriteGlobals (std::ostream& f)
{
	f << '{' << std::endl;
	for (int i=0 ; i<vm.progs->numglobaldefs ; i++)
	{
		auto def = &vm.pr_globaldefs[i];
		idType type = def->type;
		if ( !type.saveglobal())
			continue;
		switch (type) {
		case etype_t::ev_string:
		case etype_t::ev_float:
		case etype_t::ev_entity:
			f << '"' << def->name() << "\" \"" << PR_UglyValueString(type, (eval_t *)&vm.pr_globals[def->ofs]) << '"' << std::endl;
		}
		break;
	}
	f << '}' << std::endl;
}

/*
=============
ED_ParseGlobals
=============
*/
void ED_ParseGlobals (COM_Parser& parser)
{
	std::string_view keyname,value;
	while (parser.Next(keyname))
	{	
		if (keyname.empty())
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (keyname[0]  == '}')
			break;

		if (parser.Next(value))
			Sys_Error("ED_ParseEntity: closing brace without data");

		if (value[0] == '}')
			Sys_Error("ED_ParseEntity: EOF without closing brace");
		auto key = vm.ED_FindGlobal (keyname);
		if (!key)
		{
			quake::con << '"' << keyname << "\" is not global" << std::endl;
			continue;
		}

		if (!ED_ParseEpair((void*)vm.pr_globals, key, value))
			Host_Error ("ED_ParseGlobals: parse error");
	}
}




namespace string_table {

	// string system
	//string_t pr_strings_last_offset; // last valid offset for new strings in lookup


	static constexpr size_t string_table_size = 100000; // 100k, progs.dat takes about 50 so gives ups plenty of space
	static char* hunk_strings_begin = nullptr;
	static char* hunk_strings_end = nullptr;
	static char* hunk_strings_current = nullptr;
	static constexpr uint16_t alloc_flag = 0x4000;
	static constexpr uint16_t lit_flag = 0x8000;
	static constexpr uint16_t size_mask = ~(alloc_flag | lit_flag);
	static constexpr uint16_t max_string_size = (0xFFFF & size_mask)-16;
	static const char* empty_string = "";
	static bool in_string_table(const char* ptr,size_t size) {
		return hunk_strings_begin >= ptr && hunk_strings_current < (ptr + size); // && *(ptr - 3) == 0;// 0 termated string before
	}
	template<typename T>
	static T* allign_up(T* ptr) {
		return  reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(ptr) + (sizeof(uintptr_t) - 1)) & -sizeof(uintptr_t));
	}
	static uint16_t& get_flag(char* ptr) { return *reinterpret_cast<uint16_t*>(ptr - sizeof(uint16_t)); }
	static uint16_t get_flag(const char* ptr) { return *reinterpret_cast<const uint16_t*>(ptr - sizeof(uint16_t)); }
	static uint16_t get_size(const char* ptr) { return get_flag(ptr) & size_mask; }
	static bool get_alloc(const char* ptr) { return get_flag(ptr) & alloc_flag; }
	static bool get_lit(const char* ptr) { return get_flag(ptr) & lit_flag; }
	static const char* get_str(const char* ptr) {
		return get_lit(ptr) ? *allign_up(reinterpret_cast<const char* const*>(ptr)) : ptr;// check if we are storing the pointer to it
	}
	static void set_lit(char* ptr, const char* lit,size_t len) {
		const char** p = allign_up(reinterpret_cast<const char**>(ptr));
		*p = lit;
		get_flag(ptr) = static_cast<uint16_t>(len) | lit_flag;
	}

	// just so we have empty string set up
	// sooo the structure of the table is that we have a flag(uint16_t) | char[0] | char[2] ...
	// however, if we store just the lit pointer, we have to make sure we are allinged so it will look like this
	// flag(uint16_t)  | fill(uint16_t) | pointer(size_t)
	// or not, I have all the aligment stuff set up in these functions above
	// you want to set as many litteral pointers as you can early on however
	using unique_istring_t = std::unique_ptr<char, void(*)(char*)>;
	static void dezmalloc(char* s) { Z_Free(reinterpret_cast<void*>(s)); }
	static void nodemalloc(char* s) { (void)s; }
	static void hulk_revert(char* s) {  hunk_strings_current = s - sizeof(uint16_t); }
	struct istring_equal {
		bool operator()(const char* l, const char* r) const {
			return l == r || (get_size(l) == get_size(r)  && util::util::str_cmp(get_str(l), get_str(l)+get_size(l) ,  get_str(r), get_str(r)+get_size(r))==0);
		}
	};
	struct istring_hash {
		size_t operator()(const char* l) const { return util::util::hasher(get_str(l), get_size(l)); }
	};
	using string_lookup_t = std::unordered_set<const char*, istring_hash, istring_equal>; // allocated on hulk
	static string_lookup_t stringtable_lookup;

	static char* push_literal(const char* ptr, size_t size) {
		if ((hunk_strings_current + 9) >= hunk_strings_end) Sys_Error("Out of string space!");
		hunk_strings_current += sizeof(uint16_t);
		char* ret = hunk_strings_current;
		set_lit(ret, ptr, size);
		return ret;
	}
	static char* push_string(const char* ptr, size_t size) {
		if(size >= max_string_size) Sys_Error("string to long, try using a literal?");
		if ((hunk_strings_current + size) >= hunk_strings_end) Sys_Error("Out of string space!");
		uint16_t& flag = *reinterpret_cast<uint16_t*>(hunk_strings_current);
		char* ns = hunk_strings_current + sizeof(uint16_t);
		flag = 0;
		//uint8_t& count = reinterpret_cast<uint8_t&>(*start++);
		while (size--) {
			int c = *ptr++;
			if (c == '\\') {
				switch (*(++ptr)) {
				case 'n': c = '\n'; break;
				default: c = '\\'; break;
				}
			} 
			ns[flag++] = c;
		}
		ns[flag++] = '\0';
		hunk_strings_current = allign_up(ns + flag); // Round up to pointer boundry
		return ns; // default is to revert
	}

	static const char* intern(const char* s, size_t len) {
		if (in_string_table(s, len)) {
			assert(*(s - 3) == 0);// 0 termated string before
			return s;
		}
		if (s == "" || len == 0) return "";
		char* ns = push_string(s, len); // create zero length string	
		auto it = stringtable_lookup.emplace(ns);
		if (it.second) 
			return ns;
		else {
			hulk_revert(ns);
			return *it.first;
		}

	}
	static const char* intern(const std::string_view&  s) {
		return intern(s.data(), s.size());
	}
	static const char* intern_literal(const char* str,bool force=false){
		size_t len = ::strlen(str);
		if (!force&& len < 5) return intern(str); // less than the size of the physical pointer?
		char*  ns = push_literal(str, len); // create zero length string	
		auto it = stringtable_lookup.emplace(ns);
		if (it.second) return ns;
		hulk_revert(ns);
		return *it.first;
	}


	void init() {
		hunk_strings_current=hunk_strings_begin = (char*)Hunk_AllocName(string_table_size, "string");
		hunk_strings_end = hunk_strings_begin + string_table_size;
		intern_literal("", true); // create zero length string 	

	}
}
const char* string_t::intern(const char* str,size_t size) { return string_table::intern(str, size); }
const char* string_t::intern(const char* str) { return string_table::intern(str, ::strlen(str));   }
const char* string_t::intern(const std::string_view& str) { return string_table::intern(str); }

/*
=============
ED_NewString
=============
*/

void pr_system_t::ED_ClearStrings() {
	// never need to clear strings
}
namespace priv {
	// helps alot
	/* calculate absolute value */
	constexpr int abs_val(int x) { return x < 0 ? -x : x; }
	/* calculate number of digits needed, including minus sign */
	constexpr int num_digits(int x) { return x < 0 ? 1 + num_digits(-x) : x < 10 ? 1 : 1 + num_digits(x / 10); }
	/* metaprogramming string type: each different string is a unique type */
	template<char... args>
	struct metastring {
		const char data[sizeof... (args)+1] = { '*', args... };
	};
	/* recursive number-printing template, general case (for three or more digits) */
	template<int size, int x, char... args>
	struct numeric_builder {
		typedef typename numeric_builder<size - 1, x / 10, '0' + abs_val(x) % 10, args...>::type type;
	};

	/* special case for two digits; minus sign is handled here */
	template<int x, char... args>
	struct numeric_builder<2, x, args...> {
		typedef metastring < x < 0 ? '-' : '0' + x / 10, '0' + abs_val(x) % 10, args...> type;
	};

	/* special case for one digit (positive numbers only) */
	template<int x, char... args>
	struct numeric_builder<1, x, args...> {
		typedef metastring<'0' + x, args...> type;
	};


	/* convenience wrapper for numeric_builder */
	template<int x>
	class numeric_string
	{
	public:
		/* generate a unique string type representing this number */
		typedef typename numeric_builder<num_digits(x), x, '\0'>::type type;

		/* declare a static string of that type (instantiated later at file scope) */
		static constexpr type value{};
		/* returns a pointer to the instantiated string */
		static constexpr const char* get() { return value.data; }
	};
# define REQUIRES(...)  typename std::enable_if<(__VA_ARGS__), bool>::type = true
	template <int N>
	class string_literal
	{
		const char(&_lit)[N + 1];
	public:
		constexpr string_literal(const char(&lit)[N + 1]) : _lit(lit) {}
		constexpr char operator[](int i) const { return assert(i >= 0 && i < N), _lit[i]; }
	};
	template <int N_PLUS_1>
	constexpr auto literal(const char(&lit)[N_PLUS_1]) -> string_literal<N_PLUS_1 - 1> { return string_literal<N_PLUS_1 - 1>(lit); }
	
	template<int x>
	constexpr typename numeric_string<x>::type numeric_string<x>::value;


	template<unsigned N1, unsigned... I1, unsigned N2, unsigned... I2>
	constexpr std::array<char const, N1 + N2 - 1> concat(char const (&a1)[N1], char const (&a2)[N2], std::index_sequence<I1...>, std::index_sequence<I2...>) {
		return { { a1[I1]..., a2[I2]... } };
	}

	template<unsigned N1, unsigned N2>
	constexpr std::array<char const, N1 + N2 - 1> concat(char const (&a1)[N1], char const (&a2)[N2]) {
		return concat(a1, a2, std::make_index_sequence<N1 - 1>{}, std::make_index_sequence<N2>{});
	}
#if 0
	template<unsigned N>
	constexpr std::array<char const, N + 1> add_star(char const (&a1)[N]) {
		return concat("*", a1);
	}
#endif
	template<size_t... Indices>
	constexpr auto make_number_array(std::index_sequence<Indices...>)
		->std::array<const char* const, sizeof...(Indices)>
	{
		return { { numeric_string<Indices>::get() ...  }  };
	}

	template<size_t N>
	constexpr auto make_number_array()->std::array<const char* const, N>
	{
		return make_number_array(std::make_index_sequence<N>{});
	}

	constexpr auto quck_number_literals = make_number_array<256>();
}
const char* pr_system_t::ED_QuickToString(uint8_t v) const { // quickly finds a number to string, super fast
	const char* number = priv::quck_number_literals[v];
	return number;
}
#if 0
const char* pr_system_t::ED_LocalModelName(uint8_t v) const { 
	const char* number = priv::quck_number_literals[v];
	return number;
}
#endif
string_t pr_system_t::ED_NewString (const std::string_view& string,bool zmalloc) {
	return string_table::intern(string);
}


/*
=============
ED_ParseEval

Can parse either fields or globals
returns false if error
=============
*/



// globals?
qboolean	ED_ParseEpair(void *base, const ddef_t *key, const std::string_view& value) {
	eval_t* d = reinterpret_cast<eval_t*>(reinterpret_cast<uint32_t*>(base) + key->ofs);
	switch (idType::ClearSaveGlobal(key->type))
	{
	case etype_t::ev_string:
		d->string = string_t::intern(value);
		break;
	case etype_t::ev_float:
		assert(quake::to_number(value, d->_float));
		break;
	case etype_t::ev_vector: {
		std::string_view fv;
		COM_Parser parser(value);
		assert(parser.Next(fv));
		assert(quake::to_number(fv, d->vector[0]));
		assert(parser.Next(fv));
		assert(quake::to_number(fv, d->vector[1]));
		assert(parser.Next(fv));
		assert(quake::to_number(fv, d->vector[2]));
	}
	break;

	case etype_t::ev_entity:
		assert(quake::to_number(value, d->_int));
		d->edict = vm.EDICT_NUM(d->_int);
		break;

	case etype_t::ev_field:
		if ((d->field = vm.ED_FindField(value)) == nullptr) {
			quake::con << "Can't find field " << value << std::endl;
			return false;
		}
		break;

	case etype_t::ev_function:
		if ((d->function = vm.ED_FindFunction(string_t::intern(value))) == nullptr)
		{
			quake::con << "Can't find function " << value << std::endl;
			return false;
		}
		break;

	default:
		assert(0);
		break;
	}
	return true;
}
qboolean ED_ParseEpair(edict_t* ent, const ddef_t *key, const std::string_view& value) {
	return ED_ParseEpair(reinterpret_cast<void*>(&ent->v), key, value);
}
/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
Used for initial level load and for savegames.
====================
*/
static std::unordered_map<string_t, edict_t*> loaded_edicts;
edict_t * ED_ParseEdict(COM_Parser& parser, edict_t *ent) {
	std::string_view keyname, value;
	// clear it
	//if (ent)  std::memset(&ent->v, 0, vm.progs->entityfields * 4);
	//assert(!vm.is_edict_free(ent)); 		// hack

	// go through all the dictionary pairs
	// debugging
	edict_t * init = nullptr;
	while (1)
	{
		if (!parser.Next(keyname))
			Sys_Error("ED_ParseEntity: EOF without closing brace");

		if (keyname[0] == '}') break;
		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		qboolean anglehack = false;

		if (keyname == "angle") {
			keyname =  "angles";
			anglehack = true; 
		}
		// FIXME: change light to _light to get rid of this hack
		else if (keyname == "light") {
			keyname = "light_lev"; // hack for single light def
		}
		// another hack to fix heynames with trailing spaces
		// this shouldn't happen with the parser
		assert(keyname.back() != ' ');

		// parse value	
		if (!parser.Next(value) || value[0] == '}')
			Sys_Error("ED_ParseEntity: EOF without closing brace");
		if (!init) {
			if (ent && !ent->free)
				init = ent;
			else init = vm.ED_Alloc(false, ent);
		}
		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname.front() == '_')
			continue;

		auto key = vm.ED_FindField(keyname);
		if (key == nullptr) {
			quake::con << '\'' << keyname << "' is not a field" << std::endl;
			continue;
		}

		if (anglehack)
		{
			quake::fixed_string_stream<128> buf;

			buf << "0 " << value << " 0";
			if (!ED_ParseEpair(init, key, buf.str()))
				Host_Error("ED_ParseEdict: parse error");
		}
		else {
			if (!ED_ParseEpair(init, key, value))
				Host_Error("ED_ParseEdict: parse error");
		}


	}


	assert(!init->v.classname.empty());
	if (init) {
		loaded_edicts[init->v.classname] = init;
		return init;
	}
	return nullptr;
}


/*
================
ED_LoadFromFile

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

Used for both fresh maps and savegame loads.  A fresh map would also need
to call ED_CallSpawnFunctions () to let the objects initialize themselves.
================
*/
void ED_LoadFromFile (const std::string_view& data)
{	
	edict_t		*ent = nullptr;
	int			inhibit = 0;

	vm.pr_global_struct->time = static_cast<float>(sv.time);
	COM_Parser parser(data);
	std::string_view token;
	size_t ent_index = 0;
// parse ents
	while (1) 
	{
// parse the opening brace	
		if (!parser.Next(token,false)) break;

		if (token[0] != '{') {
			Sys_Error("ED_LoadFromFile:  expecting {");
		}

		if (!ent) 
			ent = sv.worldedict;
		else
			ent = vm.ED_Alloc();
		ent=ED_ParseEdict (parser, ent);

// remove things from different skill levels or deathmatch
		if (deathmatch.value)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH))
			{
				vm.ED_Free(ent);
				inhibit++;
				continue;
			}
		}
		else if ((current_skill == 0 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_EASY))
				|| (current_skill == 1 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_MEDIUM))
				|| (current_skill >= 2 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_HARD)) )
		{
			vm.ED_Free(ent);
			inhibit++;
			continue;
		}

//
// immediately call spawn function
//
		if (ent->v.classname.empty())
		{
			Con_Printf ("No classname for:\n");
			ent->Print ();
			vm.ED_Free(ent);
			continue;
		}

	// look for the spawn function
		const dfunction_t *func = vm.ED_FindFunction (ent->v.classname);

		if (!func)
		{
			Con_Printf ("No spawn function for:\n");
			ent->Print ();
			vm.ED_Free(ent);
			continue;
		}

		vm.pr_global_struct->self = ent;
		func->call();
	}	

	Con_DPrintf ("%i entities inhibited\n", inhibit);
}


/*
===============
PR_LoadProgs
===============
*/
pr_system_t::pr_system_t() :
	progs(nullptr), pr_functions(nullptr),  pr_globaldefs(nullptr), pr_fielddefs(nullptr), 
	pr_statements(nullptr), pr_global_struct(nullptr), pr_globals(nullptr), pr_edict_size(0), pr_edicts(), pr_max_edicts(MAX_EDICTS)
	,vars(_global_info)
	{
#if 0
	dprograms_t		*progs=nullptr;
	dfunction_t		*pr_functions;
	char			*pr_strings;
	ddef_t			*pr_globaldefs;
	ddef_t			*pr_fielddefs;
	dstatement_t	*pr_statements;
	globalvars_t	*vm.pr_global_struct;
	float			*pr_globals;			// same as vm.pr_global_struct

	int				pr_edict_size;	// in bytes
	edict_t *		pr_edicts;		// makes sence to put the edicts here 
	size_t			pr_max_edicts;
#endif
}
void pr_system_t::LoadProgs(void) {
	Clear(); // make sure the vm system is clear
	CRC_Init(&pr_crc);
	size_t file_size;
	byte* raw = (byte*)COM_LoadHunkFile("progs.dat", &file_size);
	progs = reinterpret_cast<dprograms_t*>(raw);
	if (!progs)
		Sys_Error("PR_LoadProgs: couldn't load progs.dat");
	Con_DPrintf("Programs occupy %iK.\n", file_size / 1024);

	for (size_t i = 0; i < static_cast<size_t>(file_size); i++)
		CRC_ProcessByte(&pr_crc, raw[i]);

	// byte swap the header
	for (size_t i = 0; i < (sizeof(*progs) / 4); i++)
		((int *)progs)[i] = LittleLong(((int *)progs)[i]);

	if (progs->version != PROG_VERSION)
		Sys_Error("progs.dat has wrong version number (%i should be %i)", progs->version, PROG_VERSION);
	if (progs->crc != PROGHEADER_CRC)
		Sys_Error("progs.dat system vars have been modified, progdefs.h is out of date");
	pr_functions = reinterpret_cast<dfunction_t *>(raw + progs->ofs_functions);
	const char* pr_strings = reinterpret_cast<const char *>(raw + progs->ofs_strings);
	pr_globaldefs = reinterpret_cast<ddef_t *>(raw + progs->ofs_globaldefs);
	pr_fielddefs = reinterpret_cast<ddef_t *>(raw + progs->ofs_fielddefs);
	pr_statements = reinterpret_cast<dstatement_t *>(raw + progs->ofs_statements);
	pr_global_struct = reinterpret_cast<globalvars_t *>(raw + progs->ofs_globals);

	pr_globals = (float *)pr_global_struct;

	pr_edict_size = sizeof(edict_t) + static_cast<size_t>(progs->entityfields * 4)  - sizeof(entvars_t);

	quake::con << "EDICT SIZE DIF IS " << (int)((progs->entityfields * 4) - sizeof(entvars_t)) << std::endl;
	// round off to next highest whole word address (esp for Alpha)
	// this ensures that pointers in the engine data area are always
	// properly aligned
	pr_edict_size += sizeof(void *) - 1;
	pr_edict_size &= ~(sizeof(void *) - 1);

	// So we don't keep allocating all new strings, we will create a reverse lookup
	// ok, no way around this, we need our own dedicated string space
	{
		for (size_t i = 0; i < static_cast<size_t>(progs->numstrings); i++) {
			std::string_view sv(pr_strings + i);
			string_table::intern(sv);
			i += sv.size();
		}
	}

	// byte swap the lumps
	for (size_t i = 0; i < static_cast<size_t>(progs->numstatements); i++)
	{
		dstatement_t* s = const_cast<dstatement_t*>(pr_statements + i);
		s->op = LittleShort(s->op);
		s->a = LittleShort(s->a);
		s->b = LittleShort(s->b);
		s->c = LittleShort(s->c);

	}
	{
		_function_info.clear();
		for (size_t i = 0; i < static_cast<size_t>(progs->numfunctions); i++)
		{
			dfunction_t* f = const_cast<dfunction_t*>(pr_functions + i);
			f->first_statement = LittleLong(f->first_statement);
			f->parm_start = LittleLong(f->parm_start);
			f->_name = string_t(pr_strings + LittleLong(f->s_name));
			f->s_file = LittleLong(f->s_file);
			f->numparms = LittleLong(f->numparms);
			f->locals = LittleLong(f->locals);
			_function_info.emplace(f->name(), f);
		}
	}
	{
		//dprograms_t::globals.clear();
		_global_info.clear();
		for (size_t i = 0; i < static_cast<size_t>(progs->numglobaldefs); i++)
		{
			ddef_t* d = const_cast<ddef_t*>(pr_globaldefs + i);
			d->itype = LittleShort(d->itype);
			d->ofs = LittleShort(d->ofs);
			d->_name = string_t(pr_strings + LittleLong(d->s_name));
			_global_info.insert(d);
		}
	}
	{
		_field_info.clear();
		for (size_t i = 0; i < static_cast<size_t>(progs->numfielddefs); i++)
		{
			ddef_t* d = const_cast<ddef_t*>(pr_fielddefs + i);
			d->itype = LittleShort(d->itype);
			if (idType(pr_fielddefs[i].type).saveglobal())
				Sys_Error("PR_LoadProgs: pr_fielddefs[i].type & DEF_SAVEGLOBAL");
			d->ofs = LittleShort(d->ofs);
			d->_name = string_t(pr_strings + LittleLong(d->s_name));
			_field_info.insert(d);
		}
	}
	// this...seems bad.  I mean if its a float value that global is fucked
	for (size_t i=0 ; i<static_cast<size_t>(progs->numglobals) ; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
}


/*
===============
PR_Init
===============
*/
void StringInit() {
	string_table::init();
}

void pr_system_t::Init(void)
{
	Cmd_AddCommand ("edict", ED_PrintEdict_f);
	Cmd_AddCommand ("edicts", (xcommand_t)ED_PrintEdicts);
	Cmd_AddCommand ("edictcount", ED_Count);
	Cmd_AddCommand ("profile", PR_Profile_f);
	Cvar_RegisterVariable("nomonsters",nomonsters);
	Cvar_RegisterVariable("gamecfg",gamecfg);
	Cvar_RegisterVariable("scratch1",scratch1);
	Cvar_RegisterVariable("scratch2",scratch2);
	Cvar_RegisterVariable("scratch3",scratch3);
	Cvar_RegisterVariable("scratch4",scratch4);
	Cvar_RegisterVariable("savedgamecfg",savedgamecfg);
	Cvar_RegisterVariable("saved1",saved1);
	Cvar_RegisterVariable("saved2",saved2);
	Cvar_RegisterVariable("saved3",saved3);
	Cvar_RegisterVariable("saved4",saved4);
	vm.Clear();
	// create string table

}



