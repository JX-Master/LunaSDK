/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SimdTransform.inl
* @author JXMaster
* @date 2022/3/27
* @brief The SIMD extension to support transform computations.
 */
#pragma once
#include "../SimdTransform.hpp"

#ifdef LUNA_SIMD
namespace Luna
{
	namespace Simd
	{
		inline float3x4 LUNA_SIMD_CALL transform2d_f3x4(float4 translation, f32 rotation, float4 scaling)
		{
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			float4 R0 = dupx_f4(translation);
			R0 = casti_f4(and_i4(castf_i4(R0), set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0, 0)));
			R0 = mul_f4(R0, set_f4(cosine, sine, 1.0f, 1.0f));
			float4 R1 = dupy_f4(translation);
			R1 = casti_f4(and_i4(castf_i4(R1), set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0, 0)));
			R1 = mul_f4(R1, set_f4(-sine, cosine, 1.0f, 1.0f));
			float4 R2 = select_f4<0, 0, 1, 1>(translation, set_f4(0.0f, 0.0f, 1.0f, 0.0f));
			float3x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			ret.r[0] = _mm256_castps128_ps256(R0);
			ret.r[0] = _mm256_insertf128_ps(ret.r[0], R1, 1);
			ret.r[1] = _mm256_castps128_ps256(R2);
#else
			ret.r[0] = R0;
			ret.r[1] = R1;
			ret.r[2] = R2;
#endif
			
		}
		inline float3x4 LUNA_SIMD_CALL transform2d_translation_f3x4(float4 translation)
		{
			float3x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			ret.r[0] = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			// x, y, 0, 0, 0, 0, 0, 0
			ret.r[1] = _mm256_and_ps(_mm256_castps128_ps256(translation), 
				_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF)));
			// x, y, 1, 0, 0, 0, 0, 0
			ret.r[1] = _mm256_or_ps(ret.r[1], _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
#else
			ret.r[0] = set_f4(1.0f, 0.0f, 0.0f, 0.0f);
			ret.r[1] = set_f4(0.0f, 1.0f, 0.0f, 0.0f);
			ret.r[2] = select_f4<0, 0, 1, 1>(translation, set_f4(0.0f, 0.0f, 1.0f, 0.0f));
#endif
			return ret;
		}
		inline float3x4 LUNA_SIMD_CALL transform2d_rotation_f3x4(f32 rotation)
		{
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			float3x4 ret;
#ifdef LUNA_AVX_INTRINSICS
			ret.r[0] = _mm256_set_ps(0.0f, 0.0f, cosine, -sine, 0.0f, 0.0f, sine, cosine);
			ret.r[1] = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
#else
			ret.r[0] = set_f4(cosine, sine, 0.0f, 0.0f);
			ret.r[1] = set_f4(-sine, cosine, 0.0f, 0.0f);
			ret.r[2] = set_f4(0.0f, 0.0f, 1.0f, 0.0f);
#endif
			return ret;
		}
		inline float3x4 LUNA_SIMD_CALL transform2d_scaling_f3x4(float4 scaling)
		{
#if defined(LUNA_AVX_INTRINSICS)
			__m256 r01 = _mm256_castps128_ps256(scaling);
			// x, y, ?, ?, x, y, ?, ?
			r01 = _mm256_permute2f128_ps(r01, r01, 0);
			// x, 0, 0, 0, 0, y, 0, 0
			r01 = _mm256_and_ps(r01, _mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0xFFFFFFFF, 0, 0, 0, 0, 0xFFFFFFFF)));
			float3x4 ret;
			ret.r[0] = r01;
			ret.r[1] = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
			return ret;
#elif defined(LUNA_SSE2_INTRINSICS)
			float3x4 ret;
			ret.r[0] = _mm_and_ps(scaling, _mm_castsi128_ps(_mm_set_epi32(0, 0, 0, 0xFFFFFFFF)));
			ret.r[1] = _mm_and_ps(scaling, _mm_castsi128_ps(_mm_set_epi32(0, 0, 0xFFFFFFFF, 0)));
			ret.r[2] = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
			return ret;
#elif defined(LUNA_NEON_INTRINSICS)
			float3x4 ret;
			ret.r[0] = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(scaling), vreinterpretq_u32_s32(set_i4(0xFFFFFFFF, 0, 0, 0))));
			ret.r[1] = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(scaling), vreinterpretq_u32_s32(set_i4(0, 0xFFFFFFFF, 0, 0))));
			ret.r[2] = set_f4(0.0f, 0.0f, 1.0f, 0.0f);
			return ret;
