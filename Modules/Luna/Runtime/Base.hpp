/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Base.hpp
* @author JXMaster
* @date 2018/10/26
* @brief Defines basic types and functions to be used in Luna SDK.
 */
#pragma once

#include "PlatformDefines.hpp"

// Include C++ STD.
// This is the only place where Luna SDK includes C++ STD. We only uses a minimum set of 
// STD headers so that the SDK is portable to most platforms.
#include <cstdint>
#include <cfloat>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cctype>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <initializer_list>
#include <limits>

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeBaseType Basic types
	//! @}

	//! @addtogroup RuntimeBaseType
	//! @{

    //------------------------------------------------------------------------------------------------------
    //  Basic Types
    //------------------------------------------------------------------------------------------------------

    //! Unsigned 8-bit integer (0~255).
    using u8 = std::uint8_t;
    //! Unsigned 16-bit integer (0~65535).
    using u16 = std::uint16_t;
    //! Unsigned 32-bit integer (0~4294967295).
    using u32 = std::uint32_t;
    //! Unsigned 64-bit integer (0~18446744073709551615).
    using u64 = std::uint64_t;
    //! Signed 8-bit integer (-128~127).
    using i8 = std::int8_t;
    //! Signed 16-bit integer (-32768~32767).
    using i16 = std::int16_t;
    //! Signed 32-bit integer (-2147482648~2147483647).
    using i32 = std::int32_t;
    //! Signed 64-bit integer (-9223372036854775808~9223372036854775807).
    using i64 = std::int64_t;
    //! 32-bit (single precision) floating point number.
    using f32 = float;
    //! 64-bit (double precision) floating point number.
    using f64 = double;

	//! An alias of `u8` that represents one byte.
	//! You may use this type to differentiate the concept of byte stream (byte_t*) from number array (u8*).
	using byte_t = u8;

    //! `usize` is the unsigned integer type of whose length marches the machine architecture.
	//! In particular, in 32-bit application, this is 32-bit unsigned integer; in 64-bit application, 
    //! this is 64-bit unsigned integer.
	//! The `usize` type is guaranteed to be large enough to store a indexable memory address, so that 
	//! any pointer can be reinterpreted casted to `usize`.
#ifdef LUNA_PLATFORM_32BIT
    using usize = u32;
#elif LUNA_PLATFORM_64BIT
    using usize = u64;
#endif

    //! `isize` is similar to `usize` , but is a signed integer type, so this can be used to 
    //! store the result of subtracting two pointers, and can be used to offset memory addresses and pointers.
#ifdef LUNA_PLATFORM_32BIT
    using isize = i32;
#elif LUNA_PLATFORM_64BIT
    using isize = i64;
#endif

	using nullptr_t = std::nullptr_t;

	//! `opaque_t` is used to represent one opaque pointer that shall not be reinterpreted or dereferred by the user.
	//! opaque_t are ususally used as arguments or returns values of interface functions to hide the implementation 
	//! from the user.
	using opaque_t = void*;

    //! 8-bit character. Signed/unsigned is unspecified, cast this to u8/i8 for fetching number.
	using c8 = char;
	static_assert(sizeof(c8) == 1, "Incorrect c8 type size.");
    //! 16-bit character. Signed/unsigned is unspecified, cast this to u16/i16 for fetching number.
    using c16 = char16_t;
    //! 32-bit character. Signed/unsigned is unspecified, cast this to u32/i32 for fetching number.
    using c32 = char32_t;

	template <typename _Ty>
	using InitializerList = std::initializer_list<_Ty>;

	using VarList = va_list;

	//------------------------------------------------------------------------------------------------------
	//  Basic Constants.
	//------------------------------------------------------------------------------------------------------

    //! The maximum number that can be represented by one i8 value.
	constexpr i8 I8_MAX = (i8)INT8_MAX;
    //! The minimum number that can be represented by one i8 value.
	constexpr i8 I8_MIN = (i8)INT8_MIN;
    //! The maximum number that can be represented by one i16 value.
	constexpr i16 I16_MAX = (i16)INT16_MAX;
    //! The minimum number that can be represented by one i16 value.
	constexpr i16 I16_MIN = (i16)INT16_MIN;
    //! The maximum number that can be represented by one i32 value.
	constexpr i32 I32_MAX = (i32)INT32_MAX;
    //! The minimum number that can be represented by one i32 value.
	constexpr i32 I32_MIN = (i32)INT32_MIN;
    //! The maximum number that can be represented by one i64 value.
	constexpr i64 I64_MAX = (i64)INT64_MAX;
    //! The minimum number that can be represented by one i64 value.
	constexpr i64 I64_MIN = (i64)INT64_MIN;
    //! The maximum number that can be represented by one u8 value.
	constexpr u8 U8_MAX = (u8)UINT8_MAX;
    //! The maximum number that can be represented by one u16 value.
	constexpr u16 U16_MAX = (u16)UINT16_MAX;
    //! The maximum number that can be represented by one u32 value.
	constexpr u32 U32_MAX = (u32)UINT32_MAX;
    //! The maximum number that can be represented by one u64 value.
	constexpr u64 U64_MAX = (u64)UINT64_MAX;
