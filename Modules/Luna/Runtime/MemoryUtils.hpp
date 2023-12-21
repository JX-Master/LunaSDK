/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MemoryUtils.hpp
* @author JXMaster
* @date 2020/2/16
* @brief Memory utility library
*/
#pragma once
#include "Iterator.hpp"
#include "Span.hpp"
#include "Algorithm.hpp"

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeMemoryUtils Memory utility library
	//! @}

	//! @addtogroup RuntimeMemoryUtils
	//! @{
	using std::memchr;
	using std::memcmp;
	using std::memset;
	using std::memcpy;
	using std::memmove;

	//! Clears the specified memory region to 0.
	//! @param[in] dst The start address of memory region to clear.
	//! @param[in] byte_count The size, in bytes, of the memory region to clear.
	//! @return Returns the `dst` pointer.
	inline void* memzero(void* dst, usize byte_count)
	{
		return memset(dst, 0, byte_count);
	}

	//! Clears the memory of the specified object to 0.
	//! @param[in] dst The object to clear. The size of the memory region to clear will be `sizeof(_Ty)`.
	//! @return Returns the `dst` pointer.
	template <typename _Ty>
	inline _Ty* memzero(_Ty* dst)
	{
		return (_Ty*)memzero(dst, sizeof(_Ty));
	}

	//! Copies the data for a 2D bitmap.
	//! @param[in] dst A pointer to the first pixel to be copied in destination bitmap.
	//! @param[in] src A pointer to the first pixel to be copied in source bitmap.
	//! @param[in] copy_size_per_row The size of the data to be copied for every row, in bytes.
	//! @param[in] num_rows The number of rows to copy.
	//! @param[in] dst_row_pitch The pitch to advance for one row in destination bitmap in bytes.
	//! @param[in] src_row_pitch The pitch to advance for one row in source bitmap in bytes.
	//! @return Returns the `dst` pointer.
	inline void* memcpy_bitmap(void* dst, const void* src, usize copy_size_per_row, usize num_rows, usize dst_row_pitch, usize src_row_pitch)
	{
		for (usize r = 0; r < num_rows; ++r)
		{
			memcpy((void*)((usize)dst + r * dst_row_pitch), (const void*)((usize)src + r * src_row_pitch), copy_size_per_row);
		}
		return dst;
	}

	//! Copies the data for a 3D bitmap.
	//! @param[in] dst A pointer to the first pixel to be copied in destination bitmap.
	//! @param[in] src A pointer to the first pixel to be copied in source bitmap.
	//! @param[in] copy_size_per_row The size of the data to be copied for every row, in bytes.
	//! @param[in] num_rows The number of rows to copy.
	//! @param[in] num_slices The number of slices (layers) to copy.
	//! @param[in] dst_row_pitch The pitch to advance for one row in destination bitmap in bytes.
	//! @param[in] src_row_pitch The pitch to advance for one row in source bitmap in bytes.
	//! @param[in] dst_slice_pitch The pitch to advance for one slice (layer) in destination bitmap in bytes.
	//! @param[in] src_slice_pitch The pitch to advance for one slice (layer) in source bitmap in bytes.
	//! @return Returns the `dst` pointer.
	inline void* memcpy_bitmap3d(
		void* dst, const void* src,
		usize copy_size_per_row,
		usize num_rows, usize num_slices,
		usize dst_row_pitch, usize src_row_pitch,
		usize dst_slice_pitch, usize src_slice_pitch)
	{
		for (usize r = 0; r < num_slices; ++r)
		{
			memcpy_bitmap((void*)((usize)dst + r * dst_slice_pitch), (const void*)((usize)src + r * src_slice_pitch), copy_size_per_row,
				num_rows, dst_row_pitch, src_row_pitch);
		}
		return dst;
	}

	//! Returns a pointer that offsets the specified pixels in the bitmap.
	//! @param[in] base The pointer to the first pixel in the bitmap.
	//! @param[in] x The x offset in pixels.
	//! @param[in] y The y offset in pixels.
	//! @param[in] z The z offset in pixels.
	//! @param[in] bytes_per_pixel The number of bytes per pixel.
	//! @param[in] row_pitch The pitch to advance for one row in bytes.
	//! @param[in] slice_pitch The pitch to advance for one slice in bytes.
	//! @return Returns the offseted pointer.
	inline void* pixel_offset(void* base, usize x, usize y, usize z, usize bytes_per_pixel, usize row_pitch, usize slice_pitch)
	{
		usize r = (usize)base;
		r += z * slice_pitch + y * row_pitch + x * bytes_per_pixel;
		return (void*)r;
	}

	inline void* pixel_offset(void* base, usize x, usize y, usize bytes_per_pixel, usize row_pitch)
	{
		usize r = (usize)base;
		r += y * row_pitch + x * bytes_per_pixel;
		return (void*)r;
	}

	//! Returns a pointer that offsets the specified pixels in the bitmap.
	//! @param[in] base The pointer to the first pixel in the bitmap.
	//! @param[in] x The x offset in pixels.
	//! @param[in] y The y offset in pixels.
	//! @param[in] z The z offset in pixels.
	//! @param[in] bytes_per_pixel The number of bytes per pixel.
	//! @param[in] row_pitch The pitch to advance for one row in bytes.
	//! @param[in] slice_pitch The pitch to advance for one slice in bytes.
	//! @return Returns the offseted pointer.
	inline const void* pixel_offset(const void* base, usize x, usize y, usize z, usize bytes_per_pixel, usize row_pitch, usize slice_pitch)
	{
		usize r = (usize)base;
		r += z * slice_pitch + y * row_pitch + x * bytes_per_pixel;
		return (const void*)r;
	}

	inline const void* pixel_offset(const void* base, usize x, usize y, usize bytes_per_pixel, usize row_pitch)
	{
		usize r = (usize)base;
		r += y * row_pitch + x * bytes_per_pixel;
		return (const void*)r;
	}

	namespace Impl
	{
		constexpr const u8 BIT_MASK[] = {
			0x01,	// 00000001
			0x02,	// 00000010
			0x04,	// 00000100
			0x08,	// 00001000
			0x10,	// 00010000
			0x20,	// 00100000
			0x40,	// 01000000
			0x80	// 10000000
		};

		constexpr const u8 BIT_MASK_REVERSE[] = {
			0xfe,	// 11111110
			0xfd,	// 11111101
			0xfb,	// 11111011
			0xf7,	// 11110111
			0xef,	// 11101111
			0xdf,	// 11011111
			0xbf,	// 10111111
			0x7f	// 01111111
		};
	}

	//! A integer literal suffix that multiples one number with 1024. Use it like `3_kb`.
	inline constexpr unsigned long long operator"" _kb(unsigned long long v)
	{
		return v * 1024;
	}
	//! A integer literal suffix that multiples one number with 1024 * 1024. Use it like `3_mb`.
	inline constexpr unsigned long long operator"" _mb(unsigned long long v)
	{
		return v * 1024 * 1024;
	}
	//! A integer literal suffix that multiples one number with 1024 * 1024 * 1024. Use it like `3_gb`.
	inline constexpr unsigned long long operator"" _gb(unsigned long long v)
	{
		return v * 1024 * 1024 * 1024;
	}
	//! A integer literal suffix that multiples one number with 1024 * 1024 * 1024 * 1024. Use it like `3_tb`.
	inline constexpr unsigned long long operator"" _tb(unsigned long long v)
	{
		return v * 1024 * 1024 * 1024 * 1024;
	}

	//! Tests if specified bit is 1.
	//! @param[in] base_addr The address of the bit to offset from.
	//! @param[in] bit_offset The number of bits shifted from the `base_addr`.
	//! @return Returns `true` if the bit is 1, `false` if the bit is 0.
	//! @remark The following cases demonstrate the index order of `bit_test`, `bit_set` and `bit_reset`.
	//! 
	//! base_addr: 0x1000, bit_offset: 0, `*((u8*)0x1000)`: 0000 1000b
	//! * test result: false.
	//! * value of `*((u8*)0x1000)` after set: 0000 1001b
	//! * value of `*((u8*)0x1000)` after reset: 0000 1000b.
	//! 
	//! base_addr: 0x1000, bit_offset: 3, `*((u8*)0x1000)`: 0000 1000b
	//! * test result: true.
	//! * value of `*((u8*)0x1000)` after set: 0000 1000b
	//! * value of `*((u8*)0x1000)` after reset: 0000 0000b.
	//! 
	//! base_addr: 0x1000, bit_offset: 8, `*((u8*)0x1001)`: 0000 1000b
	//! * test result: false.
	//! * value of `*((u8*)0x1001)` after set: 0000 1001b
	//! * value of `*((u8*)0x1001)` after reset: 0000 1000b.
	//! 
	//! base_addr: 0x1000, bit_offset: 11, `*((u8*)0x1001)`: 0000 1000b
	//! * test result: true.
	//! * value of `*((u8*)0x1001)` after set: 0000 1000b
	//! * value of `*((u8*)0x1001)` after reset: 0000 0000b.
	inline bool bit_test(const void* base_addr, usize bit_offset)
	{
		return (*(const u8*)((usize)base_addr + bit_offset / 8)) & Impl::BIT_MASK[bit_offset % 8] ? true : false;;
	}

	//! Sets the specified bit to 1.
	//! @param[in] base_addr The address of the bit to offset from.
	//! @param[in] bit_offset The number of bits shifted from the `base_addr`.
	//! @remark See remarks of @ref bit_test for details.
	inline void bit_set(void* addr, usize bit_offset)
	{
		*(u8*)((usize)addr + bit_offset / 8) |= Impl::BIT_MASK[bit_offset % 8];
	}

	//! Sets the specified bit to 0.
	//! @param[in] base_addr The address of the bit to offset from.
	//! @param[in] bit_offset The number of bits shifted from the `base_addr`.
	//! @remark See remarks of @ref bit_test for details.
	inline void bit_reset(void* addr, usize bit_offset)
	{
		*(u8*)((usize)addr + bit_offset / 8) &= Impl::BIT_MASK_REVERSE[bit_offset % 8];
	}

	//! Sets the specified bit to 1 if `value` is `true`, or to 0 if `value` is `false`.
	//! @param[in] base_addr The address of the bit to offset from.
	//! @param[in] bit_offset The number of bits shifted from the `base_addr`.
	//! @param[in] value The value to set for the bit.
	//! @remark See remarks of @ref bit_test for details.
	inline void bit_set(void* addr, usize bit_offset, bool value)
	{
		if (value) bit_set(addr, bit_offset);
		else bit_reset(addr, bit_offset);
	}

	//! Returns the address/size that aligns the origin address/size to the nearest matched aligned 
	//! address/size that is greater than or equal to the the origin address/size.
	//! @param[in] origin The unaligned address/size.
	//! @param[in] alignment The alignment boundary. If alignment is 0, `origin` will be returned as-is.
	//! @return Returns the aligned address/size.
    template <typename _Ty1, typename _Ty2>
	inline constexpr _Ty1 align_upper(_Ty1 origin, _Ty2 alignment)
	{
		return alignment ? ((origin + (_Ty1)alignment - 1) / (_Ty1)alignment * (_Ty1)alignment) : origin;
	}

	//! Represents one object that supports manual construction and destruction.
	//! @details `Unconstructed` provides a way to allocate the memory for a C++ object without their constructor/destructor
	//! being called by system. You have the ability to call their constructor/destructor manually.
	//! Such feature is useful when you need to declare some static constructed objects and want to control their construction/
	//! destruction orders.
	//! Note that the `Unconstructed` class does not actually know if the object is constructed, you need to manage it manually and
	//! always call the destructor of the object when you want to destroy it.
	template<typename _Ty>
	class Unconstructed
	{
	private:
		alignas(_Ty) u8 m_buffer[sizeof(_Ty)];
	public:

		Unconstructed() = default;
		Unconstructed(const Unconstructed&) = delete;
		Unconstructed(Unconstructed&&) = delete;
		Unconstructed& operator=(const Unconstructed&) = delete;
		Unconstructed& operator=(Unconstructed&&) = delete;

		//! Get a reference to the object.
		//! @return Returns a reference to the object.
		_Ty& get()
		{
			return *reinterpret_cast<_Ty*>(&m_buffer);
		}
		//! Get a const reference to the object.
		//! @return Returns a const reference to the object.
		const _Ty& get() const
		{
			return *reinterpret_cast<const _Ty*>(&m_buffer);
		}

		//! Constructs the object.
		//! @param[in] args Arguments that will be passed to the constructor of the object.
		template< typename... Args>
		void construct(Args&&... args)
		{
			new (m_buffer) _Ty(forward<Args>(args)...);
		}

		//! Destructs the object.
		void destruct()
		{
			get().~_Ty();
		}
	};

	//! Gets the real address for object or function `value`, even if the `operator&` of the object
	//! has been overloaded.
	//! @param[in] value The object to fetch address.
	//! @return Returns one pointer to the object.
	template<typename _Ty>
	_Ty* addressof(_Ty& value)
	{
#ifdef LUNA_COMPILER_MSVC
		return (__builtin_addressof(value));
#else
		return reinterpret_cast<_Ty*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(value)));
