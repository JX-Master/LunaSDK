/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Simd.hpp
* @author JXMaster
* @date 2022/3/19
 */
#pragma once
#include "Math.hpp"

// The whole SIMD library is available only when SIMD is enabled. No non-SIMD fallbacks. Use SIMD instructions like this:
// ```
// #ifdef LUNA_SIMD
//		using namespace Simd;
//		(SIMD codes)
// #else
//		non-SIMD implementations.
// #endif
//	```
// Note that the use of SIMD library is totally optional. The library exists from the fact that some operations are rapidly used, so 
// we can write them into library rather than copy/paste code everywhere.
// 
// For every SIMD function, we use pseudo-code to represent the function operation. The pseudo-code will use bits-form or components-form
// to represent elements or bits in one vector or scalar number. For any given variable `a`:
// Bits-form: a[M:N] represents the bits of `a` from M to N, includes both ends.
// Components-form: a.x, a.y, a.z, a.w represents the first to the forth packed elements of one vector. 
// For 128-bits vector with 4 32-bits packed elements:
// a.x equals to a[0:31]
// a.y equals to a[32:63]
// a.z equals to a[64:95]
// a.w equals to a[96:127]

#ifdef LUNA_SIMD
namespace Luna
{
	namespace Simd
	{
#if defined(LUNA_SSE2_INTRINSICS)
		using float4 = __m128;		// 4 packed 32-bit single precision float-point number.
		using int4 = __m128i;		// 4 packed 32-bit signed integer number.
#elif defined(LUNA_NEON_INTRINSICS)
		using float4 = float32x4_t;
		using int4 = int32x4_t;
#else
#error "Unknown intrinsics"
#endif
		//! Represents one SIMD matrix type with 12 (3x4) packed single-precision (32-bit) floating-point elements.
		//! Based on the implementation, the matrix may be stored using two 256-bits registers (AVX), or four 
		//! 128-bits registers (SSEn / Neon).
		struct float3x4
		{
#if defined(LUNA_AVX_INTRINSICS)
			__m256 r[2];	// ymm and xmm are actually the same register, so we use __m256 directly and convert to __m128 when needed.
#else
			float4 r[3];	// saves one register compared to float4x4.
#endif
		};

		//! Represents one SIMD matrix type with 16 (4x4) packed single-precision (32-bit) floating-point elements.
		//! Based on the implementation, the matrix may be stored using two 256-bits registers (AVX), or four 
		//! 128-bits registers (SSEn / Neon).
		struct float4x4
		{
#if defined(LUNA_AVX_INTRINSICS)
			__m256 r[2];
#else
			float4 r[4];
#endif
		};

