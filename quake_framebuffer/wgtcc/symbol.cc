#include "symbol.h"

#include <unordered_map>
#include <memory>
#include <mutex>

template<typename T>
struct ref_hasher {
	std::hash<std::remove_cv_t<T>> _hasher;
	size_t operator()(const std::reference_wrapper<std::remove_cv_t<T>>& v) const { return _hasher(v); }
};
template<typename T>
struct ref_equalto {
	std::equal_to<std::remove_cv_t<T>> _equal_to;
	bool operator()(const std::reference_wrapper<std::remove_cv_t<T>>& l, const std::reference_wrapper<std::remove_cv_t<T>>& r) const { return _equal_to(l, r); }
};
class StringTable {
	class StringContainer {
		StringContainer() = default;
		StringContainer(const StringContainer& copy) = delete;
		StringContainer(StringContainer&& move) = delete;
		size_t size;
		char str[1];
	public:
		operator Symbol::string_type() const { return Symbol::string_type(str, size); }

		static std::unique_ptr<StringContainer> Create(const char* str, size_t size) {
			char* raw = new char[sizeof(StringContainer) + size];
			StringContainer* s = new(raw) StringContainer;
			s->size = size;
			::memcpy(s->str, str, size);
			s->str[s->size] = 0;
			return std::unique_ptr<StringContainer>(s);
		}
	};
	std::unordered_map<std::string_view, std::unique_ptr<std::string>> table;
	std::mutex mutex;
	const std::string*  _empty_string;
public:	
	const std::string*  empty_string() const { return _empty_string; }
	StringTable() :_empty_string(Lookup("")) {}

	const std::string* Lookup(std::string_view  key) {
		auto it = table.find(key);
		if (it != table.end()) return it->second.get();
		else {
			std::lock_guard<std::mutex> lock(mutex);
			std::string* nstr = new std::string(key);
			key = *nstr;
			table.emplace(key, nstr);
			return nstr;
		}
	}
};
StringTable string_table;

Symbol::Symbol() : _str(string_table.empty_string()) {}

Symbol::Symbol(const std::string_view& s) : _str(string_table.Lookup(s)) {}
Symbol::Symbol(const char* s) : _str(string_table.Lookup(s)) {}
Symbol::Symbol(const char* s, size_t size) : _str(string_table.Lookup(std::string_view(s, size))) {}
Symbol::Symbol(const std::string& s) : _str(string_table.Lookup(s)) {}




