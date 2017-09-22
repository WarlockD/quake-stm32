#include "common.h"

struct symbol_hash {
	std::hash<std::string> _hasher;
	size_t operator()(const std::reference_wrapper<const std::string>& s) const { return _hasher(s); }
	size_t operator()(const std::reference_wrapper<std::string>& s) const { return _hasher(s); }
};
struct symbol_equal_to {
	std::equal_to<std::string> _equal_to;
	size_t operator()(const std::reference_wrapper<const std::string>& l, const std::reference_wrapper<std::string>& r) const { return _equal_to(l, r); }
	bool operator()(const std::reference_wrapper<std::string>& l, const std::reference_wrapper<std::string>& r) const { return _equal_to(l, r); }
};
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
		std::lock_guard<std::mutex> lock(mutex);
		auto it = table.find(std::ref(const_cast<KEY&>(key)));
		if (it != table.end()) return *it->second.get();
		KEY* nkey = new KEY(key);
		table.emplace(std::ref(*nkey), unique_ptr(nkey));
		return *nkey;
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
static intern_table<FunctionDefine> function_table;

const std::string& Symbol::EmptyString() { return symbol_table.EmptyKey(); }
const std::string& Symbol::InternString(const std::string& str) { return symbol_table.Lookup(str); }


FunctionDefine& FunctionDefine::Lookup(Symbol name) {
	FunctionDefine func(name);
	return function_table.Lookup(func);
}
