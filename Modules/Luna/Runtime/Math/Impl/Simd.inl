/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Simd.inl
* @author JXMaster
* @date 2022/3/19
 */
#pragma once
#include "../Simd.hpp"
#ifdef LUNA_SIMD
namespace Luna
{
	namespace Simd
	{
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL casti_f4(int4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castsi128_ps(a);
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_f32_s32(a);
#else
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL castf_i4(float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(a);
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_f32(a);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL load_f2(f32 const* mem_addr)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castsi128_ps(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(mem_addr)));
#elif defined(LUNA_NEON_INTRINSICS)
			float32x2_t x = vld1_f32(mem_addr);
			float32x2_t zero = vdup_n_f32(0);
			return vcombine_f32(x, zero);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL load_f4(f32 const* mem_addr)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_load_ps(mem_addr);
#elif defined(LUNA_NEON_INTRINSICS)
			return vld1q_f32(mem_addr);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE void LUNA_SIMD_CALL store_f2(f32* mem_addr, float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			_mm_storel_epi64(reinterpret_cast<__m128i*>(mem_addr), _mm_castps_si128(a));
#elif defined(LUNA_NEON_INTRINSICS)
			vst1_f32(mem_addr, vget_low_f32(a));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE void LUNA_SIMD_CALL store_f4(f32* mem_addr, float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			_mm_store_ps(mem_addr, a);
#elif defined(LUNA_NEON_INTRINSICS)
			vst1q_f32(mem_addr, a);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL set_f4(f32 e0, f32 e1, f32 e2, f32 e3)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_set_ps(e3, e2, e1, e0);
#elif defined(LUNA_NEON_INTRINSICS)
			float32x2_t t0 = vcreate_f32(
				static_cast<u64>(*reinterpret_cast<const u32*>(&e0))
				| (static_cast<u64>(*reinterpret_cast<const u32*>(&e1)) << 32));
			float32x2_t t1 = vcreate_f32(
				static_cast<u64>(*reinterpret_cast<const u32*>(&e2))
				| (static_cast<u64>(*reinterpret_cast<const u32*>(&e3)) << 32));
			return vcombine_f32(t0, t1);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL set_i4(i32 e0, i32 e1, i32 e2, i32 e3)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_set_epi32(e3, e2, e1, e0);
#elif defined(LUNA_NEON_INTRINSICS)
			int32x2_t t0 = vcreate_s32((u64)((u32)e0) | (((u64)((u32)e1)) << 32));
			int32x2_t t1 = vcreate_s32((u64)((u32)e2) | (((u64)((u32)e3)) << 32));
			return vcombine_s32(t0, t1);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL setzero_f4()
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_setzero_ps();
#elif defined(LUNA_NEON_INTRINSICS)
			return vdupq_n_f32(0);
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dup_f4(f32 e0)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_set_ps1(e0);
#elif defined(LUNA_NEON_INTRINSICS)
			return vdupq_n_f32(e0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE f32 LUNA_SIMD_CALL getx_f4(float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_cvtss_f32(a);
#elif defined(LUNA_NEON_INTRINSICS)
			return vgetq_lane_f32(a, 0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL setw_f4(float4 a, f32 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_insert_ps(a, _mm_set_ss(b), 0x30);
#elif defined(LUNA_SSE2_INTRINSICS)
			// Swap w and x
			__m128 res = permute_f4<PERMUTE_W, PERMUTE_Y, PERMUTE_Z, PERMUTE_X>(a);
			// Replace the x component
			res = _mm_move_ss(res, _mm_set_ss(b));
			// Swap w and x again
			res = permute_f4<PERMUTE_W, PERMUTE_Y, PERMUTE_Z, PERMUTE_X>(res);
			return res;
#elif defined(LUNA_NEON_INTRINSICS)
			return vsetq_lane_f32(b, a, 3);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dupx_f4(float4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			return _mm_permute_ps(a, _MM_SHUFFLE(0, 0, 0, 0));
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0));
#elif defined(LUNA_NEON_INTRINSICS)
			return vdupq_lane_f32(vget_low_f32(a), 0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dupy_f4(float4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			return _mm_permute_ps(a, _MM_SHUFFLE(1, 1, 1, 1));
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1));
#elif defined(LUNA_NEON_INTRINSICS)
			return vdupq_lane_f32(vget_low_f32(a), 1);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dupz_f4(float4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			return _mm_permute_ps(a, _MM_SHUFFLE(2, 2, 2, 2));
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 2, 2));
#elif defined(LUNA_NEON_INTRINSICS)
			return vdupq_lane_f32(vget_high_f32(a), 0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dupw_f4(float4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			return _mm_permute_ps(a, _MM_SHUFFLE(3, 3, 3, 3));
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 3, 3));
#elif defined(LUNA_NEON_INTRINSICS)
			return vdupq_lane_f32(vget_high_f32(a), 1);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL cmpeq_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_cmpeq_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vceqq_f32(a, b));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL cmpneq_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_cmpneq_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vmvnq_u32(vceqq_f32(a, b)));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL cmpgt_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_cmpgt_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vcgtq_f32(a, b));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL cmplt_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_cmplt_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vcltq_f32(a, b));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL cmpge_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_cmpge_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vcgeq_f32(a, b));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL cmple_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_cmple_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vcleq_f32(a, b));
#else 
#error "Not implemented."
#endif
		}
#if defined(LUNA_NEON_INTRINSICS)
		namespace Impl
		{
			constexpr const u32 maskint_element_index[4]{ 1, 2, 4, 8 };
		}
#endif
		LUNA_FORCEINLINE i32 LUNA_SIMD_CALL maskint_i4(int4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_movemask_ps(_mm_castsi128_ps(a));
#elif defined(LUNA_NEON_INTRINSICS)
			static const uint32x4_t mask = vld1q_u32(Impl::maskint_element_index);
			uint32x4_t temp = vandq_u32(vreinterpretq_u32_s32(a), mask);
			uint32x2_t l = vget_low_u32(temp);    // get low 2 uint32 
			uint32x2_t h = vget_high_u32(temp);  // get high 2 uint32
			l = vorr_u32(l, h);
			l = vpadd_u32(l, l);
			return static_cast<i32>(vget_lane_u32(l, 0));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL add_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_add_ps(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vaddq_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL sub_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_sub_ps(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vsubq_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL mul_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_mul_ps(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vmulq_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL div_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_div_ps(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vdivq_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL scale_f4(float4 a, f32 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_mul_ps(a, _mm_set_ps1(b));
#elif defined(LUNA_NEON_INTRINSICS)
			return vmulq_n_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL muladd_f4(float4 a, float4 b, float4 c)
		{
#if defined(LUNA_FMA3_INTRINSICS)
			return _mm_fmadd_ps(a, b, c);
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_add_ps(_mm_mul_ps(a, b), c);
#elif defined(LUNA_NEON_INTRINSICS)
			return vmlaq_f32(c, a, b);
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL negmuladd_f4(float4 a, float4 b, float4 c)
		{
#if defined(LUNA_FMA3_INTRINSICS)
			return _mm_fnmadd_ps(a, b, c);
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_sub_ps(c, _mm_mul_ps(a, b));
#elif defined(LUNA_NEON_INTRINSICS)
#ifdef LUNA_PLATFORM_ARM64
			return vfmsq_f32(c, a, b);
#else
			return vmlsq_f32(c, a, b);
#endif
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL scaleadd_f4(float4 a, f32 b, float4 c)
		{
#if defined(LUNA_FMA3_INTRINSICS)
			return _mm_fmadd_ps(a, _mm_set_ps1(b), c);
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_add_ps(_mm_mul_ps(a, _mm_set_ps1(b)), c);
#elif defined(LUNA_NEON_INTRINSICS)
			return vmlaq_n_f32(c, a, b);
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL sqrt_f4(float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_sqrt_ps(a);
#elif defined(LUNA_NEON_INTRINSICS)
			// 3 iterations of Newton-Raphson refinment of sqrt
			float32x4_t S0 = vrsqrteq_f32(a);
			float32x4_t P0 = vmulq_f32(a, S0);
			float32x4_t R0 = vrsqrtsq_f32(P0, S0);
			float32x4_t S1 = vmulq_f32(S0, R0);
			float32x4_t P1 = vmulq_f32(a, S1);
			float32x4_t R1 = vrsqrtsq_f32(P1, S1);
			float32x4_t S2 = vmulq_f32(S1, R1);
			float32x4_t P2 = vmulq_f32(a, S2);
			float32x4_t R2 = vrsqrtsq_f32(P2, S2);
			float32x4_t S3 = vmulq_f32(S2, R2);
			// Check zero.
			uint32x4_t equal_zero = vceqq_f32(a, vdupq_n_f32(0.0f));
			float32x4_t result = vmulq_f32(a, S3);
			return vbslq_f32(equal_zero, a, result);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL rsqrtest_f4(float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_rsqrt_ps(a);
#elif defined(LUNA_NEON_INTRINSICS)
			return vrsqrteq_f32(a);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL rsqrt_f4(float4 a)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			__m128 res =  _mm_sqrt_ps(a);
			return _mm_div_ps(_mm_set_ps1(1.0f), res);
#elif defined(LUNA_NEON_INTRINSICS)
			// 2 iterations of Newton-Raphson refinement of reciprocal
			float32x4_t S0 = vrsqrteq_f32(a);
			float32x4_t P0 = vmulq_f32(a, S0);
			float32x4_t R0 = vrsqrtsq_f32(P0, S0);
			float32x4_t S1 = vmulq_f32(S0, R0);
			float32x4_t P1 = vmulq_f32(a, S1);
			float32x4_t R1 = vrsqrtsq_f32(P1, S1);
			return vmulq_f32(S1, R1);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL max_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_max_ps(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vmaxq_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL min_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_min_ps(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vminq_f32(a, b);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL and_i4(int4 a, int4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_castps_si128(_mm_and_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b)));
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vandq_u32(vreinterpretq_u32_s32(a), vreinterpretq_u32_s32(b)));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE int4 LUNA_SIMD_CALL or_i4(int4 a, int4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			return _mm_or_si128(a, b);
#elif defined(LUNA_NEON_INTRINSICS)
			return vreinterpretq_s32_u32(vorrq_u32(vreinterpretq_u32_s32(a), vreinterpretq_u32_s32(b)));
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE f32 LUNA_SIMD_CALL dot2_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_cvtss_f32(_mm_dp_ps(a, b, 0x3f));
#elif defined(LUNA_SSE3_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			dot = _mm_hadd_ps(dot, dot);
			return _mm_cvtss_f32(dot);
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			__m128 temp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 1, 1, 1));
			dot = _mm_add_ss(dot, temp);
			return _mm_cvtss_f32(dot);
#elif defined(LUNA_NEON_INTRINSICS)
			float32x2_t dot = vmul_f32(vget_low_f32(a), vget_low_f32(b));
			dot = vpadd_f32(dot, dot);
			return vget_lane_f32(dot, 0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE f32 LUNA_SIMD_CALL dot3_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_cvtss_f32(_mm_dp_ps(a, b, 0x7f));
#elif defined(LUNA_SSE3_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			dot = _mm_and_ps(dot, casti_f4(set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0)));
			dot = _mm_hadd_ps(dot, dot);
			dot = _mm_hadd_ps(dot, dot);
			return _mm_cvtss_f32(dot);
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			__m128 temp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2, 1, 2, 1));
			dot = _mm_add_ss(dot, temp);
			temp = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
			dot = _mm_add_ss(dot, temp);
			return _mm_cvtss_f32(dot);
#elif defined(LUNA_NEON_INTRINSICS)
			float32x4_t temp = vmulq_f32(a, b);
			float32x2_t v1 = vget_low_f32(temp);
			float32x2_t v2 = vget_high_f32(temp);
			v1 = vpadd_f32(v1, v1);
			v1 = vadd_f32(v1, v2);
			return vget_lane_f32(v1, 0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE f32 LUNA_SIMD_CALL dot4_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_cvtss_f32(_mm_dp_ps(a, b, 0xff));
#elif defined(LUNA_SSE3_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			dot = _mm_hadd_ps(dot, dot);
			dot = _mm_hadd_ps(dot, dot);
			return _mm_cvtss_f32(dot);
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 temp2 = b;
			__m128 temp1 = _mm_mul_ps(a, temp2);
			temp2 = _mm_shuffle_ps(temp2, temp1, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
			temp2 = _mm_add_ps(temp2, temp1);          // Add Z = X+Z; W = Y+W;
			temp1 = _mm_shuffle_ps(temp1, temp2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
			temp1 = _mm_add_ps(temp1, temp2);           // Add Z and W together
			temp1 = _mm_shuffle_ps(temp1, temp1, _MM_SHUFFLE(2, 2, 2, 2));    // Splat Z and return
			return _mm_cvtss_f32(temp1);
#elif defined(LUNA_NEON_INTRINSICS)
			float32x4_t temp = vmulq_f32(a, b);
			float32x2_t v1 = vget_low_f32(temp);
			float32x2_t v2 = vget_high_f32(temp);
			v1 = vadd_f32(v1, v2);
			v1 = vpadd_f32(v1, v1);
			return vget_lane_f32(v1, 0);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dot2v_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_dp_ps(a, b, 0x3f);
#elif defined(LUNA_SSE3_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			dot = _mm_hadd_ps(dot, dot);
			dot = _mm_moveldup_ps(dot);
			return dot;
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			__m128 temp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 1, 1, 1));
			dot = _mm_add_ss(dot, temp);
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
			return dot;
#elif defined(LUNA_NEON_INTRINSICS)
			float32x2_t dot = vmul_f32(vget_low_f32(a), vget_low_f32(b));
			dot = vpadd_f32(dot, dot);
			return vcombine_f32(dot, dot);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dot3v_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_dp_ps(a, b, 0x7f);
#elif defined(LUNA_SSE3_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			dot = _mm_and_ps(dot, casti_f4(set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0)));
			dot = _mm_hadd_ps(dot, dot);
			return _mm_hadd_ps(dot, dot);
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			__m128 temp = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2, 1, 2, 1));
			dot = _mm_add_ss(dot, temp);
			temp = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1));
			dot = _mm_add_ss(dot, temp);
			return _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
#elif defined(LUNA_NEON_INTRINSICS)
			float32x4_t temp = vmulq_f32(a, b);
			float32x2_t v1 = vget_low_f32(temp);
			float32x2_t v2 = vget_high_f32(temp);
			v1 = vpadd_f32(v1, v1);
			v2 = vdup_lane_f32(v2, 0);
			v1 = vadd_f32(v1, v2);
			return vcombine_f32(v1, v1);
#else 
#error "Not implemented."
#endif
		}
		LUNA_FORCEINLINE float4 LUNA_SIMD_CALL dot4v_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_dp_ps(a, b, 0xff);
#elif defined(LUNA_SSE3_INTRINSICS)
			__m128 dot = _mm_mul_ps(a, b);
			dot = _mm_hadd_ps(dot, dot);
			return _mm_hadd_ps(dot, dot);
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 temp2 = b;
			__m128 temp = _mm_mul_ps(a, temp2);
			temp2 = _mm_shuffle_ps(temp2, temp, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
			temp2 = _mm_add_ps(temp2, temp);          // Add Z = X+Z; W = Y+W;
			temp = _mm_shuffle_ps(temp, temp2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
			temp = _mm_add_ps(temp, temp2);           // Add Z and W together
			return _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2));    // Splat Z and return
#elif defined(LUNA_NEON_INTRINSICS)
			float32x4_t temp = vmulq_f32(a, b);
			float32x2_t v1 = vget_low_f32(temp);
			float32x2_t v2 = vget_high_f32(temp);
			v1 = vadd_f32(v1, v2);
			v1 = vpadd_f32(v1, v1);
			return vcombine_f32(v1, v1);
#else 
#error "Not implemented."
#endif
		}
#if defined(LUNA_NEON_INTRINSICS)
		namespace Impl
		{
			constexpr const f32 cross2_parameter[2]{ 1.f, -1.f };
		}
#endif
		inline float4 LUNA_SIMD_CALL cross2_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			// by, bx, by, bx
			__m128 res = permute_f4<1, 0, 1, 0>(b);
			// axby, aybx, ?, ?
			res = _mm_mul_ps(res, a);
			// aybx, aybx, aybx, aybx
			__m128 temp = permute_f4<1, 1, 1, 1>(res);
			// axby-aybx, 0, ?, ?
			res = _mm_sub_ss(res, temp);
			return permute_f4<0, 0, 0, 0>(res);
#elif defined(LUNA_NEON_INTRINSICS)
			// axby, aybx
			float32x2_t temp = vmul_f32(vget_low_f32(a), vrev64_f32(vget_low_f32(b)));
			// axby, -aybx
			temp = vmul_f32(temp, vld1_f32(Impl::cross2_parameter));
			// axby-aybx, axby-aybx
			temp = vpadd_f32(temp, temp);
			return vcombine_f32(temp, temp);
#else
#error "Not implemented."
#endif
		}
		inline float4 LUNA_SIMD_CALL cross3_f4(float4 a, float4 b)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			// ay,az,ax,aw
			__m128 temp1 = permute_f4<1, 2, 0, 3>(a);
			// bz,bx,by,bw
			__m128 temp2 = permute_f4<2, 0, 1, 3>(b);
			// aybz, azbx, axby, awbw
			__m128 res = _mm_mul_ps(temp1, temp2);
			// az,ax,ay,aw
			temp1 = permute_f4<1, 2, 0, 3>(temp1);
			// by,bz,bx,bw
			temp2 = permute_f4<2, 0, 1, 3>(temp2);
			// aybz-azby, azbx-axbz, axby-aybx, awbw - awbw
			res = negmuladd_f4(temp1, temp2, res);
			return _mm_and_ps(res, casti_f4(set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000)));
#elif defined(LUNA_NEON_INTRINSICS)
			// ax, ay
			float32x2_t v1xy = vget_low_f32(a);
			// bx, by
			float32x2_t v2xy = vget_low_f32(b);
			// ay, ax
			float32x2_t v1yx = vrev64_f32(v1xy);
			// by, bx
			float32x2_t v2yx = vrev64_f32(v2xy);
			// az, az
			float32x2_t v1zz = vdup_lane_f32(vget_high_f32(a), 0);
			// bz, bz
			float32x2_t v2zz = vdup_lane_f32(vget_high_f32(b), 0);
			// aybz, axbz, axby, aybx
			float32x4_t res = vmulq_f32(vcombine_f32(v1yx, v1xy), vcombine_f32(v2zz, v2yx));
			// aybz - azby, axbz - azbx, axby - aybx, aybx - axby
			res = vmlsq_f32(res, vcombine_f32(v1zz, v1yx), vcombine_f32(v2yx, v2xy));
			// aybz - azby, azbx - axbz, axby - aybx, aybx - axby
			res = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(res), vreinterpretq_u32_s32(set_i4(0, 0x80000000, 0, 0))));
			// aybz - azby, azbx - axbz, axby - aybx, 0
			return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(res), vreinterpretq_u32_s32(set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000))));