		//! Reinterprets vector of type `int4` to type `float4` without changing the data of the vector.
		float4 LUNA_SIMD_CALL casti_f4(int4 a);
		//! Reinterprets vector of type `float4` to type `int4` without changing the data of the vector.
		int4 LUNA_SIMD_CALL castf_i4(float4 a);
		//! Loads 64-bits (composed of 2 packed single-precision (32-bit) floating-point elements) from memory into 
		//! the first two elements of `dst`.
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! dst.x := MEM[mem_addr:mem_addr+31]
		//! dst.y := MEM[mem_addr+32:mem_addr+63]
		//! dst.z := 0
		//! dst.w := 0
		//! ```
		float4 LUNA_SIMD_CALL load_f2(f32 const* mem_addr);
		//! Loads 128-bits (composed of 4 packed single-precision (32-bit) floating-point elements) from memory into `dst`. 
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! dst.x := MEM[mem_addr:mem_addr+31]
		//! dst.y := MEM[mem_addr+32:mem_addr+63]
		//! dst.z := MEM[mem_addr+64:mem_addr+95]
		//! dst.w := MEM[mem_addr+96:mem_addr+127]
		//! ```
		float4 LUNA_SIMD_CALL load_f4(f32 const* mem_addr);
		//! Stores the lower 2 single-precision (32-bit) floating-point elements from `a` into memory. 
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! MEM[mem_addr:mem_addr+31] := a.x
		//! MEM[mem_addr+32:mem_addr+63] := a.y
		//! ```
		void LUNA_SIMD_CALL store_f2(f32* mem_addr, float4 a);
		//! Stores 128-bits (composed of 4 packed single-precision (32-bit) floating-point elements) from `a` into memory. 
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! MEM[mem_addr:mem_addr+31] := a.x
		//! MEM[mem_addr+32:mem_addr+63] := a.y
		//! MEM[mem_addr+64:mem_addr+95] := a.z
		//! MEM[mem_addr+96:mem_addr+127] := a.w
		//! ```
		void LUNA_SIMD_CALL store_f4(f32* mem_addr, float4 a);
		//! Sets packed single-precision (32-bit) floating-point elements in `dst` with the supplied values.
		//! ```
		//! dst.x := e0
		//! dst.y := e1
		//! dst.z := e2
		//! dst.w := e3
		//! ```
		float4 LUNA_SIMD_CALL set_f4(f32 e0, f32 e1, f32 e2, f32 e3);
		//! Sets packed 32-bit integers in `dst` with the supplied values.
		//! ```
		//! dst.x := e0
		//! dst.y := e1
		//! dst.z := e2
		//! dst.w := e3
		//! ```
		int4 LUNA_SIMD_CALL set_i4(i32 e0, i32 e1, i32 e2, i32 e3);
		//! Returns vector of type `float4` with all elements set to zero.
		//! ```
		//! dst.x := 0
		//! dst.y := 0
		//! dst.z := 0
		//! dst.w := 0
		//! ```
		float4 LUNA_SIMD_CALL setzero_f4();
		//! Broadcasts single-precision (32-bit) floating-point value `e0` to all elements of `dst`.
		//! ```
		//! dst.x := e0
		//! dst.y := e0
		//! dst.z := e0
		//! dst.w := e0
		//! ```
		float4 LUNA_SIMD_CALL dup_f4(f32 e0);
		//! Stores the first single-precision (32-bit) floating-point element from `a` into `dst`. 
		//! ```
		//! dst := a.x
		//! ```
		f32 LUNA_SIMD_CALL getx_f4(float4 a);
		//! Replaces the forth element of `a` with `b`, and stores the results in `dst`.
		//! ```
		//! dst.x := a.x
		//! dst.y := a.y
		//! dst.z := a.z
		//! dst.w := b
		//! ```
		float4 LUNA_SIMD_CALL setw_f4(float4 a, f32 b);
		//! Broadcasts the first element of `a` to every element of `dst`.
		//! ```
		//! dst.x := a.x
		//! dst.y := a.x
		//! dst.z := a.x
		//! dst.w := a.x
		//! ```
		float4 LUNA_SIMD_CALL dupx_f4(float4 a);
		//! Broadcasts the second element of `a` to every element of `dst`.
		//! ```
		//! dst.x := a.y
		//! dst.y := a.y
		//! dst.z := a.y
		//! dst.w := a.y
		//! ```
		float4 LUNA_SIMD_CALL dupy_f4(float4 a);
		//! Broadcasts the third element of `a` to every element of `dst`.
		//! ```
		//! dst.x := a.z
		//! dst.y := a.z
		//! dst.z := a.z
		//! dst.w := a.z
		//! ```
		float4 LUNA_SIMD_CALL dupz_f4(float4 a);
		//! Broadcasts the forth element of `a` to every element of `dst`.
		//! ```
		//! dst.x := a.w
		//! dst.y := a.w
		//! dst.z := a.w
		//! dst.w := a.w
		//! ```
		float4 LUNA_SIMD_CALL dupw_f4(float4 a);
		//! Compares packed single-precision (32-bit) floating-point elements in `a` and `b` for equality, and stores the results in `dst`.
		//! ```
		//! dst.x := (a.x == b.x) ? 0xFFFFFFFF : 0
		//! dst.y := (a.y == b.y) ? 0xFFFFFFFF : 0
		//! dst.z := (a.z == b.z) ? 0xFFFFFFFF : 0
		//! dst.w := (a.w == b.w) ? 0xFFFFFFFF : 0
		//!	```
		int4 LUNA_SIMD_CALL cmpeq_f4(float4 a, float4 b);
		//! Compares packed single-precision (32-bit) floating-point elements in `a` and `b` for not-equal, and stores the results in `dst`.
		//! ```
		//! dst.x := (a.x != b.x) ? 0xFFFFFFFF : 0
		//! dst.y := (a.y != b.y) ? 0xFFFFFFFF : 0
		//! dst.z := (a.z != b.z) ? 0xFFFFFFFF : 0
		//! dst.w := (a.w != b.w) ? 0xFFFFFFFF : 0
		//!	```
		int4 LUNA_SIMD_CALL cmpneq_f4(float4 a, float4 b);
		//! Compares packed single-precision (32-bit) floating-point elements in `a` and `b` for greater-than, and stores the results in `dst`.
		//! ```
		//! dst.x := (a.x > b.x) ? 0xFFFFFFFF : 0
		//! dst.y := (a.y > b.y) ? 0xFFFFFFFF : 0
		//! dst.z := (a.z > b.z) ? 0xFFFFFFFF : 0
		//! dst.w := (a.w > b.w) ? 0xFFFFFFFF : 0
		//!	```
		int4 LUNA_SIMD_CALL cmpgt_f4(float4 a, float4 b);
		//! Compares packed single-precision (32-bit) floating-point elements in `a` and `b` for less-than, and stores the results in `dst`.
		//! ```
		//! dst.x := (a.x < b.x) ? 0xFFFFFFFF : 0
		//! dst.y := (a.y < b.y) ? 0xFFFFFFFF : 0
		//! dst.z := (a.z < b.z) ? 0xFFFFFFFF : 0
		//! dst.w := (a.w < b.w) ? 0xFFFFFFFF : 0
		//!	```
		int4 LUNA_SIMD_CALL cmplt_f4(float4 a, float4 b);
		//! Compares packed single-precision (32-bit) floating-point elements in `a` and `b` for greater-than-or-equal, and stores the results in dst.
		//! ```
		//! dst.x := (a.x >= b.x) ? 0xFFFFFFFF : 0
		//! dst.y := (a.y >= b.y) ? 0xFFFFFFFF : 0
		//! dst.z := (a.z >= b.z) ? 0xFFFFFFFF : 0
		//! dst.w := (a.w >= b.w) ? 0xFFFFFFFF : 0
		//!	```
		int4 LUNA_SIMD_CALL cmpge_f4(float4 a, float4 b);
		//! Compares packed single-precision (32-bit) floating-point elements in `a` and `b` for less-than-or-equal, and stores the results in dst.
		//! ```
		//! dst.x := (a.x <= b.x) ? 0xFFFFFFFF : 0
		//! dst.y := (a.y <= b.y) ? 0xFFFFFFFF : 0
		//! dst.z := (a.z <= b.z) ? 0xFFFFFFFF : 0
		//! dst.w := (a.w <= b.w) ? 0xFFFFFFFF : 0
		//!	```
		int4 LUNA_SIMD_CALL cmple_f4(float4 a, float4 b);
		//!
		//! Converts the comparison result mask to one 32-bit integer.
		//! ```
		//! FOR j := 0 to 3
		//! 	i := j * 32
		//! 	IF a[i:i+31] == 0xFFFFFFFF
		//! 		dst[j] := 1
		//! 	ELSE IF a[i:i+31] == 0x00000000
		//!			dst[j] := 0
		//!		ELSE
		//!			dst[j] := 0 or 1 (The behavior is undefined.)
		//! 	FI
		//! ENDFOR
		//! dst[4:MAX] := 0
		//!	```
		i32 LUNA_SIMD_CALL maskint_i4(int4 a);
		//! Adds packed single-precision (32-bit) floating-point elements in `a` and `b`, and stores the results in `dst`.
		//! ```
		//! dst.x = a.x + b.x
		//! dst.y = a.y + b.y
		//! dst.z = a.z + b.z
		//! dst.w = a.w + b.w
		//! ```
		float4 LUNA_SIMD_CALL add_f4(float4 a, float4 b);
		//! Subtracts packed single-precision (32-bit) floating-point elements in `a` and `b`, and stores the results in `dst`.
		//! ```
		//! dst.x = a.x - b.x
		//! dst.y = a.y - b.y
		//! dst.z = a.z - b.z
		//! dst.w = a.w - b.w
		//! ```
		float4 LUNA_SIMD_CALL sub_f4(float4 a, float4 b);
		//! Multiplies packed single-precision (32-bit) floating-point elements in `a` and `b`, and stores the results in `dst`.
		//! ```
		//! dst.x = a.x * b.x
		//! dst.y = a.y * b.y
		//! dst.z = a.z * b.z
		//! dst.w = a.w * b.w
		//! ```
		float4 LUNA_SIMD_CALL mul_f4(float4 a, float4 b);
		//! Divides packed single-precision (32-bit) floating-point elements in a by packed elements in b, and stores the results in dst.
		//! ```
		//! dst.x = a.x / b.x
		//! dst.y = a.y / b.y
		//! dst.z = a.z / b.z
		//! dst.w = a.w / b.w
		//! ```
		float4 LUNA_SIMD_CALL div_f4(float4 a, float4 b);
		//! Scales packed single-precision (32-bit) floating-point elements in `a` using one single-precision (32-bit) floating-point element `b`, 
		//! and stores the results in `dst`.
		//! ```
		//! dst.x = a.x * b
		//! dst.y = a.y * b
		//! dst.z = a.z * b
		//! dst.w = a.w * b
		//! ```
		float4 LUNA_SIMD_CALL scale_f4(float4 a, f32 b);
		//! Multiply packed single-precision (32-bit) floating-point elements in `a` and `b`, add the intermediate result to packed elements in `c`, 
		//! and store the results in `dst`.
		//! ```
		//! dst.x = (a.x * b.x) + c.x
		//! dst.y = (a.y * b.y) + c.y
		//! dst.z = (a.z * b.z) + c.z
		//! dst.w = (a.w * b.w) + c.w
		//! ```
		float4 LUNA_SIMD_CALL muladd_f4(float4 a, float4 b, float4 c);
		//! Multiply packed single-precision (32-bit) floating-point elements in `a` and `b`, 
		//! add the negated intermediate result to packed elements in `c`, and store the results in dst.
		//! ```
		//! dst.x = -(a.x * b.x) + c.x
		//! dst.y = -(a.y * b.y) + c.y
		//! dst.z = -(a.z * b.z) + c.z
		//! dst.w = -(a.w * b.w) + c.w
		//! ```
		float4 LUNA_SIMD_CALL negmuladd_f4(float4 a, float4 b, float4 c);
		//! Scales packed single-precision (32-bit) floating-point elements in `a` using one single-precision (32-bit) floating-point element `b`, 
		//! add the intermediate result to packed elements in `c`, and stores the results in `dst`.
		//! ```
		//! dst.x = (a.x * b) + c.x
		//! dst.y = (a.y * b) + c.y
		//! dst.z = (a.z * b) + c.z
		//! dst.w = (a.w * b) + c.w
		//! ```
		float4 LUNA_SIMD_CALL scaleadd_f4(float4 a, f32 b, float4 c);
		//! Computes the square root of packed single-precision (32-bit) floating-point elements in `a`, and stores the results in `dst`.
		//! ```
		//! dst.x = SQRT(a.x)
		//! dst.y = SQRT(a.y)
		//! dst.z = SQRT(a.z)
		//! dst.w = SQRT(a.w)
		//! ````
		float4 LUNA_SIMD_CALL sqrt_f4(float4 a);
		//! Computes the approximate reciprocal square root of packed single-precision (32-bit) floating-point elements in `a`, and stores the results in `dst`.
		//! SSE specific: The maximum relative error for this approximation is less than 1.5*2^-12.
		//! ```
		//! dst.x = 1.0 / SQRT(a.x)
		//! dst.y = 1.0 / SQRT(a.y)
		//! dst.z = 1.0 / SQRT(a.z)
		//! dst.w = 1.0 / SQRT(a.w)
		//! ```
		float4 LUNA_SIMD_CALL rsqrtest_f4(float4 a);
		//! Computes the reciprocal square root of packed single-precision (32-bit) floating-point elements in `a`, and stores the results in `dst`.
		//! ```
		//! dst.x = 1.0 / SQRT(a.x)
		//! dst.y = 1.0 / SQRT(a.y)
		//! dst.z = 1.0 / SQRT(a.z)
		//! dst.w = 1.0 / SQRT(a.w)
		//! ```
		float4 LUNA_SIMD_CALL rsqrt_f4(float4 a);
		//! Compare packed single-precision (32-bit) floating-point elements in `a` and `b`, and store packed maximum values in `dst`.
		//! SSE specific: `dst` does not follow the IEEE Standard for Floating - Point Arithmetic(IEEE 754) maximum value when inputs are NaN or signed - zero values.
		//! ```
		//! dst.x = MAX(a.x, b.x)
		//! dst.y = MAX(a.y, b.y)
		//! dst.z = MAX(a.z, b.z)
		//! dst.w = MAX(a.w, b.w)
		//! ```
		float4 LUNA_SIMD_CALL max_f4(float4 a, float4 b);
		//! Compare packed single-precision (32-bit) floating-point elements in `a` and `b`, and store packed minimum values in `dst`.
		//! SSE specific: `dst` does not follow the IEEE Standard for Floating - Point Arithmetic(IEEE 754) minimum value when inputs are NaN or signed - zero values.
		//! ```
		//! dst.x = MIN(a.x, b.x)
		//! dst.y = MIN(a.y, b.y)
		//! dst.z = MIN(a.z, b.z)
		//! dst.w = MIN(a.w, b.w)
		//! ```
		float4 LUNA_SIMD_CALL min_f4(float4 a, float4 b);
		//! 
		//! Computes the bitwise AND of every bit in `a` and `b`, and stores the results in `dst`.
		//! ```
		//! dst.x = AND(a.x, b.x)
		//! dst.y = AND(a.y, b.y)
		//! dst.z = AND(a.z, b.z)
		//! dst.w = AND(a.w, b.w)
		//! ```
		int4 LUNA_SIMD_CALL and_i4(int4 a, int4 b);
		//! 
		//! Computes the bitwise OR of every bit in `a` and `b`, and stores the results in `dst`.
		//! ```
		//! dst.x = OR(a.x, b.x)
		//! dst.y = OR(a.y, b.y)
		//! dst.z = OR(a.z, b.z)
		//! dst.w = OR(a.w, b.w)
		//! ```
		int4 LUNA_SIMD_CALL or_i4(int4 a, int4 b);
		//! Computes the dot product on the first two elements of `a` and `b`, and stores the result in `dst`.
		//! ```
		//! dst := (a.x * b.x) + (a.y * b.y)
		//! ```
		f32 LUNA_SIMD_CALL dot2_f4(float4 a, float4 b);
		//! Computes the dot product on the first three elements of `a` and `b`, and stores the result in `dst`.
		//! ```
		//! dst := (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
		//! ```
		f32 LUNA_SIMD_CALL dot3_f4(float4 a, float4 b);
		//! Computes the dot product on elements of `a` and `b`, and stores the result in `dst`.
		//! ```
		//! dst := (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w)
		//! ```
		f32 LUNA_SIMD_CALL dot4_f4(float4 a, float4 b);
		//! Computes the dot product on the first two elements of `a` and `b`, and stores the result in each element of `dst`.
		//! ```
		//! dp := (a.x * b.x) + (a.y * b.y)
		//! dst.x := dp
		//! dst.y := dp
		//! dst.z := dp
		//! dst.w := dp
		//! ```
		float4 LUNA_SIMD_CALL dot2v_f4(float4 a, float4 b);
		//! Computes the dot product on the first three elements of `a` and `b`, and stores the result in each element of `dst`.
		//! ```
		//! dp := (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
		//! dst.x := dp
		//! dst.y := dp
		//! dst.z := dp
		//! dst.w := dp
		//! ```
		float4 LUNA_SIMD_CALL dot3v_f4(float4 a, float4 b);
		//! Computes the dot product on elements of `a` and `b`, and stores the result in each element of `dst`.
		//! ```
		//! dp := (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w)
		//! dst.x := dp
		//! dst.y := dp
		//! dst.z := dp
		//! dst.w := dp
		//! ```
		float4 LUNA_SIMD_CALL dot4v_f4(float4 a, float4 b);
		//! Computes the cross product on the first two elements of `a` and `b`, and stores the result in `dst`.
		//! ```
		//! cp := (a.x * b.y) - (a.y * b.x)
		//! dst.x := cp
		//! dst.y := cp
		//! dst.z := cp
		//! dst.w := cp
		//! ```
		float4 LUNA_SIMD_CALL cross2_f4(float4 a, float4 b);
		//! Computes the cross product on the first three elements of `a` and `b`, and stores the result in each element of `dst`.
		//! ```
		//! dst.x = a.y * b.z - a.z * b.y
		//! dst.y = a.z * b.x - a.x * b.z
		//! dst.z = a.x * b.y - a.y * b.x
		//! dst.w = 0
		//! ```
		float4 LUNA_SIMD_CALL cross3_f4(float4 a, float4 b);
		//! Computes the cross product on elements of `a`, `b` and `c`, and stores the result in each element of `dst`.
		//! ```
		//! dst.x = ((b.z * c.w - b.w * c.z) * a.y) - ((b.y * c.w - b.w * c.y) * a.z) + ((b.y * c.z - b.z * c.y) * a.w)
		//! dst.y = ((b.w * c.z - b.z * c.w) * a.x) - ((b.w * c.x - b.x * c.w) * a.z) + ((b.z * c.x - b.x * c.z) * a.w)
		//! dst.z = ((b.y * c.w - b.w * c.y) * a.x) - ((b.x * c.w - b.w * c.x) * a.y) + ((b.x * c.y - b.y * c.x) * a.w)
		//! dst.w = ((b.z * c.y - b.y * c.z) * a.x) - ((b.z * c.x - b.x * c.z) * a.y) + ((b.y * c.x - b.x * c.y) * a.z)
		//! ```
		float4 LUNA_SIMD_CALL cross4_f4(float4 a, float4 b, float4 c);
		//! Normalizes the first two elements of `a`, and stores the result in `dst`.
		//! ```
		//! LENGTH := SQRT((a.x * b.x) + (a.y * b.y))
		//! dst.x = a.x / LENGTH
		//! dst.y = a.y / LENGTH
		//! dst.z = a.z / LENGTH
		//! dst.w = a.w / LENGTH
		//! ```
		//! If the length of `a` parameter is 0 or INF, the result is indefinite.
		float4 LUNA_SIMD_CALL normalize2_f4(float4 a);
		//! Normalizes the first three elements of `a`, and stores the result in `dst`.
		//! ```
		//! LENGTH := SQRT((a.x * b.x) + (a.y * b.y) + (a.z * b.z))
		//! dst.x = a.x / LENGTH
		//! dst.y = a.y / LENGTH
		//! dst.z = a.z / LENGTH
		//! dst.w = a.w / LENGTH
		//! ```
		//! If the length of `a` parameter is 0 or INF, the result is indefinite.
		float4 LUNA_SIMD_CALL normalize3_f4(float4 a);
		//! Normalizes elements of `a`, and stores the result in `dst`.
		//! ```
		//! LENGTH := SQRT((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w))
		//! dst.x = a.x / LENGTH
		//! dst.y = a.y / LENGTH
		//! dst.z = a.z / LENGTH
		//! dst.w = a.w / LENGTH
		//! ```
		//! If the length of `a` parameter is 0 or INF, the result is indefinite.
		float4 LUNA_SIMD_CALL normalize4_f4(float4 a);
		//! Performs reflection operation based on the first two elements of `i` (incident vector) and `n`(normal vector), and stores the 
		//! refected vector in `dst`.
		//! ```
		//! PROJ := (a.x * b.x) + (a.y * b.y)
		//! dst.x = i.x - 2 * PROJ * n.x
		//! dst.y = i.y - 2 * PROJ * n.y
		//! dst.z = i.z - 2 * PROJ * n.z
		//! dst.w = i.w - 2 * PROJ * n.w
		//! ```
		float4 LUNA_SIMD_CALL reflect2_f4(float4 i, float4 n);
		//! Performs reflection operation based on the first three elements of `i` (incident vector) and `n`(normal vector), and stores the 
		//! refected vector in `dst`.
		//! ```
		//! PROJ := (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
		//! dst.x = i.x - 2 * PROJ * n.x
		//! dst.y = i.y - 2 * PROJ * n.y
		//! dst.z = i.z - 2 * PROJ * n.z
		//! dst.w = i.w - 2 * PROJ * n.w
		//! ```
		float4 LUNA_SIMD_CALL reflect3_f4(float4 i, float4 n);
		//! Performs reflection operation based on elements of `i` (incident vector) and `n`(normal vector), and stores the 
		//! refected vector in `dst`.
		//! ```
		//! PROJ := (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w)
		//! dst.x = i.x - 2 * PROJ * n.x
		//! dst.y = i.y - 2 * PROJ * n.y
		//! dst.z = i.z - 2 * PROJ * n.z
		//! dst.w = i.w - 2 * PROJ * n.w
		//! ```
		float4 LUNA_SIMD_CALL reflect4_f4(float4 i, float4 n);
		//! Performs refraction operation based on the first two elements of `i` (incident vector),  `n`(normal vector) and the scalar value
		//! `index` (refraction index), and stores the refected vector in `dst`.
		//! `i` and `n` should be properly normalized before calling this function. 
		//! `index` is `Ni/Nt`, where `Ni` is the refraction index of incident media, and `Nt` is the refraction index of the transmission media.
		//! ```
		//! PROJ := (a.x * b.x) + (a.y * b.y)
		//! DETER := 1.0 - index * index  * (1.0 - PROJ * PROJ)
		//! FOR j := 0 to 4
		//!		i := j*32
		//!		IF DETER >= 0
		//!			dst[i:i+31] := index * i[i:i+31] - n[i:i+31] * (index * PROJ + SQRT(DETER))
		//!		ELSE
		//!			dst[i:i+31] := 0
		//!		ENDIF
		//! ENDFOR
		//!	```
		float4 LUNA_SIMD_CALL refract2_f4(float4 i, float4 n, f32 index);
		//! Performs refraction operation based on the first three elements of `i` (incident vector),  `n`(normal vector) and the scalar value
		//! `index` (refraction index), and stores the refected vector in `dst`.
		//! `i` and `n` should be properly normalized before calling this function. 
		//! `index` is `Ni/Nt`, where `Ni` is the refraction index of incident media, and `Nt` is the refraction index of the transmission media.
		//! ```
		//! PROJ := (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
		//! DETER := 1.0 - index * index  * (1.0 - PROJ * PROJ)
		//! FOR j := 0 to 4
		//!		i := j*32
		//!		IF DETER >= 0
		//!			dst[i:i+31] := index * i[i:i+31] - n[i:i+31] * (index * PROJ + SQRT(DETER))
		//!		ELSE
		//!			dst[i:i+31] := 0
		//!		ENDIF
		//! ENDFOR
		//!	```
		float4 LUNA_SIMD_CALL refract3_f4(float4 i, float4 n, f32 index);
		//! Performs refraction operation based on elements of `i` (incident vector),  `n`(normal vector) and the scalar value
		//! `index` (refraction index), and stores the refected vector in `dst`.
		//! `i` and `n` should be properly normalized before calling this function. 
		//! `index` is `Ni/Nt`, where `Ni` is the refraction index of incident media, and `Nt` is the refraction index of the transmission media.
		//! ```
		//! PROJ := (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w)
		//! DETER := 1.0 - index * index  * (1.0 - PROJ * PROJ)
		//! FOR j := 0 to 4
		//!		i := j*32
		//!		IF DETER >= 0
		//!			dst[i:i+31] := index * i[i:i+31] - n[i:i+31] * (index * PROJ + SQRT(DETER))
		//!		ELSE
		//!			dst[i:i+31] := 0
		//!		ENDIF
		//! ENDFOR
		//!	```
		float4 LUNA_SIMD_CALL refract4_f4(float4 i, float4 n, f32 index);
		//! Computes linear interpolation on packed single-precision (32-bit) floating-point elements in `a` and `b` using the single-precision 
		//! (32-bit) floating-point value `t`, and stores the results in `dst`.
		//! ```
		//! FOR j := 0 to 3
		//! 	i := j*32
		//!		dst[i:i+31] := a[i:i+31] + t * (b[i:i+31] - a[i:i+31])
		//! ENDFOR
		float4 LUNA_SIMD_CALL lerp_f4(float4 a, float4 b, f32 t);
		//! Computes linear interpolation on packed single-precision (32-bit) floating-point elements in `a` and `b` using the corresponding packed 
		//! single-precision (32-bit) floating-point element in `t`, and stores the results in `dst`.
		//! ```
		//! FOR j := 0 to 3
		//! 	i := j*32
		//!		dst[i:i+31] := a[i:i+31] + t[i:i+31] * (b[i:i+31] - a[i:i+31])
		//! ENDFOR
		//! ```
		float4 LUNA_SIMD_CALL lerpv_f4(float4 a, float4 b, float4 t);
		//! Computes barycentric interpolation on packed single-precision (32-bit) floating-point elements in `a`, `b` and `c` using the single-precision 
		//! (32-bit) floating-point values `f` and `g`, and stores the results in `dst`.
		//! ```
		//! FOR j := 0 to 3
		//! 	i := j*32
		//!		dst[i:i+31] := a[i:i+31] + (b[i:i+31] - a[i:i+31]) * f + (c[i:i+31] - a[i:i+31]) * g
		//! ENDFOR
		//! ```
		float4 LUNA_SIMD_CALL barycentric_f4(float4 a, float4 b, float4 c, f32 f, f32 g);
		//! Computes Catmull-Rom spline interpolation on packed single-precision (32-bit) floating-point elements in `a`, `b`, `c` and `d` 
		//! using the single-precision (32-bit) floating-point value `t`, and stores the results in `dst`.
		//! ```
		//! T0 := (-t^3 + 2 * t^2 - t) * 0.5
		//! T1 := (3 * t^3 - 5 * t^2 + 2) * 0.5
		//! T2 := (-3 * t^3 + 4 * t^2 + t) * 0.5
		//! T3 := (t^3 - t^2) * 0.5
		//! FOR j := 0 to 3
		//! 	i := j*32
		//!		dst[i:i+31] := T0 * a[i:i+31] + T1 * b[i:i+31] + T2 * c[i:i+31] + T3 * d[i:i+31]
		//! ENDFOR
		//! ```
		float4 LUNA_SIMD_CALL catmull_rom_f4(float4 a, float4 b, float4 c, float4 d, f32 t);
		//! Computes Hermite spline interpolation on packed single-precision (32-bit) floating-point elements in `v0`, `t0`, `v1` and `t1` 
		//! using the single-precision (32-bit) floating-point value `t`, and stores the results in `dst`.
		//! ```
		//! P0 := (2 * t^3 - 3 * t^2 + 1)
		//! P1 := (t^3 - 2 * t^2 + t)
		//! P2 := (-2 * t^3 + 3 * t^2)
		//! P3 := (t^3 - t^2)
		//! FOR j := 0 to 3
		//! 	i := j*32
		//!		dst[i:i+31] := P0 * v0[i:i+31] + P1 * t0[i:i+31] + P2 * v1[i:i+31] + P3 * t1[i:i+31]
		//! ENDFOR
		//! ```
		float4 LUNA_SIMD_CALL hermite_f4(float4 v0, float4 t0, float4 v1, float4 t1, f32 t);
		
