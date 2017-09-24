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
	std::unordered_map<Symbol::string_type, std::unique_ptr<StringContainer>> table;
	std::mutex mutex;
	StringTable() {}
public:
	Symbol::string_type Lookup(const Symbol::string_type& str) {
		auto it = table.find(str);
		if (it != table.end()) return *it->second.get();
		else {
			std::lock_guard<std::mutex> lock(mutex);
			auto nstr = StringContainer::Create(str.data(), str.size());
			table.emplace(nstr, nstr);
			return *nstr;
		}
	}
};
StringTable string_table;

Symbol::Symbol() : _str("") {}

Symbol::Symbol(const string_type& s) : _str(string_table.Lookup(s)) {}
Symbol::Symbol(const char* s) : _str(string_table.Lookup(s)) {}
Symbol::Symbol(const char* s, size_t size) : _str(string_table.Lookup(std::string_view(s, size))) {}
Symbol::Symbol(const std::string& s) : _str(string_table.Lookup(s)) {}