#else
#error "Not implemented."
#endif
		}

#if defined(LUNA_NEON_INTRINSICS)
		namespace Impl
		{
			constexpr const u32 MASK_X[4]{ 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 };
		}
#endif

		inline float4 LUNA_SIMD_CALL cross4_f4(float4 a, float4 b, float4 c)
		{
#if defined(LUNA_SSE2_INTRINSICS)
			// bzwyz * cwzwy
			__m128 res = permute_f4<2, 3, 1, 2>(b);
			__m128 temp3 = permute_f4<3, 2, 3, 1>(c);
			res = _mm_mul_ps(res, temp3);
			// - bwzwy * czwyz
			__m128 temp2 = permute_f4<3, 2, 3, 1>(b);
			temp3 = permute_f4<1, 0, 3, 1>(temp3);
			res = negmuladd_f4(temp2, temp3, res);
			// term1 * ayxxx
			__m128 temp1 = permute_f4<1, 0, 0, 0>(a);
			res = _mm_mul_ps(res, temp1);
			// bywxz * cwxwx
			temp2 = permute_f4<1, 3, 0, 2>(b);
			temp3 = permute_f4<3, 0, 3, 0>(c);
			temp3 = _mm_mul_ps(temp3, temp2);
			// - bwxwx * cywxz
			temp2 = permute_f4<1, 2, 1, 2>(temp2);
			temp1 = permute_f4<1, 3, 0, 2>(c);
			temp3 = negmuladd_f4(temp2, temp1, temp3);
			// res - temp * azzyy
			temp1 = permute_f4<2, 2, 1, 1>(a);
			res = negmuladd_f4(temp1, temp3, res);
			// byzxy * czxyx
			temp2 = permute_f4<1, 2, 0, 1>(b);
			temp3 = permute_f4<2, 0, 1, 0>(c);
			temp3 = _mm_mul_ps(temp3, temp2);
			// - bzxyx * cyzxy
			temp2 = permute_f4<1, 2, 0, 2>(temp2);
			temp1 = permute_f4<1, 2, 0, 1>(c);
			temp3 = negmuladd_f4(temp1, temp2, temp3);
			// res + term * awwwz
			temp1 = permute_f4<3, 3, 3, 2>(a);
			res = muladd_f4(temp3, temp1, res);
			return res;
#elif defined(LUNA_NEON_INTRINSICS)
			const uint32x2_t select = vld1_u32( Impl::MASK_X );

			// Term1: bzwyz * cwzwy
			const float32x2_t v2xy = vget_low_f32(b);
			const float32x2_t v2zw = vget_high_f32(b);
			const float32x2_t v2yx = vrev64_f32(v2xy);
			const float32x2_t v2wz = vrev64_f32(v2zw);
			const float32x2_t v2yz = vbsl_f32(select, v2yx, v2wz);

			const float32x2_t v3zw = vget_high_f32(c);
			const float32x2_t v3wz = vrev64_f32(v3zw);
			const float32x2_t v3xy = vget_low_f32(c);
			const float32x2_t v3wy = vbsl_f32(select, v3wz, v3xy);

			float32x4_t temp1 = vcombine_f32(v2zw, v2yz);
			float32x4_t temp2 = vcombine_f32(v3wz, v3wy);
			float32x4_t res = vmulq_f32(temp1, temp2);

			// - bwzwy * czwyz
			const float32x2_t v2wy = vbsl_f32(select, v2wz, v2xy);

			const float32x2_t v3yx = vrev64_f32(v3xy);
			const float32x2_t v3yz = vbsl_f32(select, v3yx, v3wz);

			temp1 = vcombine_f32(v2wz, v2wy);
			temp2 = vcombine_f32(v3zw, v3yz);
			res = vmlsq_f32(res, temp1, temp2);

			// term1 * ayxxx
			const float32x2_t v1xy = vget_low_f32(a);
			const float32x2_t v1yx = vrev64_f32(v1xy);

			temp1 = vcombine_f32(v1yx, vdup_lane_f32(v1yx, 1));
			res = vmulq_f32(res, temp1);

			// Term2: bywxz * cwxwx
			const float32x2_t v2yw = vrev64_f32(v2wy);
			const float32x2_t v2xz = vbsl_f32(select, v2xy, v2wz);

			const float32x2_t v3wx = vbsl_f32(select, v3wz, v3yx);

			temp1 = vcombine_f32(v2yw, v2xz);
			temp2 = vcombine_f32(v3wx, v3wx);
			float32x4_t term = vmulq_f32(temp1, temp2);

			// - bwxwx * cywxz
			const float32x2_t v2wx = vbsl_f32(select, v2wz, v2yx);

			const float32x2_t v3yw = vrev64_f32(v3wy);
			const float32x2_t v3xz = vbsl_f32(select, v3xy, v3wz);

			temp1 = vcombine_f32(v2wx, v2wx);
			temp2 = vcombine_f32(v3yw, v3xz);
			term = vmlsq_f32(term, temp1, temp2);

			// res - term2 * azzyy
			const float32x2_t v1zw = vget_high_f32(a);

			temp1 = vcombine_f32(vdup_lane_f32(v1zw, 0), vdup_lane_f32(v1yx, 0));
			res = vmlsq_f32(res, term, temp1);

			// Term3: byzxy * czxyx
			const float32x2_t v3zx = vrev64_f32(v3xz);

			temp1 = vcombine_f32(v2yz, v2xy);
			temp2 = vcombine_f32(v3zx, v3yx);
			term = vmulq_f32(temp1, temp2);

			// - bzxyx * cyzxy
			const float32x2_t v2zx = vrev64_f32(v2xz);

			temp1 = vcombine_f32(v2zx, v2yx);
			temp2 = vcombine_f32(v3yz, v3xy);
			term = vmlsq_f32(term, temp1, temp2);

			// res + term3 * awwwz
			const float32x2_t v1wz = vrev64_f32(v1zw);

			temp1 = vcombine_f32(vdup_lane_f32(v1wz, 0), v1wz);
			return vmlaq_f32(res, term, temp1);
#else
#error "Not implemented."
#endif
		}
		inline float4 LUNA_SIMD_CALL normalize2_f4(float4 a)
		{
			float4 reciprocal_length = rsqrt_f4(dot2v_f4(a, a));
			return mul_f4(a, reciprocal_length);
		}
		inline float4 LUNA_SIMD_CALL normalize3_f4(float4 a)
		{
			float4 reciprocal_length = rsqrt_f4(dot3v_f4(a, a));
			return mul_f4(a, reciprocal_length);
		}
		inline float4 LUNA_SIMD_CALL normalize4_f4(float4 a)
		{
			float4 reciprocal_length = rsqrt_f4(dot4v_f4(a, a));
			return mul_f4(a, reciprocal_length);
		}
		inline float4 LUNA_SIMD_CALL reflect2_f4(float4 i, float4 n)
		{
			float4 proj = dot2v_f4(i, n);
			proj = add_f4(proj, proj);
			return negmuladd_f4(proj, n, i);
		}
		inline float4 LUNA_SIMD_CALL reflect3_f4(float4 i, float4 n)
		{
			float4 proj = dot3v_f4(i, n);
			proj = add_f4(proj, proj);
			return negmuladd_f4(proj, n, i);
		}
		inline float4 LUNA_SIMD_CALL reflect4_f4(float4 i, float4 n)
		{
			float4 proj = dot4v_f4(i, n);
			proj = add_f4(proj, proj);
			return negmuladd_f4(proj, n, i);
		}
		inline float4 LUNA_SIMD_CALL refract2_f4(float4 i, float4 n, f32 index)
		{
			float4 indexv = dup_f4(index);
			float4 one = dup_f4(1.0f);
			// PROJ := DOT2(i, n)
			float4 proj = dot2v_f4(i, n);
			// DETER := 1.0 - index * index  * (1.0 - PROJ * PROJ)
			float4 deter = negmuladd_f4(proj, proj, one);
			deter = mul_f4(deter, indexv);
			deter = negmuladd_f4(deter, indexv, one);
			// Check negative.
			int4 zero_mask = cmpgt_f4(deter, setzero_f4());
			if (maskint_i4(zero_mask) == 0) return dup_f4(0.0f);
			// dst := index * i - n * (index * PROJ + SQRT(DETER))
			float4 res = sqrt_f4(deter);
			res = muladd_f4(proj, indexv, res);
			res = negmuladd_f4(res, n, mul_f4(i, indexv));
			return res;
		}
		inline float4 LUNA_SIMD_CALL refract3_f4(float4 i, float4 n, f32 index)
		{
			float4 indexv = dup_f4(index);
			float4 one = dup_f4(1.0f);
			// PROJ := DOT2(i, n)
			float4 proj = dot3v_f4(i, n);
			// DETER := 1.0 - index * index  * (1.0 - PROJ * PROJ)
			float4 deter = negmuladd_f4(proj, proj, one);
			deter = mul_f4(deter, indexv);
			deter = negmuladd_f4(deter, indexv, one);
			// Check negative.
			int4 zero_mask = cmpgt_f4(deter, setzero_f4());
			if (maskint_i4(zero_mask) == 0) return dup_f4(0.0f);
			// dst := index * i - n * (index * PROJ + SQRT(DETER))
			float4 res = sqrt_f4(deter);
			res = muladd_f4(proj, indexv, res);
			res = negmuladd_f4(res, n, mul_f4(i, indexv));
			return res;
		}
		inline float4 LUNA_SIMD_CALL refract4_f4(float4 i, float4 n, f32 index)
		{
			float4 indexv = dup_f4(index);
			float4 one = dup_f4(1.0f);
			// PROJ := DOT2(i, n)
			float4 proj = dot4v_f4(i, n);
			// DETER := 1.0 - index * index  * (1.0 - PROJ * PROJ)
			float4 deter = negmuladd_f4(proj, proj, one);
			deter = mul_f4(deter, indexv);
			deter = negmuladd_f4(deter, indexv, one);
			// Check negative.
			int4 zero_mask = cmpgt_f4(deter, setzero_f4());
			if (maskint_i4(zero_mask) == 0) return dup_f4(0.0f);
			// dst := index * i - n * (index * PROJ + SQRT(DETER))
			float4 res = sqrt_f4(deter);
			res = muladd_f4(proj, indexv, res);
			res = negmuladd_f4(res, n, mul_f4(i, indexv));
			return res;
		}
		inline float4 LUNA_SIMD_CALL lerp_f4(float4 a, float4 b, f32 t)
		{
			float4 l = sub_f4(b, a);
			return scaleadd_f4(l, t, a);
		}
		inline float4 LUNA_SIMD_CALL lerpv_f4(float4 a, float4 b, float4 t)
		{
			float4 l = sub_f4(b, a);
			return muladd_f4(l, t, a);
		}
		inline float4 LUNA_SIMD_CALL barycentric_f4(float4 a, float4 b, float4 c, f32 f, f32 g)
		{
			float4 r1 = sub_f4(b, a);
			float4 r2 = sub_f4(c, a);
			float4 res = scaleadd_f4(r1, f, a);
			res = scaleadd_f4(r2, g, res);
			return res;
		}
		inline float4 LUNA_SIMD_CALL catmull_rom_f4(float4 a, float4 b, float4 c, float4 d, f32 t)
		{
			f32 t2 = t * t;
			f32 t3 = t * t2;
			f32 p0 = (-t3 + 2.0f * t2 - t) * 0.5f;
			f32 p1 = (3.0f * t3 - 5.0f * t2 + 2.0f) * 0.5f;
			f32 p2 = (-3.0f * t3 + 4.0f * t2 + t) * 0.5f;
			f32 p3 = (t3 - t2) * 0.5f;
			float4 res = scale_f4(a, p0);
			res = scaleadd_f4(b, p1, res);
			res = scaleadd_f4(c, p2, res);
			res = scaleadd_f4(d, p3, res);
			return res;
		}
		inline float4 LUNA_SIMD_CALL hermite_f4(float4 v0, float4 t0, float4 v1, float4 t1, f32 t)
		{
			f32 f2 = t * t;
			f32 f3 = t * f2;
			f32 p0 = 2.0f * f3 - 3.0f * f2 + 1.0f;
			f32 pt0 = f3 - 2.0f * f2 + t;
			f32 p1 = -2.0f * f3 + 3.0f * f2;
			f32 pt1 = f3 - f2;
			float4 res = scale_f4(v0, p0);
			res = scaleadd_f4(t0, pt0, res);
			res = scaleadd_f4(v1, p1, res);
			res = scaleadd_f4(t1, pt1, res);
			return res;
		}
		template <u32 _SelectX, u32 _SelectY, u32 _SelectZ, u32 _SelectW>
		inline float4 LUNA_SIMD_CALL permute_f4(float4 a)
		{
			static_assert(_SelectX <= 3, "Bad _SelectX Value.");
			static_assert(_SelectY <= 3, "Bad _SelectY Value.");
			static_assert(_SelectZ <= 3, "Bad _SelectZ Value.");
			static_assert(_SelectW <= 3, "Bad _SelectW Value.");
#if defined(LUNA_AVX_INTRINSICS)
			return _mm_permute_ps(a, _MM_SHUFFLE((int)_SelectW, (int)_SelectZ, (int)_SelectY, (int)_SelectX));
#elif defined(LUNA_SSE2_INTRINSICS)
			return _mm_shuffle_ps(a, a, _MM_SHUFFLE((int)_SelectW, (int)_SelectZ, (int)_SelectY, (int)_SelectX));
#elif defined(LUNA_NEON_INTRINSICS)
			static const u32 ControlElement[4] =
			{
				0x03020100,
				0x07060504,
				0x0B0A0908,
				0x0F0E0D0C,
			};
			uint8x8x2_t tbl;
			tbl.val[0] = vreinterpret_u8_f32(vget_low_f32(a));
			tbl.val[1] = vreinterpret_u8_f32(vget_high_f32(a));
			uint32x2_t idx = vcreate_u32(static_cast<u64>(ControlElement[_SelectX]) | (static_cast<u64>(ControlElement[_SelectY]) << 32));
			const uint8x8_t rL = vtbl2_u8(tbl, vreinterpret_u8_u32(idx));
			idx = vcreate_u32(static_cast<u64>(ControlElement[_SelectZ]) | (static_cast<u64>(ControlElement[_SelectW]) << 32));
			const uint8x8_t rH = vtbl2_u8(tbl, vreinterpret_u8_u32(idx));
			return vcombine_f32(vreinterpret_f32_u8(rL), vreinterpret_f32_u8(rH));
#else 
#error "Not implemented."
#endif
		}
		template <u32 _SelectX, u32 _SelectY, u32 _SelectZ, u32 _SelectW>
		inline float4 LUNA_SIMD_CALL select_f4(float4 a, float4 b)
		{
			static_assert(_SelectX <= 1, "Bad _SelectX Value.");
			static_assert(_SelectY <= 1, "Bad _SelectY Value.");
			static_assert(_SelectZ <= 1, "Bad _SelectZ Value.");
			static_assert(_SelectW <= 1, "Bad _SelectW Value.");
#if defined(LUNA_SSE2_INTRINSICS)
			__m128 control = casti_f4(set_i4(
				_SelectX ? 0xFFFFFFFF : 0,
				_SelectY ? 0xFFFFFFFF : 0,
				_SelectZ ? 0xFFFFFFFF : 0,
				_SelectW ? 0xFFFFFFFF : 0));
			return _mm_or_ps(_mm_andnot_ps(control, a), _mm_and_ps(b, control));
#elif defined(LUNA_NEON_INTRINSICS)
            uint32x2_t t0 = vcreate_u32(static_cast<u64>(_SelectX ? 0xFFFFFFFF : 0) | (static_cast<u64>(_SelectY ? 0xFFFFFFFF : 0) << 32));
            uint32x2_t t1 = vcreate_u32(static_cast<u64>(_SelectZ ? 0xFFFFFFFF : 0) | (static_cast<u64>(_SelectW ? 0xFFFFFFFF : 0) << 32));
            uint32x4_t control = vcombine_u32(t0, t1);
			return vbslq_f32(control, b, a);
#else 
#error "Not implemented."
#endif
		}

		template <u32 _SelectX, u32 _SelectY, u32 _SelectZ, u32 _SelectW>
		inline float4 LUNA_SIMD_CALL permute2_f4(float4 a, float4 b)
		{
			static_assert(_SelectX <= 7, "Bad _SelectX Value.");
			static_assert(_SelectY <= 7, "Bad _SelectY Value.");
			static_assert(_SelectZ <= 7, "Bad _SelectZ Value.");
			static_assert(_SelectW <= 7, "Bad _SelectW Value.");
			float4 temp1 = permute_f4<_SelectX & 3, _SelectY & 3, _SelectZ & 3, _SelectW & 3>(a);
			float4 temp2 = permute_f4<_SelectX & 3, _SelectY & 3, _SelectZ & 3, _SelectW & 3>(b);
			return select_f4<(_SelectX > 3 ? 1 : 0), (_SelectY > 3 ? 1 : 0), (_SelectZ > 3 ? 1 : 0), (_SelectW > 3 ? 1 : 0)>
				(temp1, temp2);
		}

		inline float3x4 LUNA_SIMD_CALL load_f3x4(f32 const* mem_addr)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_loadu_ps(mem_addr);
			res.r[1] = _mm256_castps128_ps256(_mm_load_ps(mem_addr + 8));
			return res;
