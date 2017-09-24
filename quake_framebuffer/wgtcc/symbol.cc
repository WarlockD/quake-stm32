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

template<typename KEY>
class intern_table {
	using HASH = ref_hasher<KEY>;
	using EQUALTO = ref_equalto<KEY>;
	using unique_ptr = std::unique_ptr<KEY>;
	using key_type = std::reference_wrapper<KEY>;
	std::unordered_map<key_type, unique_ptr, HASH, EQUALTO> table;
	const KEY& _empty;
	std::mutex mutex;
public:
	friend  KEY;

	intern_table() : _empty(Lookup(KEY())) {    }
	const KEY& EmptyKey() { return _empty; }
	KEY& Lookup(const KEY& key) {

		auto it = table.find(std::ref(const_cast<KEY&>(key)));
		if (it != table.end()) return *it->second.get();
		else {
			std::lock_guard<std::mutex> lock(mutex);
			KEY* nkey = new KEY(key);
			table.emplace(std::ref(*nkey), unique_ptr(nkey));
			return *nkey;
		}

	}
	KEY& Lookup(KEY&& key) {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = table.find(std::ref(key));
		if (it != table.end()) return *it->second.get();
		KEY* nkey = new KEY(std::move(key));
		table.emplace(std::ref(*nkey), unique_ptr(nkey));
		return *nkey;
	}
};

static intern_table<std::string> symbol_table;
Symbol::Symbol() : _str(symbol_table.EmptyKey()) {}

Symbol(const string_type& s) : _str(s) {}
Symbol(const char* s) : _str(s) {}
Symbol Symbol::Lookup(const std::string& str) { return Symbol(symbol_table.Lookup(str)); }