#ifdef LUNA_PLATFORM_64BIT
    //! The maximum number that can be represented by one usize value.
	constexpr usize USIZE_MAX = U64_MAX;
    //! The maximum number that can be represented by one isize value.
	constexpr isize ISIZE_MAX = I64_MAX;
	//! The minimum number that can be represented by one isize value.
	constexpr isize ISIZE_MIN = I64_MIN;
#else
    //! The maximum number that can be represented by one usize value.
	constexpr usize USIZE_MAX = U32_MAX;
    //! The maximum number that can be represented by one isize value.
	constexpr isize ISIZE_MAX = I32_MAX;
	//! The minimum number that can be represented by one isize value.
	constexpr isize ISIZE_MIN = I32_MIN;
#endif

	//! The maximum alignment requirement for a standard-layout scalar value.
	constexpr usize MAX_ALIGN = alignof(long double);

	constexpr f32 F32_MIN = (f32)FLT_MIN;
	constexpr f32 F32_MAX = (f32)FLT_MAX;
	constexpr f32 F32_EPSILON = (f32)FLT_EPSILON;
	constexpr f64 F64_MIN = (f64)DBL_MIN;
	constexpr f64 F64_MAX = (f64)DBL_MAX;
	constexpr f64 F64_EPSILON = (f64)DBL_EPSILON;
	
	template <typename _Ty>
	using numeric_limits = std::numeric_limits<_Ty>;

	// 1-bit sign, 8-bits exponent, 27-bits fraction.

	constexpr u32 F32_SIGN_MASK = 0x80000000;
	constexpr u32 F32_EXPONENT_MASK = 0x7F800000;
	constexpr u32 F32_FRACTION_MASK = 0x007FFFFF;

	// 1-bit sign, 11-bits exponent, 52-bits fraction.

	constexpr u64 F64_SIGN_MASK 	= 0x8000000000000000ULL;
	constexpr u64 F64_EXPONENT_MASK = 0x7FF0000000000000ULL;
	constexpr u64 F64_FRACTION_MASK = 0x000FFFFFFFFFFFFFULL;

	//! @}

	//------------------------------------------------------------------------------------------------------
	//  Type Traits
	//------------------------------------------------------------------------------------------------------

	using std::integral_constant;
	using std::true_type;
	using std::false_type;

	using std::is_void;
	using std::is_null_pointer;
	using std::is_integral;
	using std::is_floating_point;
	using std::is_array;
	using std::is_enum;
	using std::is_union;
	using std::is_class;
	using std::is_function;
	using std::is_pointer;
	using std::is_lvalue_reference;
	using std::is_rvalue_reference;
	using std::is_member_object_pointer;
	using std::is_member_function_pointer;

	template<class _T> constexpr bool is_void_v = is_void<_T>::value;
	template<class _T> constexpr bool is_null_pointer_v = is_null_pointer<_T>::value;
	template<class _T> constexpr bool is_integral_v = is_integral<_T>::value;
	template<class _T> constexpr bool is_floating_point_v = is_floating_point<_T>::value;
	template<class _T> constexpr bool is_enum_v = is_enum<_T>::value;
	template<class _T> constexpr bool is_union_v = is_union<_T>::value;
	template<class _T> constexpr bool is_class_v = is_class<_T>::value;
	template<class _T> constexpr bool is_function_v = is_function<_T>::value;
	template<class _T> constexpr bool is_pointer_v = is_pointer<_T>::value;
	template<class _T> constexpr bool is_lvalue_reference_v = is_lvalue_reference<_T>::value;
	template<class _T> constexpr bool is_rvalue_reference_v = is_rvalue_reference<_T>::value;
	template<class _T> constexpr bool is_member_object_pointer_v = is_member_object_pointer<_T>::value;
	template<class _T> constexpr bool is_member_function_pointer_v = is_member_function_pointer<_T>::value;

	using std::is_fundamental;
	using std::is_arithmetic;
	using std::is_scalar;
	using std::is_object;
	using std::is_compound;
	using std::is_reference;
	using std::is_member_pointer;

	template<class _T> constexpr bool is_fundamental_v = is_fundamental<_T>::value;
	template<class _T> constexpr bool is_arithmetic_v = is_arithmetic<_T>::value;
	template<class _T> constexpr bool is_scalar_v = is_scalar<_T>::value;
	template<class _T> constexpr bool is_object_v = is_object<_T>::value;
	template<class _T> constexpr bool is_compound_v = is_compound<_T>::value;
	template<class _T> constexpr bool is_reference_v = is_reference<_T>::value;
	template<class _T> constexpr bool is_member_pointer_v = is_member_pointer<_T>::value;

	using std::is_const;
	using std::is_volatile;
	using std::is_trivial;
	using std::is_trivially_copyable;
	using std::is_standard_layout;
	using std::is_empty;
	using std::is_polymorphic;
	using std::is_abstract;
	using std::is_final;
	using std::is_signed;
	using std::is_unsigned;

	template<class _T> constexpr bool is_const_v = is_const<_T>::value;
	template<class _T> constexpr bool is_volatile_v = is_volatile<_T>::value;
	template<class _T> constexpr bool is_trivial_v = is_trivial<_T>::value;
	template<class _T> constexpr bool is_trivially_copyable_v = is_trivially_copyable<_T>::value;
	template<class _T> constexpr bool is_standard_layout_v = is_standard_layout<_T>::value;
	template<class _T> constexpr bool is_empty_v = is_empty<_T>::value;
	template<class _T> constexpr bool is_polymorphic_v = is_polymorphic<_T>::value;
	template<class _T> constexpr bool is_abstract_v = is_abstract<_T>::value;
	template<class _T> constexpr bool is_final_v = is_final<_T>::value;
	template<class _T> constexpr bool is_signed_v = is_signed<_T>::value;
	template<class _T> constexpr bool is_unsigned_v = is_unsigned<_T>::value;

	using std::is_constructible;
	using std::is_trivially_constructible;
	using std::is_nothrow_constructible;
	using std::is_default_constructible;
	using std::is_trivially_default_constructible;
	using std::is_nothrow_default_constructible;
	using std::is_copy_constructible;
	using std::is_trivially_copy_constructible;
	using std::is_nothrow_copy_constructible;
	using std::is_move_constructible;
	using std::is_trivially_move_constructible;
	using std::is_nothrow_move_constructible;
	using std::is_assignable;
	using std::is_trivially_assignable;
	using std::is_nothrow_assignable;
	using std::is_copy_assignable;
	using std::is_trivially_copy_assignable;
	using std::is_nothrow_copy_assignable;
	using std::is_move_assignable;
	using std::is_trivially_move_assignable;
	using std::is_nothrow_move_assignable;
	using std::is_destructible;
	using std::is_trivially_destructible;
	using std::is_nothrow_destructible;
	using std::has_virtual_destructor;

	template<class _T> constexpr bool is_constructible_v = is_constructible<_T>::value;
	template<class _T> constexpr bool is_trivially_constructible_v = is_trivially_constructible<_T>::value;
	template<class _T> constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<_T>::value;
	template<class _T> constexpr bool is_default_constructible_v = is_default_constructible<_T>::value;
	template<class _T> constexpr bool is_trivially_default_constructible_v = is_trivially_default_constructible<_T>::value;
	template<class _T> constexpr bool is_nothrow_default_constructible_v = is_nothrow_default_constructible<_T>::value;
	template<class _T> constexpr bool is_copy_constructible_v = is_copy_constructible<_T>::value;
	template<class _T> constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<_T>::value;
	template<class _T> constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<_T>::value;
	template<class _T> constexpr bool is_move_constructible_v = is_move_constructible<_T>::value;
	template<class _T> constexpr bool is_trivially_move_constructible_v = is_trivially_move_constructible<_T>::value;
	template<class _T> constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<_T>::value;
	template<class _T, class _U> constexpr bool is_assignable_v = is_assignable<_T, _U>::value;
	template<class _T, class _U> constexpr bool is_trivially_assignable_v = is_trivially_assignable<_T, _U>::value;
	template<class _T, class _U> constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<_T, _U>::value;
	template<class _T> constexpr bool is_copy_assignable_v = is_copy_assignable<_T>::value;
	template<class _T> constexpr bool is_trivially_copy_assignable_v = is_trivially_copy_assignable<_T>::value;
	template<class _T> constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<_T>::value;
	template<class _T> constexpr bool is_move_assignable_v = is_move_assignable<_T>::value;
	template<class _T> constexpr bool is_trivially_move_assignable_v = is_trivially_move_assignable<_T>::value;
	template<class _T> constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<_T>::value;
	template<class _T> constexpr bool is_destructible_v = is_destructible<_T>::value;
	template<class _T> constexpr bool is_trivially_destructible_v = is_trivially_destructible<_T>::value;
	template<class _T> constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<_T>::value;
	template<class _T> constexpr bool has_virtual_destructor_v = has_virtual_destructor<_T>::value;

	using std::is_same;
	using std::is_base_of;
	using std::is_convertible;
	template<class _T, class _U> constexpr bool is_same_v = is_same<_T, _U>::value;
	template<class _Base, class _Derived> constexpr bool is_base_of_v = is_base_of<_Base, _Derived>::value;
	template<class _From, class _To> constexpr bool is_convertible_v = is_convertible<_From, _To>::value;

	using std::remove_cv;
	using std::remove_const;
	using std::remove_volatile;
	using std::remove_cv_t;
	using std::remove_const_t;
	using std::remove_volatile_t;

	using std::add_cv;
	using std::add_const;
	using std::add_volatile;
	using std::add_cv_t;
	using std::add_const_t;
	using std::add_volatile_t;

	using std::remove_reference;
	using std::remove_reference_t;
	using std::add_lvalue_reference;
	using std::add_lvalue_reference_t;
	using std::add_rvalue_reference;
	using std::add_rvalue_reference_t;

	using std::remove_pointer;
	using std::remove_pointer_t;
	using std::add_pointer;
	using std::add_pointer_t;
	using std::make_signed;
	using std::make_signed_t;
	using std::make_unsigned;
	using std::make_unsigned_t;
	using std::remove_extent;
	using std::remove_extent_t;
	using std::remove_all_extents;
	using std::remove_all_extents_t;

	using std::aligned_storage;
	using std::aligned_storage_t;
	using std::aligned_union;
	using std::aligned_union_t;
	using std::decay;
	using std::decay_t;
	using std::enable_if;
	using std::enable_if_t;
	using std::conditional;
	using std::conditional_t;
	using std::common_type;
	using std::common_type_t;
	using std::underlying_type;
	using std::underlying_type_t;