#else
			float3x4 res;
			res.r[0] = load_f4(mem_addr);
			res.r[1] = load_f4(mem_addr + 4);
			res.r[2] = load_f4(mem_addr + 8);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL castf3x4_f4x4(float3x4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = a.r[0];
			res.r[1] = a.r[1];
			return res;
#else
			float4x4 res;
			res.r[0] = a.r[0];
			res.r[1] = a.r[1];
			res.r[2] = a.r[2];
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL castf4x4_f3x4(float4x4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = a.r[0];
			res.r[1] = a.r[1];
			return res;
#else
			float3x4 res;
			res.r[0] = a.r[0];
			res.r[1] = a.r[1];
			res.r[2] = a.r[2];
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL load_f4x4(f32 const* mem_addr)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_loadu_ps(mem_addr);
			res.r[1] = _mm256_loadu_ps(mem_addr + 8);
			return res;
#else
			float4x4 res;
			res.r[0] = load_f4(mem_addr);
			res.r[1] = load_f4(mem_addr + 4);
			res.r[2] = load_f4(mem_addr + 8);
			res.r[3] = load_f4(mem_addr + 12);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL setf4_f4x4(float4 r0, float4 r1, float4 r2, float4 r3)
		{
			float4x4 res;
#if defined(LUNA_AVX_INTRINSICS)
			res.r[0] = _mm256_castps128_ps256(r0);
			res.r[0] = _mm256_insertf128_ps(res.r[0], r1, 1);
			res.r[1] = _mm256_castps128_ps256(r2);
			res.r[1] = _mm256_insertf128_ps(res.r[1], r3, 1);
#else
			res.r[0] = r0;
			res.r[1] = r1;
			res.r[2] = r2;
			res.r[3] = r3;
#endif
			return res;
		}
		inline void LUNA_SIMD_CALL store_f3x4(f32* mem_addr, float3x4 m)
		{
#if defined(LUNA_AVX_INTRINSICS)
			_mm256_storeu_ps(mem_addr, m.r[0]);
			_mm_store_ps(mem_addr + 8, _mm256_castps256_ps128(m.r[1]));
#else
			store_f4(mem_addr, m.r[0]);
			store_f4(mem_addr + 4, m.r[1]);
			store_f4(mem_addr + 8, m.r[2]);
#endif
		}
		inline void LUNA_SIMD_CALL store_f4x4(f32* mem_addr, float4x4 m)
		{
#if defined(LUNA_AVX_INTRINSICS)
			_mm256_storeu_ps(mem_addr, m.r[0]);
			_mm256_storeu_ps(mem_addr + 8, m.r[1]);
#else
			store_f4(mem_addr, m.r[0]);
			store_f4(mem_addr + 4, m.r[1]);
			store_f4(mem_addr + 8, m.r[2]);
			store_f4(mem_addr + 12, m.r[3]);
#endif
		}
		inline float3x4 LUNA_SIMD_CALL setzero_f3x4()
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_setzero_ps();
			res.r[1] = _mm256_setzero_ps();
			return res;
#else
			float3x4 res;
			res.r[0] = setzero_f4();
			res.r[1] = setzero_f4();
			res.r[2] = setzero_f4();
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL setzero_f4x4()
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_setzero_ps();
			res.r[1] = _mm256_setzero_ps();
			return res;
#else
			float4x4 res;
			res.r[0] = setzero_f4();
			res.r[1] = setzero_f4();
			res.r[2] = setzero_f4();
			res.r[3] = setzero_f4();
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL dup_f3x4(f32 e0)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_set1_ps(e0);
			res.r[1] = _mm256_set1_ps(e0);
			return res;
#else
			float3x4 res;
			res.r[0] = dup_f4(e0);
			res.r[1] = dup_f4(e0);
			res.r[2] = dup_f4(e0);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL dup_f4x4(f32 e0)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_set1_ps(e0);
			res.r[1] = _mm256_set1_ps(e0);
			return res;
#else
			float4x4 res;
			res.r[0] = dup_f4(e0);
			res.r[1] = dup_f4(e0);
			res.r[2] = dup_f4(e0);
			res.r[3] = dup_f4(e0);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL add_f3x4(float3x4 a, float3x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_add_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_add_ps(a.r[1], b.r[1]);
			return res;
#else
			float3x4 res;
			res.r[0] = add_f4(a.r[0], b.r[0]);
			res.r[1] = add_f4(a.r[1], b.r[1]);
			res.r[2] = add_f4(a.r[2], b.r[2]);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL add_f4x4(float4x4 a, float4x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_add_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_add_ps(a.r[1], b.r[1]);
			return res;
#else
			float4x4 res;
			res.r[0] = add_f4(a.r[0], b.r[0]);
			res.r[1] = add_f4(a.r[1], b.r[1]);
			res.r[2] = add_f4(a.r[2], b.r[2]);
			res.r[3] = add_f4(a.r[3], b.r[3]);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL sub_f3x4(float3x4 a, float3x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_sub_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_sub_ps(a.r[1], b.r[1]);
			return res;
#else
			float3x4 res;
			res.r[0] = sub_f4(a.r[0], b.r[0]);
			res.r[1] = sub_f4(a.r[1], b.r[1]);
			res.r[2] = sub_f4(a.r[2], b.r[2]);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL sub_f4x4(float4x4 a, float4x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_sub_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_sub_ps(a.r[1], b.r[1]);
			return res;
#else
			float4x4 res;
			res.r[0] = sub_f4(a.r[0], b.r[0]);
			res.r[1] = sub_f4(a.r[1], b.r[1]);
			res.r[2] = sub_f4(a.r[2], b.r[2]);
			res.r[3] = sub_f4(a.r[3], b.r[3]);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL mul_f3x4(float3x4 a, float3x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_mul_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_mul_ps(a.r[1], b.r[1]);
			return res;
#else
			float3x4 res;
			res.r[0] = mul_f4(a.r[0], b.r[0]);
			res.r[1] = mul_f4(a.r[1], b.r[1]);
			res.r[2] = mul_f4(a.r[2], b.r[2]);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL mul_f4x4(float4x4 a, float4x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_mul_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_mul_ps(a.r[1], b.r[1]);
			return res;
#else
			float4x4 res;
			res.r[0] = mul_f4(a.r[0], b.r[0]);
			res.r[1] = mul_f4(a.r[1], b.r[1]);
			res.r[2] = mul_f4(a.r[2], b.r[2]);
			res.r[3] = mul_f4(a.r[3], b.r[3]);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL div_f3x4(float3x4 a, float3x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			res.r[0] = _mm256_div_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_div_ps(a.r[1], b.r[1]);
			return res;
#else
			float3x4 res;
			res.r[0] = div_f4(a.r[0], b.r[0]);
			res.r[1] = div_f4(a.r[1], b.r[1]);
			res.r[2] = div_f4(a.r[2], b.r[2]);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL div_f4x4(float4x4 a, float4x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			res.r[0] = _mm256_div_ps(a.r[0], b.r[0]);
			res.r[1] = _mm256_div_ps(a.r[1], b.r[1]);
			return res;
#else
			float4x4 res;
			res.r[0] = div_f4(a.r[0], b.r[0]);
			res.r[1] = div_f4(a.r[1], b.r[1]);
			res.r[2] = div_f4(a.r[2], b.r[2]);
			res.r[3] = div_f4(a.r[3], b.r[3]);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL add_f3x4_f1(float3x4 a, f32 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			__m256 v = _mm256_set1_ps(b);
			res.r[0] = _mm256_add_ps(a.r[0], v);
			res.r[1] = _mm256_add_ps(a.r[1], v);
			return res;
#else
			float3x4 res;
			float4 v = dup_f4(b);
			res.r[0] = add_f4(a.r[0], v);
			res.r[1] = add_f4(a.r[1], v);
			res.r[2] = add_f4(a.r[2], v);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL add_f4x4_f1(float4x4 a, f32 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			__m256 v = _mm256_set1_ps(b);
			res.r[0] = _mm256_add_ps(a.r[0], v);
			res.r[1] = _mm256_add_ps(a.r[1], v);
			return res;
#else
			float4x4 res;
			float4 v = dup_f4(b);
			res.r[0] = add_f4(a.r[0], v);
			res.r[1] = add_f4(a.r[1], v);
			res.r[2] = add_f4(a.r[2], v);
			res.r[3] = add_f4(a.r[3], v);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL sub_f3x4_f1(float3x4 a, f32 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			__m256 v = _mm256_set1_ps(b);
			res.r[0] = _mm256_sub_ps(a.r[0], v);
			res.r[1] = _mm256_sub_ps(a.r[1], v);
			return res;
#else
			float3x4 res;
			float4 v = dup_f4(b);
			res.r[0] = sub_f4(a.r[0], v);
			res.r[1] = sub_f4(a.r[1], v);
			res.r[2] = sub_f4(a.r[2], v);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL sub_f4x4_f1(float4x4 a, f32 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			__m256 v = _mm256_set1_ps(b);
			res.r[0] = _mm256_sub_ps(a.r[0], v);
			res.r[1] = _mm256_sub_ps(a.r[1], v);
			return res;
#else
			float4x4 res;
			float4 v = dup_f4(b);
			res.r[0] = sub_f4(a.r[0], v);
			res.r[1] = sub_f4(a.r[1], v);
			res.r[2] = sub_f4(a.r[2], v);
			res.r[3] = sub_f4(a.r[3], v);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL sub_f1_f3x4(f32 a, float3x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			__m256 v = _mm256_set1_ps(a);
			res.r[0] = _mm256_sub_ps(v, b.r[0]);
			res.r[1] = _mm256_sub_ps(v, b.r[1]);
			return res;
#else
			float3x4 res;
			float4 v = dup_f4(a);
			res.r[0] = sub_f4(v, b.r[0]);
			res.r[1] = sub_f4(v, b.r[1]);
			res.r[2] = sub_f4(v, b.r[2]);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL sub_f1_f4x4(f32 a, float4x4 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			__m256 v = _mm256_set1_ps(a);
			res.r[0] = _mm256_sub_ps(v, b.r[0]);
			res.r[1] = _mm256_sub_ps(v, b.r[1]);
			return res;
#else
			float4x4 res;
			float4 v = dup_f4(a);
			res.r[0] = sub_f4(v, b.r[0]);
			res.r[1] = sub_f4(v, b.r[1]);
			res.r[2] = sub_f4(v, b.r[2]);
			res.r[3] = sub_f4(v, b.r[3]);
			return res;
#endif
		}
		inline float3x4 LUNA_SIMD_CALL mul_f3x4_f1(float3x4 a, f32 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float3x4 res;
			__m256 v = _mm256_set1_ps(b);
			res.r[0] = _mm256_mul_ps(a.r[0], v);
			res.r[1] = _mm256_mul_ps(a.r[1], v);
			return res;
#else
			float3x4 res;
			float4 v = dup_f4(b);
			res.r[0] = mul_f4(a.r[0], v);
			res.r[1] = mul_f4(a.r[1], v);
			res.r[2] = mul_f4(a.r[2], v);
			return res;
#endif
		}
		inline float4x4 LUNA_SIMD_CALL mul_f4x4_f1(float4x4 a, f32 b)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4x4 res;
			__m256 v = _mm256_set1_ps(b);
			res.r[0] = _mm256_mul_ps(a.r[0], v);
			res.r[1] = _mm256_mul_ps(a.r[1], v);
			return res;
#else
			float4x4 res;
			float4 v = dup_f4(b);
			res.r[0] = mul_f4(a.r[0], v);
			res.r[1] = mul_f4(a.r[1], v);
			res.r[2] = mul_f4(a.r[2], v);
			res.r[3] = mul_f4(a.r[3], v);
			return res;
#endif
		}
		namespace Impl
		{
#ifdef LUNA_AVX_INTRINSICS
			LUNA_FORCEINLINE __m256 LUNA_SIMD_CALL negmuladd_f8(__m256 a, __m256 b, __m256 c)
			{
#ifdef LUNA_FMA3_INTRINSICS
				return _mm256_fnmadd_ps(a, b, c);
#else
				return _mm256_sub_ps(c, _mm256_mul_ps(a, b));
#endif
			}
			LUNA_FORCEINLINE __m256 LUNA_SIMD_CALL muladd_f8(__m256 a, __m256 b, __m256 c)
			{
#ifdef LUNA_FMA3_INTRINSICS
				return _mm256_fmadd_ps(a, b, c);
#else
				return _mm256_add_ps(_mm256_mul_ps(a, b), c);
#endif
			}
#endif

		}
		inline float3x4 LUNA_SIMD_CALL matmul_f3x3(float3x4 a, float3x4 b)
		{
		// dst[0].x := a[0].x * b[0].x + a[0].y * b[1].x + a[0].z * b[2].x
		// dst[0].y := a[0].x * b[0].y + a[0].y * b[1].y + a[0].z * b[2].y
		// dst[0].z := a[0].x * b[0].z + a[0].y * b[1].z + a[0].z * b[2].z
		// 
		// dst[1].x := a[1].x * b[0].x + a[1].y * b[1].x + a[1].z * b[2].x
		// dst[1].y := a[1].x * b[0].y + a[1].y * b[1].y + a[1].z * b[2].y
		// dst[1].z := a[1].x * b[0].z + a[1].y * b[1].z + a[1].z * b[2].z
		// 
		// dst[2].x := a[2].x * b[0].x + a[2].y * b[1].x + a[2].z * b[2].x
		// dst[2].y := a[2].x * b[0].y + a[2].y * b[1].y + a[2].z * b[2].y
		// dst[2].z := a[2].x * b[0].z + a[2].y * b[1].z + a[2].z * b[2].z
#if defined(LUNA_AVX_INTRINSICS)
			// b0x, b0y, b0z, b0w, b0x, b0y, b0z, b0w
			__m256 B0xyz = _mm256_permute2f128_ps(b.r[0], b.r[0], 0);
			__m256 B1xyz = _mm256_permute2f128_ps(b.r[0], b.r[0], 0x11);
			__m256 B2xyz = _mm256_permute2f128_ps(b.r[1], b.r[1], 0);

			// a0x, a0x, a0x, a0x, a1x, a1x, a1x, a1x
			__m256 temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(0, 0, 0, 0));
			__m256 R01 = _mm256_mul_ps(temp, B0xyz);
			temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(1, 1, 1, 1));
			R01 = Impl::muladd_f8(temp, B1xyz, R01);
			temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(2, 2, 2, 2));
			R01 = Impl::muladd_f8(temp, B2xyz, R01);

			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(0, 0, 0, 0));
			__m256 R2 = _mm256_mul_ps(temp, B0xyz);
			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(1, 1, 1, 1));
			R2 = Impl::muladd_f8(temp, B1xyz, R2);
			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(2, 2, 2, 2));
			R2 = Impl::muladd_f8(temp, B2xyz, R2);

			float3x4 ret;
			ret.r[0] = R01;
			ret.r[1] = R2;
			return ret;
#else
			float4 temp = permute_f4<0, 0, 0, 0>(a.r[0]);
			float4 R0 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[0]);
			R0 = muladd_f4(temp, b.r[1], R0);
			temp = permute_f4<2, 2, 2, 2>(a.r[0]);
			R0 = muladd_f4(temp, b.r[2], R0);

			temp = permute_f4<0, 0, 0, 0>(a.r[1]);
			float4 R1 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[1]);
			R1 = muladd_f4(temp, b.r[1], R1);
			temp = permute_f4<2, 2, 2, 2>(a.r[1]);
			R1 = muladd_f4(temp, b.r[2], R1);

			temp = permute_f4<0, 0, 0, 0>(a.r[2]);
			float4 R2 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[2]);
			R2 = muladd_f4(temp, b.r[1], R2);
			temp = permute_f4<2, 2, 2, 2>(a.r[2]);
			R2 = muladd_f4(temp, b.r[2], R2);

			float3x4 ret;
			ret.r[0] = R0;
			ret.r[1] = R1;
			ret.r[2] = R2;
			return ret;
#endif
		}

		inline float4x4 LUNA_SIMD_CALL matmul_f4x4(float4x4 a, float4x4 b)
		{
			// dst[0].x := a[0].x * b[0].x + a[0].y * b[1].x + a[0].z * b[2].x + a[0].w * b[3].x
			// dst[0].y := a[0].x * b[0].y + a[0].y * b[1].y + a[0].z * b[2].y + a[0].w * b[3].y
			// dst[0].z := a[0].x * b[0].z + a[0].y * b[1].z + a[0].z * b[2].z + a[0].w * b[3].z
			// dst[0].w := a[0].x * b[0].w + a[0].y * b[1].w + a[0].z * b[2].w + a[0].w * b[3].w
			// 
			// dst[1].x := a[1].x * b[0].x + a[1].y * b[1].x + a[1].z * b[2].x + a[1].w * b[3].x
			// dst[1].y := a[1].x * b[0].y + a[1].y * b[1].y + a[1].z * b[2].y + a[1].w * b[3].y
			// dst[1].z := a[1].x * b[0].z + a[1].y * b[1].z + a[1].z * b[2].z + a[1].w * b[3].z
			// dst[1].w := a[1].x * b[0].w + a[1].y * b[1].w + a[1].z * b[2].w + a[1].w * b[3].w
			// 
			// dst[2].x := a[2].x * b[0].x + a[2].y * b[1].x + a[2].z * b[2].x + a[2].w * b[3].x
			// dst[2].y := a[2].x * b[0].y + a[2].y * b[1].y + a[2].z * b[2].y + a[2].w * b[3].y
			// dst[2].z := a[2].x * b[0].z + a[2].y * b[1].z + a[2].z * b[2].z + a[2].w * b[3].z
			// dst[2].w := a[2].x * b[0].w + a[2].y * b[1].w + a[2].z * b[2].w + a[2].w * b[3].w
			// 
			// dst[3].x := a[3].x * b[0].x + a[3].y * b[1].x + a[3].z * b[2].x + a[3].w * b[3].x
			// dst[3].y := a[3].x * b[0].y + a[3].y * b[1].y + a[3].z * b[2].y + a[3].w * b[3].y
			// dst[3].z := a[3].x * b[0].z + a[3].y * b[1].z + a[3].z * b[2].z + a[3].w * b[3].z
			// dst[3].w := a[3].x * b[0].w + a[3].y * b[1].w + a[3].z * b[2].w + a[3].w * b[3].w

#if defined(LUNA_AVX_INTRINSICS)
			__m256 B0xyz = _mm256_permute2f128_ps(b.r[0], b.r[0], 0);
			__m256 B1xyz = _mm256_permute2f128_ps(b.r[0], b.r[0], 0x11);
			__m256 B2xyz = _mm256_permute2f128_ps(b.r[1], b.r[1], 0);
			__m256 B3xyz = _mm256_permute2f128_ps(b.r[1], b.r[1], 0x11);

			__m256 temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(0, 0, 0, 0));
			__m256 R01 = _mm256_mul_ps(temp, B0xyz);
			temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(1, 1, 1, 1));
			R01 = Impl::muladd_f8(temp, B1xyz, R01);
			temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(2, 2, 2, 2));
			R01 = Impl::muladd_f8(temp, B2xyz, R01);
			temp = _mm256_permute_ps(a.r[0], _MM_SHUFFLE(3, 3, 3, 3));
			R01 = Impl::muladd_f8(temp, B3xyz, R01);

			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(0, 0, 0, 0));
			__m256 R23 = _mm256_mul_ps(temp, B0xyz);
			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(1, 1, 1, 1));
			R23 = Impl::muladd_f8(temp, B1xyz, R23);
			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(2, 2, 2, 2));
			R23 = Impl::muladd_f8(temp, B2xyz, R23);
			temp = _mm256_permute_ps(a.r[1], _MM_SHUFFLE(3, 3, 3, 3));
			R23 = Impl::muladd_f8(temp, B3xyz, R23);

			float4x4 ret;
			ret.r[0] = R01;
			ret.r[1] = R23;
			return ret;