		constexpr u32 PERMUTE_X = 0;
		constexpr u32 PERMUTE_Y = 1;
		constexpr u32 PERMUTE_Z = 2;
		constexpr u32 PERMUTE_W = 3;
		
		//! Shuffles single-precision (32-bit) floating-point elements in `a` based on the control parameter `_SelectX`, `_SelectY`, `_SelectZ` 
		//! and `_SelectW`, and stores the results in `dst`.
		//! ```
		//! DEFINE SELECT4(src, control)
		//!		CASE(control) OF
		//!		0:	res := src.x
		//!		1:	res := src.y
		//!		2:	res := src.z
		//!		3:	res := src.w
		//!		ESAC
		//!		RETURN res
		//! ENDDEF
		//! dst.x := SELECT4(a, _SelectX)
		//! dst.y := SELECT4(a, _SelectY)
		//! dst.z := SELECT4(a, _SelectZ)
		//! dst.w := SELECT4(a, _SelectW)
		//!	```
		template <u32 _SelectX, u32 _SelectY, u32 _SelectZ, u32 _SelectW>
		float4 LUNA_SIMD_CALL permute_f4(float4 a);

		constexpr u32 SELECT_A = 0;
		constexpr u32 SELECT_B = 1;

		//! Performs a per-component selection between `a` and `b` based on the control parameter `_SelectX`, `_SelectY`, `_SelectZ` 
		//! and `_SelectW`, and stores the results in `dst`.
		//! ```
		//! DEFINE SELECT2(a, b control)
		//!		CASE(control) OF
		//!		0:	res := a
		//!		1:	res := b
		//!		ESAC
		//!		RETURN res
		//! ENDDEF
		//! dst.x := SELECT2(a.x, b.x, _SelectX)
		//! dst.y := SELECT4(a.y, b.y, _SelectY)
		//! dst.z := SELECT4(a.z, b.z, _SelectZ)
		//! dst.w := SELECT4(a.w, b.w, _SelectW)
		//!	```
		template <u32 _SelectX, u32 _SelectY, u32 _SelectZ, u32 _SelectW>
		float4 LUNA_SIMD_CALL select_f4(float4 a, float4 b);

