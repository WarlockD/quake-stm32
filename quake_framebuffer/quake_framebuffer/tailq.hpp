#ifndef _TAILQ_HPP_
#define _TAILQ_HPP_


#include <cstdint>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <cassert>


#ifndef _DEBUG_STRUCTURES_DEFINED
#define _DEBUG_STRUCTURES_DEFINED


//#define TRACK_LIST_OWNER

#ifdef QUEUE_MACRO_DEBUG
#warn Use QUEUE_MACRO_DEBUG_TRACE and/or QUEUE_MACRO_DEBUG_TRASH
#define	QUEUE_MACRO_DEBUG_TRACE
#define	QUEUE_MACRO_DEBUG_TRASH
#endif

#ifdef QUEUE_MACRO_DEBUG_TRACE
#define _DEBUG_STRUCTURES_DEFINED
/* Store the last 2 places the queue element or head was altered */
struct qm_trace {
	unsigned long	 lastline;
	unsigned long	 prevline;
	const char	*lastfile;
	const char	*prevfile;
	qm_trace() : lastline(0), prevline(0), lastfile(nullptr), prevfile(nullptr) {}
};

#define	TRACEBUF	qm_trace trace;
#define	TRACEBUF_INITIALIZER	{ __LINE__, 0, __FILE__, NULL } ,

#define	QMD_TRACE_HEAD(head) do {					\
	(head)->trace.prevline = (head)->trace.lastline;		\
	(head)->trace.prevfile = (head)->trace.lastfile;		\
	(head)->trace.lastline = __LINE__;				\
	(head)->trace.lastfile = __FILE__;				\
} while (0)

#define	QMD_TRACE_ELEM(elem) do {					\
	(elem)->trace.prevline = (elem)->trace.lastline;		\
	(elem)->trace.prevfile = (elem)->trace.lastfile;		\
	(elem)->trace.lastline = __LINE__;				\
	(elem)->trace.lastfile = __FILE__;				\
} while (0)

#else	/* !QUEUE_MACRO_DEBUG_TRACE */
#define _DEBUG_STRUCTURES_DEFINED
#define	QMD_TRACE_ELEM(elem)
#define	QMD_TRACE_HEAD(head)
#define	TRACEBUF
#define	TRACEBUF_INITIALIZER
#endif	/* QUEUE_MACRO_DEBUG_TRACE */



#if defined(QUEUE_MACRO_DEBUG_TRACE)
#define	QMD_SAVELINK(name, link)	void **name = (void *)&(link)
#else	/* !QUEUE_MACRO_DEBUG_TRACE && !QUEUE_MACRO_DEBUG_TRASH */
#define	QMD_SAVELINK(name, link)
#endif	/* QUEUE_MACRO_DEBUG_TRACE || QUEUE_MACRO_DEBUG_TRASH */


#define TAILQ_TRACK_HEAD_IN_ENTRY