#else
			float4 temp = permute_f4<0, 0, 0, 0>(a.r[0]);
			float4 R0 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[0]);
			R0 = muladd_f4(temp, b.r[1], R0);
			temp = permute_f4<2, 2, 2, 2>(a.r[0]);
			R0 = muladd_f4(temp, b.r[2], R0);
			temp = permute_f4<3, 3, 3, 3>(a.r[0]);
			R0 = muladd_f4(temp, b.r[3], R0);

			temp = permute_f4<0, 0, 0, 0>(a.r[1]);
			float4 R1 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[1]);
			R1 = muladd_f4(temp, b.r[1], R1);
			temp = permute_f4<2, 2, 2, 2>(a.r[1]);
			R1 = muladd_f4(temp, b.r[2], R1);
			temp = permute_f4<3, 3, 3, 3>(a.r[1]);
			R1 = muladd_f4(temp, b.r[3], R1);

			temp = permute_f4<0, 0, 0, 0>(a.r[2]);
			float4 R2 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[2]);
			R2 = muladd_f4(temp, b.r[1], R2);
			temp = permute_f4<2, 2, 2, 2>(a.r[2]);
			R2 = muladd_f4(temp, b.r[2], R2);
			temp = permute_f4<3, 3, 3, 3>(a.r[2]);
			R2 = muladd_f4(temp, b.r[3], R2);

			temp = permute_f4<0, 0, 0, 0>(a.r[3]);
			float4 R3 = mul_f4(temp, b.r[0]);
			temp = permute_f4<1, 1, 1, 1>(a.r[3]);
			R3 = muladd_f4(temp, b.r[1], R3);
			temp = permute_f4<2, 2, 2, 2>(a.r[3]);
			R3 = muladd_f4(temp, b.r[2], R3);
			temp = permute_f4<3, 3, 3, 3>(a.r[3]);
			R3 = muladd_f4(temp, b.r[3], R3);

			float4x4 ret;
			ret.r[0] = R0;
			ret.r[1] = R1;
			ret.r[2] = R2;
			ret.r[3] = R3;
			return ret;
