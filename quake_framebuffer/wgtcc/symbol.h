#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <string>
#include <type_traits>


class Symbol {
public:
	using string_type = std::string;
	using traits_type = typename string_type::traits_type;  
	using allocator_type = typename string_type::allocator_type;


	using value_type = typename string_type::value_type;
	using size_type = typename string_type::size_type;
	using difference_type = typename string_type::difference_type;
	using pointer = typename string_type::pointer;
	using const_pointer = typename string_type::const_pointer;
	using reference = typename string_type::reference;
	using const_reference = typename string_type::const_reference;

	using iterator = typename string_type::iterator;
	using const_iterator = typename string_type::const_iterator;

	using reverse_iterator = typename string_type::reverse_iterator;
	using const_reverse_iterator = typename string_type::const_reverse_iterator;

	static Symbol Lookup(const std::string& str);
	Symbol();
	Symbol(const Symbol& copy) = default;
	Symbol(Symbol&& move) = default;
	Symbol& operator=(const Symbol& copy) { *this = Symbol(copy._str); return *this; }
	Symbol& operator=(Symbol&& move) { *this = Symbol(move._str); return *this; }
	~Symbol() = default;
	Symbol(const string_type& s);
	Symbol(const char* s);
	
	const string_type& str() const { return _str; }

	operator const string_type&() const { return _str; }
	// string interface, only const functions
	const char* c_str() const { return _str.c_str(); }
	size_t size() const { return _str.size(); }
	size_t length() const { return _str.length(); }
	auto begin() const { return _str.begin(); }
	auto end() const { return _str.end(); }
	auto rbegin() const { return _str.rbegin(); }
	auto rend() const { return _str.rend(); }
	bool empty() const { return _str.empty(); }
	auto front() const { return _str.front(); }
	auto back() const { return _str.back(); }
	// just cheap forwards to std::string so I don't have to rebuild std::string from scrach
	// I REALLY wish it was an interface:(
	template<typename ... Args> auto compare(Args&& ... args) const { return _str.compare(std::forward<Args>(args)...); }
	template<typename ... Args> auto find(Args&& ... args) const { return _str.find(std::forward<Args>(args)...); }
	template<typename ... Args> auto rfind(Args&& ... args) const { return _str.rfind(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_first_of(Args&& ... args) const { return _str.find_first_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_first_not_of(Args&& ... args) const { return _str.find_first_not_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_last_of(Args&& ... args) const { return _str.find_last_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_last_not_of(Args&& ... args) const { return _str.find_last_not_of(std::forward<Args>(args)...); }

	bool operator==(const Symbol& r) const { return &_str == &r._str; }
	bool operator!=(const Symbol& r) const { return &_str != &r._str; }
	bool operator==(const std::string& r) const { return _str == r; }
	bool operator!=(const std::string& r) const { return _str != r; }
	bool operator<(const Symbol& r) const { return _str < r._str; } // for sets
private:
	
	const string_type& _str;
};

static inline std::ostream& operator <<(std::ostream& os, const Symbol& s) {
	os << s.str();
	return os;
}
namespace std {
	template<>
	struct hash<Symbol> {
		std::hash<std::string> _hasher;
		size_t operator()(const Symbol& s) const { return _hasher(s); }
	};
};







#endif
