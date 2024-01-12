/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MathBase.hpp
* @author JXMaster
* @date 2018/10/26
* @brief Base math operation defines.
 */
#pragma once
#include "../Base.hpp"
#include <cmath>

#ifdef LUNA_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeMath Math library
	//! @}

	//! @addtogroup RuntimeMath
	//! @{
	
	//! The constant value `pi`.
	constexpr f32 PI = 3.141592654f;
	//! The constant value `pi * 2.0`.
	constexpr f32 TWO_PI = 6.283185307f;
	//! The constant value `1.0 / pi`.
	constexpr f32 ONE_DIV_PI = 0.318309886f;
	//! The constant value `0.5 / pi`.
	constexpr f32 ONE_DIV_TWO_PI = 0.159154943f;
	//! The constant value `pi / 2.0`.
	constexpr f32 PI_DIV_TWO = 1.570796327f;
	//! The constant value `pi / 4.0`.
	constexpr f32 PI_DIV_FOUR = 0.785398163f;
	//! The INF value for f32 type.
	constexpr f32 F32_INFINITY = (f32)HUGE_VALF;
	//! The INF value for f64 type.
	constexpr f64 F64_INFINITY = (f64)HUGE_VAL;
	//! The NaN value for f32 type.
	constexpr f32 F32_NAN = std::numeric_limits<f32>::quiet_NaN();
	//! The NaN value for f64 type.
	constexpr f64 F64_NAN = std::numeric_limits<f64>::quiet_NaN();

	//! Converts degree angle to radian angle.
	//! @param[in] degrees The degree angle to convert.
	//! @return Returns the angle converted to radian angle.
	inline constexpr f32 deg_to_rad(f32 degrees)
	{
		return degrees * (PI / 180.0f);
	}
	//! Converts radian angle to degree angle.
	//! @param[in] radians The radian angle to convert.
	//! @return Returns the angle converted to degree angle.
	inline constexpr f32 rad_to_deg(f32 radians)
	{
		return radians * (180.0f / PI);
	}

	//! Check whether the provided number is power of two.
	//! @tparam _Ty The value type.
	//! @param[in] n The value to check.
	//! @return Returns `ture` if the value is power of two. Returns `false` otherwise.
	template <typename _Ty>
	constexpr bool is_pow_of_two(_Ty n)
	{
		return (((n)&((n)-1)) == 0 && (n) != 0);
	}

	//! Used to specify a 2D rectangle region using position and size.
	//! @tparam _Ty The type that represents values of the rectangle.
	template <typename _Ty>
	struct Rect
	{
		//! The X position of the rectangle.
		_Ty offset_x;
		//! The Y position of the rectangle.
		_Ty offset_y;
		//! The width of the rectangle.
		_Ty width;
		//! The height of the rectangle.
		_Ty height;

		//! Constructs one rectangle. Values of the rectangle is uninitialized.
		Rect() = default;
		//! Constructs one rectangle.
		//! @param[in] offset_x The X position of the rectangle.
		//! @param[in] offset_y The Y position of the rectangle.
		//! @param[in] width The width of the rectangle.
		//! @param[in] height The height of the rectangle.
		Rect(_Ty offset_x, _Ty offset_y, _Ty width, _Ty height) :
			offset_x(offset_x),
			offset_y(offset_y),
			width(width),
			height(height) {}
		//! Compares two rectangles for equality.
		//! @param[in] rhs The rectangle to compare with.
		//! @return Returns `true` if two rectangles are equal. Returns `false` otherwise.
		bool operator==(const Rect& rhs) const
		{
			return ((offset_x == rhs.offset_x) &&
				(offset_y == rhs.offset_y) &&
				(width == rhs.width) &&
				(height == rhs.height));
		}
		//! Compares two rectangles for non-equality.
		//! @param[in] rhs The rectangle to compare with.
		//! @return Returns `true` if two rectangles are not equal. Returns `false` otherwise.
		bool operator!= (const Rect& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! Used to specify a 2D rectangle region using offsets with its container rectangle.
	//! @tparam _Ty The type that represents values of the rectangle.
	template <typename _Ty>
	struct OffsetRect
	{
		//! The left offset of the rectangle from its container.
		_Ty left;
		//! The top offset of the rectangle from its container.
		_Ty top;
		//! The right offset of the rectangle from its container.
		_Ty right;
		//! The bottom offset of the rectangle from its container.
		_Ty bottom;

		//! Constructs one rectangle. Values of the rectangle is uninitialized.
		OffsetRect() = default;
		//! Constructs one rectangle.
		//! @param[in] left The left offset of the rectangle from its container.
		//! @param[in] top The top offset of the rectangle from its container.
		//! @param[in] right The right offset of the rectangle from its container.
		//! @param[in] bottom The bottom offset of the rectangle from its container.
		OffsetRect(_Ty left, _Ty top, _Ty right, _Ty bottom) :
			left(left),
			top(top),
			right(right),
			bottom(bottom) {}
		//! Compares two rectangles for equality.
		//! @param[in] rhs The rectangle to compare with.
		//! @return Returns `true` if two rectangles are equal. Returns `false` otherwise.
		bool operator==(const OffsetRect& rhs) const
		{
			return ((left == rhs.left) &&
				(top == rhs.top) &&
				(right == rhs.right) &&
				(bottom == rhs.bottom));
		}
		//! Compares two rectangles for non-equality.
		//! @param[in] rhs The rectangle to compare with.
		//! @return Returns `true` if two rectangles are not equal. Returns `false` otherwise.
		bool operator!= (const OffsetRect& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! A instanced type of @ref Rect that uses @ref i32 as value type.
	using RectI = Rect<i32>;
	//! A instanced type of @ref Rect that uses @ref u32 as value type.
	using RectU = Rect<u32>;
	//! A instanced type of @ref Rect that uses @ref f32 as value type.
	using RectF = Rect<f32>;
	//! A instanced type of @ref OffsetRect that uses @ref i32 as value type.
	using OffsetRectI = OffsetRect<i32>;
	//! A instanced type of @ref OffsetRect that uses @ref u32 as value type.
	using OffsetRectU = OffsetRect<u32>;
	//! A instanced type of @ref OffsetRect that uses @ref f32 as value type.
	using OffsetRectF = OffsetRect<f32>;

	//! Used to specify a 3D box region using position and size.
	//! @tparam _Ty The type that represents values of the box.
	template <typename _Ty>
	struct Box
	{
		//! The X position of the box.
		_Ty offset_x;
		//! The Y position of the box.
		_Ty offset_y;
		//! The Z position of the box.
		_Ty offset_z;
		//！The width of the box.
		_Ty width;
		//! The height of the box.
		_Ty height;
		//! The depth of the box.
		_Ty depth;

		//! Constructs one box. Values of the box is uninitialized.
		Box() = default;
		//! Constructs one box.
		//! @param[in] offset_x The X position of the box.
		//! @param[in] offset_y The Y position of the box.
		//! @param[in] offset_z The Z position of the box.
		//! @param[in] width The width of the box.
		//! @param[in] height The height of the box.
		//! @param[in] depth The depth of the box.
		Box(_Ty offset_x, _Ty offset_y, _Ty offset_z, _Ty width, _Ty height, _Ty depth) :
			offset_x(offset_x),
			offset_y(offset_y),
			offset_z(offset_z),
			width(width),
			height(height),
			depth(depth) {}
		//! Compares two boxes for equality.
		//! @param[in] rhs The box to compare with.
		//! @return Returns `true` if two boxes are equal. Returns `false` otherwise.
		bool operator==(const Box& rhs) const
		{
			return (offset_x == rhs.offset_x) &&
				(offset_y == rhs.offset_y) &&
				(offset_z == rhs.offset_z) &&
				(width == rhs.width) &&
				(height == rhs.height) &&
				(depth == rhs.depth);
		}
		//! Compares two boxes for non-equality.
		//! @param[in] rhs The box to compare with.
		//! @return Returns `true` if two boxes are not equal. Returns `false` otherwise.
		bool operator!=(const Box& rhs) const
		{
			return !(*this == rhs);
		}
	};
	
	//! Used to specify a 3D box region using offsets with its container box.
	//! @tparam _Ty The type that represents values of the box.
	template <typename _Ty>
	struct OffsetBox
	{
		//! The left offset of the rectangle from its container.
		_Ty left;
		//! The top offset of the rectangle from its container.
		_Ty top;
		//! The front offset of the rectangle from its container.
		_Ty front;
		//! The right offset of the rectangle from its container.
		_Ty right;
		//! The bottom offset of the rectangle from its container.
		_Ty bottom;
		//! The back offset of the rectangle from its container.
		_Ty back;

		//! Constructs one box. Values of the box is uninitialized.
		OffsetBox() = default;
		//! Constructs one box.
		//! @param[in] left The left offset of the rectangle from its container.
		//! @param[in] top The top offset of the rectangle from its container.
		//! @param[in] front The front offset of the rectangle from its container.
		//! @param[in] right The right offset of the rectangle from its container.
		//! @param[in] bottom The bottom offset of the rectangle from its container.
		//! @param[in] back The back offset of the rectangle from its container.
		OffsetBox(_Ty left, _Ty top, _Ty front, _Ty right, _Ty bottom, _Ty back) :
			left(left),
			top(top),
			front(front),
			right(right),
			bottom(bottom),
			back(back) {}
		//! Compares two boxes for equality.
		//! @param[in] rhs The box to compare with.
		//! @return Returns `true` if two boxes are equal. Returns `false` otherwise.
		bool operator==(const OffsetBox& rhs) const
		{
			return (left == rhs.left) &&
				(top == rhs.top) &&
				(front == rhs.front) &&
				(right == rhs.right) &&
				(bottom == rhs.bottom) &&
				(back == rhs.back);
		}
		//! Compares two boxes for non-equality.
		//! @param[in] rhs The box to compare with.
		//! @return Returns `true` if two boxes are not equal. Returns `false` otherwise.
		bool operator!=(const OffsetBox& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! A instanced type of @ref Box that uses @ref i32 as value type.
	using BoxI = Box<i32>;
	//! A instanced type of @ref Box that uses @ref u32 as value type.
	using BoxU = Box<u32>;
	//! A instanced type of @ref Box that uses @ref f32 as value type.
	using BoxF = Box<f32>;
	//! A instanced type of @ref OffsetBox that uses @ref i32 as value type.
	using OffsetBoxI = OffsetBox<i32>;
	//! A instanced type of @ref OffsetBox that uses @ref u32 as value type.
	using OffsetBoxU = OffsetBox<u32>;
	//! A instanced type of @ref OffsetBox that uses @ref f32 as value type.
	using OffsetBoxF = OffsetBox<f32>;

	//! Performs linear interpolation on the given values.
	//! @tparam _VTy The type of the interpolated value.
	//! @tparam _TTy The type of the weight value.
	//! @param[in] f1 The first value to interpolate.
	//! @param[in] f2 The second value to interpolate.
	//! @param[in] t The interpolation weight to use.
	//! @return Returns the interpolated value. The value is calculated as `f1 * (1 - t) + f2 * t`.
	template <typename _VTy, typename _TTy>
	inline _VTy lerp(_VTy f1, _VTy f2, _TTy t)
	{
		return f1 + t * (f2 - f1);
	}
	
	//! Performs smoothstep interpolation on the given values.
	//! @param[in] f1 The first value to interpolate.
	//! @param[in] f2 The second value to interpolate.
	//! @param[in] t The interpolation weight to use. The value is clamped to [0, 1] before use.
	//! @return Returns the interpolated value.
	inline f32 smoothstep(f32 f1, f32 f2, f32 t)
	{
		t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
		t = t * t * (3.f - 2.f * t);
		return lerp(f1, f2, t);
	}

	//! Clamps the value to the specified range.
	//! @tparam _Ty1 The type of the value to clamp.
	//! @tparam _Ty2 The type of the low clamp threshold.
	//! @tparam _Ty3 The type of the high clamp threshold.
	//! @param[in] v The value to clamp.
	//! @param[in] min_v The low clamp threshold.
	//! @param[in] max_v The high clamp threshold.
	//! @return Returns the value clamped to [`min_v`, `max_v`].
	template <typename _Ty1, typename _Ty2, typename _Ty3>
	inline _Ty1 clamp(_Ty1 v, _Ty2 min_v, _Ty3 max_v)
	{
		v = v > (_Ty1)min_v ? v : (_Ty1)min_v;
		v = v < (_Ty1)max_v ? v : (_Ty1)max_v;
		return v;
	}

	//! Calculates the Greatest Common Divisor of two numbers.
	//! @tparam[in] _Ty The number type.
	//! @param[in] a The first number.
	//! @param[in] b The second number.
	//! @return Returns the Greatest Common Divisor of two numbers.
	template <typename _Ty>
	inline constexpr _Ty gcd(_Ty a, _Ty b)
	{
		// make sure a <= b.
		if (a > b)
		{
			auto t = a;
			a = b;
			b = t;
		}
		auto i = a;
		while (i)
		{
			if ((a % i == 0) && (b % i == 0))
			{
				return i;
			}
			--i;
		}
		return 1;
	}

	using std::isinf;
	using std::isnan;

	//! @}
}

// Luna math library uses SIMD (Single-Instruction-Multiple-Data) for accelerating vector math compuations when possible. See
// https://en.wikipedia.org/wiki/Single_instruction,_multiple_data for details about SIMD.
// The user may disable SIMD by defining `LUNA_DISABLE_SIMD`.
// 
// SIMD is implemented by SSE and AVX extensions on x86/x86-64 platforms, and Arm Neon extensions on ARM/ARM64 platforms, consult
// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html or
// https://developer.arm.com/architectures/instruction-sets/intrinsics/ for how to programming using SIMD.
// 
// The following macros can be used to test whether one SIMD extensions can be used in the target platform:
// * LUNA_SSE2_INTRINSICS
// * LUNA_SSE3_INTRINSICS
// * LUNA_SSE4_INTRINSICS
// * LUNA_AVX_INTRINSICS
// * LUNA_AVX2_INTRINSICS
// * LUNA_FMA3_INTRINSICS
// * LUNA_SVML_INTRINSICS
// * LUNA_NEON_INTRINSICS

#ifndef LUNA_DISABLE_SIMD

#define LUNA_SIMD

#if defined(LUNA_PLATFORM_X86) || defined(LUNA_PLATFORM_X86_64)
// SSE/SSE2 should work for all x86 and x86-64 platforms, if SIMD is not disabled. 
// The first Intel CPU that supports SSE2 is Pentium 4, shipped at Dec. 2000.
// The first AMD CPU that supports SSE2 is Opteron, shipped at Apr. 2003.
// If your target is x86 and does not support SSE, define `LUNA_DISABLE_SIMD` manually.
#define LUNA_SSE2_INTRINSICS
#include <xmmintrin.h>
#include <emmintrin.h>

#if defined(LUNA_COMPILER_GCC) || defined(LUNA_COMPILER_CLANG)
// On GCC/Clang we can use the following macros to test for additional intrinsics.
#if defined(__SSE3__) && !defined(LUNA_SSE3_INTRINSICS)
#define LUNA_SSE3_INTRINSICS
#endif
#if defined(__SSE4_1__) && !defined(LUNA_SSE4_INTRINSICS)
#define LUNA_SSE4_INTRINSICS
#endif
#if defined(__AVX__) && !defined(LUNA_AVX_INTRINSICS)
#define LUNA_AVX_INTRINSICS
#endif
#if defined(__AVX2__) && !defined(LUNA_AVX2_INTRINSICS)
#define LUNA_AVX2_INTRINSICS
#endif
#elif defined(LUNA_COMPILER_MSVC)
// It seems that at MSVC side, all intrinsics are defined by Visual Studio so that we can use them directly.
#define LUNA_SSE3_INTRINSICS
#define LUNA_SSE4_INTRINSICS
#define LUNA_AVX_INTRINSICS
#define LUNA_AVX2_INTRINSICS
#endif

#if defined(LUNA_AVX2_INTRINSICS) && !defined(LUNA_FMA3_INTRINSICS)
#define LUNA_FMA3_INTRINSICS
#endif

// MSVC supports SVML since 1920.
#if defined(LUNA_COMPILER_MSVC) && (LUNA_COMPILER_VERSION >= 1920)
#define LUNA_SVML_INTRINSICS
#endif

#if defined(LUNA_COMPILER_MSVC)
// For MSVC we can simply include intrin.h to get all intrinsics.
#include <intrin.h>
#elif defined(LUNA_COMPILER_GCC) || defined(LUNA_COMPILER_CLANG)
// For GCC/Clang, we need to include header files manually.
#ifdef LUNA_SSE3_INTRINSICS
#include <pmmintrin.h>
#endif
#ifdef LUNA_SSE4_INTRINSICS
#include <smmintrin.h>
#endif
#ifdef LUNA_AVX_INTRINSICS
#include <immintrin.h> // AVX2 and FMA3 intrinsics also included, if present.
#endif
#endif

#elif defined(LUNA_PLATFORM_ARM32) || defined(LUNA_PLATFORM_ARM64)
// Use Neon intrinsics.
#define LUNA_NEON_INTRINSICS
#include <arm_neon.h>
#endif

#ifdef LUNA_COMPILER_MSVC
#define LUNA_SIMD_CALL __vectorcall
#else
#define LUNA_SIMD_CALL 
#endif

#endif