#endif
		}

		inline float4x4 LUNA_SIMD_CALL transpose_f4x4(float4x4 src)
		{
#if defined(LUNA_AVX_INTRINSICS)
			__m256 t0 = src.r[0];
			__m256 t1 = src.r[1];
			__m256 temp1 = _mm256_unpacklo_ps(t0, t1);
			__m256 temp2 = _mm256_unpackhi_ps(t0, t1);
			__m256 temp3 = _mm256_permute2f128_ps(temp1, temp2, 0x20);
			__m256 temp4 = _mm256_permute2f128_ps(temp1, temp2, 0x31);
			temp1 = _mm256_unpacklo_ps(temp3, temp4);
			temp2 = _mm256_unpackhi_ps(temp3, temp4);
			t0 = _mm256_permute2f128_ps(temp1, temp2, 0x20);
			t1 = _mm256_permute2f128_ps(temp1, temp2, 0x31);
			float4x4 res;
			res.r[0] = t0;
			res.r[1] = t1;
			return res;
#elif defined(LUNA_SSE2_INTRINSICS)
			// x.x,x.y,y.x,y.y
			__m128 temp1 = _mm_shuffle_ps(src.r[0], src.r[1], _MM_SHUFFLE(1, 0, 1, 0));
			// x.z,x.w,y.z,y.w
			__m128 temp3 = _mm_shuffle_ps(src.r[0], src.r[1], _MM_SHUFFLE(3, 2, 3, 2));
			// z.x,z.y,w.x,w.y
			__m128 temp2 = _mm_shuffle_ps(src.r[2], src.r[3], _MM_SHUFFLE(1, 0, 1, 0));
			// z.z,z.w,w.z,w.w
			__m128 temp4 = _mm_shuffle_ps(src.r[2], src.r[3], _MM_SHUFFLE(3, 2, 3, 2));
			float4x4 res;
			// x.x,y.x,z.x,w.x
			res.r[0] = _mm_shuffle_ps(temp1, temp2, _MM_SHUFFLE(2, 0, 2, 0));
			// x.y,y.y,z.y,w.y
			res.r[1] = _mm_shuffle_ps(temp1, temp2, _MM_SHUFFLE(3, 1, 3, 1));
			// x.z,y.z,z.z,w.z
			res.r[2] = _mm_shuffle_ps(temp3, temp4, _MM_SHUFFLE(2, 0, 2, 0));
			// x.w,y.w,z.w,w.w
			res.r[3] = _mm_shuffle_ps(temp3, temp4, _MM_SHUFFLE(3, 1, 3, 1));
			return res;
#elif defined(LUNA_NEON_INTRINSICS)
			float32x4x2_t P0 = vzipq_f32(src.r[0], src.r[2]);
			float32x4x2_t P1 = vzipq_f32(src.r[1], src.r[3]);
			float32x4x2_t T0 = vzipq_f32(P0.val[0], P1.val[0]);
			float32x4x2_t T1 = vzipq_f32(P0.val[1], P1.val[1]);
			float4x4 res;
			res.r[0] = T0.val[0];
			res.r[1] = T0.val[1];
			res.r[2] = T1.val[0];
			res.r[3] = T1.val[1];
			return res;
#else
#error "Not implemented."
#endif
		}
		inline f32 LUNA_SIMD_CALL determinant_f3x3(float3x4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4 R0 = _mm256_castps256_ps128(a.r[0]);
			float4 R1 = _mm256_extractf128_ps(a.r[0], 1);
			float4 R2 = _mm256_castps256_ps128(a.r[1]);
#else
			float4 R0 = a.r[0];
			float4 R1 = a.r[1];
			float4 R2 = a.r[2];
#endif
			/*
			r0.x * (r1.y * r2.z - r1.z * r2.y) +
			r0.y * (r1.z * r2.x - r1.x * r2.z) +
			r0.z * (r1.x * r2.y - r1.y * r2.x);
			*/
			// r1.y, r1.z, r1.x, r1.w
			float4 V1 = permute_f4<1, 2, 0, 3>(R1);
			// r2.z, r2.x, r2.y, r2.w
			float4 V2 = permute_f4<2, 0, 1, 3>(R2);
			// r1.y * r2.z, r1.z * r2.x, r1.x * r2.y, r1.w * r2.w
			float4 res = mul_f4(V1, V2);
			// r1.z, r1.x, r1.y
			V1 = permute_f4<2, 0, 1, 3>(R1);
			// r2.y, r2.z, r2.x
			V2 = permute_f4<1, 2, 0, 3>(R2);
			// r1.y * r2.z - r1.z * r2.y, 
			// r1.z * r2.x - r1.x * r2.z, 
			// r1.x * r2.y - r1.y * r2.x
			res = negmuladd_f4(V1, V2, res);
			return dot3_f4(R0, res);
		}
		inline float4 LUNA_SIMD_CALL determinantv_f3x3(float3x4 a)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4 R0 = _mm256_castps256_ps128(a.r[0]);
			float4 R1 = _mm256_extractf128_ps(a.r[0], 1);
			float4 R2 = _mm256_castps256_ps128(a.r[1]);
#else
			float4 R0 = a.r[0];
			float4 R1 = a.r[1];
			float4 R2 = a.r[2];