#endif
	}

	//! Calls the default constructor for the object.
	//! @param[in] dst An iterator to the object to be constructed.
	template <typename _Iter>
	inline void default_construct(_Iter dst)
	{
		new (static_cast<void*>(addressof(*dst)))
			typename iterator_traits<_Iter>::value_type;
	}

	//! Calls the value constructor for the object.
	//! @param[in] dst An iterator to the object to be constructed.
	template <typename _Iter>
	inline void value_construct(_Iter dst)
	{
		new (static_cast<void*>(addressof(*dst)))
			typename iterator_traits<_Iter>::value_type();
	}

	//! Calls the copy constructor for the object.
	//! @param[in] dst An iterator to the object to be constructed.
	//! @param[in] src An iterator to the object to copy from.
	template <typename _Iter1, typename _Iter2>
	inline void copy_construct(_Iter1 dst, _Iter2 src)
	{
		new (static_cast<void*>(addressof(*dst)))
			typename iterator_traits<_Iter1>::value_type(*src);
	}

	//! Calls the move constructor for the object.
	//! @param[in] dst An iterator to the object to be constructed.
	//! @param[in] src An iterator to the object to move from.
	template <typename _Iter1, typename _Iter2>
	inline void move_construct(_Iter1 dst, _Iter2 src)
	{
		new (static_cast<void*>(addressof(*dst)))
			typename iterator_traits<_Iter1>::value_type(move(*src));
	}

	//! Calls the direct constructor for the object.
	//! @param[in] dst An iterator to the object to be constructed.
	//! @param[in] args Arguments that will be passed to the constructor.
	template <typename _Iter, typename... _Args>
	inline void direct_construct(_Iter dst, _Args&&... args)
	{
		new (static_cast<void*>(addressof(*dst)))
			typename iterator_traits<_Iter>::value_type(forward<_Args>(args)...);
	}

	//! Calls the destructor of the object.
	//! @param[in] dst An iterator to the object to be destructed.
	template <typename _Iter>
	inline void destruct(_Iter dst)
	{
		using value_type = typename iterator_traits<_Iter>::value_type;
		static_cast<value_type*>(addressof(*dst))->~value_type();
	}

	//! Calls the copy assignment operator of the object.
	//! @param[in] dst An iterator to the object to be assigned.
	//! @param[in] src An iterator to the object to copy from.
	template <typename _Iter1, typename _Iter2>
	inline void copy_assign(_Iter1 dst, _Iter2 src)
	{
		(*dst) = (*src);
	}

	//! Calls the move assignment operator of the object.
	//! @param[in] dst An iterator to the object to be assigned.
	//! @param[in] src An iterator to the object to move from.
	template <typename _Iter1, typename _Iter2>
	inline void move_assign(_Iter1 dst, _Iter2 src)
	{
		(*dst) = move(*src);
	}

	namespace Impl
	{
		template <typename _Iter>
		using default_construct_range_is_value_type_class = typename is_class<typename iterator_traits<_Iter>::value_type>::type;
	}

	//! Default-constructs a range of objects.
	//! @param[in] first An iterator to the first object to be constructed.
	//! @param[in] last An iterator to one-past-last object to be constructed.
	//! @details This function performs default initialization on each object in the range [`first`, `last`).
	//! @remark If _Ty is a class type (`is_class<_Ty>::value` is `true_type`), this function calls the default constructor for each
	//! object. Otherwise, this call does nothing, as described by C++ standard.
	//! ref: https://en.cppreference.com/w/cpp/language/default_initialization
	template <typename _Iter>
	inline auto default_construct_range(_Iter first, _Iter last) -> enable_if_t<Impl::default_construct_range_is_value_type_class<_Iter>::value, void>	// is_class is true
	{
		for (; first != last; ++first)
		{
			default_construct(first);
		}
	}
	template <typename _Iter>
	inline auto default_construct_range(_Iter first, _Iter last) -> enable_if_t<!Impl::default_construct_range_is_value_type_class<_Iter>::value, void>// is_class is false
	{
		// do nothing.
	}

	namespace Impl
	{
		template <typename _Iter>
		using value_construct_range_is_value_type_trivial = typename conjunction<
			is_pointer<_Iter>,												// `_Iter` is a pointer type.
			negation<is_class<typename iterator_traits<_Iter>::value_type>>	// `value_type` of `_Iter` is not a class type.
		>::type;
	}

	//! Value-constructs a range of objects.
	//! @param[in] first An iterator to the first object to be constructed.
	//! @param[in] last An iterator to one-past-last object to be constructed.
	//! @details This function performs value initialization on each object in the range [`first`, `last`).
	//! @remark This call uses `memzero` to clear all bytes in the storage to 0 if all of the following conditions are matched:
	//! 1. `_Iter` is a pointer type.
	//! 2. `value_type` of `_Iter` is not a class type.
	//! Otherwise, this function calls the value constructor of each object.
	//! ref: https://en.cppreference.com/w/cpp/language/value_initialization
	//! @todo For types that satisfy `is_trivially_constructible<typename iterator_traits<_Iter>::value_type>`, when 
	//! the iterator is a pointer, memzero can also be applied.
	template <typename _Iter>
	inline auto value_construct_range(_Iter* first, _Iter* last) -> enable_if_t<Impl::value_construct_range_is_value_type_trivial<_Iter>::value, void> // is a trivial type.
	{
		memzero(static_cast<void*>(first), (size_t)(last - first) * sizeof(_Iter));
	}
	template <typename _Iter>
	inline auto value_construct_range(_Iter first, _Iter last, false_type) -> enable_if_t<!Impl::value_construct_range_is_value_type_trivial<_Iter>::value, void> // is not a trivial type.
	{
		for (; first != last; ++first)
		{
			value_construct(first);
		}
	}

	namespace Impl
	{
		template<typename _Iter1, typename _Iter2>
		using copy_construct_range_is_value_type_trivial = typename conjunction <
			is_pointer<_Iter1>,
			is_pointer<_Iter2>,
			is_same<typename iterator_traits<_Iter1>::value_type, typename iterator_traits<_Iter2>::value_type>,
			is_trivially_copy_constructible<typename iterator_traits<_Iter1>::value_type>
		>::type;
	}

	//! Copy-constructs a range of objects.
	//! @param[in] first An iterator to the first object to be copied from.
	//! @param[in] last An iterator to one-past-last object to be copied from.
	//! @param[in] d_first An iterator to the first object to be constructed.
	//! @return Returns an iterator to the one-past-last object to be constructed.
	//! @details This function uses each object in the range [`first`, `last`) to performs copy initialization on corresponding objects beginning with `d_first`.
	//! @par Valid Usage
	//! * The source range and the destination range must not overlap.
	//! @remark `memcpy` is used to copy the data directly if:
	//! 1. Both `_Iter1` and `_Iter2` are pointer types.
	//! 2. `value_type` of `_Iter1` and `_Iter2` is same.
	//! 3. `value_type` is trivially copy constructible.
	//! Otherwise, the copy constructor is called for every object to construct objects in destination range.
	template<typename _Iter1, typename _Iter2>
	inline auto copy_construct_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!Impl::copy_construct_range_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>
	{
		for (; first != last; ++d_first, (void) ++first)
		{
			copy_construct(d_first, first);
		}
		return d_first;
	}
	template<typename _Ty1, typename _Ty2>
	inline auto copy_construct_range(_Ty1* first, _Ty1* last, _Ty2* d_first) -> enable_if_t<Impl::copy_construct_range_is_value_type_trivial<_Ty1*, _Ty2*>::value, _Ty2*>
	{
		memcpy(static_cast<void*>(d_first), static_cast<const void*>(first), (size_t)(last - first) * sizeof(_Ty2));
		return d_first + (last - first);
	}

	//! Copy-constructs a range of objects. The range is provided by first object and object count.
	//! @param[in] first An iterator to the first object to be copied from.
	//! @param[in] count The number of objects to copy-construct.
	//! @param[in] d_first An iterator to the first object to be constructed.
	//! @return Returns an iterator to the one-past-last object to be constructed.
	//! @details This function uses each object in the range [`first`, `first + count`) to performs copy initialization on corresponding objects beginning with `d_first`.
	//! @par Valid Usage
	//! * The source range and the destination range must not overlap.
	//! @remark `memcpy` is used to copy the data directly if:
	//! 1. Both `_Iter1` and `_Iter2` are pointer types.
	//! 2. `value_type` of `_Iter1` and `_Iter2` is same.
	//! 3. `value_type` is trivially copy constructible.
	//! Otherwise, the copy constructor is called for every object to construct objects in destination range.
	template<typename _Iter1, typename _Iter2>
	inline auto copy_construct_range_n(_Iter1 first, usize count, _Iter2 d_first) -> enable_if_t<!Impl::copy_construct_range_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>
	{
		for (usize i = 0; i < count; ++i)
		{
			copy_construct(d_first, first);
			++d_first;
			++first;
		}
		return d_first;
	}
	template<typename _Ty1, typename _Ty2>
	inline auto copy_construct_range_n(_Ty1* first, usize count, _Ty2* d_first) -> enable_if_t<Impl::copy_construct_range_is_value_type_trivial<_Ty1*, _Ty2*>::value, _Ty2*>
	{
		_Ty1* last = first + count;
		memcpy(static_cast<void*>(d_first), static_cast<const void*>(first), (size_t)(last - first) * sizeof(_Ty2));
		return d_first + (last - first);
	}

	namespace Impl
	{
		template<typename _Iter1, typename _Iter2>
		using move_construct_range_is_value_type_trivial = typename is_trivially_move_constructible<typename iterator_traits<_Iter1>::value_type>::type;
	}

	//! Move-constructs a range of objects.
	//! @param[in] first An iterator to the first object to be moved from.
	//! @param[in] last An iterator to one-past-last object to be moved from.
	//! @param[in] d_first An iterator to the first object to be constructed.
	//! @return Returns an iterator to the one-past-last object to be constructed.
	//! @details This function uses each object in the range [`first`, `last`) to performs move initialization on corresponding objects beginning with `d_first`.
	//! @par Valid Usage
	//! * The source range and the destination range must not overlap.
	//! @remark If `value_type` of `_Iter2` is trivially move constructible, calls `copy_construct_range` to do the move
	//! construct.
	//! Otherwise, the move constructor is called for every object to construct objects in destination range.
	template<typename _Iter1, typename _Iter2>
	inline auto move_construct_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<Impl::move_construct_range_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>	// is trivially move constructible
	{
		return Luna::copy_construct_range(first, last, d_first);
	}
	template<typename _Iter1, typename _Iter2>
	inline auto move_construct_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!Impl::move_construct_range_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>	// is not trivially move constructible
	{
		for (; first != last; ++d_first, (void) ++first)
		{
			move_construct(d_first, first);
		}
		return d_first;
	}

	namespace Impl
	{
		template <typename _Iter>
		using destruct_range_is_value_type_trivial = typename is_trivially_destructible <typename iterator_traits<_Iter>::value_type>::type;
	}

	//! Destructs every object in the range.
	//! @param[in] first An iterator to the first object to be destructed.
	//! @param[in] last An iterator to one-past-last object to be destructed.
	//! @remark If `value_type` is trivially destructible, do nothing.
	//! Else, calls the destructor for each object in the range.
	template <typename _Iter>
	inline auto destruct_range(_Iter first, _Iter last) -> enable_if_t<Impl::destruct_range_is_value_type_trivial<_Iter>::value, void>	// is_trivially_destructible
	{
		// do nothing.
	}
	template <typename _Iter>
	inline auto destruct_range(_Iter first, _Iter last)	-> enable_if_t<!Impl::destruct_range_is_value_type_trivial<_Iter>::value, void>// is_trivially_destructible
	{
		for (; first != last; ++first)
		{
			destruct(first);
		}
	}

	namespace Impl
	{
		template<typename _Iter1, typename _Iter2>
		using copy_assign_range_is_value_type_trivial = typename conjunction <
			is_pointer<_Iter1>,
			is_pointer<_Iter2>,
			is_same<typename iterator_traits<_Iter1>::value_type, typename iterator_traits<_Iter2>::value_type>,
			is_trivially_copy_assignable<typename iterator_traits<_Iter1>::value_type>
		>::type;
	}

	//! Performs copy assignment operation on every object in the destination range using the corresponding object in the source range.
	//! @param[in] first An iterator to the first object to be copied from.
	//! @param[in] last An iterator to one-past-last object to be copied from.
	//! @param[in] d_first An iterator to the first object to be assigned.
	//! @return Returns an iterator to the one-past-last object to be assigned.
	//! @par Valid Usage
	//! * The source range and the destination range must not overlap.
	//! @remark `memcpy` is used to copy the data directly if:
	//! 1. Both `_Iter1` and `_Iter2` are pointer types.
	//! 2. `value_type` of `_Iter1` and `_Iter2` is same.
	//! 3. `value_type` is trivially copy assignable.
	//! Otherwise, the copy assignment operator is called for every object to copy assign objects in destination range.
	template<typename _Iter1, typename _Iter2>
	inline auto copy_assign_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!Impl::copy_assign_range_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>
	{
		for (; first != last; ++d_first, (void) ++first)
		{
			copy_assign(d_first, first);
		}
		return d_first;
	}
	template<typename _Ty1, typename _Ty2>
	inline auto copy_assign_range(_Ty1* first, _Ty1* last, _Ty2* d_first) -> enable_if_t<Impl::copy_assign_range_is_value_type_trivial<_Ty1*, _Ty2*>::value, _Ty2*>
	{
		memcpy(static_cast<void*>(d_first), static_cast<const void*>(first), (size_t)(last - first) * sizeof(_Ty2));
		return d_first + (last - first);
	}

	namespace Impl
	{
		template<typename _Iter1, typename _Iter2>
		using move_assign_range_is_value_type_trivial = typename conjunction <
			is_pointer<_Iter1>,
			is_pointer<_Iter2>,
			is_same<typename iterator_traits<_Iter1>::value_type, typename iterator_traits<_Iter2>::value_type>,
			is_trivially_copy_assignable<typename iterator_traits<_Iter1>::value_type>,
			is_trivially_move_assignable<typename iterator_traits<_Iter1>::value_type>
		>::type;

		template<typename _Iter1, typename _Iter2>
		using move_assign_range_backward_is_value_type_trivial = typename conjunction <
			is_pointer<_Iter1>,
			is_pointer<_Iter2>,
			is_same<typename iterator_traits<_Iter1>::value_type, typename iterator_traits<_Iter2>::value_type>,
			is_trivially_copy_assignable<typename iterator_traits<_Iter1>::value_type>,
			is_trivially_move_assignable<typename iterator_traits<_Iter1>::value_type>
		>::type;
	}

	//! Performs move assignment operation on every object in the destination range using the corresponding object in the source range.
	//! @param[in] first An iterator to the first object to be moved from.
	//! @param[in] last An iterator to one-past-last object to be moved from.
	//! @param[in] d_first An iterator to the first object to be assigned.
	//! @return Returns an iterator to the one-past-last object to be assigned.
	//! @remark The move operation is performed from first to last, the first element in destination range must not in the source range.
	//! `memmove` is used to move the data directly if:
	//! 1. Both `_Iter1` and `_Iter2` are pointer types.
	//! 2. `value_type` of `_Iter1` and `_Iter2` is same.
	//! 3. `value_type` is trivially copy assignable and trivially move assignable. (If the type is trivially move assignable but not trivially copy assignable, the move assignment operator
	//! behaves the same as the copy assignment operator.)
	//! Otherwise, the move assignment operator is called for every object to move assign objects in destination range.
	template<typename _Iter1, typename _Iter2>
	inline auto move_assign_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!Impl::move_assign_range_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>
	{
		for (; first != last; ++d_first, (void) ++first)
		{
			move_assign(d_first, first);
		}
		return d_first;
	}
	template<typename _Ty1, typename _Ty2>
	inline auto move_assign_range(_Ty1* first, _Ty1* last, _Ty2* d_first) -> enable_if_t<Impl::move_assign_range_is_value_type_trivial<_Ty1*, _Ty2*>::value, _Ty2*>
	{
		memmove(static_cast<void*>(d_first), static_cast<const void*>(first), (size_t)(last - first) * sizeof(_Ty2));
		return d_first + (last - first);
	}

	//! Same as @ref move_assign_range, but performs the move assign from back to front.
	//! @param[in] first An iterator to the first object to be moved from.
	//! @param[in] last An iterator to one-past-last object to be moved from.
	//! @param[in] d_last An iterator to the one-past-back object in the range to be assigned.
	//! @return Returns an iterator to the front object in the range to be assigned.
	//! @details The last element in destination range must not in the source range.
	template<typename _Iter1, typename _Iter2>
	inline auto move_assign_range_backward(_Iter1 first, _Iter1 last, _Iter2 d_last) -> enable_if_t<!Impl::move_assign_range_backward_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>
	{
		while (first != last)
		{
			--d_last;
			--last;
			move_assign(d_last, last);
		}
		return d_last;
	}
	template<typename _Ty1, typename _Ty2>
	inline auto move_assign_range_backward(_Ty1* first, _Ty1* last, _Ty2* d_last) -> enable_if_t<Impl::move_assign_range_backward_is_value_type_trivial<_Ty1*, _Ty2*>::value, _Ty2*>
	{
		ptrdiff_t sz = (last - first);
		memmove(static_cast<void*>(d_last - sz), static_cast<const void*>(first), (size_t)(sz) * sizeof(_Ty2));
		return d_last - sz;
	}

	//! Performs copy construct on each of the object in the range by taking a copy of the provided object.
	//! @param[in] first An iterator to the first object to be constructed.
	//! @param[in] last An iterator to one-past-last object to be constructed.
	//! @param[in] value The object to copy from for each object to be constructed.
	//! @return Returns an iterator to one-past-last object to be constructed.
	template <typename _Iter1, typename _Ty>
	inline _Iter1 fill_construct_range(_Iter1 first, _Iter1 last, const _Ty& value)
	{
		for (; first != last; ++first) 
		{
			new (static_cast<void*>(addressof(*first)))
				typename iterator_traits<_Iter1>::value_type(value);
		}
		return first;
	}

	//! Performs copy assignment on each of the object in the range by taking a copy of the provided object.
	//! @param[in] first An iterator to the first object to be assigned.
	//! @param[in] last An iterator to one-past-last object to be assigned.
	//! @param[in] value The object to copy from for each object to be assigned.
	//! @return Returns an iterator to one-past-last object to be assigned.
	template <typename _Iter1, typename _Ty>
	inline _Iter1 fill_assign_range(_Iter1 first, _Iter1 last, const _Ty& value)
	{
		for (; first != last; ++first) 
		{
			*first = value;
		}
		return first;
	}

	namespace Impl
	{
		template <typename _Iter1, typename _Iter2>
		using copy_relocate_range_is_iterator_pointer = typename conjunction<is_pointer<_Iter1>, is_pointer<_Iter2>>::type;

		template <typename _Ty1, typename _Ty2>
		inline auto copy_relocate_range_trivial(_Ty1* first, _Ty1* last, _Ty2* d_first) -> enable_if_t<copy_relocate_range_is_iterator_pointer<_Ty1*, _Ty2*>::value, _Ty2*>// is_pointer
		{
			memcpy(static_cast<void*>(d_first), static_cast<const void*>(first), (size_t)(last - first) * sizeof(_Ty2));
			return d_first + (last - first);
		}

		template <typename _Iter1, typename _Iter2>
		inline auto copy_relocate_range_trivial(_Iter1 first, _Iter1 last, _Iter2 d_first, false_type) -> enable_if_t<!copy_relocate_range_is_iterator_pointer<_Iter1, _Iter2>::value, _Iter2> // not is_pointer
		{
			for (; first != last; ++d_first, (void) ++first)
			{
				memcpy(static_cast<void*>(addressof(*d_first)), static_cast<const void*>(addressof(*first)), sizeof(typename iterator_traits<_Iter2>::value_type));
			}
			return d_first;
		}

		template <typename _Iter1, typename _Iter2>
		using copy_relocate_is_value_type_trivial = 
			typename conjunction<
			typename is_trivially_relocatable<typename iterator_traits<_Iter1>::value_type>::type,
			typename is_trivially_relocatable<typename iterator_traits<_Iter2>::value_type>::type>::type;

		template <typename _Iter1, typename _Iter2>
		inline _Iter1 copy_relocate_trivial(_Iter1 dst, _Iter2 src)
		{
			memcpy(static_cast<void*>(addressof(*dst)), static_cast<const void*>(addressof(*src)), sizeof(typename iterator_traits<_Iter1>::value_type));
			return dst;
		}
	}

	//! Relocates objects in the source range to a new range that is not overlap with the source range.
	//! @details After this call, the objects in the destination range behaves the same as the corresponding objects 
	//! formerly in the source range, except that the places(memory addresses) for objects are changed.
	//! @param[in] first An iterator to the first object to be relocated from.
	//! @param[in] last An iterator to one-past-last object to be relocated from.
	//! @param[in] d_first An iterator to the first object to be relocated to.
	//! @return Returns an iterator to the one-past-last object to be relocated to.
	//! @par Valid Usage
	//! * The iterator for the source range and destination range must have the same `value_type`.
	//! @remark 
	//! All objects in the destination range must in uninitialized state before this call. And after this call,
	//! all objects in the source range is in uninitialized state.
	//! 
	//! This call behaves differently in the following different conditions:
	//! 
	//! 1. If `value_type` of `_Iter1` is trivially relocatable (which is always true unless the user explicitly
	//! demonstrates), `memcpy` is used to copy the data from source place to destination place. If `_Iter` and 
	//! `_Iter2` is both pointer types, the full range will be copied by `memcpy` in one call, otherwise, `memcpy`
	//! will be called once per object. No constructors and destructors are called in this period.
	//! 
	//! 2. If `value_type` of `_Iter1` is not trivially relocatable, for every object in the destination range, 
	//! the move constructor will be called with the corresponding object in the source range passed in, then 
	//! the destructor of the corresponding object in the source range will be called.
	template <typename _Iter1, typename _Iter2>
	inline auto copy_relocate_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<Impl::copy_relocate_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>	// is_trivially_relocatable
	{
		return Impl::copy_relocate_range_trivial(first, last, d_first);
	}
	template <typename _Iter1, typename _Iter2>
	inline auto copy_relocate_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!Impl::copy_relocate_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>// not is_trivially_relocatable
	{
		for (; first != last; ++d_first, (void) ++first)
		{
			move_construct(d_first, first);
			destruct(first);
		}
		return d_first;
	}
	//! Relocates one object.
	//! @details After this call, the object in the destination memory behaves the same as the object 
	//! formerly in the source memory, except that the place(memory address) for the object is changed.
	//! @param[in] dst An iterator to relocation destination.
	//! @param[in] src An iterator to the object to be relocated.
	//! @return Returns `dst`.
	template <typename _Iter1, typename _Iter2>
	inline auto copy_relocate(_Iter1 dst, _Iter2 src) -> enable_if_t<Impl::copy_relocate_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter1>	// is_trivially_relocatable
	{
		return Impl::copy_relocate_trivial(dst, src);
	}
	template <typename _Iter1, typename _Iter2>
	inline auto copy_relocate(_Iter1 dst, _Iter2 src) -> enable_if_t<!Impl::copy_relocate_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter1>// not is_trivially_relocatable
	{
		move_construct(dst, src);
		destruct(src);
		return dst;
	}

	namespace Impl
	{
		template <typename _Iter1, typename _Iter2>
		using move_relocate_range_is_iterator_pointer = typename conjunction<is_pointer<_Iter1>, is_pointer<_Iter2>>::type;

		template <typename _Ty1, typename _Ty2>
		inline auto move_relocate_range_trivial(_Ty1* first, _Ty1* last, _Ty2* d_first) -> enable_if_t<move_relocate_range_is_iterator_pointer<_Ty1*, _Ty2*>::value, _Ty2*>	// is_pointer
		{
			memmove(static_cast<void*>(d_first), static_cast<const void*>(first), (size_t)(last - first) * sizeof(_Ty2));
			return d_first + (last - first);
		}

		template <typename _Iter1, typename _Iter2>
		inline auto move_relocate_range_trivial(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!move_relocate_range_is_iterator_pointer<_Iter1, _Iter2>::value, _Iter2>// not is_pointer
		{
			for (; first != last; ++d_first, (void) ++first)
			{
				memcpy(static_cast<void*>(addressof(*d_first)), static_cast<const void*>(addressof(*first)), sizeof(typename iterator_traits<_Iter2>::value_type));
			}
			return d_first;
		}

		template <typename _Iter1, typename _Iter2>
		using move_relocate_is_value_type_trivial =
			typename conjunction<
			typename is_trivially_relocatable<typename iterator_traits<_Iter1>::value_type>::type,
			typename is_trivially_relocatable<typename iterator_traits<_Iter2>::value_type>::type>::type;
	}

	//! Relocates objects in the source range to a new range.
	//! @details This function behaves the same as @ref copy_relocate_range, except that it allows the destination range 
	//! overlaps with the source range, proved that the first object in the destination range does not in the source range.
	//! @ref memmove is used instead of @ref memcpy if the whole range can be relocated in one call.
	//! @param[in] first An iterator to the first object to be relocated from.
	//! @param[in] last An iterator to one-past-last object to be relocated from.
	//! @param[in] d_first An iterator to the first object to be relocated to.
	//! @return Returns an iterator to the one-past-last object to be relocated to.
	template <typename _Iter1, typename _Iter2>
	inline auto move_relocate_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<Impl::move_relocate_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>	// is_trivially_relocatable
	{
		return Impl::move_relocate_range_trivial(first, last, d_first);
	}
	template <typename _Iter1, typename _Iter2>
	inline auto move_relocate_range(_Iter1 first, _Iter1 last, _Iter2 d_first) -> enable_if_t<!Impl::move_relocate_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2> // not is_trivially_relocatable
	{
		for (; first != last; ++d_first, (void) ++first)
		{
			move_construct(d_first, first);
			destruct(first);
		}
		return d_first;
	}

	namespace Impl
	{
		template <typename _Iter1, typename _Iter2>
		using move_relocate_range_backward_is_iterator_pointer = typename conjunction<is_pointer<_Iter1>, is_pointer<_Iter2>>::type;

		template <typename _Ty1, typename _Ty2>
		inline auto move_relocate_range_backward_trivial(_Ty1* first, _Ty1* last, _Ty2* d_last) -> enable_if_t<move_relocate_range_backward_is_iterator_pointer<_Ty1*, _Ty2*>::value, _Ty2*>	// iterator is_pointer
		{
			isize sz = last - first;
			memmove(static_cast<void*>(d_last - sz), static_cast<const void*>(first), (size_t)(sz) * sizeof(_Ty2));
			return d_last - sz;
		}

		template <typename _Iter1, typename _Iter2>
		inline auto move_relocate_range_backward_trivial(_Iter1 first, _Iter1 last, _Iter2 d_last) -> enable_if_t<!move_relocate_range_backward_is_iterator_pointer<_Iter1, _Iter2>::value, _Iter2> // iterator not is_pointer
		{
			while (first != last)
			{
				--last;
				--d_last;
				memcpy(static_cast<void*>(addressof(*d_last)), static_cast<const void*>(addressof(*last)), sizeof(typename iterator_traits<_Iter2>::value_type));
			}
			return d_last;
		}

		template <typename _Iter1, typename _Iter2>
		using move_relocate_backward_is_value_type_trivial = 
			typename conjunction<
			typename is_trivially_relocatable<typename iterator_traits<_Iter1>::value_type>::type,
			typename is_trivially_relocatable<typename iterator_traits<_Iter2>::value_type>::type>::type;
	}

	//! Relocates objects in the source range to a new range.
	//! @details This function behaves the same as @ref move_relocate_range, except that it relocates object from last to first, so
	//! the last object in the destination range should not in the source range.
	//! @ref memmove is used instead of @ref memcpy if the whole range can be relocated in one call.
	//! @param[in] first An iterator to the first object to be relocated from.
	//! @param[in] last An iterator to one-past-last object to be relocated from.
	//! @param[in] d_last An iterator to the one-past-last object to be relocated to.
	//! @return Returns an iterator to the front object to be relocated to in the destination range.
	template <typename _Iter1, typename _Iter2>
	inline auto move_relocate_range_backward(_Iter1 first, _Iter1 last, _Iter2 d_last) -> enable_if_t<Impl::move_relocate_backward_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2>	// is_trivially_relocatable
	{
		return Impl::move_relocate_range_backward_trivial(first, last, d_last);
	}
	template <typename _Iter1, typename _Iter2>
	inline auto move_relocate_range_backward(_Iter1 first, _Iter1 last, _Iter2 d_last) -> enable_if_t<!Impl::move_relocate_backward_is_value_type_trivial<_Iter1, _Iter2>::value, _Iter2> // not is_trivially_relocatable
	{
		while (first != last)
		{
			--last;
			--d_last;
			move_construct(d_last, last);
			destruct(last);
		}
		return d_last;
	}

	//! Describes one member used by memory layouting algorithms.
	struct MemoryLayoutMember
	{
		//! The size of this member in bytes. This property is filled by the user before calculating memory layouts.
		usize size;
		//! The alignment requirement of this member in bytes. This property is filled by the user before calculating memory layouts.
		usize alignment;
		//! The offset of this member in bytes. This property is filled by the memory layouting algorithm and can remain
		//! uninitialized when calculating memory layouts.
		usize offset;
		MemoryLayoutMember() = default;
		MemoryLayoutMember(usize size, usize alignment) :
			size(size),
			alignment(alignment) {}
	};

	//! Calculates the size, alignment and memory layout for one structure type.
	//! @details The calculated size, alignment and memory layout is compatible with C standard structure layout.
	//! @param[inout] members One span that provides members of the structure.
	//! @param[out] out_size The calculated size of the structure.
	//! @param[out] out_alignment The calculated alignment of the structure.
	inline void calculate_struct_memory_layout(Span<MemoryLayoutMember> members, usize& out_size, usize& out_alignment)
	{
		usize size = 0;
		usize alignment = 0;
		for (auto& i : members)
		{
			size = align_upper(size, i.alignment);
			i.offset = size;
			size += i.size;
			alignment = max(alignment, i.alignment);
		}
		size = align_upper(size, alignment);
		out_size = size;
		out_alignment = alignment;
	}

	//! Calculates the size, alignment and memory layout for one union type.
	//! @details The calculated size, alignment and memory layout is compatible with C standard union layout.
	//! @param[inout] members One span that provides members of the union.
	//! @param[out] out_size The calculated size of the union.
	//! @param[out] out_alignment The calculated alignment of the union.
	inline void calculate_union_memory_layout(Span<MemoryLayoutMember> members, usize& out_size, usize& out_alignment)
	{
		usize size = 0;
		usize alignment = 0;
		for (auto& i : members)
		{
			i.offset = 0;
			size = max(size, i.size);
			alignment = max(alignment, i.alignment);
		}
		size = align_upper(size, alignment);
		out_size = size;
		out_alignment = alignment;
	}

	//! @}
}
