#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
template<typename KEY> class intern_table;
class Symbol {
	friend intern_table<std::string>;
public:
	struct Hasher {
		std::hash<std::string> _hasher;
		size_t operator()(const Symbol& s) const { return _hasher(s); }
	};
	static const std::string& EmptyString();
	static const std::string& InternString(const std::string& str);
	Symbol() : _str(EmptyString()) {}
	Symbol(const std::string& s) : _str(InternString(s)) {}
	operator const std::string&() const { return _str; }
	const char* c_str() const { return _str.c_str(); }
	size_t size() const { return _str.size(); }
	auto begin() const { return _str.begin(); }
	auto end() const { return _str.end(); }
	bool empty() const { return _str.empty(); }
	bool operator==(const Symbol& r) const { return &_str == &r._str; }
	bool operator!=(const Symbol& r) const { return &_str != &r._str; }
	bool operator==(const std::string& r) const { return _str == r; }
	bool operator!=(const std::string& r) const { return _str != r; }
	bool operator<(const Symbol& r) const { return _str < r._str; }
private:	
	const std::string& _str;
};
static inline std::ostream& operator <<(std::ostream& os, const Symbol& s) {
	os << static_cast<std::string>(s);
	return os;
}
namespace std {
	template<>
	struct hash<Symbol> : public Symbol::Hasher {};
};

class FilePosition {
public:
	FilePosition() :_filename(), _lineno(0U), _col(0U) {}
	FilePosition(Symbol filename, size_t lineno, size_t col) :_filename(filename), _lineno(lineno), _col(col) {}
	Symbol filename() const { return _filename; }
	size_t lineno() const { return _lineno; }
	size_t col() const { return _col; }
	bool empty() const { return _filename.empty(); }
	bool operator==(const FilePosition& r) const { return _filename == r._filename && _lineno == r._lineno && _col == r._col; }
	bool operator!=(const FilePosition& r) const { return !(*this == r); }
	bool operator<(const FilePosition& r) const { return _filename < r._filename && _lineno < r._lineno && _col < r._col;}
private:
	Symbol _filename;
	size_t _lineno;
	size_t _col;
};
static inline std::ostream& operator <<(std::ostream& os, const FilePosition& s) {
	os << "{ filename : " << s.filename() << ", lineno : " << s.lineno() << ", col : " << s.col() << " }";
	return os;
}


class FunctionDefine {
	friend intern_table<FunctionDefine>;
	FunctionDefine(Symbol name): _name(name) , _returnType("int") {}
	FunctionDefine() {}
	FunctionDefine(const FunctionDefine& copy) = default;
public:
	FunctionDefine(FunctionDefine&& move) = default;
	static FunctionDefine& Lookup(Symbol name);
	using argumet_list_t = std::vector<std::pair<Symbol, Symbol>>;
	argumet_list_t& arguments() { return _arguments; }
	const argumet_list_t& arguments() const { return _arguments; }
	std::vector<FilePosition>& definedAt() { return _definedAt; }
	const std::vector<FilePosition>& definedAt() const { return _definedAt; }
	FilePosition& codeAt() { return _codeAt; }
	const FilePosition& codeAt() const { return _codeAt; }
	Symbol name() const { return _name; }
	Symbol returnType() const { return _returnType; }
	bool operator==(const FunctionDefine& r) const { return name() == r.name(); }
	bool operator!=(const FunctionDefine& r) const { return name() != r.name(); }
private:
	Symbol _name;
	argumet_list_t _arguments;
	std::vector<FilePosition> _definedAt;
	FilePosition _codeAt;
	Symbol _returnType;

};

static inline std::ostream& operator <<(std::ostream& os, const FunctionDefine& s) {
	os << static_cast<std::string>(s.name());
	return os;
}
namespace std {
	template<>
	struct hash<FunctionDefine> {
		std::hash<Symbol> _hasher;
		size_t operator()(const FunctionDefine& v) const { 
			return _hasher(v.name());
		}
	};
};

#endif