namespace tailq {
	template<typename T> class entry;
	template<typename T> using field = entry<T> T::*;
	template<typename T, field<T> FIELD> class entry_internal;
	template<typename T, field<T> FIELD> class head_impl;
	template<typename T, field<T> FIELD, bool _is_const> struct iterator;
	template<typename T, field<T> FIELD> struct pcontainer;
	constexpr static uintptr_t UNLINK_MAGIC_VALUE = 0x87654321;
#ifdef TRACK_LIST_OWNER
	// better than using a void pointer
	strict container{};
#endif

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
		constexpr static uintptr_t UNLINK_MAGIC = UNLINK_MAGIC_VALUE;
		constexpr static uintptr_t NOENTRY = UNLINK_MAGIC;
	};

	template<typename T, field<T> FIELD>
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
		constexpr static uintptr_t NOENTRY = btraits::NOENTRY;
		using entry_type = entry<T>;
		using field_type = field<T>;
		using head_type = head_impl<T, FIELD>;
		using container_type = pcontainer<T, FIELD>;
		constexpr static uintptr_t UNLINK_MAGIC = UNLINK_MAGIC_VALUE;
		friend head_type;
		constexpr static field_type member = FIELD;
		template<typename T, typename U> static constexpr size_t offsetOf(U T::*member)
		{
			return (char*)&((T*)nullptr->*member) - (char*)nullptr;
		}
		constexpr static inline  pointer& next(pointer p) {
			static_assert(member != nullptr, "Don't know what field you want!");
			return (p->*member).tqe_next;
		}
		constexpr static inline  pointer*& prev(pointer p) {
			static_assert(member != nullptr, "Don't know what field you want!");
			return (p->*member).tqe_prev;
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

#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
		constexpr static inline head_type** elm_head(pointer p) {
			static_assert(member != nullptr, "Don't know what field you want!");
			assert(p);
			return reinterpret_cast<head_type**>(&(p->*member).tqe_head);
		}
#endif
		static inline void unlink(pointer elm) { next(elm) = UNLINK_MAGIC; }
		static inline bool islinked(pointer elm) { return next(elm) != UNLINK_MAGIC; }
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
		* QMD_TAILQ_CHECK_HEAD(TAILQ_HEAD *head, TAILQ_ENTRY NAME)
		*
		* If the tailq is non-empty, validates that the first element of the tailq
		* points back at 'head.'
		*/
		static constexpr inline void check_head(const head_type& head) {
			if (!empty(head) && prev(head.tqh_first) != &head.tqh_first)
				LIST_DEBUG("Bad tailq head %p first->prev != head", &(head));
		}
		/*
		* QMD_TAILQ_CHECK_TAIL(TAILQ_HEAD *head, TAILQ_ENTRY NAME)
		*
		* Validates that the tail of the tailq is a pointer to pointer to NULL.
		*/
		static constexpr inline void check_tail(const head_type& head) {
			if (*head.tqh_last != nullptr)
				LIST_DEBUG("Bad tailq NEXT(%p->tqh_last) != NULL", &(head));
		}
		/*
		* QMD_TAILQ_CHECK_NEXT(TYPE *elm, TAILQ_ENTRY NAME)
		*
		* If an element follows 'elm' in the tailq, validates that the next element
		* points back at 'elm.'
		*/
		static constexpr inline void check_next(const pointer elm) {
			if (next((elm)) != nullptr && prev(next(elm)) != &next(elm))
				LIST_DEBUG("Bad link elm %p next->prev != elm", &(elm));
		}

		/*
		* QMD_TAILQ_CHECK_PREV(TYPE *elm, TAILQ_ENTRY NAME)
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
#endif
		// #define	TAILQ_PREV(elm, headname, field)				\ (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

		template<typename M>
		constexpr static inline pointer container_of_entry(M* ptr) { return (pointer)((char*)ptr - member_offset); }
		template<typename M>
		constexpr static inline const_pointer container_of_entry(const M* ptr) { return (pointer)((char*)ptr - member_offset); }

		static inline bool empty(const head_type& head) { return head.tqh_first == nullptr; }
		static inline T* last(const head_type& head) { return head.lh_first; }
		static inline T* first(const head_type& head) { return *head.tqh_last; }
		static inline void init(head_type& head) {
			head.tqh_first = nullptr;
			head.tqh_last = &head.tqh_first;
		}
		// entry is just here, technicaly we don't check for these values
		static inline void init(entry_type& elm) {
			elm.tqe_next = nullptr;
			elm.tqe_prev = &elm.tqe_next;
		}
		static inline void init(pointer elm) {
			elm->tqe_next = nullptr;
			elm->tqe_prev = &elm->tqe_next;
		}
		static inline void concat(head_type& head1, head_type& head2) {
			if (!empty(head2)) {
				*head1.tqh_last = head2.tqh_first;
				prev(head2.tqh_first) = head1.tqh_last;
				head1.tqh_last = head2.tqh_last;
				init(head2);
				QMD_TRACE_HEAD(head1);
				QMD_TRACE_HEAD(head2);
			}
		}
		static inline void insert_after(head_type& head, pointer listelm, pointer elm) {
			check_next(listelm);
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
			if (islinked(elm)) remove(elm);
			*elm_head(elm) = &head;
#endif
			if ((next(elm) = next(listelm)) != nullptr)
				prev(next(elm)) = &next(elm);
			else {
				head.tqh_last = &next(elm);
				QMD_TRACE_HEAD(head);
			}
			next(listelm) = elm;
			prev(elm) = &next(listelm);
			QMD_TRACE_ELEM(&(elm)->field);
			QMD_TRACE_ELEM(&(listelm)->field);
		}
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
		static inline void insert_after(pointer listelm, pointer elm) {
			assert(islinked(elm));
			insert_after(**elm_head(listelm), listelm, elm);
		}
#endif
		static inline void insert_before(pointer listelm, pointer elm) {
			check_prev(listelm);
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
			if (islinked(elm)) remove(elm);
			*elm_head(elm) = *elm_head(listelm);
#endif
			prev(elm) = prev(listelm);
			next(elm) = listelm;
			*prev(listelm) = elm;
			prev(listelm) = &next(elm);
			QMD_TRACE_ELEM(&(elm)->field);
			QMD_TRACE_ELEM(&(listelm)->field);
		}
		static inline void insert_head(head_type& head, pointer elm) {
			check_head(head);
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
			if (islinked(elm)) remove(elm);
			*elm_head(elm) = &head;
#endif
			if ((next(elm) = head.tqh_first) != nullptr)
				prev(head.tqh_first) = &next(elm);
			else
				head.tqh_last = &next(elm);
			head.tqh_first = elm;
			prev(elm) = &head.tqh_first;
			QMD_TRACE_HEAD(head);
			QMD_TRACE_ELEM(&(elm)->field);
		}
		static inline void insert_tail(head_type& head, pointer elm) {
			check_tail(head);
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
			if (islinked(elm)) remove(elm);
			*elm_head(elm) = &head;
#endif
			next(elm) = nullptr;
			prev(elm) = head.tqh_last;
			*head.tqh_last = elm;
			head.tqh_last = &next(elm);
			QMD_TRACE_HEAD(head);
			QMD_TRACE_ELEM(&(elm)->field);
		}


		static inline void remove(head_type& head, pointer elm) {
#ifdef QUEUE_MACRO_DEBUG_TRASH
			auto& oldnext = next(elm);
			auto& oldprev = prev(elm);
#endif
			assert(islinked(elm));
			check_next(elm);
			check_prev(elm);
			if (next(elm) != nullptr)
				prev(next(elm)) = prev(elm);
			else {
				head.tqh_last = prev(elm);
				QMD_TRACE_HEAD(head);
			}
			*prev(elm) = next(elm);
#ifdef QUEUE_MACRO_DEBUG_TRASH
			trashit(oldnext);
			trashit(*oldprev);
#endif
			QMD_TRACE_ELEM(&(elm)->field);
			unlink(elm);
		}
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
		static inline void remove(pointer elm) {
			assert(islinked(elm));
			remove(**elm_head(elm), elm);
		}

#endif
		// hack for now
		//template<typename A, typename B>
		//static inline void swap(A& head1, B& head2){
		static inline void swap(head_type& head1, head_type& head2) {
			auto swap_first = head1.tqh_first;
			auto swap_last = head1.tqh_last;
			head1.tqh_first = head2.tqh_first;
			head1.tqh_last = head2.tqh_last;
			head2.tqh_first = swap_first;
			head2.tqh_last = swap_last;
			if ((swap_first = head1.tqh_first) != nullptr)
				prev(swap_first) = &head1.tqh_first;
			else
				head1.tqh_last = &head1.tqh_first;
			if ((swap_first = head2.tqh_first) != nullptr)
				prev(swap_first) = &head2.tqh_first;
			else
				head2.tqh_last = &head2.tqh_first;

		}
		static inline  void splice(pointer pos, head_type& target, head_type& other, pointer other_first, pointer other_last) {
			// fuck it, make it a loop till I figure out tis pointer math
			for (auto c = other_first; c != other_last; c = next(c)) {
				remove(other, c);
				insert_after(target, pos, c);
				pos = c;
			}
		}
	};
	template<typename T>
	class entry {
	public:
		using traits = entry_traits<T>;
		using entry_type = entry<T>;
		using type = entry_type;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		constexpr static uintptr_t NOENTRY = traits::NOENTRY;
		friend T;
#ifdef TAILQ_TRACK_HEAD_IN_ENTRY
		void* tqe_head;
		constexpr entry() : tqe_head(nullptr), tqe_next(traits::UNLINK_MAGIC), tqe_prev(&tqe_next) {}
#else
		constexpr entry() : tqe_next(traits::UNLINK_MAGIC), tqe_prev(&tqe_next) {}
#endif

		//	bool unlinked() const { return &tqe_next == tqe_prev && tqe_next == nullptr; }
		pointer tqe_next;	/* next element */
		pointer *tqe_prev;	/* address of previous next element */

		inline bool islinked() const { return tqe_next != traits::UNLINK_MAGIC; }
	};

	// not sure of a way not to make this public since I cannot keep
	// the type information in here
	template<typename T, field<T> FIELD>
	class entry_internal : public entry<T> {
	public:
		using traits = entry<T>;
		using entry_type = entry_internal<T, FIELD>;
		using type = entry_type;
		using difference_type = typename traits::difference_type;
		using iterator_category = typename traits::iterator_category;
		using value_type = typename traits::value_type;
		using pointer = typename traits::pointer;
		using reference = typename traits::reference;;
		using const_pointer = typename traits::const_pointer;
		using const_reference = typename traits::const_reference;
		using head_type = head_impl<T, FIELD>;
		friend head_type;
		friend T;
		constexpr entry_internal() : entry<T>() {}
		bool unlinked() const { return &entry_type::tqe_next == entry_type::tqe_prev && entry_type::tqe_next == nullptr; }
	protected:
		constexpr inline  pointer& next() { return entry_type::tqe_next; }
		constexpr inline  pointer& prev() { return entry_type::tqe_prev; }
		void unlink() { entry_type::tqe_next = nullptr; entry_type::tqe_prev = &entry_type::tqe_next; }
		//pointer tqe_next;	/* next element */
		//	pointer *tqe_prev;	/* address of previous next element */
		TRACEBUF
	};

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

		inline void swap(head_type& head2) {

			traits::swap(*this, head2);
		}
		constexpr head_impl() noexcept : tqh_first(nullptr), tqh_last(&tqh_first) { QMD_TRACE_HEAD(*this); }
		constexpr head_impl(const head_impl& copy) = delete;
		constexpr head_impl& operator=(const head_impl& copy) = delete; // obviously  cannot be copyied
		constexpr head_impl(head_impl&& move) noexcept { std::swap(*this, move); }
		constexpr head_impl& operator=(head_impl&& move) noexcept {
			std::swap(*this, move);
			return *this;
		}
		bool empty() const { return traits::empty(*this); }
		iterator begin() { return iterator(tqh_first); }
		iterator end() { return iterator(nullptr); }
		const_iterator begin() const { return const_iterator(tqh_first); }
		const_iterator end() const { return const_iterator(nullptr); }
	protected:
		inline void _insert_after(pointer listelm, pointer elm) { traits::insert_after(*this, listelm, elm); }
		inline void _insert_before(pointer listelm, pointer elm) { traits::insert_before(listelm, elm); }
		inline void _insert_head(pointer elm) { traits::insert_head(*this, elm); }
		inline void _insert_tail(pointer elm) { traits::insert_tail(*this, elm); }
		inline void _remove(pointer elm) { traits::remove(*this, elm); }
		inline  void _splice(pointer pos, head_type& other, pointer other_first, pointer other_last) {
			traits::splice(pos, *this, other, other_first, other_last);
		}
		constexpr inline pointer _first() { return  tqh_first; }
		constexpr inline pointer _last() { return  *tqh_last; }
		constexpr inline const pointer _first() const { return  tqh_first; }
		constexpr inline const pointer _last() const { return  *tqh_last; }
		pointer tqh_first;	/* first element */
		pointer *tqh_last;	/* addr of last next element */
	};

	template<typename T, field<T> FIELD>
	class head : public head_impl<T, FIELD> {
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
		using head_type = typename traits::head_type;
		using field_type = typename traits::field_type;
		using iterator = typename traits::iterator;
		using const_iterator = typename traits::const_iterator;

		constexpr head() : traits::head_impl() {}
		head(const head& copy) = delete;
		head& operator==(const head& copy) = delete;
		head(head&& move) { traits::swap(move); }
		head& operator==(head&& move) {
			traits::swap(move);
			return *this;
		}

		inline pointer first_entry() { return traits::_first(); }
		inline pointer last_entry() { return traits::_last(); }
		inline const pointer first_entry()  const { return traits::_first(); }
		inline const pointer last_entry()  const { return traits::_last(); }

		reference front() { return *first_entry(); }
		reference back() { return *last_entry(); }
		const_reference front() const { return *first_entry(); }
		const_reference back() const { return *last_entry(); }
		inline void swap(head_type& head) { traits::swap(head); }


		inline void insert_after(pointer listelm, pointer elm) { traits::_insert_after(listelm, elm); }
		inline void insert_before(pointer listelm, pointer elm) { traits::_insert_before(listelm, elm); }
		inline void push_back(pointer elm) { traits::_insert_tail(elm); }
		inline void push_front(pointer elm) { traits::_insert_head(elm); }
		void push(pointer elm) { traits::_insert_tail(elm); }
		pointer pop() { return pop_back(); }

		// some list interface stuff, nothing to serious
		iterator insert_after(iterator position, pointer elm) {
			insert_after(&(*position), elm);
			return iterator(elm);
		}
		iterator insert_before(iterator position, pointer elm) {
			insert_before(&(*position), elm);
			return iterator(elm);
		}
		iterator insert(iterator position, pointer elm) {
			return insert_after(position, elm);
		}

		inline pointer pop_front() {
			pointer head = first_entry();
			remove(head);
			return head;
		}
		inline pointer pop_back() {
			pointer tail = last_entry();
			remove(tail);
			return tail;
		}
		inline iterator erase(iterator it) {
			pointer curelm = &(*it);
			if (curelm != nullptr) {
				++it;
				remove(curelm);
			}
			return it;
		}
		inline iterator erase(iterator first, iterator last) {
			pointer curelm = &(*first);
			while (curelm != nullptr && curelm != &(*last)) {
				auto next = traits::next(curelm);
				remove(curelm);
				curelm = next;
			}
			return last;
		}
		inline void remove(pointer elm) { traits::_remove(elm); }
		void splice(iterator pos, head_type& other, iterator first, iterator last) {
			traits::traits::splice(&(*pos), *this, other, &(*first), &(*last));
		}
		void splice(iterator pos, head_type& other, iterator it) {
			splice(pos, other, it, other.end());
		}
		void splice(iterator pos, head_type& other) {
			splice(pos, other, other.begin(), other.end());
		}

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
		using head_type = typename traits::head_type;
		using field_type = typename traits::field_type;
		using iterator = typename traits::iterator;
		using const_iterator = typename traits::const_iterator;

		constexpr prio_head() : traits::head_impl() {}

		inline pointer first_entry() { return traits::_first(); }
		inline pointer last_entry() { return traits::_last(); }
		inline const pointer first_entry()  const { return traits::_first(); }
		inline const pointer last_entry()  const { return traits::_last(); }

		reference front() { return *first_entry(); }
		reference back() { return *last_entry(); }
		const_reference front() const { return *first_entry(); }
		const_reference back() const { return *last_entry(); }

		iterator insert(iterator position, pointer elm) { return insert_after(position); }
		inline void swap(type& head) { traits::swap(*this, head); }
		bool empty() const { return traits::empty(*this); }
		const_reference top() const { return *traits::_first(); }
		void push(pointer elm) {
			assert(elm); // checking
			pointer curelm = traits::_first();
			if (curelm == nullptr)
				traits::_insert_head(elm);
			else {
				COMPARE comp;
				pointer ptr_before = nullptr;
				while (curelm != nullptr) {
					if (ptr_before != nullptr) {
						if (comp(std::forward<const_reference>(*curelm), std::forward<const_reference>(*elm))) {
							ptr_before = curelm;
						}
					}
					if (curelm == elm) {
						// its already in here, this is stupid but makes it easy
						traits::_remove(elm);
						push(elm); // this is a stupid hack to requeue
						return;
					}
					curelm = traits::next_entry(curelm);
				}
				if (ptr_before == nullptr)
					traits::_insert_tail(elm);
			}
		}
		pointer pop() {
			pointer f = traits::_first();
			if (f) traits::_remove(f);
			return f;
		}
		inline void remove(pointer elm) { traits::_remove(elm); }
		inline iterator erase(iterator it) {
			pointer curelm = &(*it);
			if (curelm != nullptr) {
				++it;
				remove(curelm);
			}
			return it;
		}
	};
};



#endif