#endif
			/*
			r0.x * (r1.y * r2.z - r1.z * r2.y) +
			r0.y * (r1.z * r2.x - r1.x * r2.z) +
			r0.z * (r1.x * r2.y - r1.y * r2.x);
			*/
			// r1.y, r1.z, r1.x, r1.w
			float4 V1 = permute_f4<1, 2, 0, 3>(R1);
			// r2.z, r2.x, r2.y, r2.w
			float4 V2 = permute_f4<2, 0, 1, 3>(R2);
			// r1.y * r2.z, r1.z * r2.x, r1.x * r2.y, r1.w * r2.w
			float4 res = mul_f4(V1, V2);
			// r1.z, r1.x, r1.y
			V1 = permute_f4<2, 0, 1, 3>(R1);
			// r2.y, r2.z, r2.x
			V2 = permute_f4<1, 2, 0, 3>(R2);
			// r1.y * r2.z - r1.z * r2.y, 
			// r1.z * r2.x - r1.x * r2.z, 
			// r1.x * r2.y - r1.y * r2.x
			res = negmuladd_f4(V1, V2, res);
			return dot3v_f4(R0, res);
		}
		inline float3x4 LUNA_SIMD_CALL inverse_f3x3(float3x4 a, f32* out_determinant)
		{
#if defined(LUNA_AVX_INTRINSICS)
			float4 R0 = _mm256_castps256_ps128(a.r[0]);
			float4 R1 = _mm256_extractf128_ps(a.r[0], 1);
			float4 R2 = _mm256_castps256_ps128(a.r[1]);
#else
			float4 R0 = a.r[0];
			float4 R1 = a.r[1];
			float4 R2 = a.r[2];
#endif
			/*
			det_inv := 1.0f / determinant(m)
			r.r0.x = det_inv * (m.r1.y * m.r2.z - m.r1.z * m.r2.y);
			r.r1.x = det_inv * (m.r1.z * m.r2.x - m.r1.x * m.r2.z);
			r.r2.x = det_inv * (m.r1.x * m.r2.y - m.r1.y * m.r2.x);

			r.r0.y = det_inv * (m.r0.z * m.r2.y - m.r0.y * m.r2.z);
			r.r1.y = det_inv * (m.r0.x * m.r2.z - m.r0.z * m.r2.x);
			r.r2.y = det_inv * (m.r0.y * m.r2.x - m.r0.x * m.r2.y);

			r.r0.z = det_inv * (m.r0.y * m.r1.z - m.r0.z * m.r1.y);
			r.r1.z = det_inv * (m.r0.z * m.r1.x - m.r0.x * m.r1.z);
			r.r2.z = det_inv * (m.r0.x * m.r1.y - m.r0.y * m.r1.x);
			*/
			// Load coefficents.
			float4 R0yzx = permute_f4<PERMUTE_Y, PERMUTE_Z, PERMUTE_X, PERMUTE_W>(R0);
			float4 R0zxy = permute_f4<PERMUTE_Z, PERMUTE_X, PERMUTE_Y, PERMUTE_W>(R0);
			float4 R1yzx = permute_f4<PERMUTE_Y, PERMUTE_Z, PERMUTE_X, PERMUTE_W>(R1);
			float4 R1zxy = permute_f4<PERMUTE_Z, PERMUTE_X, PERMUTE_Y, PERMUTE_W>(R1);
			float4 R2yzx = permute_f4<PERMUTE_Y, PERMUTE_Z, PERMUTE_X, PERMUTE_W>(R2);
			float4 R2zxy = permute_f4<PERMUTE_Z, PERMUTE_X, PERMUTE_Y, PERMUTE_W>(R2);
			// r1.y * r2.z,
			// r1.z * r2.x,
			// r1.x * r2.y
			float4 V0 = mul_f4(R1yzx, R2zxy);
			// r1.y * r2.z - r1.z * r2.y, 
			// r1.z * r2.x - r1.x * r2.z, 
			// r1.x * r2.y - r1.y * r2.x
			V0 = negmuladd_f4(R1zxy, R2yzx, V0);
			float4 deter = dot3v_f4(R0, V0);
			if (out_determinant) *out_determinant = getx_f4(deter);
			// r0.z * r2.y,
			// r0.x * r2.z,
			// r0.y * r2.x
			float4 V1 = mul_f4(R0zxy, R2yzx);
			// r0.z * r2.y - r0.y * r2.z,
			// r0.x * r2.z - r0.z * r2.x,
			// r0.y * r2.x - r0.x * r2.y
			V1 = negmuladd_f4(R0yzx, R2zxy, V1);
			// r0.y * r1.z
			// r0.z * r1.x
			// r0.x * r1.y
			float4 V2 = mul_f4(R0yzx, R1zxy);
			// r0.y * r1.z - r0.z * r1.y
			// r0.z * r1.x - r0.x * r1.z
			// r0.x * r1.y - r0.y * r1.x
			V2 = negmuladd_f4(R0zxy, R1yzx, V2);
			deter = div_f4(dup_f4(1.0f), deter);
			V0 = mul_f4(V0, deter);
			V1 = mul_f4(V1, deter);
			V2 = mul_f4(V2, deter);
			float3x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			ret.r[0] = _mm256_castps128_ps256(V0);
			ret.r[0] = _mm256_insertf128_ps(ret.r[0], V1, 1);
			ret.r[1] = _mm256_castps128_ps256(V2);
#else
			ret.r[0] = V0;
			ret.r[1] = V1;
			ret.r[2] = V2;
#endif
			return castf4x4_f3x4(transpose_f4x4(castf3x4_f4x4(ret)));
		}
		inline f32 LUNA_SIMD_CALL determinant_f4x4(float4x4 a)
		{
			/*
			 r0.x * ( r1.y * ( r2.z * r3.w - r2.w * r3.z) + r1.z * (r2.w * r3.y - r2.y * r3.w) + r1.w * (r2.y * r3.z - r2.z * r3.y) )
			-r0.y * ( r1.x * ( r2.z * r3.w - r2.w * r3.z) + r1.z * (r2.w * r3.x - r2.x * r3.w) + r1.w * (r2.x * r3.z - r2.z * r3.x) )
			+r0.z * ( r1.x * ( r2.y * r3.w - r2.w * r3.y) + r1.y * (r2.w * r3.x - r2.x * r3.w) + r1.w * (r2.x * r3.y - r2.y * r3.x) )
			-r0.w * ( r1.x * ( r2.y * r3.z - r2.z * r3.y) + r1.y * (r2.z * r3.x - r2.x * r3.z) + r1.z * (r2.x * r3.y - r2.y * r3.x) )
			*/
#if defined(LUNA_AVX_INTRINSICS)
			float4 R0 = _mm256_castps256_ps128(a.r[0]);
			float4 R1 = _mm256_extractf128_ps(a.r[0], 1);
			float4 R2 = _mm256_castps256_ps128(a.r[1]);
			float4 R3 = _mm256_extractf128_ps(a.r[1], 1);
#else
			float4 R0 = a.r[0];
			float4 R1 = a.r[1];
			float4 R2 = a.r[2];
			float4 R3 = a.r[3];
#endif
			float4 R1yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R1);
			float4 R1zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R1);
			float4 R1wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R1);
			float4 R2yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R2);
			float4 R2zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R2);
			float4 R2wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R2);
			float4 R3yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R3);
			float4 R3wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R3);
			float4 R3zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R3);
			float4 res = mul_f4(R2zzyy, R3wwwz);
			res = negmuladd_f4(R2wwwz, R3zzyy, res);
			res = mul_f4(R1yxxx, res);
			float4 temp = mul_f4(R2wwwz, R3yxxx);
			temp = negmuladd_f4(R2yxxx, R3wwwz, temp);
			res = muladd_f4(R1zzyy, temp, res);
			temp = mul_f4(R2yxxx, R3zzyy);
			temp = negmuladd_f4(R2zzyy, R3yxxx, temp);
			res = muladd_f4(R1wwwz, temp, res);
			res = mul_f4(set_f4(1.0f, -1.0f, 1.0f, -1.0f), res);
			return dot4_f4(R0, res);
		}
		inline float4 LUNA_SIMD_CALL determinantv_f4x4(float4x4 a)
		{
			/*
			 r0.x * ( r1.y * ( r2.z * r3.w - r2.w * r3.z) + r1.z * (r2.w * r3.y - r2.y * r3.w) + r1.w * (r2.y * r3.z - r2.z * r3.y) )
			-r0.y * ( r1.x * ( r2.z * r3.w - r2.w * r3.z) + r1.z * (r2.w * r3.x - r2.x * r3.w) + r1.w * (r2.x * r3.z - r2.z * r3.x) )
			+r0.z * ( r1.x * ( r2.y * r3.w - r2.w * r3.y) + r1.y * (r2.w * r3.x - r2.x * r3.w) + r1.w * (r2.x * r3.y - r2.y * r3.x) )
			-r0.w * ( r1.x * ( r2.y * r3.z - r2.z * r3.y) + r1.y * (r2.z * r3.x - r2.x * r3.z) + r1.z * (r2.x * r3.y - r2.y * r3.x) )
			*/
#if defined(LUNA_AVX_INTRINSICS)
			float4 R0 = _mm256_castps256_ps128(a.r[0]);
			float4 R1 = _mm256_extractf128_ps(a.r[0], 1);
			float4 R2 = _mm256_castps256_ps128(a.r[1]);
			float4 R3 = _mm256_extractf128_ps(a.r[1], 1);
#else
			float4 R0 = a.r[0];
			float4 R1 = a.r[1];
			float4 R2 = a.r[2];
			float4 R3 = a.r[3];
#endif
			float4 R1yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R1);
			float4 R1zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R1);
			float4 R1wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R1);
			float4 R2yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R2);
			float4 R2zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R2);
			float4 R2wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R2);
			float4 R3yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R3);
			float4 R3wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R3);
			float4 R3zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R3);
			float4 res = mul_f4(R2zzyy, R3wwwz);
			res = negmuladd_f4(R2wwwz, R3zzyy, res);
			res = mul_f4(R1yxxx, res);
			float4 temp = mul_f4(R2wwwz, R3yxxx);
			temp = negmuladd_f4(R2yxxx, R3wwwz, temp);
			res = muladd_f4(R1zzyy, temp, res);
			temp = mul_f4(R2yxxx, R3zzyy);
			temp = negmuladd_f4(R2zzyy, R3yxxx, temp);
			res = muladd_f4(R1wwwz, temp, res);
			res = mul_f4(set_f4(1.0f, -1.0f, 1.0f, -1.0f), res);
			return dot4v_f4(R0, res);
		}
		inline float4x4 LUNA_SIMD_CALL inverse_f4x4(float4x4 a, f32* out_determinant)
		{
			/*
			LOW
			r0.x =  det_inv * (r1.y * (r2.z * r3.w - r2.w * r3.z) + r1.z * (r2.w * r3.y - r2.y * r3.w) + r1.w * (r2.y * r3.z - r2.z * r3.y));
			r1.x = -det_inv * (r1.x * (r2.z * r3.w - r2.w * r3.z) + r1.z * (r2.w * r3.x - r2.x * r3.w) + r1.w * (r2.x * r3.z - r2.z * r3.x));
			r2.x =  det_inv * (r1.x * (r2.y * r3.w - r2.w * r3.y) + r1.y * (r2.w * r3.x - r2.x * r3.w) + r1.w * (r2.x * r3.y - r2.y * r3.x));
			r3.x = -det_inv * (r1.x * (r2.y * r3.z - r2.z * r3.y) + r1.y * (r2.z * r3.x - r2.x * r3.z) + r1.z * (r2.x * r3.y - r2.y * r3.x));

			r0.y = -det_inv * (r0.y * (r2.z * r3.w - r2.w * r3.z) + r0.z * (r2.w * r3.y - r2.y * r3.w) + r0.w * (r2.y * r3.z - r2.z * r3.y));
			r1.y =  det_inv * (r0.x * (r2.z * r3.w - r2.w * r3.z) + r0.z * (r2.w * r3.x - r2.x * r3.w) + r0.w * (r2.x * r3.z - r2.z * r3.x));
			r2.y = -det_inv * (r0.x * (r2.y * r3.w - r2.w * r3.y) + r0.y * (r2.w * r3.x - r2.x * r3.w) + r0.w * (r2.x * r3.y - r2.y * r3.x));
			r3.y =  det_inv * (r0.x * (r2.y * r3.z - r2.z * r3.y) + r0.y * (r2.z * r3.x - r2.x * r3.z) + r0.z * (r2.x * r3.y - r2.y * r3.x));

			r0.z =  det_inv * (r3.w * (r0.y * r1.z - r0.z * r1.y) + r3.z * (r0.w * r1.y - r0.y * r1.w) + r3.y * (r0.z * r1.w - r0.w * r1.z));
			r1.z = -det_inv * (r3.w * (r0.x * r1.z - r0.z * r1.x) + r3.z * (r0.w * r1.x - r0.x * r1.w) + r3.x * (r0.z * r1.w - r0.w * r1.z));
			r2.z =  det_inv * (r3.w * (r0.x * r1.y - r0.y * r1.x) + r3.y * (r0.w * r1.x - r0.x * r1.w) + r3.x * (r0.y * r1.w - r0.w * r1.y));
			r3.z = -det_inv * (r3.z * (r0.x * r1.y - r0.y * r1.x) + r3.y * (r0.z * r1.x - r0.x * r1.z) + r3.x * (r0.y * r1.z - r0.z * r1.y));
		
			r0.w = -det_inv * (r2.w * (r0.y * r1.z - r0.z * r1.y) + r2.z * (r0.w * r1.y - r0.y * r1.w) + r2.y * (r0.z * r1.w - r0.w * r1.z));
			r1.w =  det_inv * (r2.w * (r0.x * r1.z - r0.z * r1.x) + r2.z * (r0.w * r1.x - r0.x * r1.w) + r2.x * (r0.z * r1.w - r0.w * r1.z));
			r2.w = -det_inv * (r2.w * (r0.x * r1.y - r0.y * r1.x) + r2.y * (r0.w * r1.x - r0.x * r1.w) + r2.x * (r0.y * r1.w - r0.w * r1.y));
			r3.w =  det_inv * (r2.z * (r0.x * r1.y - r0.y * r1.x) + r2.y * (r0.z * r1.x - r0.x * r1.z) + r2.x * (r0.y * r1.z - r0.z * r1.y));
			HIGH
			*/
#if defined(LUNA_AVX_INTRINSICS)
			__m256 V0;
			{
				__m256 R2zzyy = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[1], a.r[1], 0x00), _MM_SHUFFLE(1, 1, 2, 2));
				__m256 R2wwwz = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[1], a.r[1], 0x00), _MM_SHUFFLE(2, 3, 3, 3));
				__m256 R2yxxx = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[1], a.r[1], 0x00), _MM_SHUFFLE(0, 0, 0, 1));
				__m256 R3zzyy = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[1], a.r[1], 0x11), _MM_SHUFFLE(1, 1, 2, 2));
				__m256 R3wwwz = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[1], a.r[1], 0x11), _MM_SHUFFLE(2, 3, 3, 3));
				__m256 R3yxxx = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[1], a.r[1], 0x11), _MM_SHUFFLE(0, 0, 0, 1));
				V0 = _mm256_mul_ps(R2zzyy, R3wwwz);
				V0 = Impl::negmuladd_f8(R2wwwz, R3zzyy, V0);
				V0 = _mm256_mul_ps(_mm256_permute_ps(a.r[0], _MM_SHUFFLE(0, 0, 0, 1)), V0);
				__m256 temp = _mm256_mul_ps(R2wwwz, R3yxxx);
				temp = Impl::negmuladd_f8(R2yxxx, R3wwwz, temp);
				V0 = Impl::muladd_f8(_mm256_permute_ps(a.r[0], _MM_SHUFFLE(1, 1, 2, 2)), temp, V0);
				temp = _mm256_mul_ps(R2yxxx, R3zzyy);
				temp = Impl::negmuladd_f8(R2zzyy, R3yxxx, temp);
				V0 = Impl::muladd_f8(_mm256_permute_ps(a.r[0], _MM_SHUFFLE(2, 3, 3, 3)), temp, V0);
				V0 = _mm256_permute2f128_ps(V0, V0, 0x01);
				V0 = _mm256_mul_ps(_mm256_set_ps(1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f), V0);
			}
			float4 deter = dot4v_f4(_mm256_castps256_ps128(a.r[0]), _mm256_castps256_ps128(V0));
			if (out_determinant) *out_determinant = getx_f4(deter);
			__m256 V1;
			{
				__m256 R0zzyy = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[0], a.r[0], 0x00), _MM_SHUFFLE(1, 1, 2, 2));
				__m256 R0wwwz = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[0], a.r[0], 0x00), _MM_SHUFFLE(2, 3, 3, 3));
				__m256 R0yxxx = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[0], a.r[0], 0x00), _MM_SHUFFLE(0, 0, 0, 1));
				__m256 R1zzyy = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[0], a.r[0], 0x11), _MM_SHUFFLE(1, 1, 2, 2));
				__m256 R1wwwz = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[0], a.r[0], 0x11), _MM_SHUFFLE(2, 3, 3, 3));
				__m256 R1yxxx = _mm256_permute_ps(_mm256_permute2f128_ps(a.r[0], a.r[0], 0x11), _MM_SHUFFLE(0, 0, 0, 1));
				V1 = _mm256_mul_ps(R0yxxx, R1zzyy);
				V1 = Impl::negmuladd_f8(R0zzyy, R1yxxx, V1);
				V1 = _mm256_mul_ps(_mm256_permute_ps(a.r[1], _MM_SHUFFLE(2, 3, 3, 3)), V1);
				__m256 temp = _mm256_mul_ps(R0wwwz, R1yxxx);
				temp = Impl::negmuladd_f8(R0yxxx, R1wwwz, temp);
				V1 = Impl::muladd_f8(_mm256_permute_ps(a.r[1], _MM_SHUFFLE(1, 1, 2, 2)), temp, V1);
				temp = _mm256_mul_ps(R0zzyy, R1wwwz);
				temp = Impl::negmuladd_f8(R0wwwz, R1zzyy, temp);
				V1 = Impl::muladd_f8(_mm256_permute_ps(a.r[1], _MM_SHUFFLE(0, 0, 0, 1)), temp, V1);
				V1 = _mm256_permute2f128_ps(V1, V1, 0x01);
				V1 = _mm256_mul_ps(_mm256_set_ps(1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f), V1);
			}
			__m256 deterv = _mm256_castps128_ps256(deter);
			deterv = _mm256_permute2f128_ps(deterv, deterv, 0x00);
			deterv = _mm256_div_ps(_mm256_set1_ps(1.0f), deterv);
			float4x4 ret;
			ret.r[0] = _mm256_mul_ps(deterv, V0);
			ret.r[1] = _mm256_mul_ps(deterv, V1);
			return transpose_f4x4(ret);