		constexpr u32 PERMUTE_AX = 0;
		constexpr u32 PERMUTE_AY = 1;
		constexpr u32 PERMUTE_AZ = 2;
		constexpr u32 PERMUTE_AW = 3;
		constexpr u32 PERMUTE_BX = 4;
		constexpr u32 PERMUTE_BY = 5;
		constexpr u32 PERMUTE_BZ = 6;
		constexpr u32 PERMUTE_BW = 7;

		//! Shuffles single-precision (32-bit) floating-point elements in `a` and `b` based on the control parameter `_SelectX`, `_SelectY`, `_SelectZ` 
		//! and `_SelectW`, and stores the results in `dst`.
		//! ```
		//! DEFINE SELECT8(a, b, control)
		//!		CASE(control) OF
		//!		0:	res := a.x
		//!		1:	res := a.y
		//!		2:	res := a.z
		//!		3:	res := a.w
		//!		4:	res := b.x
		//!		5:	res := b.y
		//!		6:	res := b.z
		//!		7:	res := b.w
		//!		ESAC
		//!		RETURN res
		//! ENDDEF
		//! dst.x := SELECT4(a, b _SelectX)
		//! dst.y := SELECT4(a, b, _SelectY)
		//! dst.z := SELECT4(a, b, _SelectZ)
		//! dst.w := SELECT4(a, b, _SelectW)
		//!	```
		template <u32 _SelectX, u32 _SelectY, u32 _SelectZ, u32 _SelectW>
		float4 LUNA_SIMD_CALL permute2_f4(float4 a, float4 b);