#else 
#error "Not implemented."
#endif
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_f4x4(float4 translation, float4 rotation_quaternion, float4 scaling)
		{
			float4 trans = setw_f4(translation, 1.0f);
			float4x4 res = transform3d_scaling_f4x4(scaling);
			float4x4 mat_rot = transform3d_rotation_quaternion_f4x4(rotation_quaternion);
			res = matmul_f4x4(res, mat_rot);
#if defined(LUNA_AVX_INTRINSICS)
			res.r[1] = _mm256_insertf128_ps(res.r[1], trans, 1);
#else
			res.r[3] = trans;
#endif
			return res;
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_translation_f4x4(float4 translation)
		{
			float4x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			translation = setw_f4(translation, 1.0f);
			ret.r[0] = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			// x, y, z, 0, 0, 0, 0, 0
			ret.r[1] = _mm256_insertf128_ps(_mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f), translation, 1);
#else
			ret.r[0] = set_f4(1.0f, 0.0f, 0.0f, 0.0f);
			ret.r[1] = set_f4(0.0f, 1.0f, 0.0f, 0.0f);
			ret.r[2] = set_f4(0.0f, 0.0f, 1.0f, 0.0f);
			ret.r[3] = select_f4<0, 0, 0, 1>(translation, set_f4(0.0f, 0.0f, 0.0f, 1.0f));
#endif
			return ret;
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_rotation_quaternion_f4x4(float4 quaternion)
		{	
			// 2x, 2y, 2z, 2w
			float4 Q2 = add_f4(quaternion, quaternion);
			// 2xx, 2yy, 2zz, 2ww
			float4 T0 = mul_f4(Q2, quaternion);
			// 2yy, 2xx, 2xx, 2ww
			float4 T1 = permute_f4<PERMUTE_Y, PERMUTE_X, PERMUTE_X, PERMUTE_W>(T0);
			// 2zz, 2zz, 2yy, 2ww
			float4 T2 = permute_f4<PERMUTE_Z, PERMUTE_Z, PERMUTE_Y, PERMUTE_W>(T0);
			// 2(yy + zz), 2(xx + zz), 2(xx + yy), 2(ww + ww)
			T1 = add_f4(T1, T2);
			// 1 - 2(yy + zz), 1 - 2(xx + zz), 1 - 2(xx + yy), 1 - 2(ww + ww)
			float4 R0 = sub_f4(dup_f4(1.0f), T1);
			// 1 - 2(yy + zz), 1 - 2(xx + zz), 1 - 2(xx + yy), 0
			R0 = casti_f4(and_i4(castf_i4(R0), set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0)));

			// 2x, 2x, 2y, 2w
			T0 = permute_f4<PERMUTE_X, PERMUTE_X, PERMUTE_Y, PERMUTE_W>(Q2);
			// z, y, z, w
			T1 = permute_f4<PERMUTE_Z, PERMUTE_Y, PERMUTE_Z, PERMUTE_W>(quaternion);
			// 2xz, 2xy, 2yz, 2ww
			T0 = mul_f4(T0, T1);

			// 2y, 2z, 2x, 2w
			T1 = permute_f4<PERMUTE_Y, PERMUTE_Z, PERMUTE_X, PERMUTE_W>(Q2);
			// w, w, w, w
			T2 = dupw_f4(quaternion);
			// 2yw, 2zw, 2xw, 2ww
			T1 = mul_f4(T1, T2);

			// 2(xz + yw), 2(xy + zw), 2(yz + xw), 2(ww + ww)
			float4 R1 = add_f4(T0, T1);
			// 2(xz - yw), 2(xy - zw), 2(yz - xw), 2(ww - ww)
			float4 R2 = sub_f4(T0, T1);

			// 2(xy + zw), 2(xz - yw), 2(xy - zw), 2(yz + xw)
			T0 = permute2_f4<PERMUTE_AY, PERMUTE_BX, PERMUTE_BY, PERMUTE_AZ>(R1, R2);
			// 2(xz + yw), 2(yz - xw), 2(xz + yw), 2(yz - xw)
			T1 = permute2_f4<PERMUTE_AX, PERMUTE_BZ, PERMUTE_AX, PERMUTE_BZ>(R1, R2);

			// 1 - 2(yy + zz), 2(xy + zw), 2(xz - yw), 0
			float4 D0 = permute2_f4<PERMUTE_AX, PERMUTE_BX, PERMUTE_BY, PERMUTE_AW>(R0, T0);
			// 2(xy - zw), 1 - 2(xx + zz), 2(yz + xw), 0
			float4 D1 = permute2_f4<PERMUTE_BZ, PERMUTE_AY, PERMUTE_BW, PERMUTE_AW>(R0, T0);
			// 2(xz + yw), 2(yz - xw), 1 - 2(xx + yy), 0
			float4 D2 = permute2_f4<PERMUTE_BX, PERMUTE_BY, PERMUTE_AZ, PERMUTE_AW>(R0, T1);
			float4 D3 = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
			return setf4_f4x4(D0, D1, D2, D3);
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_rotation_x_f4x4(f32 rotation)
		{
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			float4x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			ret.r[0] = _mm256_set_ps(0.0f, sine, cosine, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
			ret.r[1] = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cosine, -sine, 0.0f);
#else
			ret.r[0] = set_f4(1.0f, 0.0f, 0.0f, 0.0f);
			ret.r[1] = set_f4(0.0f, cosine, sine, 0.0f);
			ret.r[2] = set_f4(0.0f, -sine, cosine, 0.0f);
			ret.r[3] = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_rotation_y_f4x4(f32 rotation)
		{
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			float4x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			ret.r[0] = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -sine, 0.0f, cosine);
			ret.r[1] = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cosine, 0.0f, sine);
#else
			ret.r[0] = set_f4(cosine, 0.0f, -sine, 0.0f);
			ret.r[1] = set_f4(0.0f, 1.0f, 0.0f, 0.0f);
			ret.r[2] = set_f4(sine, 0.0f, cosine, 0.0f);
			ret.r[3] = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_rotation_z_f4x4(f32 rotation)
		{
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			float4x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			ret.r[0] = _mm256_set_ps(0.0f, 0.0f, cosine, -sine, 0.0f, 0.0f, sine, cosine);
			ret.r[1] = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
#else
			ret.r[0] = set_f4(cosine, sine, 0.0f, 0.0f);
			ret.r[1] = set_f4(-sine, cosine, 0.0f, 0.0f);
			ret.r[2] = set_f4(0.0f, 0.0f, 1.0f, 0.0f);
			ret.r[3] = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_rotation_normal_angle_f4x4(float4 normal, f32 angle)
		{
			f32 sine = sinf(angle);
			f32 cosine = cosf(angle);

			float4 A = set_f4(sine, cosine, 1.0f - cosine, 0.0f);

			float4 C2 = dupz_f4(A);
			float4 C1 = dupy_f4(A);
			float4 C0 = dupx_f4(A);

			float4 N0 = permute_f4<PERMUTE_Y, PERMUTE_Z, PERMUTE_X, PERMUTE_W>(normal);
			float4 N1 = permute_f4<PERMUTE_Z, PERMUTE_X, PERMUTE_Y, PERMUTE_W>(normal);

			float4 V0 = mul_f4(C2, N0);
			V0 = mul_f4(V0, N1);

			float4 R0 = mul_f4(C2, normal);
			R0 = muladd_f4(R0, normal, C1);

			float4 R1 = muladd_f4(C0, normal, V0);
			float4 R2 = negmuladd_f4(C0, normal, V0);

			V0 = select_f4<SELECT_B, SELECT_B, SELECT_B, SELECT_A>(A, R0);
			float4 V1 = permute2_f4<PERMUTE_AZ, PERMUTE_BY, PERMUTE_BZ, PERMUTE_AX>(R1, R2);
			float4 V2 = permute2_f4<PERMUTE_AY, PERMUTE_BX, PERMUTE_AY, PERMUTE_BX>(R1, R2);

			float4 D0 = permute2_f4<PERMUTE_AX, PERMUTE_BX, PERMUTE_BY, PERMUTE_AW>(V0, V1);
			float4 D1 = permute2_f4<PERMUTE_BZ, PERMUTE_AY, PERMUTE_BW, PERMUTE_AW>(V0, V1);
			float4 D2 = permute2_f4<PERMUTE_BX, PERMUTE_BY, PERMUTE_AZ, PERMUTE_AW>(V0, V2);
			float4 D3 = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
			return setf4_f4x4(D0, D1, D2, D3);
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_rotation_euler_angles_f4x4(float4 pitch_yaw_roll)
		{
			float4  Sign = set_f4(1.0f, -1.0f, -1.0f, 1.0f);

			float4 cosine;
			float4 sine = sincos_f4(cosine, pitch_yaw_roll);

			float4 P0 = permute2_f4<PERMUTE_BX, PERMUTE_AZ, PERMUTE_BZ, PERMUTE_BX>(sine, cosine);
			float4 Y0 = permute2_f4<PERMUTE_AY, PERMUTE_BX, PERMUTE_BX, PERMUTE_BY>(sine, cosine);
			float4 P1 = permute2_f4<PERMUTE_BZ, PERMUTE_AZ, PERMUTE_BZ, PERMUTE_AZ>(sine, cosine);
			float4 Y1 = permute2_f4<PERMUTE_BY, PERMUTE_BY, PERMUTE_AY, PERMUTE_AY>(sine, cosine);
			float4 P2 = permute2_f4<PERMUTE_AZ, PERMUTE_BZ, PERMUTE_AZ, PERMUTE_BZ>(sine, cosine);
			float4 P3 = permute2_f4<PERMUTE_AY, PERMUTE_AY, PERMUTE_BY, PERMUTE_BY>(sine, cosine);
			float4 Y2 = dupx_f4(sine);
			float4 NS = sub_f4(setzero_f4(), sine);

			float4 Q0 = mul_f4(P0, Y0);
			float4 Q1 = mul_f4(P1, Sign);
			Q1 = mul_f4(Q1, Y1);
			float4 Q2 = mul_f4(P2, Y2);
			Q2 = muladd_f4(Q2, P3, Q1);

			float4 V0 = permute2_f4<PERMUTE_BX, PERMUTE_AY, PERMUTE_BZ, PERMUTE_AW>(Q0, Q2);
			float4 V1 = permute2_f4<PERMUTE_BY, PERMUTE_AZ, PERMUTE_BW, PERMUTE_AW>(Q0, Q2);
			float4 V2 = permute2_f4<PERMUTE_AX, PERMUTE_BX, PERMUTE_AW, PERMUTE_AW>(Q0, NS);

			float4 D0 = casti_f4(and_i4(castf_i4(V0), set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0)));
			float4 D1 = casti_f4(and_i4(castf_i4(V1), set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0)));
			float4 D2 = casti_f4(and_i4(castf_i4(V2), set_i4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0)));
			float4 D3 = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
			return setf4_f4x4(D0, D1, D2, D3);
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_scaling_f4x4(float4 scaling)
		{
			float4x4 ret;
#if defined(LUNA_AVX_INTRINSICS)
			__m256 s = _mm256_castps128_ps256(scaling);
			s = _mm256_permute2f128_ps(s, s, 0);
			ret.r[0] = _mm256_and_ps(s, _mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0xFFFFFFFF, 0, 0, 0, 0, 0xFFFFFFFF)));
			ret.r[1] = _mm256_and_ps(s, _mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0, 0, 0xFFFFFFFF, 0, 0)));
			ret.r[1] = _mm256_or_ps(ret.r[1], _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
#else
			ret.r[0] = casti_f4(and_i4(castf_i4(scaling), set_i4(0xFFFFFFFF, 0, 0, 0)));
			ret.r[1] = casti_f4(and_i4(castf_i4(scaling), set_i4(0, 0xFFFFFFFF, 0, 0)));
			ret.r[2] = casti_f4(and_i4(castf_i4(scaling), set_i4(0, 0, 0xFFFFFFFF, 0)));
			ret.r[3] = set_f4(0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline float4x4 LUNA_SIMD_CALL transform3d_look_to_f4x4(float4 eye, float4 eyedir, float4 updir)
		{
			float4 rz = eyedir;
			float4 rx = cross3_f4(updir, rz);
			float4 ry = cross3_f4(rz, rx);
			float4 neg_eye = sub_f4(setzero_f4(), eye);

			float4 tx = dot3v_f4(rx, neg_eye);
			float4 ty = dot3v_f4(ry, neg_eye);
			float4 tz = dot3v_f4(rz, neg_eye);
			float4x4 ret = setf4_f4x4(
				select_f4<1, 1, 1, 0>(tx, rx),
				select_f4<1, 1, 1, 0>(ty, ry),
				select_f4<1, 1, 1, 0>(tz, rz),
				set_f4(0.0f, 0.0f, 0.0f, 1.0f)
			);
			return transpose_f4x4(ret);
		}
	}
}
#endif