#else
			float4 R0 = a.r[0];
			float4 R1 = a.r[1];
			float4 R2 = a.r[2];
			float4 R3 = a.r[3];
			float4 R0yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R0);
			float4 R0zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R0);
			float4 R0wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R0);
			float4 R1yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R1);
			float4 R1zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R1);
			float4 R1wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R1);
			float4 R2yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R2);
			float4 R2zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R2);
			float4 R2wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R2);
			float4 R3yxxx = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_X>(R3);
			float4 R3wwwz = permute_f4<PERMUTE_W, PERMUTE_W, PERMUTE_W, PERMUTE_Z>(R3);
			float4 R3zzyy = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_Y>(R3);
			float4 V0 = mul_f4(R2zzyy, R3wwwz);
			V0 = negmuladd_f4(R2wwwz, R3zzyy, V0);
			V0 = mul_f4(R1yxxx, V0);
			float4 temp = mul_f4(R2wwwz, R3yxxx);
			temp = negmuladd_f4(R2yxxx, R3wwwz, temp);
			V0 = muladd_f4(R1zzyy, temp, V0);
			temp = mul_f4(R2yxxx, R3zzyy);
			temp = negmuladd_f4(R2zzyy, R3yxxx, temp);
			V0 = muladd_f4(R1wwwz, temp, V0);
			V0 = mul_f4(set_f4(1.0f, -1.0f, 1.0f, -1.0f), V0);
			float4 deter = dot4v_f4(R0, V0);
			if (out_determinant) *out_determinant = getx_f4(deter);
			float4 V1 = mul_f4(R2zzyy, R3wwwz);
			V1 = negmuladd_f4(R2wwwz, R3zzyy, V1);
			V1 = mul_f4(R0yxxx, V1);
			temp = mul_f4(R2wwwz, R3yxxx);
			temp = negmuladd_f4(R2yxxx, R3wwwz, temp);
			V1 = muladd_f4(R0zzyy, temp, V1);
			temp = mul_f4(R2yxxx, R3zzyy);
			temp = negmuladd_f4(R2zzyy, R3yxxx, temp);
			V1 = muladd_f4(R0wwwz, temp, V1);
			V1 = mul_f4(set_f4(-1.0f, 1.0f, -1.0f, 1.0f), V1);
			float4 V2 = mul_f4(R0yxxx, R1zzyy);
			V2 = negmuladd_f4(R0zzyy, R1yxxx, V2);
			V2 = mul_f4(R3wwwz, V2);
			temp = mul_f4(R0wwwz, R1yxxx);
			temp = negmuladd_f4(R0yxxx, R1wwwz, temp);
			V2 = muladd_f4(R3zzyy, temp, V2);
			temp = mul_f4(R0zzyy, R1wwwz);
			temp = negmuladd_f4(R0wwwz, R1zzyy, temp);
			V2 = muladd_f4(R3yxxx, temp, V2);
			V2 = mul_f4(set_f4(1.0f, -1.0f, 1.0f, -1.0f), V2);
			float4 V3 = mul_f4(R0yxxx, R1zzyy);
			V3 = negmuladd_f4(R0zzyy, R1yxxx, V3);
			V3 = mul_f4(R2wwwz, V3);
			temp = mul_f4(R0wwwz, R1yxxx);
			temp = negmuladd_f4(R0yxxx, R1wwwz, temp);
			V3 = muladd_f4(R2zzyy, temp, V3);
			temp = mul_f4(R0zzyy, R1wwwz);
			temp = negmuladd_f4(R0wwwz, R1zzyy, temp);
			V3 = muladd_f4(R2yxxx, temp, V3);
			V3 = mul_f4(set_f4(-1.0f, 1.0f, -1.0f, 1.0f), V3);
			deter = div_f4(dup_f4(1.0f), deter);
			V0 = mul_f4(deter, V0);
			V1 = mul_f4(deter, V1);
			V2 = mul_f4(deter, V2);
			V3 = mul_f4(deter, V3);
			float4x4 ret;
			ret.r[0] = V0;
			ret.r[1] = V1;
			ret.r[2] = V2;
			ret.r[3] = V3;
			return transpose_f4x4(ret);