		//! Loads 12 packed single-precision (32-bit) floating-point elements from `mem_addr` to `dst`. The highest
		//! 4 packed single-precision (32-bit) floating-point elements are uninitialized.
		//!  ```
		//! FOR r := 0 to 2
		//!		i := 128 * r
		//!		dst[r].x := MEM[mem_addr + i : mem_addr + i + 31]
		//!		dst[r].y := MEM[mem_addr + i + 32 : mem_addr + i + 63]
		//!		dst[r].z := MEM[mem_addr + i + 64 : mem_addr + i + 95]
		//!		dst[r].w := MEM[mem_addr + i + 96 : mem_addr + i + 127]
		//! ENDFOR
		//! ```
		float3x4 LUNA_SIMD_CALL load_f3x4(f32 const* mem_addr);

		float4x4 LUNA_SIMD_CALL castf3x4_f4x4(float3x4 a);
		float3x4 LUNA_SIMD_CALL castf4x4_f3x4(float4x4 a);

		//! Loads 16 packed single-precision (32-bit) floating-point elements from `mem_addr` to `dst`.
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! FOR r := 0 to 3
		//!		i := 128 * r
		//!		dst[r].x := MEM[mem_addr + i : mem_addr + i + 31]
		//!		dst[r].y := MEM[mem_addr + i + 32 : mem_addr + i + 63]
		//!		dst[r].z := MEM[mem_addr + i + 64 : mem_addr + i + 95]
		//!		dst[r].w := MEM[mem_addr + i + 96 : mem_addr + i + 127]
		//! ENDFOR
		//! ```
		float4x4 LUNA_SIMD_CALL load_f4x4(f32 const* mem_addr);

