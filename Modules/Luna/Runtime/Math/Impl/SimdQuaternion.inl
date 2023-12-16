/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SimdQuaternion.inl
* @author JXMaster
* @date 2022/3/25
* @brief The SIMD extension to support Quaternion computations.
*/
#include "../SimdQuaternion.hpp"

#ifdef LUNA_SIMD
namespace Luna
{
	namespace Simd
	{
		inline float4 LUNA_SIMD_CALL mulquat_f4(float4 a, float4 b)
		{
			/*
				dst.x := (a.x * b.w) + (a.w * b.x) + (a.z * b.y) - (a.y * b.z)
				dst.y := (a.y * b.w) - (a.z * b.x) + (a.w * b.y) + (a.x * b.z)
				dst.z := (a.z * b.w) + (a.y * b.x) - (a.x * b.y) + (a.w * b.z)
				dst.w := (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z)
			*/
#if defined(LUNA_SSE2_INTRINSICS)
			__m128 res = _mm_mul_ps(a, dupw_f4(b));
			__m128 temp = _mm_mul_ps(set_f4(1.0f, -1.0f, 1.0f, -1.0f), permute_f4<PERMUTE_W, PERMUTE_Z, PERMUTE_Y, PERMUTE_X>(a));
			res = muladd_f4(temp, dupx_f4(b), res);
			temp = _mm_mul_ps(set_f4(1.0f, 1.0f, -1.0f, -1.0f), permute_f4<PERMUTE_Z, PERMUTE_W, PERMUTE_X, PERMUTE_Y>(a));
			res = muladd_f4(temp, dupy_f4(b), res);
			temp = _mm_mul_ps(set_f4(-1.0f, 1.0f, 1.0f, -1.0f), permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_W, PERMUTE_Z>(a));
			res = muladd_f4(temp, dupz_f4(b), res);
			return res;
#elif defined(LUNA_NEON_INTRINSICS)
			float32x2_t bl = vget_low_f32(b);
			float32x2_t bh = vget_high_f32(b);
			float32x4_t bx = vdupq_lane_f32(bl, 0);
			float32x4_t by = vdupq_lane_f32(bl, 1);
			float32x4_t bz = vdupq_lane_f32(bh, 0);
			// axbw, aybw, azbw, awbw
			float32x4_t res = vmulq_lane_f32(a, bh, 1);

			float32x4_t temp = vrev64q_f32(a);
			temp = vcombine_f32(vget_high_f32(temp), vget_low_f32(temp));
			temp = vmulq_f32(bx, temp);
			res = vmlaq_f32(res, temp, set_f4(1.0f, -1.0f, 1.0f, -1.0f));

			temp = vreinterpretq_f32_u32(vrev64q_u32(vreinterpretq_u32_f32(temp)));
			temp = vmulq_f32(by, temp);
			res = vmlaq_f32(res, temp, set_f4(1.0f, 1.0f, -1.0f, -1.0f));

			temp = vreinterpretq_f32_u32(vrev64q_u32(vreinterpretq_u32_f32(temp)));
			temp = vcombine_f32(vget_high_f32(temp), vget_low_f32(temp));
			temp = vmulq_f32(bz, temp);
			res = vmlaq_f32(res, temp, set_f4(-1.0f, 1.0f, 1.0f, -1.0f));
			return res;
#else 
#error "Not implemented."
#endif
		}
		inline float4 LUNA_SIMD_CALL quatinverse_f4(float4 a)
		{
			float4 length_sq = dot4v_f4(a, a);
			float4 res = mul_f4(set_f4(-1.0f, -1.0f, -1.0f, 1.0f), a);
			res = div_f4(res, length_sq);
			return res;
		}
		inline float4 LUNA_SIMD_CALL quatnormalangle_f4(float4 n, f32 a)
		{
			n = setw_f4(n, 1.0f);
			float4 scale = dup_f4(0.5f * a);
			float4 cosine;
			float4 sine = sincos_f4(cosine, scale);
			scale = select_f4<SELECT_A, SELECT_A, SELECT_A, SELECT_B>(sine, cosine);
			n = mul_f4(n, scale);
			return n;
		}
		inline float4 LUNA_SIMD_CALL quateulerangles_f4(float4 a)
		{
			float4 sign = set_f4(1.0f, -1.0f, -1.0f, 1.0f);
			a = mul_f4(a, dup_f4(0.5f));
			float4 cosine;
			float4 sine = sincos_f4(cosine, a);
			float4 P0 = permute2_f4<PERMUTE_AX, PERMUTE_BX, PERMUTE_BX, PERMUTE_BX>(sine, cosine);
			float4 Y0 = permute2_f4<PERMUTE_BY, PERMUTE_AY, PERMUTE_BY, PERMUTE_BY>(sine, cosine);
			float4 R0 = permute2_f4<PERMUTE_BZ, PERMUTE_BZ, PERMUTE_AZ, PERMUTE_BZ>(sine, cosine);
			float4 P1 = permute2_f4<PERMUTE_AX, PERMUTE_BX, PERMUTE_BX, PERMUTE_BX>(cosine, sine);
			float4 Y1 = permute2_f4<PERMUTE_BY, PERMUTE_AY, PERMUTE_BY, PERMUTE_BY>(cosine, sine);
			float4 R1 = permute2_f4<PERMUTE_BZ, PERMUTE_BZ, PERMUTE_AZ, PERMUTE_BZ>(cosine, sine);
			float4 Q1 = mul_f4(P1, sign);
			float4 Q0 = mul_f4(P0, Y0);
			Q1 = mul_f4(Q1, Y1);
			Q0 = mul_f4(Q0, R0);
			float4 Q = muladd_f4(Q1, R1, Q0);
			return Q;
		}
		inline float4 LUNA_SIMD_CALL quatlerp_f4(float4 a, float4 b, f32 t)
		{
			f32 cos_omega = dot4_f4(a, b);
			if (cos_omega >= 0.0f)
			{
				return normalize4_f4(lerp_f4(a, b, t));
			}
			// (1 - t) * a - t * b
			float4 tv = dup_f4(t);
			float4 t1v = dup_f4(1.0f - t);
			float4 res = mul_f4(a, t1v);
			res = negmuladd_f4(b, tv, res);
			return normalize4_f4(res);
		}
		inline float4 LUNA_SIMD_CALL quatslerp_f4(float4 a, float4 b, f32 t)
		{
			f32 cos_omega = dot4_f4(a, b);
			cos_omega = cos_omega < 0.0f ? -cos_omega : cos_omega;
			f32 sin_omega = sqrtf(1.0f - cos_omega * cos_omega);
			f32 omega = atan2f(sin_omega, cos_omega);
			f32 wa = sinf((1.0f - t) * omega) / sin_omega;
			f32 wb = sinf(t * omega) / sin_omega;
			float4 res = scale_f4(a, wa);
			res = scaleadd_f4(b, wb, res);
			return res;
		}
	}
}
#endif