#ifndef _LIST_HPP_
#define _LIST_HPP_

#include <cstdint>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <cassert>



#define LIST_ENTRY_DETECT_IN_LIST

namespace list {
	template<typename T> struct entry;
	template<typename T> using field = entry<T> T::*;
	template<typename T, field<T> FIELD> class head_impl;
	template<typename T, field<T> FIELD, bool _is_const> struct iterator;
	template<typename T, field<T> FIELD> struct container;
	constexpr static uintptr_t UNLINK_MAGIC_VALUE = 0x87654321;
	//template<typename T, field<T> FIELD> struct head;
	//template<typename T, field<T> FIELD> struct head;
	template<typename T>
	struct entry_traits {
		using difference_type = ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using const_value_type = const typename std::remove_const<T>::type;
		using pointer = T*;
		using reference = T&;
		using const_pointer = const_value_type*;
		using const_reference = const_value_type&;
	};

	template<typename T, field<T> FIELD = nullptr>
	struct head_traits {
		using btraits = entry_traits<T>;
		using difference_type = typename btraits::difference_type;
		using value_type = typename btraits::value_type;
		using const_value_type = typename btraits::const_value_type;
		using pointer = typename btraits::pointer;
		using reference = typename btraits::reference;
		using const_pointer = typename btraits::const_pointer;
		using const_reference = typename btraits::const_reference;
		using iterator_category = std::bidirectional_iterator_tag;
		using entry_type = entry<T>;
		using field_type = field<T>;
		using head_type = head_impl<T, FIELD>;
		using container_type = container<T, FIELD>;
		friend head_type;

		constexpr static field_type member = FIELD;
		 template<typename T, typename U> static constexpr size_t offsetOf(U T::*member)
		{
			return (char*)&((T*)nullptr->*member) - (char*)nullptr;
		}
		//constexpr static size_t member_offset = member == nullptr ? 0 : (size_t) &(reinterpret_cast<const pointer>(0)->*member);
	//	constexpr static size_t member_offset = offsetOf(member);
		constexpr static inline  pointer& next(pointer p) {
			static_assert(member != nullptr, "Don't know what field you want!");
			return (p->*member).le_next;
		}
		constexpr static inline  pointer*& prev(pointer p) {
			static_assert(member != nullptr, "Don't know what field you want!");
			return (p->*member).le_prev;
		}
		// from offset
		constexpr static inline  pointer from_entry(entry_type* p) {
			return reinterpret_cast<pointer>(reinterpret_cast<char*>(p) - offsetOf(member));
		}
		constexpr static inline  const pointer from_entry(const  entry_type* p) {
			return reinterpret_cast<const  pointer>(reinterpret_cast<const  char*>(p) - offsetOf(member));
		}
		constexpr static inline  entry_type* to_entry(pointer p) {
			return reinterpret_cast<entry_type*>(reinterpret_cast<char*>(p) + offsetOf(member));
		}
		constexpr static inline  const entry_type* to_entry(const_pointer p) {
			return reinterpret_cast<const  entry_type*>(reinterpret_cast<const char*>(p) + offsetOf(member));
		}
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (int)&(((t *)0)->m)))
		template<typename M>
		constexpr static inline pointer container_of_entry(M* ptr) { return (pointer)((char*)ptr - offsetOf(member)); }
		template<typename M>
		constexpr static inline const_pointer container_of_entry(const M* ptr) { return (pointer)((char*)ptr - offsetOf(member)); }
#ifdef QUEUE_MACRO_DEBUG_TRASH
		static constexpr inline void trashit(pointer& elm) {
			elm = reinterpret_cast<pointer>((void*)-1);
		}
		static constexpr inline bool is_trashed(const pointer elm) {
			return elm == reinterpret_cast<pointer>((void*)-1);
		}
#else
		static constexpr inline void trashit(pointer& elm) {}
		static constexpr inline bool is_trashed(const pointer elm) { return true; }
#endif