		//! Creates one 4x4 matrix by loading four vectors, and stores the result in `dst`.
		//! ```
		//! dst[0] := r0
		//! dst[1] := r1
		//! dst[2] := r2
		//! dst[3] := r3
		//! ```
		float4x4 LUNA_SIMD_CALL setf4_f4x4(float4 r0, float4 r1, float4 r2, float4 r3);
		
		//! Stores the first 12 packed single-precision (32-bit) floating-point elements from `m` to `dst`.
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! FOR r := 0 to 2
		//!		i := 128 * r
		//!		MEM[mem_addr + i : mem_addr + i + 31] := m[r].x
		//!		MEM[mem_addr + i + 32 : mem_addr + i + 63] := m[r].y
		//!		MEM[mem_addr + i + 64 : mem_addr + i + 95] := m[r].z
		//!		MEM[mem_addr + i + 96 : mem_addr + i + 127] := m[r].w
		//! ENDFOR
		//! ```
		void LUNA_SIMD_CALL store_f3x4(f32* mem_addr, float3x4 m);

		//! Stores packed single-precision (32-bit) floating-point elements from `m` to `dst`.
		//! `mem_addr` must be aligned on a 16-byte boundary or a general-protection exception may be generated.
		//! ```
		//! FOR r := 0 to 3
		//!		i := 128 * r
		//!		MEM[mem_addr + i : mem_addr + i + 31] := m[r].x
		//!		MEM[mem_addr + i + 32 : mem_addr + i + 63] := m[r].y
		//!		MEM[mem_addr + i + 64 : mem_addr + i + 95] := m[r].z
		//!		MEM[mem_addr + i + 96 : mem_addr + i + 127] := m[r].w
		//! ENDFOR
		//! ```
		void LUNA_SIMD_CALL store_f4x4(f32* mem_addr, float4x4 m);

		float3x4 LUNA_SIMD_CALL setzero_f3x4();
		//! Returns matrix of type `float4x4` with all elements set to zero.
		//! ```
		//! FOR r := 0 to 3
		//!		dst[r].x := 0
		//!		dst[r].y := 0
		//!		dst[r].z := 0
		//!		dst[r].w := 0
		//! ENDFOR
		//! ```
		float4x4 LUNA_SIMD_CALL setzero_f4x4();
		float3x4 LUNA_SIMD_CALL dup_f3x4(f32 e0);
		float4x4 LUNA_SIMD_CALL dup_f4x4(f32 e0);
		float3x4 LUNA_SIMD_CALL add_f3x4(float3x4 a, float3x4 b);
		float4x4 LUNA_SIMD_CALL add_f4x4(float4x4 a, float4x4 b);
		float3x4 LUNA_SIMD_CALL sub_f3x4(float3x4 a, float3x4 b);
		float4x4 LUNA_SIMD_CALL sub_f4x4(float4x4 a, float4x4 b);
		float3x4 LUNA_SIMD_CALL mul_f3x4(float3x4 a, float3x4 b);
		float4x4 LUNA_SIMD_CALL mul_f4x4(float4x4 a, float4x4 b);
		float3x4 LUNA_SIMD_CALL div_f3x4(float3x4 a, float3x4 b);
		float4x4 LUNA_SIMD_CALL div_f4x4(float4x4 a, float4x4 b);
		float3x4 LUNA_SIMD_CALL add_f3x4_f1(float3x4 a, f32 b);
		float4x4 LUNA_SIMD_CALL add_f4x4_f1(float4x4 a, f32 b);
		float3x4 LUNA_SIMD_CALL sub_f3x4_f1(float3x4 a, f32 b);
		float4x4 LUNA_SIMD_CALL sub_f4x4_f1(float4x4 a, f32 b);
		float3x4 LUNA_SIMD_CALL sub_f1_f3x4(f32 a, float3x4 b);
		float4x4 LUNA_SIMD_CALL sub_f1_f4x4(f32 a, float4x4 b);
		float3x4 LUNA_SIMD_CALL mul_f3x4_f1(float3x4 a, f32 b);
		float4x4 LUNA_SIMD_CALL mul_f4x4_f1(float4x4 a, f32 b);

		//! Performs 3x3 matrix multiplication on `a` and `b`, and stores the result 
		//! in `dst`.
		//! ```
		//! dst[0].x := a[0].x * b[0].x + a[0].y * b[1].x + a[0].z * b[2].x
		//! dst[0].y := a[0].x * b[0].y + a[0].y * b[1].y + a[0].z * b[2].y
		//! dst[0].z := a[0].x * b[0].z + a[0].y * b[1].z + a[0].z * b[2].z
		//! dst[1].x := a[1].x * b[0].x + a[1].y * b[1].x + a[1].z * b[2].x
		//! dst[1].y := a[1].x * b[0].y + a[1].y * b[1].y + a[1].z * b[2].y
		//! dst[1].z := a[1].x * b[0].z + a[1].y * b[1].z + a[1].z * b[2].z
		//! dst[2].x := a[2].x * b[0].x + a[2].y * b[1].x + a[2].z * b[2].x
		//! dst[2].y := a[2].x * b[0].y + a[2].y * b[1].y + a[2].z * b[2].y
		//! dst[2].z := a[2].x * b[0].z + a[2].y * b[1].z + a[2].z * b[2].z
		//! ```
		//! The forth element of every row of `dst` is indefinite.
		float3x4 LUNA_SIMD_CALL matmul_f3x3(float3x4 a, float3x4 b);

		//! Performs 4x4 matrix multiplication on `a` and `b`, and stores the result 
		//! in `dst`.
		//! ```
		//! dst[0].x := a[0].x * b[0].x + a[0].y * b[1].x + a[0].z * b[2].x + a[0].w * b[3].x
		//! dst[0].y := a[0].x * b[0].y + a[0].y * b[1].y + a[0].z * b[2].y + a[0].w * b[3].y
		//! dst[0].z := a[0].x * b[0].z + a[0].y * b[1].z + a[0].z * b[2].z + a[0].w * b[3].z
		//! dst[0].w := a[0].x * b[0].w + a[0].y * b[1].w + a[0].z * b[2].w + a[0].w * b[3].w
		//! dst[1].x := a[1].x * b[0].x + a[1].y * b[1].x + a[1].z * b[2].x + a[1].w * b[3].x
		//! dst[1].y := a[1].x * b[0].y + a[1].y * b[1].y + a[1].z * b[2].y + a[1].w * b[3].y
		//! dst[1].z := a[1].x * b[0].z + a[1].y * b[1].z + a[1].z * b[2].z + a[1].w * b[3].z
		//! dst[1].w := a[1].x * b[0].w + a[1].y * b[1].w + a[1].z * b[2].w + a[1].w * b[3].w
		//! dst[2].x := a[2].x * b[0].x + a[2].y * b[1].x + a[2].z * b[2].x + a[2].w * b[3].x
		//! dst[2].y := a[2].x * b[0].y + a[2].y * b[1].y + a[2].z * b[2].y + a[2].w * b[3].y
		//! dst[2].z := a[2].x * b[0].z + a[2].y * b[1].z + a[2].z * b[2].z + a[2].w * b[3].z
		//! dst[2].w := a[2].x * b[0].w + a[2].y * b[1].w + a[2].z * b[2].w + a[2].w * b[3].w
		//! dst[3].x := a[3].x * b[0].x + a[3].y * b[1].x + a[3].z * b[2].x + a[3].w * b[3].x
		//! dst[3].y := a[3].x * b[0].y + a[3].y * b[1].y + a[3].z * b[2].y + a[3].w * b[3].y
		//! dst[3].z := a[3].x * b[0].z + a[3].y * b[1].z + a[3].z * b[2].z + a[3].w * b[3].z
		//! dst[3].w := a[3].x * b[0].w + a[3].y * b[1].w + a[3].z * b[2].w + a[3].w * b[3].w
		//! ```
		float4x4 LUNA_SIMD_CALL matmul_f4x4(float4x4 a, float4x4 b);