#ifdef LUNA_COMPILER_CPP17
	using std::invoke_result;
	using std::invoke_result_t;
#else
	template <typename _Func, typename... _Args>
	struct invoke_result
	{
		using type = typename std::result_of<_Func(_Args...)>::type;
	};
	template <typename _Func, typename... _Args>
	using invoke_result_t = typename invoke_result<_Func, _Args...>::type;
#endif


	/*template <typename _Ty>
	typename add_rvalue_reference<_Ty>::type declval()
	{
		static_assert(false, "declval not allowed in an evaluated context");
	}*/

	namespace impl
	{
		template <typename _Ty>
		inline constexpr bool is_over_aligned()
		{
			return alignof(_Ty) > MAX_ALIGN;
		}
	}

	//! Checks if the specified type's alignment requirement is greater than alignof(max_align_t).
	template <typename _Ty>
	struct is_over_aligned : integral_constant<bool, impl::is_over_aligned<_Ty>()> {};

	template<typename _Ty>
	constexpr bool is_over_aligned_v = is_over_aligned<_Ty>::value;

	//! Checks if the specified type can be trivially relocatable.
	//! One object is trivially relocatable if the data of one well-constructed instance of object can be copied to a new 
	//! uninitialized memory by bitwise copy (for example, `memcpy`, `memmove` or `realloc`) and the new copied object behaves
	//! the same as the original object, such copy is called a "relocating operation". After the operation, the original memory
	//! for the object is treated as uninitialized and does not have destructor called before it is freed.
	//! 
	//! If one object can be trivially relocated, then when the memory that holding the object needs to be reallocated (for example
	//! when the container needs to expand its capacity), it performs `memcpy`, `memmove` or any other bitwise copy algorithms 
	//! directly on the data without any move constructors and destructors being called. If one object cannot be trivially relocated,
	//! then the relocating operation will call the move constructor for the object on the new memory (passing the old object as rvalue 
	//! reference), then call the destructor for the old object to properly destruct itself before freeing the memory for the old object.
	//! 
	//! All objects are trivially relocatable unless the user explicitly creates a template specialization that evaluates this to 
	//! `false_type` for the object that cannot be trivially relocated. In fact, the case that one type cannot be trivially relocatable
	//! is very rare, this only happens if the type holds a pointer to `this` and needs to update the pointer if the object is relocated.
	template <typename _Ty>
	struct is_trivially_relocatable : integral_constant<bool, true> {};

	template <typename _Ty>
	constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<_Ty>::value;

	// Introduced in C++17, but is very useful.

	using std::conjunction;
	using std::disjunction;
	using std::negation;

	template<typename... _B>
	constexpr bool conjunction_v = conjunction<_B...>::value;
	template<typename... _B>
	constexpr bool disjunction_v = disjunction<_B...>::value;
	template<typename _B>
	constexpr bool negation_v = negation<_B>::value;

	// Define forward and move.

	template <typename _Ty>
	constexpr _Ty&& forward(typename remove_reference<_Ty>::type& x)
	{
		return static_cast<_Ty&&>(x);
	}

	template <typename _Ty>
	constexpr _Ty&& forward(typename remove_reference<_Ty>::type&& x)
	{
		static_assert(!is_lvalue_reference<_Ty>::value, "forward _Ty isn't lvalue reference");
		return static_cast<_Ty&&>(x);
	}

	template <typename _Ty>
	constexpr typename remove_reference<_Ty>::type&& move(_Ty&& x)
	{
		return ((typename remove_reference<_Ty>::type&&)x);
	}

	//------------------------------------------------------------------------------------------------------
	//  Miscellaneous
	//------------------------------------------------------------------------------------------------------
	
	//! Represents a Globally Unique Identifier.
	//! @details `Guid` is a shortcut for Globally Unique Identifier. It is a 128-bit unsigned integer that is usually randomly
	//! generated and is used to identify one instance across multiple devices or domains.
	//! 
	//! `Guid` can be constructed from a literal string that is formatted in its canonical textual representation, which 
	//! appears like "xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx". One open branket and one close branket may be added to the 
	//! string, so both "123e4567-e89b-12d3-a456-426614174000" and "{123e4567-e89b-12d3-a456-426614174000}" are valid 
	//! canonical textural representations of one `Guid`.
	//! 
	//! See https://en.wikipedia.org/wiki/Universally_unique_identifier for details about GUID and its canonical textual 
	//! representation.
	struct Guid
	{
	public:
		//! The high 64-bits of the GUID.
		u64 high;
		//! The low 64-bits of the GUID.
		u64 low;
	private:
		//! Decodes one hex char to corresponding number. 
		//! @param[in] x The hex char to decode. The char must be in `[0-9]|[a-f]|[A-F]` range.
		//! @return Returns the decoded number in range [0, 16).
		//! Returns 0 if the provided char is not a hex char.
		static constexpr u8 atohex(c8 x)
		{
			return x >= '0' && x <= '9' ? x - '0' :
				x >= 'a' && x <= 'f' ? x - 'a' + 10 :
				x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
		}
		constexpr u64 parse_high(const c8* s)
		{
			s = (*s == '{') ? s + 1 : s;
			return (u64)(atohex(s[0])) << 60 |
				(u64)(atohex(s[1])) << 56 |
				(u64)(atohex(s[2])) << 52 |
				(u64)(atohex(s[3])) << 48 |
				(u64)(atohex(s[4])) << 44 |
				(u64)(atohex(s[5])) << 40 |
				(u64)(atohex(s[6])) << 36 |
				(u64)(atohex(s[7])) << 32 |
				(u64)(atohex(s[9])) << 28 |
				(u64)(atohex(s[10])) << 24 |
				(u64)(atohex(s[11])) << 20 |
				(u64)(atohex(s[12])) << 16 |
				(u64)(atohex(s[14])) << 12 |
				(u64)(atohex(s[15])) << 8 |
				(u64)(atohex(s[16])) << 4 |
				(u64)(atohex(s[17]));
		}

		constexpr u64 parse_low(const c8* s)
		{
			s = (*s == '{') ? s + 1 : s;
			return (u64)(atohex(s[19])) << 60 |
				(u64)(atohex(s[20])) << 56 |
				(u64)(atohex(s[21])) << 52 |
				(u64)(atohex(s[22])) << 48 |
				(u64)(atohex(s[24])) << 44 |
				(u64)(atohex(s[25])) << 40 |
				(u64)(atohex(s[26])) << 36 |
				(u64)(atohex(s[27])) << 32 |
				(u64)(atohex(s[28])) << 28 |
				(u64)(atohex(s[29])) << 24 |
				(u64)(atohex(s[30])) << 20 |
				(u64)(atohex(s[31])) << 16 |
				(u64)(atohex(s[32])) << 12 |
				(u64)(atohex(s[33])) << 8 |
				(u64)(atohex(s[34])) << 4 |
				(u64)(atohex(s[35]));
		}
	public:
		constexpr Guid() : high(0), low(0) {}
		constexpr Guid(u64 h, u64 l) : high(h), low(l) {}
		constexpr Guid(const c8* s) :
			high(parse_high(s)), low(parse_low(s)) {}
		constexpr Guid(const Guid& rhs) : high(rhs.high), low(rhs.low) {}

		constexpr bool operator==(const Guid& rhs) const
		{
			return low == rhs.low && high == rhs.high;
		}
		constexpr bool operator!=(const Guid& rhs) const
		{
			return low != rhs.low || high != rhs.high;
		}
		constexpr bool operator<(const Guid& rhs) const
		{
			return (high != rhs.high) ? (high < rhs.high) : (low < rhs.low);
		}
		constexpr Guid& operator&=(const Guid& rhs)
		{
			low &= rhs.low;
			high &= rhs.high;
			return *this;
		}
		constexpr Guid& operator|=(const Guid& rhs)
		{
			low |= rhs.low;
			high |= rhs.high;
			return *this;
		}
		constexpr Guid& operator^=(const Guid& rhs)
		{
			low ^= rhs.low;
			high ^= rhs.high;
			return *this;
		}
	};

	constexpr Guid operator&(const Guid& lhs, const Guid& rhs)
	{
		return Guid(lhs.high & rhs.high, lhs.low & rhs.low);
	}

	constexpr Guid operator|(const Guid& lhs, const Guid& rhs)
	{
		return Guid(lhs.high | rhs.high, lhs.low | rhs.low);
	}

	constexpr Guid operator^(const Guid& lhs, const Guid& rhs)
	{
		return Guid(lhs.high ^ rhs.high, lhs.low ^ rhs.low);
	}

	static_assert(sizeof(Guid) == 16, "Wrong Guid size");

	struct Version
	{
		u32 major;
		u32 minor;
		u32 patch;

		Version() = default;
		constexpr Version(u32 major, u32 minor, u32 patch) :
			major(major), minor(minor), patch(patch) {}
		Version(const Version&) = default;
		Version(Version&&) = default;
		Version& operator=(const Version&) = default;
		Version& operator=(Version&&) = default;
		constexpr bool operator==(const Version& rhs) const
		{
			return major == rhs.major && minor == rhs.minor && patch == rhs.patch;
		}
		constexpr bool operator!=(const Version& rhs) const
		{
			return !(*this == rhs);
		}
		constexpr bool operator>(const Version& rhs) const
		{
			return (major > rhs.major) || (major == rhs.major && minor > rhs.minor) || (major == rhs.major && minor == rhs.minor && patch > rhs.patch);
		}
		constexpr bool operator<(const Version& rhs) const
		{
			return (major < rhs.major) || (major == rhs.major && minor < rhs.minor) || (major == rhs.major && minor == rhs.minor && patch < rhs.patch);
		}
		constexpr bool operator<=(const Version& rhs) const
		{
			return !(*this > rhs);
		}
		constexpr bool operator>=(const Version& rhs) const
		{
			return !(*this < rhs);
		}
	};

	// Grants bitwise operations for all enum class types. The operations may be invalid for non-bitwise enum class types,
	// but the users will not mess up with that.

	template <typename _Ty>
	constexpr auto operator|(_Ty const left, _Ty const right) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		return static_cast<_Ty>(static_cast<underlying_type_t<_Ty>>(left) | static_cast<underlying_type_t<_Ty>>(right));
	}

	template <typename _Ty>
	constexpr auto operator|=(_Ty& left, _Ty const right) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		left = left | right;
		return left;
	}

	template <typename _Ty>
	constexpr auto operator&(_Ty const left, _Ty const right) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		return static_cast<_Ty>(static_cast<underlying_type_t<_Ty>>(left) & static_cast<underlying_type_t<_Ty>>(right));
	}

	template <typename _Ty>
	constexpr auto operator&=(_Ty& left, _Ty const right) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		left = left & right;
		return left;
	}

	template <typename _Ty>
	constexpr auto operator~(_Ty const value) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		return static_cast<_Ty>(~static_cast<underlying_type_t<_Ty>>(value));
	}

	template <typename _Ty>
	constexpr auto operator^(_Ty const left, _Ty const right) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		return static_cast<_Ty>(static_cast<underlying_type_t<_Ty>>(left) ^ static_cast<underlying_type_t<_Ty>>(right));
	}

	template <typename _Ty>
	constexpr auto operator^=(_Ty& left, _Ty const right) noexcept -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		left = left ^ right;
		return left;
	}

	//! Tests if the provided enumeration contains the specified enumeration option.
	//! @param[in] flags The flags to test.
	//! @param[in] options A combination of flags to test.
	//! @return Returns `true` if all flags in the `options` combination are set in `flags`, returns `false` otherwise.
	template <typename _Ty>
	inline constexpr auto test_flags(_Ty flags, _Ty options) -> enable_if_t<is_enum_v<_Ty>, bool>
	{
		return static_cast<underlying_type_t<_Ty>>(flags & options) == static_cast<underlying_type_t<_Ty>>(options);
	}

	//! Sets the provided enumeration options to 1.
	//! @param[in] flags The flag variable to set.
	//! @param[in] options The options to set to 1.
	template <typename _Ty>
	inline constexpr auto set_flags(_Ty& flags, _Ty options) -> enable_if_t<is_enum_v<_Ty>, void>
	{
		flags = static_cast<_Ty>(static_cast<underlying_type_t<_Ty>>(flags) | static_cast<underlying_type_t<_Ty>>(options));
	}

	//! Resets the provided enumeration options to 0.
	//! @param[in] flags The flag variable to reset.
	//! @param[in] options The options to set to 0.
	template <typename _Ty>
	inline constexpr auto reset_flags(_Ty& flags, _Ty options) -> enable_if_t<is_enum_v<_Ty>, void>
	{
		flags = static_cast<_Ty>(static_cast<underlying_type_t<_Ty>>(flags) & ~static_cast<underlying_type_t<_Ty>>(options));
	}

	//! Sets the provided enumeration options to 1 or 0 based on the value provided.
	//! @param[in] flags The original flags before set.
	//! @param[in] options The options to set to 1 or 0.
	//! @param[in] value If this is `true`, all options specified will be set to 1, else they will be set to 0.
	//! @return Returns the enumeration after all options specified being set.
	template <typename _Ty>
	inline constexpr auto set_flags(_Ty flags, _Ty options, bool value) -> enable_if_t<is_enum_v<_Ty>, _Ty>
	{
		return value ? set_flags(flags, options) : reset_flags(flags, options);
	}

	//! Stores a pair of values. 
	//! @details This struct is ABI-compatible and can be used as parameters for interface methods, whether by value, by pointer
	//! or by reference.
	template <typename _Ty1, typename _Ty2>
	struct Pair
	{
		using first_type = _Ty1;
		using second_type = _Ty2;
		// -------------------- Begin of ABI compatible part --------------------
		first_type first;
		second_type second;
		// --------------------  End of ABI compatible part  --------------------

		constexpr Pair() = default;

		constexpr Pair(const _Ty1& x, const _Ty2& y) :
			first(x), second(y) {}

		template <typename _U1, typename _U2>
		constexpr Pair(_U1&& x, _U2&& y) :
			first(forward<_U1>(x)), second(forward<_U2>(y)) {}

		template <typename _U1, typename _U2>
		constexpr Pair(const Pair<_U1, _U2>& p) :
			first(p.first), second(p.second) {}

		template <typename _U1, typename _U2>
		constexpr Pair(Pair<_U1, _U2>&& p) :
			first(move(p.first)), second(move(p.second)) {}

		Pair(const Pair&) = default;
		Pair(Pair&&) = default;
		constexpr Pair& operator=(const Pair& rhs)
		{
			first = rhs.first;
			second = rhs.second;
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr Pair& operator=(const Pair<_U1, _U2>& rhs)
		{
			first = rhs.first;
			second = rhs.second;
			return *this;
		}
		constexpr Pair& operator=(Pair&& rhs)
		{
			first = move(rhs.first);
			second = move(rhs.second);
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr Pair& operator=(Pair<_U1, _U2>&& rhs)
		{
			first = move(rhs.first);
			second = move(rhs.second);
			return *this;
		}
		constexpr void swap(Pair& rhs)
		{
			Pair tmp(move(*this));
			*this = move(rhs);
			rhs = move(tmp);
		}
	};

	template <typename _Ty1, typename _Ty2>
	bool operator==(const Pair<_Ty1, _Ty2>& lhs, const Pair<_Ty1, _Ty2>& rhs)
	{
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}

	template <typename _Ty1, typename _Ty2>
	bool operator!=(const Pair<_Ty1, _Ty2>& lhs, const Pair<_Ty1, _Ty2>& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename _Ty1, typename _Ty2>
	bool operator<(const Pair<_Ty1, _Ty2>& lhs, const Pair<_Ty1, _Ty2>& rhs)
	{
		return lhs.first < rhs.first || (!(rhs.first < lhs.first) && lhs.second < rhs.second);
	}

	template <typename _Ty1, typename _Ty2>
	bool operator>(const Pair<_Ty1, _Ty2>& lhs, const Pair<_Ty1, _Ty2>& rhs)
	{
		return rhs < lhs;
	}

	template <typename _Ty1, typename _Ty2>
	bool operator<=(const Pair<_Ty1, _Ty2>& lhs, const Pair<_Ty1, _Ty2>& rhs)
	{
		return !(rhs < lhs);
	}

	template <typename _Ty1, typename _Ty2>
	bool operator>=(const Pair<_Ty1, _Ty2>& lhs, const Pair<_Ty1, _Ty2>& rhs)
	{
		return !(lhs < rhs);
	}

	template <typename _Ty1, typename _Ty2>
	constexpr Pair<decay_t<_Ty1>, decay_t<_Ty2>> make_pair(_Ty1&& first, _Ty2&& second)
	{
		return Pair<decay_t<_Ty1>, decay_t<_Ty2>>(forward<_Ty1>(first), forward<_Ty2>(second));
	}

	//! Similar to `Pair`, but allows one element in the pair to be empty and does not have one byte dummy allocation.
	template <typename _Ty1, typename _Ty2, bool = is_empty_v<_Ty1> && !is_final_v<_Ty1>, bool = is_empty_v<_Ty2> && !is_final_v<_Ty2>>
	class OptionalPair final
	{
	private:
		_Ty1 m_first;
		_Ty2 m_second;
	public:
		using first_type = _Ty1;
		using second_type = _Ty2;

		constexpr first_type& first()
		{
			return m_first;
		}
		constexpr const first_type& first() const
		{
			return m_first;
		}
		constexpr second_type& second()
		{
			return m_second;
		}
		constexpr const second_type& second() const
		{
			return m_second;
		}
		
		constexpr OptionalPair() = default;
		constexpr OptionalPair(const _Ty1& x, const _Ty2& y) :
			m_first(x), m_second(y) {}
		template <typename _U1, typename _U2>
		constexpr OptionalPair(_U1&& x, _U2&& y) :
			m_first(forward<_U1>(x)), m_second(forward<_U2>(y)) {}

		template <typename _U1, typename _U2>
		constexpr OptionalPair(const OptionalPair<_U1, _U2>& p) :
			m_first(p.first()), m_second(p.second()) {}

		template <typename _U1, typename _U2>
		constexpr OptionalPair(OptionalPair<_U1, _U2>&& p) :
			m_first(move(p.first())), m_second(move(p.second())) {}

		OptionalPair(const OptionalPair&) = default;
		OptionalPair(OptionalPair&&) = default;
		constexpr OptionalPair& operator=(const OptionalPair& rhs)
		{
			first() = rhs.first();
			second() = rhs.second();
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr OptionalPair& operator=(const OptionalPair<_U1, _U2>& rhs)
		{
			first() = rhs.first();
			second() = rhs.second();
			return *this;
		}
		constexpr OptionalPair& operator=(OptionalPair&& rhs)
		{
			first() = move(rhs.first());
			second() = move(rhs.second());
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr OptionalPair& operator=(OptionalPair<_U1, _U2>&& rhs)
		{
			first() = move(rhs.first());
			second() = move(rhs.second());
			return *this;
		}
		constexpr void swap(OptionalPair& rhs)
		{
			OptionalPair tmp(move(*this));
			*this = move(rhs);
			rhs = move(tmp);
		}
	};
	template <typename _Ty1, typename _Ty2>
	class OptionalPair<_Ty1, _Ty2, true, false> final : private _Ty1
	{
	private:
		_Ty2 m_second;
	public:
		using first_type = _Ty1;
		using second_type = _Ty2;

		constexpr first_type& first()
		{
			return *this;
		}
		constexpr const first_type& first() const
		{
			return *this;
		}
		constexpr second_type& second()
		{
			return m_second;
		}
		constexpr const second_type& second() const
		{
			return m_second;
		}

		constexpr OptionalPair() = default;
		constexpr OptionalPair(const _Ty1 & x, const _Ty2 & y) :
			first_type(x), m_second(y) {}
		template <typename _U1, typename _U2>
		constexpr OptionalPair(_U1 && x, _U2 && y) :
			first_type(forward<_U1>(x)), m_second(forward<_U2>(y)) {}

		template <typename _U1, typename _U2>
		constexpr OptionalPair(const OptionalPair<_U1, _U2>&p) :
			first_type(p.first()), m_second(p.second()) {}

		template <typename _U1, typename _U2>
		constexpr OptionalPair(OptionalPair<_U1, _U2> && p) :
			first_type(move(p.first())), m_second(move(p.second())) {}

		OptionalPair(const OptionalPair&) = default;
		OptionalPair(OptionalPair&&) = default;
		constexpr OptionalPair& operator=(const OptionalPair& rhs)
		{
			first() = rhs.first();
			second() = rhs.second();
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr OptionalPair& operator=(const OptionalPair<_U1, _U2>&rhs)
		{
			first() = rhs.first();
			second() = rhs.second();
			return *this;
		}
		constexpr OptionalPair& operator=(OptionalPair && rhs)
		{
			first() = move(rhs.first());
			second() = move(rhs.second());
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr OptionalPair& operator=(OptionalPair<_U1, _U2> && rhs)
		{
			first() = move(rhs.first());
			second() = move(rhs.second());
			return *this;
		}
		constexpr void swap(OptionalPair & rhs)
		{
			OptionalPair tmp(move(*this));
			*this = move(rhs);
			rhs = move(tmp);
		}
	};
	template <typename _Ty1, typename _Ty2>
	class OptionalPair<_Ty1, _Ty2, false, true> final : private _Ty2
	{
	private:
		_Ty1 m_first;
	public:
		using first_type = _Ty1;
		using second_type = _Ty2;
		constexpr first_type& first()
		{
			return m_first;
		}
		constexpr const first_type& first() const
		{
			return m_first;
		}
		constexpr second_type& second()
		{
			return *this;
		}
		constexpr const second_type& second() const
		{
			return *this;
		}

		constexpr OptionalPair() = default;
		constexpr OptionalPair(const _Ty1 & x, const _Ty2 & y) :
			m_first(x), second_type(y) {}
		template <typename _U1, typename _U2>
		constexpr OptionalPair(_U1 && x, _U2 && y) :
			m_first(forward<_U1>(x)), second_type(forward<_U2>(y)) {}

		template <typename _U1, typename _U2>
		constexpr OptionalPair(const OptionalPair<_U1, _U2>&p) :
			m_first(p.first()), second_type(p.second()) {}

		template <typename _U1, typename _U2>
		constexpr OptionalPair(OptionalPair<_U1, _U2> && p) :
			m_first(move(p.first())), second_type(move(p.second())) {}

		OptionalPair(const OptionalPair&) = default;
		OptionalPair(OptionalPair&&) = default;
		constexpr OptionalPair& operator=(const OptionalPair& rhs)
		{
			first() = rhs.first();
			second() = rhs.second();
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr OptionalPair& operator=(const OptionalPair<_U1, _U2>&rhs)
		{
			first() = rhs.first();
			second() = rhs.second();
			return *this;
		}
		constexpr OptionalPair& operator=(OptionalPair && rhs)
		{
			first() = move(rhs.first());
			second() = move(rhs.second());
			return *this;
		}
		template <typename _U1, typename _U2>
		constexpr OptionalPair& operator=(OptionalPair<_U1, _U2> && rhs)
		{
			first() = move(rhs.first());
			second() = move(rhs.second());
			return *this;
		}
		constexpr void swap(OptionalPair & rhs)
		{
			OptionalPair tmp(move(*this));
			*this = move(rhs);
			rhs = move(tmp);
		}
	};

	//! @}
}
//! Prevent recognizing commas in one single macro argument.
#define LUNA_SINGLE_ARG(...) __VA_ARGS__

#ifdef LUNA_COMPILER_CPP17
#define LUNA_IF_CONSTEXPR if constexpr
#else
#define LUNA_IF_CONSTEXPR if
#endif