#ifdef LIST_DEBUG
		/*
		* QMD_LIST_CHECK_HEAD(TAILQ_HEAD *head, TAILQ_ENTRY NAME)
		*
		* If the tailq is non-empty, validates that the first element of the tailq
		* points back at 'head.'
		*/
		static constexpr inline void check_head(const head_type& head) {
			if (!empty(head) && prev(head.lh_first) != &head.lh_first)
				LIST_DEBUG("Bad tailq head %p first->prev != head", &(head));
		}
		/*
		* QMD_LIST_CHECK_NEXT(TYPE *elm, TAILQ_ENTRY NAME)
		*
		* If an element follows 'elm' in the tailq, validates that the next element
		* points back at 'elm.'
		*/
		static constexpr inline void check_next(const pointer elm) {
			if (next((elm)) != nullptr && prev(next(elm)) != &next(elm))
				LIST_DEBUG("Bad link elm %p next->prev != elm", &(elm));
		}

		/*
		* QMD_LIST_CHECK_PREV(TYPE *elm, TAILQ_ENTRY NAME)
		*
		* Validates that the previous element (or head of the tailq) points to 'elm.'
		*/
		static constexpr inline void check_prev(const pointer elm) {
			if (*prev(elm) != (elm))
				LIST_DEBUG("Bad link elm %p prev->next != elm", &(elm));
		}
#else
		static constexpr inline void check_head(const head_type& head) {}
		static constexpr inline void check_tail(const head_type& head) {}
		static constexpr inline void check_next(const pointer elm) {}
		static constexpr inline void check_prev(const pointer elm) {}
#endif

		static inline bool empty(const head_type& head) { return head.lh_first == nullptr; }
		static inline T* first(const head_type& head) { return head.lh_first; }

		static inline T* last(const head_type& head) {
			auto current = head.lh_first;
			auto prev = current;
			while (current != nullptr) { prev = current; current = next(current); }
			return prev;
		}
		static inline void unlink(pointer elm) { 
			entry_type* e =  reinterpret_cast< entry_type*>(reinterpret_cast< char*>(elm) + offsetOf(member));
			e->le_next = nullptr;
			e->le_prev = &e->le_next;
		} //next(elm) = UNLINK_MAGIC;
	
		static inline bool islinked(const pointer elm) { return next(elm) != nullptr || prev(elm) != &next(elm); } //next(elm) != UNLINK_MAGIC;

		static inline void init(head_type& head) { head.lh_first = nullptr; }
		// entry is just here, technicaly we don't check for these values
#if 0
		static inline void init(entry_type& elm) {
			elm.le_next = nullptr;
			elm.le_prev = &elm.le_next;
		}
		static inline void init(pointer elm) {
			elm->le_next = nullptr;
			elm->le_prev = &elm->le_next;
		}