		//! Performs matrix transpose on `a`, and stores the result in `dst`.
		//! ```
		//! dst[0].x = a[0].x
		//! dst[0].y = a[1].x
		//! dst[0].z = a[2].x
		//! dst[0].w = a[3].x
		//! dst[1].x = a[0].y
		//! dst[1].y = a[1].y
		//! dst[1].z = a[2].y
		//! dst[1].w = a[3].y
		//! dst[2].x = a[0].z
		//! dst[2].y = a[1].z
		//! dst[2].z = a[2].z
		//! dst[2].w = a[3].z
		//! dst[3].x = a[0].w
		//! dst[3].y = a[1].w
		//! dst[3].z = a[2].w
		//! dst[3].w = a[3].w
		//! ```
		float4x4 LUNA_SIMD_CALL transpose_f4x4(float4x4 a);

		//! Computes the determinant of the 3x3 matrix `a`, and stores the result in `dst`.
		//! ```
		//! dst :=	a[0].x * (a[1].y * a[2].z - a[1].z * a[2].y) +
		//!			a[0].y * (a[1].z * a[2].x - a[1].x * a[2].z) +
		//!			a[0].z * (a[1].x * a[2].y - a[1].y * a[2].x)
		//! ```
		f32 LUNA_SIMD_CALL determinant_f3x3(float3x4 a);
		//! Computes the determinant of the 3x3 matrix `a`, and stores the result in every element of `dst`.
		//! ```
		//! DETER :=a[0].x * (a[1].y * a[2].z - a[1].z * a[2].y) +
		//!			a[0].y * (a[1].z * a[2].x - a[1].x * a[2].z) +
		//!			a[0].z * (a[1].x * a[2].y - a[1].y * a[2].x)
		//! dst.x := DETER
		//! dst.y := DETER
		//! dst.z := DETER
		//! dst.w := DETER
		//! ```
		float4 LUNA_SIMD_CALL determinantv_f3x3(float3x4 a);

		//! Computes the determinant and the inverse matrix of `a`, stores the determinant in `out_determinant`, and
		//! stores the inverse matrix in `dst`.
		//! ```
		//!	DETER :=a[0].x * (a[1].y * a[2].z - a[1].z * a[2].y) +
		//!			a[0].y * (a[1].z * a[2].x - a[1].x * a[2].z) +
		//!			a[0].z * (a[1].x * a[2].y - a[1].y * a[2].x)
		//! IF out_determinant != 0
		//!		MEM[out_determinant:out_determinant+31] := DETER
		//! ENDIF
		//! INV_DETER := 1.0 / DETER
		//! dst[0].x := INV_DETER * (a[1].y * a[2].z - a[1].z * a[2].y);
		//! dst[1].x := INV_DETER * (a[1].z * a[2].x - a[1].x * a[2].z);
		//! dst[2].x := INV_DETER * (a[1].x * a[2].y - a[1].y * a[2].x);
		//! dst[0].y := INV_DETER * (a[0].z * a[2].y - a[0].y * a[2].z);
		//! dst[1].y := INV_DETER * (a[0].x * a[2].z - a[0].z * a[2].x);
		//! dst[2].y := INV_DETER * (a[0].y * a[2].x - a[0].x * a[2].y);
		//! dst[0].z := INV_DETER * (a[0].y * a[1].z - a[0].z * a[1].y);
		//! dst[1].z := INV_DETER * (a[0].z * a[1].x - a[0].x * a[1].z);
		//! dst[2].z := INV_DETER * (a[0].x * a[1].y - a[0].y * a[1].x);
		//! ```
		//! The forth element of every row of `a` is indefinite.
		//! If `DETER` is 0, `dst` is indefinite.
		float3x4 LUNA_SIMD_CALL inverse_f3x3(float3x4 a, f32* out_determinant);

		//! Calculates the determinant of matrix `a`, and stores the result in `dst`.
		//! ```
		//! dst :=	 a[0].x * (a[1].y * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].y - a[2].y * a[3].w) + a[1].w * (a[2].y * a[3].z - a[2].z * a[3].y))
		//!			-a[0].y * (a[1].x * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].z - a[2].z * a[3].x))
		//!			+a[0].z * (a[1].x * (a[2].y * a[3].w - a[2].w * a[3].y) + a[1].y * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].y - a[2].y * a[3].x))
		//!			-a[0].w * (a[1].x * (a[2].y * a[3].z - a[2].z * a[3].y) + a[1].y * (a[2].z * a[3].x - a[2].x * a[3].z) + a[1].z * (a[2].x * a[3].y - a[2].y * a[3].x))
		//! ```
		f32 LUNA_SIMD_CALL determinant_f4x4(float4x4 a);

		//! Calculates the determinant of matrix `a`, and stores the result in every element of `dst`.
		//! ```
		//! DETER := a[0].x * (a[1].y * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].y - a[2].y * a[3].w) + a[1].w * (a[2].y * a[3].z - a[2].z * a[3].y))
		//!			-a[0].y * (a[1].x * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].z - a[2].z * a[3].x))
		//!			+a[0].z * (a[1].x * (a[2].y * a[3].w - a[2].w * a[3].y) + a[1].y * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].y - a[2].y * a[3].x))
		//!			-a[0].w * (a[1].x * (a[2].y * a[3].z - a[2].z * a[3].y) + a[1].y * (a[2].z * a[3].x - a[2].x * a[3].z) + a[1].z * (a[2].x * a[3].y - a[2].y * a[3].x))
		//! dst.x := DETER
		//! dst.y := DETER
		//! dst.z := DETER
		//! dst.w := DETER
		//! ```
		float4 LUNA_SIMD_CALL determinantv_f4x4(float4x4 a);

