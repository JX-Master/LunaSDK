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
	constexpr f32 PI = 3.141592654f;
	constexpr f32 TWO_PI = 6.283185307f;
	constexpr f32 ONE_DIV_PI = 0.318309886f;
	constexpr f32 ONE_DIV_TWO_PI = 0.159154943f;
	constexpr f32 PI_DIV_TWO = 1.570796327f;
	constexpr f32 PI_DIV_FOUR = 0.785398163f;

	constexpr f32 F32_INFINITY = (f32)HUGE_VALF;
	constexpr f64 F64_INFINITY = (f64)HUGE_VAL;
	constexpr f32 F32_NAN = (f32)NAN;
	constexpr f64 F64_NAN = F64_INFINITY * 0.0;

	//! Convert degree angle to radius angle.
	inline constexpr f32 deg_to_rad(f32 degrees)
	{
		return degrees * (PI / 180.0f);
	}
	//! Convert radius angle to degree angle.
	inline constexpr f32 rad_to_deg(f32 radians)
	{
		return radians * (180.0f / PI);
	}

	//! Check whether the provided number is power of two.
	template <typename _Ty>
	constexpr bool is_pow_of_two(_Ty n)
	{
		return (((n)&((n)-1)) == 0 && (n) != 0);
	}

	//! Rect is used to specify a region in the bitmap, texture or window.
	template <typename _Ty>
	struct Rect
	{
		_Ty offset_x;
		_Ty offset_y;
		_Ty width;
		_Ty height;

		Rect() = default;
		Rect(_Ty offset_x, _Ty offset_y, _Ty width, _Ty height) :
			offset_x(offset_x),
			offset_y(offset_y),
			width(width),
			height(height) {}

		bool operator==(const Rect& rhs) const
		{
			return ((offset_x == rhs.offset_x) &&
				(offset_y == rhs.offset_y) &&
				(width == rhs.width) &&
				(height == rhs.height));
		}
		bool operator!= (const Rect& rhs) const
		{
			return !(*this == rhs);
		}
	};

	template <typename _Ty>
	struct OffsetRect
	{
		_Ty left;
		_Ty top;
		_Ty right;
		_Ty bottom;

		OffsetRect() = default;
		OffsetRect(_Ty left, _Ty top, _Ty right, _Ty bottom) :
			left(left),
			top(top),
			right(right),
			bottom(bottom) {}
		bool operator==(const OffsetRect& rhs) const
		{
			return ((left == rhs.left) &&
				(top == rhs.top) &&
				(right == rhs.right) &&
				(bottom == rhs.bottom));
		}
		bool operator!= (const OffsetRect& rhs) const
		{
			return !(*this == rhs);
		}
	};

	using RectI = Rect<i32>;
	using RectU = Rect<u32>;
	using RectF = Rect<f32>;
	using OffsetRectI = OffsetRect<i32>;
	using OffsetRectU = OffsetRect<u32>;
	using OffsetRectF = OffsetRect<f32>;

	template <typename _Ty>
	struct Box
	{
		_Ty offset_x;
		_Ty offset_y;
		_Ty offset_z;
		_Ty width;		// The size in x-axis
		_Ty height;		// The size in y-axis
		_Ty depth;		// The size in z-axis

		Box() = default;
		Box(_Ty offset_x, _Ty offset_y, _Ty offset_z, _Ty width, _Ty height, _Ty depth) :
			offset_x(offset_x),
			offset_y(offset_y),
			offset_z(offset_z),
			width(width),
			height(height),
			depth(depth) {}
		bool operator==(const Box& rhs) const
		{
			return (offset_x == rhs.offset_x) &&
				(offset_y == rhs.offset_y) &&
				(offset_z == rhs.offset_z) &&
				(width == rhs.width) &&
				(height == rhs.height) &&
				(depth == rhs.depth);
		}
		bool operator!=(const Box& rhs) const
		{
			return !(*this == rhs);
		}
	};

	template <typename _Ty>
	struct OffsetBox
	{
		_Ty left;
		_Ty top;
		_Ty front;
		_Ty right;
		_Ty bottom;
		_Ty back;

		OffsetBox() = default;
		OffsetBox(_Ty left, _Ty top, _Ty front, _Ty right, _Ty bottom, _Ty back) :
			left(left),
			top(top),
			front(front),
			right(right),
			bottom(bottom),
			back(back) {}
		bool operator==(const OffsetBox& rhs) const
		{
			return (left == rhs.left) &&
				(top == rhs.top) &&
				(front == rhs.front) &&
				(right == rhs.right) &&
				(bottom == rhs.bottom) &&
				(back == rhs.back);
		}
		bool operator!=(const OffsetBox& rhs) const
		{
			return !(*this == rhs);
		}
	};

	using BoxI = Box<i32>;
	using BoxU = Box<u32>;
	using BoxF = Box<f32>;
	using OffsetBoxI = OffsetBox<i32>;
	using OffsetBoxU = OffsetBox<u32>;
	using OffsetBoxF = OffsetBox<f32>;

	inline f32 lerp(f32 f1, f32 f2, f32 t)
	{
		return f1 + t * (f2 - f1);
	}

	inline f32 smoothstep(f32 f1, f32 f2, f32 t)
	{
		t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
		t = t * t * (3.f - 2.f * t);
		return lerp(f1, f2, t);
	}

	inline f32 clamp(f32 v, f32 min_v, f32 max_v)
	{
		v = v > min_v ? v : min_v;
		v = v < max_v ? v : max_v;
		return v;
	}

	template <typename _Ty>
	struct Fraction
	{
		_Ty numerator;
		_Ty denominator;

		Fraction() = default;
		Fraction(_Ty numerator, _Ty denominator) :
			numerator(numerator),
			denominator(denominator) {}
		bool operator==(const Fraction& rhs) const
		{
			return (numerator == rhs.numerator) &&
				(denominator == rhs.denominator);
		}
		bool operator!=(const Fraction& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! Calculates the Greatest Common Divisor of two numbers.
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