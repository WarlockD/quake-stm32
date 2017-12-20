#ifndef ULIB_H_
#define ULIB_H_
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>
#include <chrono>
#include <array>
#include <ostream>
#include <istream>
#include <string_view>
#include <memory>
#include <fstream>
#include <variant>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include  <functional>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace ustl {
	using rotate = ::std::rotate;
	using  distance = ::std::distance;
	using copy_n = std::copy_n;

	enum class Endian { Little, Big };
	constexpr Endian native_byte_order = Endian::Little;
	/// Returns the number of bits in the given type
	template<typename T> static constexpr size_t  BitsInType() { return	sizeof(T) * CHAR_BIT; }
	template<typename T> static constexpr size_t  BitsInType(T v) { return BitsInType<T>(); }

	/// Indexes into a static array with bounds limit
	template <typename T, size_t N>
	inline constexpr T& VectorElement(T(&v)[N], size_t i) { return v[min(i, N - 1)]; }

	/// The alignment performed by default.
	constexpr static  size_t c_DefaultAlignment = sizeof(void*);

	/// \brief Rounds \p n up to be divisible by \p grain
	template <typename T> inline constexpr T AlignDown(T n, size_t grain = c_DefaultAlignment) { return n - n % grain; }
	template <typename T> inline constexpr T Align(T n, size_t grain = c_DefaultAlignment) { return AlignDown(n + grain - 1, grain); }


	template <typename T, bool IsSigned> struct __is_negative { inline constexpr bool operator()(const T& v) const { return v < 0; } };
	template <typename T> struct __is_negative<T, false> { inline constexpr bool operator()(const T&) const { return false; } };
	template <typename T> inline constexpr bool is_negative(const T& v) { return __is_negative<T, std::numeric_limits<T>::is_signed()>()(v); }
	template <typename T> inline constexpr T absv(T v) { return is_negative(v) ? -v : v; }
	template <typename T> inline constexpr T sign(T v) { return (0 < v) - is_negative(v); }
	template <typename T1, typename T2> inline constexpr size_t abs_distance(const T1& i1, const T2& i2) { return absv(std::distance(i1, i2)); }
	template <typename T> inline constexpr size_t size_of_elements(size_t n, const T*) { return n * sizeof(T); }

	template <typename T> constexpr T gcd(T a, T b) { return b ? gcd(b, a%b) : absv(a); }
	template <typename T> constexpr T lcm(T a, T b) { return a / gcd(a, b)*b; }
	constexpr inline uint16_t bswap_16(uint16_t v) { return v << 8 | v >> 8; }
	constexpr inline uint32_t bswap_32(uint32_t v) { return v << 24 | (v & 0xFF00) << 8 | ((v >> 8) & 0xFF00) | v >> 24; }
	constexpr inline uint64_t bswap_32(uint64_t v) { return (uint64_t(bswap_32(v)) << 32) | bswap_32(v >> 32); }
	/// \brief Swaps the byteorder of \p v.
	template <typename T>
	inline   T bswap(const T& v)
	{
		return T(BitsInType<T>() == 16 ?
			bswap_16(uint16_t(v)) : BitsInType<T>() == 32 ?
			bswap_32(uint32_t(v)) : BitsInType<T>() == 64 ?
			bswap_64(uint64_t(v)) : throw std::invalid_argument(), v); // not sure how this happened
	}
	template <typename T> constexpr inline T le_to_native(const T& v) { return native_byte_order == Endian::Little ? v : bswap(v); }
	template <typename T> constexpr inline T be_to_native(const T& v) { return native_byte_order == Endian::Big ? v : bswap(v); }
	template <typename T> constexpr inline T native_to_le(const T& v) { return native_byte_order == Endian::Little ? v : bswap(v); }
	template <typename T>constexpr  inline T native_to_be(const T& v) { return native_byte_order == Endian::Big ? v : bswap(v); }

	template <typename T, size_t N> static constexpr size_t  VectorSize(const T(&v)[N]) { return sizeof(v) / size_of_elements(1, v); }
	/// Packs \p s multiple times into \p b. Useful for loop unrolling.
	template <typename TSmall, typename TBig>
	inline void pack_type(TSmall s, TBig& b)
	{
		b = s;
		for (unsigned h = BitsInType(TSmall); h < BitsInType(TBig); h *= 2)
			b = (b << h) | b;
	}
	/// \brief Divides \p n1 by \p n2 and rounds the result up.
	/// This is in contrast to regular division, which rounds down.
	template <typename T1, typename T2>
	inline T1 DivRU(T1 n1, T2 n2)
	{
		T2 adj = n2 - 1;
		if (is_negative(n1))
			adj = -adj;
		return (n1 + adj) / n2;
	}


	/// \class cmemlink cmemlink.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief A read-only pointer to a sized block of memory.
	///
	/// Use this class the way you would a const pointer to an allocated unstructured block.
	/// The pointer and block size are available through member functions and cast operator.
	///
	/// Example usage:
	///
	/// \code
	///     void* p = malloc (46721);
	///     cmemlink a, b;
	///     a.link (p, 46721);
	///     assert (a.size() == 46721));
	///     b = a;
	///     assert (b.size() == 46721));
	///     assert (b.DataAt(34) == a.DataAt(34));
	///     assert (0 == memcmp (a, b, 12));
	/// \endcode
	///
	class cmemlink {
	public:
		typedef char		value_type;
		typedef const value_type*	pointer;
		typedef const value_type*	const_pointer;
		typedef value_type		reference;
		typedef value_type		const_reference;
		typedef size_t		size_type;
		typedef uint32_t		written_size_type;
		typedef ptrdiff_t		difference_type;
		typedef const_pointer	const_iterator;
		typedef const_iterator	iterator;
		typedef const cmemlink&	rcself_t;
	public:
		inline		cmemlink(void) : _data(nullptr), _size(0) { }
		inline		cmemlink(const void* p, size_type n) : _data(const_pointer(p)), _size(n) { assert(p || !n); }
		inline		cmemlink(const cmemlink& l) : _data(l._data), _size(l._size) {}
		inline virtual     ~cmemlink(void) noexcept {}
		void		link(const void* p, size_type n) {
			if (!p && n)
				throw bad_alloc(n);
			unlink();
			relink(p, n);
		}
		inline void		link(const cmemlink& l) { link(l.begin(), l.size()); }
		inline void		link(const void* first, const void* last) { link(first, distance(first, last)); }
		inline void		relink(const void* p, size_type n);
		virtual void	unlink(void) noexcept { _data = nullptr; _size = 0; }
		inline rcself_t	operator= (const cmemlink& l) { link(l); return *this; }
		bool		operator== (const cmemlink& l) const noexcept {
			return l._size == _size && (l._data == _data || 0 == std::memcmp(l._data, _data, _size));
		}
		inline void		swap(cmemlink& l) { ::ustl::swap(_data, l._data); ::ustl::swap(_size, l._size); }
		inline size_type	size(void) const { return _size; }
		inline size_type	max_size(void) const { return size(); }
		inline size_type	readable_size(void) const { return size(); }
		inline bool		empty(void) const { return !size(); }
		inline const_pointer	data(void) const { return _data; }
		inline const_pointer	cdata(void) const { return _data; }
		inline iterator	begin(void) const { return iterator(cdata()); }
		inline iterator	iat(size_type i) const { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) const { return iat(size()); }
		inline void		resize(size_type n) { _size = n; }
		inline void		read(istream&) { assert(!"ustl::cmemlink is a read-only object."); }
		//void		write(ostream& os) const;
		//size_type		stream_size(void) const noexcept;
		//void		text_write(ostringstream& os) const;
		//void		write_file(const char* filename, int mode = 0644) const;
	private:
		const_pointer	_data;		///< Pointer to the data block (const)
		size_type		_size;		///< size of the data block
	};

	/// \class memlink memlink.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief Wrapper for pointer to block with size.
	///
	/// Use this class the way you would a pointer to an allocated unstructured block.
	/// The pointer and block size are available through member functions and cast operator.
	///
	/// Example usage:
	/// \code
	///     void* p = malloc (46721);
	///     memlink a, b;
	///     a.link (p, 46721);
	///     assert (a.size() == 46721));
	///     b = a;
	///     assert (b.size() == 46721));
	///     assert (b.begin() + 34 == a.begin + 34);
	///     assert (0 == memcmp (a, b, 12));
	///     a.fill (673, b, 42, 67);
	///     b.erase (87, 12);
	/// \endcode
	///
	class memlink : public cmemlink {
	public:
		typedef value_type*			pointer;
		typedef cmemlink::pointer		const_pointer;
		typedef cmemlink::const_iterator	const_iterator;
		typedef pointer			iterator;
		typedef const memlink&		rcself_t;
	public:
		inline		memlink(void) : cmemlink() {}
		inline		memlink(void* p, size_type n) : cmemlink(p, n) {}
		inline		memlink(const void* p, size_type n) : cmemlink(p, n) {}
		inline		memlink(rcself_t l) : cmemlink(l) {}
		inline explicit	memlink(const cmemlink& l) : cmemlink(l) {}
		inline pointer	data(void) { return const_cast<pointer>(cmemlink::data()); }
		inline const_pointer	data(void) const { return cmemlink::data(); }
		inline iterator	begin(void) { return iterator(data()); }
		inline iterator	iat(size_type i) { assert(i <= size()); return begin() + i; }
		inline iterator	end(void) { return iat(size()); }
		inline const_iterator	begin(void) const { return cmemlink::begin(); }
		inline const_iterator	end(void) const { return cmemlink::end(); }
		inline const_iterator	iat(size_type i) const { return cmemlink::iat(i); }
		size_type		writable_size(void) const { return size(); }
		inline rcself_t	operator= (const cmemlink& l) { cmemlink::operator= (l); return *this; }
		inline rcself_t	operator= (rcself_t l) { cmemlink::operator= (l); return *this; }
		inline void		link(const void* p, size_type n) { cmemlink::link(p, n); }
		inline void		link(void* p, size_type n) { cmemlink::link(p, n); }
		inline void		link(const cmemlink& l) { cmemlink::link(l); }
		inline void		link(memlink& l) { cmemlink::link(l); }
		inline void		link(const void* first, const void* last) { link(first, distance(first, last)); }
		inline void		link(void* first, void* last) { link(first, distance(first, last)); }
		inline void		relink(const void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		relink(void* p, size_type n) { cmemlink::relink(p, n); }
		inline void		swap(memlink& l) { cmemlink::swap(l); }
		void		fill(const_iterator start, const void* p, size_type elsize, size_type elCount = 1) noexcept {
			assert(data() || !elCount || !elSize);
			assert(cstart >= begin() && cstart + elSize * elCount <= end());
			iterator start = const_cast<iterator>(cstart);
			if (elSize == 1)
				std::fill_n(start, elCount, *reinterpret_cast<const uint8_t*>(p));
			else while (elCount--)
				start = std::copy_n(const_iterator(p), elSize, start);
		}
		inline void		insert(const_iterator start, size_type size);
		inline void		erase(const_iterator start, size_type size);
		//void		read(istream& is);
	};

	/// Shifts the data in the linked block from \p start to \p start + \p n.
	/// The contents of the uncovered bytes is undefined.
	inline void memlink::insert(const_iterator cstart, size_type n)
	{
		assert(data() || !n);
		assert(cmemlink::begin() || !n);
		assert(cstart >= begin() && cstart + n <= end());
		iterator start = const_cast<iterator>(cstart);
		rotate(start, end() - n, end());
	}

	/// Shifts the data in the linked block from \p start + \p n to \p start.
	/// The contents of the uncovered bytes is undefined.
	inline void memlink::erase(const_iterator cstart, size_type n)
	{
		assert(data() || !n);
		assert(cmemlink::begin() || !n);
		assert(cstart >= begin() && cstart + n <= end());
		iterator start = const_cast<iterator>(cstart);
		rotate(start, start + n, end());
	}

	//----------------------------------------------------------------------

	/// A fast alternative to link which can be used when relinking to the same block (i.e. when it is resized)
	inline void cmemlink::relink(const void* p, size_type n)
	{
		_data = reinterpret_cast<const_pointer>(p);
		_size = n;
	}

	//----------------------------------------------------------------------



	/// \class memblock memblock.h ustl.h
	/// \ingroup MemoryManagement
	///
	/// \brief Allocated memory block.
	///
	/// Adds memory management capabilities to memlink. Uses malloc and realloc to
	/// maintain the internal pointer, but only if allocated using members of this class,
	/// or if linked to using the Manage() member function. Managed memory is automatically
	/// freed in the destructor.
	///
	template<typename ALLOC=std::allocater<char>>
	class memblock : public memlink {
	public:
		memblock(ALLOC alloc=ALLOC()) noexcept		: memlink(), _capacity(0) ,_alloc(alloc){ }
		memblock(const void* p, size_type n, ALLOC alloc = ALLOC()) : memlink(), _capacity(0), _alloc(alloc) { assign(p, n); }
		memblock(size_type n, ALLOC alloc = ALLOC()) : memlink(), _capacity(0), _alloc(alloc) { resize(n); }
		memblock(const cmemlink& b) : memlink(), _capacity(0), _alloc(b._alloc) { assign(b); }
		memblock(const memlink& b) : memlink(), _capacity(0), _alloc(b._alloc) { assign(b); }
		memblock(const memblock& b) : memlink(), _capacity(0), _alloc(b._alloc) { assign(b); }
		~memblock(void) noexcept { deallocate(); }
		inline			memblock(memblock&& b) : memlink(), _capacity(0) { swap(b); }
		inline memblock&		operator= (memblock&& b) { swap(b); return *this; }
		void unlink(void) noexcept { _capacity = 0; memlink::unlink(); }
		inline void			assign(const cmemlink& l) { assign(l.cdata(), l.readable_size()); }
		inline const memblock&	operator= (const cmemlink& l) { assign(l); return *this; }
		inline const memblock&	operator= (const memlink& l) { assign(l); return *this; }
		inline const memblock&	operator= (const memblock& l) { assign(l); return *this; }
		inline void			swap(memblock& l) noexcept { memlink::swap(l); ::ustl::swap(_capacity, l._capacity); }
		void			assign(const void* p, size_type n) {
			assert((p != (const void*)cdata() || size() == n) && "Self-assignment can not resize");
			resize(n);
			copy_n(const_pointer(p), n, begin());
		}
		void reserve(size_type newSize, bool bExact = false) {
			if ((newSize += minimumFreeCapacity()) <= _capacity)
				return;
			pointer oldBlock(is_linked() ? nullptr : data());
			const size_t alignedSize(NextPow2(newSize));
			if (!bExact)
				newSize = alignedSize;
			pointer newBlock = (pointer)ALLOC.allocate(newSize,oldBlock);
			if (!newBlock)
				throw bad_alloc(newSize);
			if (!oldBlock & (cdata() != nullptr))
				copy_n(cdata(), min(size() + 1, newSize), newBlock);
			link(newBlock, size());
			_capacity = newSize;
		}
		void resize(size_type newSize, bool bExact=true)
		{
			if (_capacity < newSize + minimumFreeCapacity())
				reserve(newSize, bExact);
			memlink::resize(newSize);
		}
		iterator			insert(const_iterator start, size_type size) {
			const uoff_t ip = start - begin();
			assert(ip <= size());
			resize(size() + n, false);
			memlink::insert(iat(ip), n);
			return iat(ip);
		}
		iterator			erase(const_iterator start, size_type size) {
			const uoff_t ep = start - begin();
			assert(ep + n <= size());
			reserve(size() - n);
			iterator iep = iat(ep);
			memlink::erase(iep, n);
			memlink::resize(size() - n);
			return iep;
		}
		inline void			clear(void) noexcept { resize(0); }
		inline size_type		capacity(void) const { return _capacity; }
		inline bool			is_linked(void) const { return !capacity(); }
		inline size_type		max_size(void) const { return is_linked() ? memlink::max_size() : SIZE_MAX; }
		inline void			manage(memlink& l) { manage(l.begin(), l.size()); }
		void deallocate(void) noexcept {
			if (_capacity) {
				assert(cdata() && "Internal error: space allocated, but the pointer is nullptr");
				assert(data() && "Internal error: read-only block is marked as allocated space");
				_alloc.deallocate(data(), _capacity);
			}
			unlink();
		}
		void shrink_to_fit(void) {
			if (is_linked())
				return;
			pointer newBlock = (pointer)LLOC.allocate(size(),begin());
			if (!newBlock && size())
				throw bad_alloc(size());
			_capacity = size();
			memlink::relink(newBlock, size());
		}
		void manage(void* p, size_type n) noexcept {
			assert(p || !n);
			assert(!_capacity && "Already managing something. deallocate or unlink first.");
			link(p, n);
			_capacity = n;
		}
		void copy_link(void) {
			const pointer p(begin());
			const size_t sz(size());
			if (is_linked())
				unlink();
			assign(p, sz);
		}
		//void			read(istream& is);
		//void			read_file(const char* filename);
	protected:
		virtual size_type minimumFreeCapacity(void) const noexcept { return 0; }
	private:
		size_type			_capacity;	///< Number of bytes allocated by Resize.
		ALLOC _alloc;
	};










};












#endif