		//! Computes the determinant and the inverse matrix of `a`, stores the determinant in `out_determinant`, and
		//! stores the inverse matrix in `dst`.
		//! ```
		//! DETER := a[0].x * (a[1].y * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].y - a[2].y * a[3].w) + a[1].w * (a[2].y * a[3].z - a[2].z * a[3].y))
		//!			-a[0].y * (a[1].x * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].z - a[2].z * a[3].x))
		//!			+a[0].z * (a[1].x * (a[2].y * a[3].w - a[2].w * a[3].y) + a[1].y * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].y - a[2].y * a[3].x))
		//!			-a[0].w * (a[1].x * (a[2].y * a[3].z - a[2].z * a[3].y) + a[1].y * (a[2].z * a[3].x - a[2].x * a[3].z) + a[1].z * (a[2].x * a[3].y - a[2].y * a[3].x))
		//! IF out_determinant != 0
		//!		MEM[out_determinant:out_determinant+31] := DETER
		//! ENDIF
		//! INV_DETER := 1.0 / DETER
		//! dst[0].x :=  INV_DETER * (a[1].y * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].y - a[2].y * a[3].w) + a[1].w * (a[2].y * a[3].z - a[2].z * a[3].y));
		//! dst[1].x := -INV_DETER * (a[1].x * (a[2].z * a[3].w - a[2].w * a[3].z) + a[1].z * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].z - a[2].z * a[3].x));
		//! dst[2].x :=  INV_DETER * (a[1].x * (a[2].y * a[3].w - a[2].w * a[3].y) + a[1].y * (a[2].w * a[3].x - a[2].x * a[3].w) + a[1].w * (a[2].x * a[3].y - a[2].y * a[3].x));
		//! dst[3].x := -INV_DETER * (a[1].x * (a[2].y * a[3].z - a[2].z * a[3].y) + a[1].y * (a[2].z * a[3].x - a[2].x * a[3].z) + a[1].z * (a[2].x * a[3].y - a[2].y * a[3].x));
		//! dst[0].y := -INV_DETER * (a[0].y * (a[2].z * a[3].w - a[2].w * a[3].z) + a[0].z * (a[2].w * a[3].y - a[2].y * a[3].w) + a[0].w * (a[2].y * a[3].z - a[2].z * a[3].y));
		//! dst[1].y :=  INV_DETER * (a[0].x * (a[2].z * a[3].w - a[2].w * a[3].z) + a[0].z * (a[2].w * a[3].x - a[2].x * a[3].w) + a[0].w * (a[2].x * a[3].z - a[2].z * a[3].x));
		//! dst[2].y := -INV_DETER * (a[0].x * (a[2].y * a[3].w - a[2].w * a[3].y) + a[0].y * (a[2].w * a[3].x - a[2].x * a[3].w) + a[0].w * (a[2].x * a[3].y - a[2].y * a[3].x));
		//! dst[3].y :=  INV_DETER * (a[0].x * (a[2].y * a[3].z - a[2].z * a[3].y) + a[0].y * (a[2].z * a[3].x - a[2].x * a[3].z) + a[0].z * (a[2].x * a[3].y - a[2].y * a[3].x));
		//! dst[0].z :=  INV_DETER * (a[3].w * (a[0].y * a[1].z - a[0].z * a[1].y) + a[3].z * (a[0].w * a[1].y - a[0].y * a[1].w) + a[3].y * (a[0].z * a[1].w - a[0].w * a[1].z));
		//! dst[1].z := -INV_DETER * (a[3].w * (a[0].x * a[1].z - a[0].z * a[1].x) + a[3].z * (a[0].w * a[1].x - a[0].x * a[1].w) + a[3].x * (a[0].z * a[1].w - a[0].w * a[1].z));
		//! dst[2].z :=  INV_DETER * (a[3].w * (a[0].x * a[1].y - a[0].y * a[1].x) + a[3].y * (a[0].w * a[1].x - a[0].x * a[1].w) + a[3].x * (a[0].y * a[1].w - a[0].w * a[1].y));
		//! dst[3].z := -INV_DETER * (a[3].z * (a[0].x * a[1].y - a[0].y * a[1].x) + a[3].y * (a[0].z * a[1].x - a[0].x * a[1].z) + a[3].x * (a[0].y * a[1].z - a[0].z * a[1].y));
		//! dst[0].w := -INV_DETER * (a[2].w * (a[0].y * a[1].z - a[0].z * a[1].y) + a[2].z * (a[0].w * a[1].y - a[0].y * a[1].w) + a[2].y * (a[0].z * a[1].w - a[0].w * a[1].z));
		//! dst[1].w :=  INV_DETER * (a[2].w * (a[0].x * a[1].z - a[0].z * a[1].x) + a[2].z * (a[0].w * a[1].x - a[0].x * a[1].w) + a[2].x * (a[0].z * a[1].w - a[0].w * a[1].z));
		//! dst[2].w := -INV_DETER * (a[2].w * (a[0].x * a[1].y - a[0].y * a[1].x) + a[2].y * (a[0].w * a[1].x - a[0].x * a[1].w) + a[2].x * (a[0].y * a[1].w - a[0].w * a[1].y));
		//! dst[3].w :=  INV_DETER * (a[2].z * (a[0].x * a[1].y - a[0].y * a[1].x) + a[2].y * (a[0].z * a[1].x - a[0].x * a[1].z) + a[2].x * (a[0].y * a[1].z - a[0].z * a[1].y));
		//! ```
		//! If `DETER` is 0, `dst` is indefinite.
		float4x4 LUNA_SIMD_CALL inverse_f4x4(float4x4 a, f32* out_determinant);

		//! Rounds each component of `a` to the nearest even integer.
		//! ```
		//! dst.x := ROUND(a.x)
		//! dst.y := ROUND(a.y)
		//! dst.z := ROUND(a.z)
		//! dst.w := ROUND(a.w)
		//! ```
		float4 LUNA_SIMD_CALL round_f4(float4 a);

		//! Computes the per-component angle modulo 2PI for `a`, and stores the results in `dst`. The angle is expressed in radians.
		//! The result is rounded in [-PI, PI].
		//! ```
		//! dst.x := a.x - 2 * PI * round( a.x / (2 * PI) )
		//! dst.y := a.y - 2 * PI * round( a.y / (2 * PI) )
		//! dst.z := a.z - 2 * PI * round( a.z / (2 * PI) )
		//! dst.w := a.w - 2 * PI * round( a.w / (2 * PI) )
		//! ```
		float4 LUNA_SIMD_CALL modangle_f4(float4 a);

		//! Computes the sine of packed single-precision (32-bit) floating-point elements in `a` expressed in radians, 
		//! and stores the results in `dst`.
		//! ```
		//! dst.x := SIN(a.x)
		//! dst.y := SIN(a.y)
		//! dst.z := SIN(a.z)
		//! dst.w := SIN(a.w)
		//! ```
		float4 LUNA_SIMD_CALL sin_f4(float4 a);

		//! Computes the cosine of packed single-precision (32-bit) floating-point elements in `a` expressed in radians, 
		//! and stores the results in `dst`.
		//! ```
		//! dst.x := COS(a.x)
		//! dst.y := COS(a.y)
		//! dst.z := COS(a.z)
		//! dst.w := COS(a.w)
		//! ```
		float4 LUNA_SIMD_CALL cos_f4(float4 a);

		//! Computes the sine and cosine of packed single-precision (32-bit) floating-point elements in `a` expressed in radians, 
		//! and stores the results in `dst` and `out_cos`.
		//! ```
		//! dst.x := SIN(a.x)
		//! dst.y := SIN(a.y)
		//! dst.z := SIN(a.z)
		//! dst.w := SIN(a.w)
		//! out_cos.x := COS(a.x)
		//! out_cos.y := COS(a.y)
		//! out_cos.z := COS(a.z)
		//! out_cos.w := COS(a.w)
		//! ```
		float4 LUNA_SIMD_CALL sincos_f4(float4& out_cos, float4 a);
	}
}
#include "Impl/Simd.inl"
#endif