#endif
		}
		inline float4 LUNA_SIMD_CALL round_f4(float4 a)
		{
#if defined(LUNA_SSE4_INTRINSICS)
			return _mm_round_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
#elif defined(LUNA_SSE2_INTRINSICS)
			__m128 no_fraction = dup_f4(8388608.0f);
			__m128 sign = _mm_and_ps(a, casti_f4(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			__m128 magic = _mm_or_ps(no_fraction, sign);
			__m128 R1 = _mm_add_ps(a, magic);
			R1 = _mm_sub_ps(R1, magic);
			__m128 R2 = _mm_and_ps(a, casti_f4(set_i4(0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF)));
			__m128 mask = _mm_cmple_ps(R2, no_fraction);
			R2 = _mm_andnot_ps(mask, a);
			R1 = _mm_and_ps(R1, mask);
			return _mm_xor_ps(R1, R2);
#elif defined(LUNA_NEON_INTRINSICS)
#ifdef LUNA_PLATFORM_ARM64
			return vrndnq_f32(a);
#else
			float32x4_t no_fraction = dup_f4(8388608.0f);
			uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(a), vreinterpretq_u32_s32(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			float32x4_t magic = vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(no_fraction), sign));
			float32x4_t R1 = vaddq_f32(a, magic);
			R1 = vsubq_f32(R1, magic);
			float32x4_t R2 = vabsq_f32(a);
			uint32x4_t mask = vcleq_f32(R2, no_fraction);
			return vbslq_f32(mask, R1, a);
#endif
#else 
#error "Not implemented."
#endif
		}
		inline float4 LUNA_SIMD_CALL modangle_f4(float4 a)
		{
			float4 res = mul_f4(a, dup_f4(ONE_DIV_TWO_PI));
			res = round_f4(res);
			res = negmuladd_f4(dup_f4(TWO_PI), res, a);
			return res;
		}
		namespace Impl
		{
			LUNA_FORCEINLINE float4 sincoeff0()
			{
				return set_f4(-0.16666667f, +0.0083333310f, -0.00019840874f, +2.7525562e-06f);
			}
			LUNA_FORCEINLINE float4 sincoeff1()
			{
				return set_f4(-2.3889859e-08f, -0.16665852f /*Est1*/, +0.0083139502f /*Est2*/, -0.00018524670f /*Est3*/);
			}
			LUNA_FORCEINLINE float4 coscoeff0()
			{
				return set_f4(-0.5f, +0.041666638f, -0.0013888378f, +2.4760495e-05f);
			}
			LUNA_FORCEINLINE float4 coscoeff1()
			{
				return set_f4(-2.6051615e-07f, -0.49992746f /*Est1*/, +0.041493919f /*Est2*/, -0.0012712436f /*Est3*/);
			}
		}
		inline float4 LUNA_SIMD_CALL sin_f4(float4 a)
		{
#if defined(LUNA_SVML_INTRINSICS)
			return _mm_sin_ps(a);
#elif defined(LUNA_SSE2_INTRINSICS)
			// Force the value within the bounds of pi
			__m128 x = modangle_f4(a);

			// Map in [-pi/2,pi/2] with sin(y) = sin(x).
			__m128 sign = _mm_and_ps(x, casti_f4(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			__m128 c = _mm_or_ps(dup_f4(PI), sign);  // pi when x >= 0, -pi when x < 0
			__m128 absx = _mm_andnot_ps(sign, x);  // |x|
			__m128 rflx = _mm_sub_ps(c, x);
			__m128 comp = _mm_cmple_ps(absx, dup_f4(PI_DIV_TWO));
			__m128 select0 = _mm_and_ps(comp, x);
			__m128 select1 = _mm_andnot_ps(comp, rflx);
			x = _mm_or_ps(select0, select1);
			__m128 x2 = _mm_mul_ps(x, x);

			// Compute polynomial approximation
			const __m128 SC1 = Impl::sincoeff1();
			__m128 ConstantsB = permute_f4<0, 0, 0, 0>(SC1);
			const __m128 SC0 = Impl::sincoeff0();
			__m128 Constants = permute_f4<3, 3, 3, 3>(SC0);
			__m128 res = muladd_f4(ConstantsB, x2, Constants);
			Constants = permute_f4<2, 2, 2, 2>(SC0);
			res = muladd_f4(res, x2, Constants);
			Constants = permute_f4<1, 1, 1, 1>(SC0);
			res = muladd_f4(res, x2, Constants);
			Constants = permute_f4<0, 0, 0, 0>(SC0);
			res = muladd_f4(res, x2, Constants);
			res = muladd_f4(res, x2, dup_f4(1.0f));
			res = _mm_mul_ps(res, x);
			return res;
#elif defined(LUNA_NEON_INTRINSICS)
			// Force the value within the bounds of pi
			float32x4_t x = modangle_f4(a);

			// Map in [-pi/2,pi/2] with sin(y) = sin(x).
			uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(x), 
				vreinterpretq_u32_s32(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			uint32x4_t c = vorrq_u32(dup_f4(PI), sign);  // pi when x >= 0, -pi when x < 0
			float32x4_t absx = vabsq_f32(x);
			float32x4_t rflx = vsubq_f32(vreinterpretq_f32_u32(c), x);
			uint32x4_t comp = vcleq_f32(absx, dup_f4(PI_DIV_TWO));
			x = vbslq_f32(comp, x, rflx);
			float32x4_t x2 = vmulq_f32(x, x);

			// Compute polynomial approximation
			const float32x4_t SC1 = Impl::sincoeff1();
			const float32x4_t SC0 = Impl::sincoeff0();
			float32x4_t Constants = vdupq_lane_f32(vget_high_f32(SC0), 1);
			float32x4_t res = vmlaq_lane_f32(Constants, x2, vget_low_f32(SC1), 0);
			Constants = vdupq_lane_f32(vget_high_f32(SC0), 0);
			res = vmlaq_f32(Constants, res, x2);
			Constants = vdupq_lane_f32(vget_low_f32(SC0), 1);
			res = vmlaq_f32(Constants, res, x2);
			Constants = vdupq_lane_f32(vget_low_f32(SC0), 0);
			res = vmlaq_f32(Constants, res, x2);
			res = vmlaq_f32(dup_f4(1.0f), res, x2);
			res = vmulq_f32(res, x);
			return res;
#else 
#error "Not implemented."
#endif
		}
		inline float4 LUNA_SIMD_CALL cos_f4(float4 a)
		{
#if defined(LUNA_SVML_INTRINSICS)
			return _mm_cos_ps(a);
#elif defined(LUNA_SSE2_INTRINSICS)
			// Map V to x in [-pi,pi].
			__m128 x = modangle_f4(a);

			// Map in [-pi/2,pi/2] with cos(y) = sign*cos(x).
			__m128 sign = _mm_and_ps(x, casti_f4(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			__m128 c = _mm_or_ps(dup_f4(PI), sign);  // pi when x >= 0, -pi when x < 0
			__m128 absx = _mm_andnot_ps(sign, x);  // |x|
			__m128 rflx = _mm_sub_ps(c, x);
			__m128 comp = _mm_cmple_ps(absx, dup_f4(PI_DIV_TWO));
			__m128 select0 = _mm_and_ps(comp, x);
			__m128 select1 = _mm_andnot_ps(comp, rflx);
			x = _mm_or_ps(select0, select1);
			select0 = _mm_and_ps(comp, dup_f4(1.0f));
			select1 = _mm_andnot_ps(comp, dup_f4(-1.0f));
			sign = _mm_or_ps(select0, select1);
			__m128 x2 = _mm_mul_ps(x, x);

			// Compute polynomial approximation
			const __m128 CC1 = Impl::coscoeff1();
			__m128 ConstantsB = permute_f4<0, 0, 0, 0>(CC1);
			const __m128 CC0 = Impl::coscoeff0();
			__m128 Constants = permute_f4<3, 3, 3, 3>(CC0);
			__m128 Result = muladd_f4(ConstantsB, x2, Constants);
			Constants = permute_f4<2, 2, 2, 2>(CC0);
			Result = muladd_f4(Result, x2, Constants);
			Constants = permute_f4<1, 1, 1, 1>(CC0);
			Result = muladd_f4(Result, x2, Constants);
			Constants = permute_f4<0, 0, 0, 0>(CC0);
			Result = muladd_f4(Result, x2, Constants);
			Result = muladd_f4(Result, x2, dup_f4(1.0f));
			Result = _mm_mul_ps(Result, sign);
			return Result;
#elif defined(LUNA_NEON_INTRINSICS)
			// Map V to x in [-pi,pi].
			float32x4_t x = modangle_f4(a);

			// Map in [-pi/2,pi/2] with cos(y) = sign*cos(x).
			uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(x), 
				vreinterpretq_u32_s32(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			uint32x4_t c = vorrq_u32(dup_f4(PI), sign);  // pi when x >= 0, -pi when x < 0
			float32x4_t absx = vabsq_f32(x);
			float32x4_t rflx = vsubq_f32(vreinterpretq_f32_u32(c), x);
			uint32x4_t comp = vcleq_f32(absx, dup_f4(PI_DIV_TWO));
			x = vbslq_f32(comp, x, rflx);
			float32x4_t fsign = vbslq_f32(comp, dup_f4(1.0f), dup_f4(-1.0f));
			float32x4_t x2 = vmulq_f32(x, x);

			// Compute polynomial approximation
			const float32x4_t CC1 = Impl::coscoeff1();
			const float32x4_t CC0 = Impl::coscoeff0();
			float32x4_t Constants = vdupq_lane_f32(vget_high_f32(CC0), 1);
			float32x4_t Result = vmlaq_lane_f32(Constants, x2, vget_low_f32(CC1), 0);
			Constants = vdupq_lane_f32(vget_high_f32(CC0), 0);
			Result = vmlaq_f32(Constants, Result, x2);
			Constants = vdupq_lane_f32(vget_low_f32(CC0), 1);
			Result = vmlaq_f32(Constants, Result, x2);
			Constants = vdupq_lane_f32(vget_low_f32(CC0), 0);
			Result = vmlaq_f32(Constants, Result, x2);
			Result = vmlaq_f32(dup_f4(1.0f), Result, x2);
			Result = vmulq_f32(Result, fsign);
			return Result;
#else 
#error "Not implemented."
#endif
		}
		inline float4 LUNA_SIMD_CALL sincos_f4(float4& out_cos, float4 a)
		{
#if defined(LUNA_SVML_INTRINSICS)
			return _mm_sincos_ps(&out_cos, a);
#elif defined(LUNA_SSE2_INTRINSICS)
			float4 dst;
			// Force the value within the bounds of pi
			__m128 x = modangle_f4(a);

			// Map in [-pi/2,pi/2] with sin(y) = sin(x), cos(y) = sign*cos(x).
			__m128 sign = _mm_and_ps(x, casti_f4(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			__m128 c = _mm_or_ps(dup_f4(PI), sign);  // pi when x >= 0, -pi when x < 0
			__m128 absx = _mm_andnot_ps(sign, x);  // |x|
			__m128 rflx = _mm_sub_ps(c, x);
			__m128 comp = _mm_cmple_ps(absx, dup_f4(PI_DIV_TWO));
			__m128 select0 = _mm_and_ps(comp, x);
			__m128 select1 = _mm_andnot_ps(comp, rflx);
			x = _mm_or_ps(select0, select1);
			select0 = _mm_and_ps(comp, dup_f4(1.0f));
			select1 = _mm_andnot_ps(comp, dup_f4(-1.0f));
			sign = _mm_or_ps(select0, select1);
			__m128 x2 = _mm_mul_ps(x, x);

			// Compute polynomial approximation of sine
			const __m128 SC1 = Impl::sincoeff1();
			__m128 ConstantsB = permute_f4<0, 0, 0, 0>(SC1);
			const __m128 SC0 = Impl::sincoeff0();
			__m128 Constants = permute_f4<3, 3, 3, 3>(SC0);
			__m128 Result = muladd_f4(ConstantsB, x2, Constants);
			Constants = permute_f4<2, 2, 2, 2>(SC0);
			Result = muladd_f4(Result, x2, Constants);
			Constants = permute_f4<1, 1, 1, 1>(SC0);
			Result = muladd_f4(Result, x2, Constants);
			Constants = permute_f4<0, 0, 0, 0>(SC0);
			Result = muladd_f4(Result, x2, Constants);
			Result = muladd_f4(Result, x2, dup_f4(1.0f));
			Result = _mm_mul_ps(Result, x);
			dst = Result;

			// Compute polynomial approximation of cosine
			const __m128 CC1 = Impl::coscoeff1();
			ConstantsB = permute_f4<0, 0, 0, 0>(CC1);
			const __m128 CC0 = Impl::coscoeff0();
			Constants = permute_f4<3, 3, 3, 3>(CC0);
			Result = muladd_f4(ConstantsB, x2, Constants);
			Constants = permute_f4<2, 2, 2, 2>(CC0);
			Result = muladd_f4(Result, x2, Constants);
			Constants = permute_f4<1, 1, 1, 1>(CC0);
			Result = muladd_f4(Result, x2, Constants);
			Constants = permute_f4<0, 0, 0, 0>(CC0);
			Result = muladd_f4(Result, x2, Constants);
			Result = muladd_f4(Result, x2, dup_f4(1.0f));
			Result = _mm_mul_ps(Result, sign);
			out_cos = Result;
			return dst;
#elif defined(LUNA_NEON_INTRINSICS)
			float4 dst;
			// Force the value within the bounds of pi
			float32x4_t x = modangle_f4(a);

			// Map in [-pi/2,pi/2] with cos(y) = sign*cos(x).
			uint32x4_t sign = vandq_u32(vreinterpretq_u32_f32(x), 
				vreinterpretq_u32_s32(set_i4(0x80000000, 0x80000000, 0x80000000, 0x80000000)));
			uint32x4_t c = vorrq_u32(dup_f4(PI), sign);  // pi when x >= 0, -pi when x < 0
			float32x4_t absx = vabsq_f32(x);
			float32x4_t  rflx = vsubq_f32(vreinterpretq_f32_u32(c), x);
			uint32x4_t comp = vcleq_f32(absx, dup_f4(PI_DIV_TWO));
			x = vbslq_f32(comp, x, rflx);
			float32x4_t fsign = vbslq_f32(comp, dup_f4(1.0f), dup_f4(-1.0f));
			float32x4_t x2 = vmulq_f32(x, x);

			// Compute polynomial approximation for sine
			const float32x4_t SC1 = Impl::sincoeff1();
			const float32x4_t SC0 = Impl::sincoeff0();
			float32x4_t Constants = vdupq_lane_f32(vget_high_f32(SC0), 1);
			float32x4_t Result = vmlaq_lane_f32(Constants, x2, vget_low_f32(SC1), 0);
			Constants = vdupq_lane_f32(vget_high_f32(SC0), 0);
			Result = vmlaq_f32(Constants, Result, x2);
			Constants = vdupq_lane_f32(vget_low_f32(SC0), 1);
			Result = vmlaq_f32(Constants, Result, x2);
			Constants = vdupq_lane_f32(vget_low_f32(SC0), 0);
			Result = vmlaq_f32(Constants, Result, x2);
			Result = vmlaq_f32(dup_f4(1.0f), Result, x2);
			dst = vmulq_f32(Result, x);

			// Compute polynomial approximation for cosine
			const float32x4_t CC1 = Impl::coscoeff1();
			const float32x4_t CC0 = Impl::coscoeff0();
			Constants = vdupq_lane_f32(vget_high_f32(CC0), 1);
			Result = vmlaq_lane_f32(Constants, x2, vget_low_f32(CC1), 0);
			Constants = vdupq_lane_f32(vget_high_f32(CC0), 0);
			Result = vmlaq_f32(Constants, Result, x2);
			Constants = vdupq_lane_f32(vget_low_f32(CC0), 1);
			Result = vmlaq_f32(Constants, Result, x2);
			Constants = vdupq_lane_f32(vget_low_f32(CC0), 0);
			Result = vmlaq_f32(Constants, Result, x2);
			Result = vmlaq_f32(dup_f4(1.0f), Result, x2);
			out_cos = vmulq_f32(Result, fsign);
			return dst;
#else 
#error "Not implemented."
#endif
		}
	}
}
#endif