#endif
		static inline void concat(head_type& head1, head_type& head2) {
			pointer curelm = head1.lh_first;
			if (curelm == nullptr) {
				if ((head1.lh_first = head2.lh_first) != nullptr)
					prev(head2.lh_first) = &head1.lh_first;
			}
			else if (head2.lh_first == nullptr) {
				while (next(curelm) != nullptr)
					curelm = next(curelm);
				next(curelm) = head2.lh_first;
				prev(head2.lh_first) = &next(curelm);
			}
			init(head2);
		}
		static inline void insert_after(pointer listelm, pointer elm) {
			if (islinked(elm)) remove(elm);
			check_next(listelm);
			if ((next(elm) = next(listelm)) != nullptr)
				prev(next(elm)) = &next(elm);
			next(listelm) = elm;
			prev(elm) = &next(listelm);
		}
		static inline void insert_before(pointer listelm, pointer elm) {
			if (islinked(elm)) remove(elm);
			check_prev(listelm);
			prev(elm) = prev(listelm);
			next(elm) = listelm;
			*prev(listelm) = elm;
			prev(listelm) = next(elm);
		}
		static inline void insert_head(head_type& head, pointer elm) {
			check_head(head);
			if (islinked(elm)) remove(elm);
			if ((next(elm) = head.lh_first) != nullptr)
				prev(head.lh_first) = &next(elm);
			head.lh_first = elm;
		}
		static inline void remove(pointer elm) {
			check_next(elm);
			check_prev(elm);
			if (next(elm) != nullptr)
				prev(next(elm)) = prev(elm);
			*prev(elm) = next(elm);
			unlink(elm);
		}
		static inline void swap(head_type& head1, head_type& head2) {
			auto swap_tmp = head1.lh_first;
			head1.lh_first = head2.lh_first;
			head2.lh_first = swap_tmp;
			if ((swap_tmp = head1.lh_first) != nullptr)
				prev(swap_tmp) = &head1.lh_first;
			if ((swap_tmp = head2.lh_first) != nullptr)
				prev(swap_tmp) = &head2.lh_first;
		}
	};

	template<typename T>
	struct entry {
		friend T;
		using field_type = entry<T> T::*;
		using traits = entry_traits<T>;
		using type = entry<T>;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		using entry_type = type;

		template<field<T> FIELD>
		inline void remove() {
			using traits = head_traits<T, FIELD>;
			traits::remove(traits::from_entry(this));
		}

		template<field<T> FIELD>
		inline void insert_after( pointer elm) {
			using traits = head_traits<T, FIELD>;
			traits::insert_after(traits::from_entry(this),elm);
		}
		template<field<T> FIELD>
		inline void insert_before( pointer elm) {
			using traits = head_traits<T, FIELD>;
			traits::insert_before(traits::from_entry(this),elm);
		}
		template<field<T> FIELD>
		inline pointer next() const {
			using traits = head_traits<T, FIELD>;
			return traits::next(traits::from_entry(this));
		}
		template<field<T> FIELD>
		inline pointer prev() const {
			using traits = head_traits<T, FIELD>;
			return traits::prev(traits::from_entry(this));
		}
#ifdef LIST_ENTRY_SAVES_CONTAINER
		constexpr entry() : le_list(nullptr), le_next(nullptr), le_prev(&le_next) {}
		void* le_list;
#else
		//constexpr entry() : le_next(traits::UNLINK_MAGIC), le_prev(&le_next) {}
		constexpr entry() :  le_next(nullptr), le_prev(&le_next) {}
#endif
		pointer le_next;	/* next element */
		pointer *le_prev;	/* address of previous next element */
		bool isunlinked() const { return le_prev == &le_next && le_next == nullptr; }
		bool islinked() const { return !isunlinked(); }
	};
	//template<typename T>
	template<typename T, field<T> FIELD>
	struct head_impl {
		using traits = head_traits<T, FIELD>;
		using type = head_impl<T, FIELD>;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		using entry_type = typename traits::entry_type;
		using head_type = typename traits::head_type;
		using field_type = typename traits::field_type;
		template<bool _is_const>
		struct iterator_impl {
			using traits = head_traits<typename std::remove_const<T>::type, FIELD>;
			using difference_type = typename traits::difference_type;
			using value_type = typename std::conditional<_is_const, const  T, T>::type;
			using pointer = value_type*;
			using reference = value_type&;
			using iterator_category = std::bidirectional_iterator_tag;
			using iterator = iterator_impl<_is_const>;
			friend head_impl;
			constexpr iterator_impl(pointer current = nullptr) noexcept : _current(current) {}
			inline iterator operator++() noexcept {
				if (_current) _current = traits::next(_current);
				return iterator(*this);
			}
			inline iterator operator++(int) noexcept {
				iterator tmp(*this);
				if (_current) _current = traits::next(_current);
				return tmp;
			}
			inline iterator operator--() noexcept {
				if (_current) _current = traits::prev(_current);
				return iterator(*this);
			}
			inline iterator operator--(int) noexcept {
				iterator tmp(*this);
				if (_current) _current = traits::prev(_current);
				return tmp;
			}
			// Must downcast from _List_node_base to _List_node to get to _M_data.
			value_type& operator*()  const noexcept { return *_current; }
			//const_value_type& operator*()  noexcept const { return *_current; }
			value_type& operator&()  const noexcept { return *_current; }
			pointer operator->()  const noexcept { return _current; }

			// do I need to test if the field pointers are equal?
			bool operator==(const iterator_impl& r) const { return _current == r._current; }
			bool operator!=(const iterator_impl& r) const { return _current != r._current; }
		protected:
			pointer _current;
		};
		using iterator = iterator_impl<false>;
		using const_iterator = iterator_impl<true>;
		friend iterator;
		friend traits;

		static inline pointer next_entry(pointer p) { return traits::next(p); }
		static inline pointer prev_entry(pointer p) { return traits::prev(p); }
		inline void swap(head_type& head2) { traits::swap(*this, head2); }
		constexpr head_impl() noexcept : lh_first(nullptr) { 	}
		constexpr head_impl(const head_impl& copy) = delete;
		constexpr head_impl& operator=(const head_impl& copy) = delete; // obviously  cannot be copyied
		constexpr head_impl(head_impl&& move) noexcept { std::swap(*this, move); }
		constexpr head_impl& operator=(head_impl&& move) noexcept {
			std::swap(*this, move);
			return *this;
		}

		iterator begin() { return iterator(lh_first); }
		iterator end() { return iterator(nullptr); }
		const_iterator begin() const { return const_iterator(lh_first); }
		const_iterator end() const { return const_iterator(nullptr); }
		bool empty() const { return traits::empty(*this); }
	protected:
		inline void _insert_after(pointer listelm, pointer elm) { traits::insert_after(listelm, elm); }
		inline void _insert_before(pointer listelm, pointer elm) { traits::insert_before(listelm, elm); }
		inline void _insert_head(pointer elm) { traits::insert_head(*this, elm); }
		inline void _insert_tail(pointer elm) { traits::insert_tail(*this, elm); }
		inline void _remove(pointer elm) { traits::remove(elm); }

		constexpr inline pointer _first() { return  lh_first; }
		constexpr inline const pointer _first() const { return  lh_first; }

		pointer lh_first;	/* first element */
	};

	//template<typename T>
	template<typename T, field<T> FIELD>
	class head : public head_impl<T, FIELD> {
	public:

		using traits = head_impl<T, FIELD>;
		//using begin = traits::begin;
		//using end = traits::end;
		using type = head<T, FIELD>;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		using entry_type = typename traits::entry_type;
		using head_type = head<T, FIELD>;
		using field_type = typename traits::field_type;
		using iterator = typename traits::iterator;
		using const_iterator = typename traits::const_iterator;

		constexpr head() : traits::head_impl() {}

		inline pointer first_entry() { return traits::_first(); }
		inline pointer last_entry() { return traits::_last(); }
		inline const pointer first_entry()  const { return traits::_first(); }
		inline const pointer last_entry()  const { return traits::_last(); }

		reference front() { return *first_entry(); }
		reference back() { return *last_entry(); }
		const_reference front() const { return *first_entry(); }
		const_reference back() const { return *last_entry(); }
		inline void swap(type& head) { traits::swap(*this, head); }

		inline void clear() {
			// safety clear, unlink eveything
			while (lh_first) { // we just reuse this pointer for clearing them all
				auto elm = lh_first;
				lh_first = traits::next(lh_first);
				traits::unlink(elm);
			}
		}
		inline void insert_after(pointer listelm, pointer elm) { traits::_insert_after(listelm, elm); }
		inline void insert_before(pointer listelm, pointer elm) { traits::_insert_before(listelm, elm); }
		inline void push_front(pointer elm) { traits::_insert_head(elm); }
		void push(pointer elm) { traits::_insert_tail(elm); }

		// some list interface stuff, nothing to serious
		iterator insert_after(iterator position, pointer elm) {
			insert_after(&(*position), elm);
			return iterator(elm);
		}
		iterator insert_before(iterator position, pointer elm) {
			insert_before(&(*position), elm);
			return iterator(elm);
		}
		iterator insert(iterator position, pointer elm) { return insert_after(position, elm); }

		inline pointer pop_front() {
			pointer head = first_entry();
			remove(head);
			return head;
		}
		inline iterator erase(iterator it) {
			pointer curelm = &(*it);
			if (curelm != nullptr) {
				++it;
				remove(curelm);
			}
			return it;
		}
		inline void remove(pointer elm) { traits::_remove(elm); }

	};
	// same as a normal head except its priority sorted
	template<typename T, field<T> FIELD, class COMPARE = std::less<T>>
	class prio_head : public head_impl<T, FIELD> {
	public:
		using traits = head_impl<T, FIELD>;
		using type = head<T, FIELD>;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		using entry_type = typename traits::entry_type;
		using head_type = head<T, FIELD>;
		using field_type = typename traits::field_type;
		using iterator = typename traits::iterator;
		using const_iterator = typename traits::const_iterator;

		constexpr prio_head() : traits::head_impl() {}

		inline pointer first_entry() { return traits::_first(); }
		inline pointer last_entry() { return traits::_last(); }
		inline const pointer first_entry()  const { return traits::_first(); }
		inline const pointer last_entry()  const { return traits::_last(); }

		reference front() { return *first_entry(); }
		const_reference front() const { return *first_entry(); }

		iterator insert(iterator position, pointer elm) { return insert_after(position); }
		inline void swap(type& head) { traits::swap(*this, head); }
		bool empty() const { return traits::empty(*this); }

		void push(pointer elm) {
			assert(elm); // checking
			pointer curelm = traits::_first();
			if (curelm == nullptr)
				traits::_insert_head(elm);
			else {
				COMPARE comp;
				while (next(curelm) != nullptr) {
					if (comp(*curelm, *elm)) {
						traits::_insert_before(curelm, elm);
						return;
					}
					if (curelm == elm) return;
					curelm = next(curelm);
				}
				traits::_insert_after(curelm, elm);
			}
		}
		pointer pop() {
			pointer f = traits::_first();
			if (f) traits::_remove(f);
			return f;
		}
	};
	enum class status {
		ok = 0,		// insert or remove went fine
		exists,		// already exists on insert
		dup,		// already inside by another pointer
	};
	template<typename T>
	struct int_hasher {
		constexpr size_t operator()(uintptr_t x) const {
			x = ((x >> 16) ^ x) * 0x45d9f3b;
			x = ((x >> 16) ^ x) * 0x45d9f3b;
			x = (x >> 16) ^ x;
			return x;
		}
		constexpr size_t operator()(const void* x) const {
			return operator()(reinterpret_cast<uintptr_t>(x));
		}
		constexpr size_t operator()(const T x) const {
			return operator()(static_cast<uintptr_t>(x));
		}
		constexpr size_t operator()(const T* x) const {
			return operator()(reinterpret_cast<uintptr_t>(x));
		}
	};
	template<typename T>
	struct pointer_hasher {
		constexpr size_t operator()(const T& x) const { return int_hasher<T>()(*x); }
		constexpr size_t operator()(const T* x) const { return int_hasher<T>()(x); }
		constexpr size_t operator()(uintptr_t x) const { return int_hasher<T>()(x); }
	};

	template<typename T>
	struct pointer_equals {
		constexpr bool operator()(const T& a, const T& b) const { return &a == &b; }
		constexpr bool operator()(const T& a, const T* b) const { return &a == b; }
		constexpr bool operator()(const T& a, uintptr_t b) const { return reinterpret_cast<uintptr_t>(&a) == b; }
	};
	template<typename T, list::field<T> FIELD, size_t _BUCKET_COUNT, typename HASHER = pointer_hasher<T>, typename EQUALS = pointer_equals<T>>
	class hash {
	public:
		using hasher = HASHER;
		using equals = EQUALS;
		using traits = head<T, FIELD>;
		using type = hash<T, FIELD, _BUCKET_COUNT, HASHER, EQUALS>;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		using entry_type = typename traits::entry_type;
		using head_type = typename traits::head_type;
		using field_type = typename traits::field_type;

		constexpr static size_t BUCKET_COUNT = _BUCKET_COUNT;
		class head_iterator {
			using difference_type = typename traits::difference_type;
			using iterator_category = std::forward_iterator_tag; // only forward for right now
			using value_type = typename traits::value_type;
			using pointer = typename traits::pointer;
			using reference = typename traits::reference;
			using iterator = head_iterator;
			type& _hlist;
			head_type* _cbucket;
			pointer _current;
			void _get_next() {
				do {
					if (_cbucket >= &_hlist._buckets[BUCKET_COUNT]) break;
					if (_current == nullptr && (_current = &_cbucket->front()) != nullptr) break;
					if (_current != nullptr && (_current = traits::next(_current)) != nullptr) break;
					++_cbucket;
				} while (1);
			}
			//	size_t load_count;
		public:
			//size_t load_avg() const { return  load_count/
			head_iterator(type& hlist, size_t bindex) : _hlist(hlist), _cbucket(&hlist._buckets[bindex]), _current(nullptr) { _get_next(); }
			inline iterator operator++() noexcept {
				_get_next();
				return iterator(*this);
			}
			inline iterator operator++(int) noexcept {
				iterator tmp(*this);
				_get_next();
				return tmp;
			}
			// Must downcast from _List_node_base to _List_node to get to _M_data.
			value_type& operator*()  const noexcept { return *_current; }
			//const_value_type& operator*()  noexcept const { return *_current; }
			value_type& operator&()  const noexcept { return *_current; }
			pointer operator->()  const noexcept { return _current; }

			// do I need to test if the field pointers are equal?
			bool operator==(const iterator& r) const { return _current == r._current && _cbucket == r._cbucket; }
			bool operator!=(const iterator& r) const { return !(*this == r); }


		};
		using iterator = head_iterator;

		iterator begin() { return iterator(*this, 0); }
		iterator end() { return iterator(*this, BUCKET_COUNT); }
		hash() : _hasher{}, _equals{} {}
		// this is for debugging

		//const HASHER _hasher = HASHER{};
		//const EQUALS _equals= EQUALS{};

		inline bool obj_equal(const_reference a, value_type&& b) const {
			return _equals(a, std::forward<value_type>(b));
		}
		inline bool obj_equal(const_reference a, const_pointer b) const { return obj_equal(a, *b); }
		inline bool obj_equal(const_reference a, pointer b) const { return obj_equal(a, *b); }
		template<typename ... Args>
		inline bool obj_equal(const_reference a, Args... args) const {
			return _equals(a, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		pointer search(Args... args) {
			size_t hash = _hasher(std::forward<Args>(args)...);
			head_type& bucket = _buckets[hash% BUCKET_COUNT];
			if (!bucket.empty()) {
				for (reference o : bucket) {
					if (obj_equal(std::forward<const_reference>(o), std::forward<Args>(args)...)) {
						return &o;
					}
				}
			}
			return nullptr;
		}

		status insert(pointer obj) {
			const_reference obj_ref = *obj;
			size_t hash = _hasher(obj_ref);
			head_type& bucket = _buckets[hash% BUCKET_COUNT];
			if (!bucket.empty()) {
				for (reference o : bucket) {
					if (&o == obj)
						return status::exists;
					else if (obj_equal(std::forward<const_reference>(o), std::forward<const_reference>(obj_ref)))
						return status::dup; // return the pointer its equal to
				} // keep searching
			}
			// insert it then
			bucket.push_front(obj);
			return status::ok;
		}
		status insert(reference obj) { return insert(&obj); }

		template<typename ... Args>
		pointer remove(Args... args) noexcept {
			size_t hash = _hasher(std::forward<Args>(args)...) % BUCKET_COUNT;
			head_type& bucket = _buckets[hash];
			if (!bucket.empty()) {
				for (reference o : bucket) {
					if (obj_equal(o, std::forward<Args>(args)...)) {
						bucket.remove(&o);
						return &o;
					}
				}
			}
			return nullptr;
		}

		//
		status remove(pointer p) {
			const_reference obj_ref = *p;
			size_t hash = _hasher(obj_ref) % BUCKET_COUNT;
			head_type& bucket = _buckets[hash];
			if (!bucket.empty()) {
				for (reference o : bucket) {
					if (&o == p) {
						bucket.remove(p);
						return status::ok;
					}
					else if (_equals(std::forward<const_reference>(o), std::forward<const_reference>(obj_ref))) {
						bucket.remove(p);
						return status::dup;
					}
				}
			}
			return status::exists; // dosn't exist
		}
		// this is for debuging
		head_type* dbg_bucket(size_t i) { return  i < BUCKET_COUNT ? &_buckets[i] : nullptr; }
	protected:
		head_type _buckets[BUCKET_COUNT];
		const HASHER _hasher;
		const EQUALS _equals;
	};
};
#endif