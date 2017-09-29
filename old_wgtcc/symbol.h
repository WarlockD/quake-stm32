#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <string>
#include <type_traits>


class Symbol {
public:
	using string_type = std::string_view;
	using traits_type = typename string_type::traits_type;  

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

	Symbol() = default;
	Symbol(const Symbol& copy) = default;
	Symbol(Symbol&& move) = default;
	Symbol& operator=(const Symbol& copy) = default;
	Symbol& operator=(Symbol&& move) = default;
	~Symbol() = default;
	Symbol(const std::string& s);
	Symbol(const char* s);
	Symbol(const string_type& s);
	Symbol(const char* s, size_t size);
	

	operator const string_type&() const { return _str; }
	// string interface, only const functions
	const char* c_str() const { return _str.data(); }
	size_t size() const { return _str.size(); }
	size_t length() const { return _str.length(); }
	auto begin() const { return _str.begin(); }
	auto end() const { return _str.end(); }
	auto rbegin() const { return _str.rbegin(); }
	auto rend() const { return _str.rend(); }
	bool empty() const { return _str.empty(); }
	auto front() const { return _str.front(); }
	auto back() const { return _str.back(); }

	auto at(size_type i) const { return _str.at(i); }
	auto operator[](size_type i) const { return _str.operator[](i); }
	// just cheap forwards to std::string so I don't have to rebuild std::string from scrach
	// I REALLY wish it was an interface:(
	template<typename ... Args> auto compare(Args&& ... args) const { return _str.compare(std::forward<Args>(args)...); }
	template<typename ... Args> auto find(Args&& ... args) const { return _str.find(std::forward<Args>(args)...); }
	template<typename ... Args> auto rfind(Args&& ... args) const { return _str.rfind(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_first_of(Args&& ... args) const { return _str.find_first_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_first_not_of(Args&& ... args) const { return _str.find_first_not_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_last_of(Args&& ... args) const { return _str.find_last_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto find_last_not_of(Args&& ... args) const { return _str.find_last_not_of(std::forward<Args>(args)...); }
	template<typename ... Args> auto substr(Args&& ... args) const { return _str.substr(std::forward<Args>(args)...); }

	bool operator==(const Symbol& r) const { return _str.data() == r._str.data(); }
	bool operator!=(const Symbol& r) const { return _str.data() != r._str.data(); }
	bool operator==(const std::string& r) const { return _str == r; }
	bool operator!=(const std::string& r) const { return _str != r; }
	bool operator<(const Symbol& r) const { return _str < r._str; } // for sets
private:
	string_type _str;
};

static inline std::ostream& operator <<(std::ostream& os, const Symbol& s) {
	os << static_cast<Symbol::string_type>(s);
	return os;
}
namespace std {
	template<>
	struct hash<Symbol> {
		std::hash<Symbol::string_type> _hasher;
		size_t operator()(const Symbol& s) const { return _hasher(s); }
	};
};







#endif
