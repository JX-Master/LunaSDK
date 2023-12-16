/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Transform.inl
* @author JXMaster
* @date 2022/3/26
 */
#pragma once
#include "../Transform.hpp"
#include "../SimdTransform.hpp"

namespace Luna
{
	namespace AffineMatrix
	{
		inline Float3x3 make(const Float2& translation, f32 rotation, const Float2& scaling)
		{
			Float3x3 affine_matrix;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 t = load_f2(translation.m);
			float4 s = load_f2(scaling.m);
			float3x4 m = transform2d_f3x4(t, rotation, s);
			store_f3x4(affine_matrix.r[0].m, m);
#else
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			affine_matrix = Float3x3(
				scaling.x * cosine, scaling.x * sine, 0.0f,
				scaling.y * -sine, scaling.y * cosine, 0.0f,
				translation.x, translation.y, 1.0f);
#endif
			return affine_matrix;
		}
		inline Float4x4 make(const Float3& translation, const Quaternion& rotation, const Float3& scaling)
		{
			Float4x4 affine_matrix;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 t = load_f4(translation.m);
			float4 r = load_f4(rotation.m);
			float4 s = load_f4(scaling.m);
			float4x4 res = transform3d_f4x4(t, r, s);
			store_f4x4(affine_matrix.r[0].m, res);
#else
			affine_matrix = make_scaling(scaling);
			affine_matrix = mul(affine_matrix, make_rotation(rotation));
			affine_matrix.r[3] = Float4(translation.x, translation.y, translation.z, 1.0f);
#endif	
			return affine_matrix;
		}
		inline Float2 up(const Float3x3& affine_matrix)
		{
			return affine_matrix.r[1].xy();
		}
		inline Float3 up(const Float4x4& affine_matrix)
		{
			return affine_matrix.r[1].xyz();
		}
		inline Float2 down(const Float3x3& affine_matrix)
		{
			return -affine_matrix.r[1].xy();
		}
		inline Float3 down(const Float4x4& affine_matrix)
		{
			return -affine_matrix.r[1].xyz();
		}
		inline Float2 left(const Float3x3& affine_matrix)
		{
			return -affine_matrix.r[0].xy();
		}
		inline Float3 left(const Float4x4& affine_matrix)
		{
			return -affine_matrix.r[0].xyz();
		}
		inline Float2 right(const Float3x3& affine_matrix)
		{
			return affine_matrix.r[0].xy();
		}
		inline Float3 right(const Float4x4& affine_matrix)
		{
			return affine_matrix.r[0].xyz();
		}
		inline Float3 forward(const Float4x4& affine_matrix)
		{
			return affine_matrix.r[2].xyz();
		}
		inline Float3 backward(const Float4x4& affine_matrix)
		{
			return -affine_matrix.r[2].xyz();
		}
		inline Float2 translation(const Float3x3& affine_matrix)
		{
			return affine_matrix.r[2].xy();
		}
		inline Float3 translation(const Float4x4& affine_matrix)
		{
			return affine_matrix.r[3].xyz();
		}
		inline f32 rotation(const Float3x3& affine_matrix)
		{
			if (affine_matrix.r[0].x != 0.0f || affine_matrix.r[0].y != 0.0f)
			{
				return atan2f(affine_matrix.r[0].y, affine_matrix.r[0].x);
			}
			else if (affine_matrix.r[1].x != 0.0f || affine_matrix.r[1].y != 0.0f)
			{
				return atan2f(-affine_matrix.r[1].x, affine_matrix.r[1].y);
			}
			else
			{
				// Both scalex and scaley is 0, rotation is meaningless.
				return 0.0f;
			}
		}
		inline Quaternion rotation(const Float4x4& affine_matrix)
		{
			return Quaternion::from_euler_angles(euler_angles(affine_matrix));
		}
		inline Float3 euler_angles(const Float4x4& affine_matrix)
		{
			// Roll, Pitch, Yaw (ZXY).
			Float3 v;
			if (affine_matrix.r[2].y < 0.999f)
			{
				if (affine_matrix.r[2].y > -0.999f)
				{
					v.x = -asinf(affine_matrix.r[2].y);
					v.z = -atan2f(-affine_matrix.r[0].y, affine_matrix.r[1].y);
					v.y = -atan2f(-affine_matrix.r[2].x, affine_matrix.r[2].z);
				}
				else
				{
					v.x = PI / 2.0f;
					v.z = atan2f(affine_matrix.r[0].z, affine_matrix.r[0].x);
					v.y = 0.0f;
				}
			}
			else
			{
				v.x = -PI / 2.0f;
				v.z = -atan2f(affine_matrix.r[0].z, affine_matrix.r[0].x);
				v.y = 0.0f;
			}
			return v;
		}
		inline Float2 scaling(const Float3x3& affine_matrix)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 r0 = load_f4(affine_matrix.r[0].m);
			float4 r1 = load_f4(affine_matrix.r[1].m);
			float4 d0 = dot2v_f4(r0, r0);
			float4 d1 = dot2v_f4(r1, r1);
			d0 = select_f4<0, 1, 0, 0>(d0, d1);
			d0 = sqrt_f4(d0);
			Float2 ret;
			store_f2(ret.m, d0);
			return ret;
#else
			return Float2(
				sqrtf(affine_matrix.r[0].x * affine_matrix.r[0].x + affine_matrix.r[0].y * affine_matrix.r[0].y),
				sqrtf(affine_matrix.r[1].x * affine_matrix.r[1].x + affine_matrix.r[1].y * affine_matrix.r[1].y));
#endif
		}
		inline Float3 scaling(const Float4x4& affine_matrix)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 v1 = set_f4(affine_matrix.r[0].x, affine_matrix.r[1].x, affine_matrix.r[2].x, 0.0f);
			float4 v2 = set_f4(affine_matrix.r[0].y, affine_matrix.r[1].y, affine_matrix.r[2].y, 0.0f);
			float4 v3 = set_f4(affine_matrix.r[0].z, affine_matrix.r[1].z, affine_matrix.r[2].z, 0.0f);
			float4 res = mul_f4(v1, v1);
			res = muladd_f4(v2, v2, res);
			res = muladd_f4(v3, v3, res);
			res = sqrt_f4(res);
			Float3 ret;
			store_f4(ret.m, res);
			return ret;
#else
			return Float3(
				sqrtf(affine_matrix.r[0].x * affine_matrix.r[0].x + affine_matrix.r[0].y * affine_matrix.r[0].y + affine_matrix.r[0].z * affine_matrix.r[0].z),
				sqrtf(affine_matrix.r[1].x * affine_matrix.r[1].x + affine_matrix.r[1].y * affine_matrix.r[1].y + affine_matrix.r[1].z * affine_matrix.r[1].z),
				sqrtf(affine_matrix.r[2].x * affine_matrix.r[2].x + affine_matrix.r[2].y * affine_matrix.r[2].y + affine_matrix.r[2].z * affine_matrix.r[2].z)
			);
#endif
		}
		inline Float3x3 translation_matrix(const Float3x3& affine_matrix)
		{
			Float3x3 ret;
			ret.r[0] = Float3(1.0f, 0.0f, 0.0f);
			ret.r[1] = Float3(0.0f, 1.0f, 0.0f);
			ret.r[2] = affine_matrix.r[2];
			return ret;
		}
		inline Float4x4 translation_matrix(const Float4x4& affine_matrix)
		{
			Float4x4 ret;
			ret.r[0] = Float4(1.0f, 0.0f, 0.0f, 0.0f);
			ret.r[1] = Float4(0.0f, 1.0f, 0.0f, 0.0f);
			ret.r[2] = Float4(0.0f, 0.0f, 1.0f, 0.0f);
			ret.r[3] = affine_matrix.r[3];
			return ret;
		}
		inline Float3x3 rotation_matrix(const Float3x3& affine_matrix)
		{
			Float2 scale = scaling(affine_matrix);
			Float3x3 ret;
			ret.r[0] = (scale.x == 0.0f) ? affine_matrix.r[0] : (affine_matrix.r[0] / scale.x);
			ret.r[1] = (scale.y == 0.0f) ? affine_matrix.r[1] : (affine_matrix.r[1] / scale.y);
			ret.r[2] = Float3(0.0f, 0.0f, 1.0f);
			return ret;
		}
		inline Float4x4 rotation_matrix(const Float4x4& affine_matrix)
		{
			Float3 scale = scaling(affine_matrix);
			Float4x4 ret;
			ret.r[0] = (scale.x == 0.0f) ? affine_matrix.r[0] : (affine_matrix.r[0] / scale.x);
			ret.r[1] = (scale.y == 0.0f) ? affine_matrix.r[1] : (affine_matrix.r[1] / scale.y);
			ret.r[2] = (scale.z == 0.0f) ? affine_matrix.r[2] : (affine_matrix.r[2] / scale.z);
			ret.r[3] = Float4(0.0f, 0.0f, 0.0f, 1.0f);
			return ret;
		}
		inline Float3x3 scaling_matrix(const Float3x3& affine_matrix)
		{
			Float2 scale = scaling(affine_matrix);
			return make_scaling(scale);
		}
		inline Float4x4 scaling_matrix(const Float4x4& affine_matrix)
		{
			return make_scaling(scaling(affine_matrix));
		}
		inline Float3x3 make_translation(const Float2& translation)
		{
			Float3x3 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = load_f2(translation.m);
			float3x4 m = transform2d_translation_f3x4(a);
			store_f3x4(ret.r[0].m, m);
#else
			ret = Float3x3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, translation.x, translation.y, 1.0f);
#endif
			return ret;
		}
		inline Float3x3 make_translation(f32 x, f32 y)
		{
			Float3x3 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = set_f4(x, y, 0.0f, 0.0f);
			float3x4 m = transform2d_translation_f3x4(a);
			store_f3x4(ret.r[0].m, m);
#else
			ret = Float3x3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, x, y, 1.0f);
#endif
			return ret;
		}
		inline Float4x4 make_translation(const Float3& translation)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = load_f4(translation.m);
			a = setw_f4(a, 1.0f);
			float4x4 m = transform3d_translation_f4x4(a);
			store_f4x4(ret.r[0].m, m);
#else
			ret.r[0] = Float4(1.0f, 0.0f, 0.0f, 0.0f);
			ret.r[1] = Float4(0.0f, 1.0f, 0.0f, 0.0f);
			ret.r[2] = Float4(0.0f, 0.0f, 1.0f, 0.0f);
			ret.r[3] = Float4(translation.x, translation.y, translation.z, 1.0f);
#endif
			return ret;
		}
		inline Float4x4 make_translation(f32 x, f32 y, f32 z)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = set_f4(x, y, z, 0.0f);
			float4x4 m = transform3d_translation_f4x4(a);
			store_f4x4(ret.r[0].m, m);
#else
			ret.r[0] = Float4(1.0f, 0.0f, 0.0f, 0.0f);
			ret.r[1] = Float4(0.0f, 1.0f, 0.0f, 0.0f);
			ret.r[2] = Float4(0.0f, 0.0f, 1.0f, 0.0f);
			ret.r[3] = Float4(x, y, z, 1.0f);
#endif
			return ret;
		}
		inline Float3x3 make_rotation(f32 rotation)
		{
			Float3x3 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float3x4 m = transform2d_rotation_f3x4(rotation);
			store_f3x4(ret.r[0].m, m);
#else
			f32 sine = sinf(rotation);
			f32 cosine = cosf(rotation);
			ret = Float3x3(cosine, sine, 0.0f, -sine, cosine, 0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline Float4x4 make_rotation(const Quaternion& rotation)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = load_f4(rotation.m);
			float4x4 m = transform3d_rotation_quaternion_f4x4(a);
			store_f4x4(ret.r[0].m, m);
#else
			f32 qx = rotation.x;
			f32 qxx = qx * qx;
			f32 qy = rotation.y;
			f32 qyy = qy * qy;
			f32 qz = rotation.z;
			f32 qzz = qz * qz;
			f32 qw = rotation.w;
			Float4x4& mat = ret;
			mat.r[0].x = 1.f - 2.f * qyy - 2.f * qzz;
			mat.r[0].y = 2.f * qx * qy + 2.f * qz * qw;
			mat.r[0].z = 2.f * qx * qz - 2.f * qy * qw;
			mat.r[0].w = 0.f;
			mat.r[1].x = 2.f * qx * qy - 2.f * qz * qw;
			mat.r[1].y = 1.f - 2.f * qxx - 2.f * qzz;
			mat.r[1].z = 2.f * qy * qz + 2.f * qx * qw;
			mat.r[1].w = 0.f;
			mat.r[2].x = 2.f * qx * qz + 2.f * qy * qw;
			mat.r[2].y = 2.f * qy * qz - 2.f * qx * qw;
			mat.r[2].z = 1.f - 2.f * qxx - 2.f * qyy;
			mat.r[2].w = 0.f;
			mat.r[3].x = 0.f;
			mat.r[3].y = 0.f;
			mat.r[3].z = 0.f;
			mat.r[3].w = 1.0f;
#endif
			return ret;
		}
		inline Float4x4 make_rotation_x(f32 angle)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4x4 m = transform3d_rotation_x_f4x4(angle);
			store_f4x4(ret.r[0].m, m);
#else
			f32 sine = sinf(angle);
			f32 cosine = cosf(angle);
			Float4x4& mat = ret;
			mat.r[0].x = 1.0f;
			mat.r[0].y = 0.0f;
			mat.r[0].z = 0.0f;
			mat.r[0].w = 0.0f;
			mat.r[1].x = 0.0f;
			mat.r[1].y = cosine;
			mat.r[1].z = sine;
			mat.r[1].w = 0.0f;
			mat.r[2].x = 0.0f;
			mat.r[2].y = -sine;
			mat.r[2].z = cosine;
			mat.r[2].w = 0.0f;
			mat.r[3].x = 0.0f;
			mat.r[3].y = 0.0f;
			mat.r[3].z = 0.0f;
			mat.r[3].w = 1.0f;
#endif
			return ret;
		}
		inline Float4x4 make_rotation_y(f32 angle)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4x4 m = transform3d_rotation_y_f4x4(angle);
			store_f4x4(ret.r[0].m, m);
#else
			f32 sine = sinf(angle);
			f32 cosine = cosf(angle);
			Float4x4& mat = ret;
			mat.r[0].x = cosine;
			mat.r[0].y = 0.0f;
			mat.r[0].z = -sine;
			mat.r[0].w = 0.0f;
			mat.r[1].x = 0.0f;
			mat.r[1].y = 1.0f;
			mat.r[1].z = 0.0f;
			mat.r[1].w = 0.0f;
			mat.r[2].x = sine;
			mat.r[2].y = 0.0f;
			mat.r[2].z = cosine;
			mat.r[2].w = 0.0f;
			mat.r[3].x = 0.0f;
			mat.r[3].y = 0.0f;
			mat.r[3].z = 0.0f;
			mat.r[3].w = 1.0f;
#endif
			return ret;
		}
		inline Float4x4 make_rotation_z(f32 angle)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4x4 m = transform3d_rotation_z_f4x4(angle);
			store_f4x4(ret.r[0].m, m);
#else
			f32 sine = sinf(angle);
			f32 cosine = cosf(angle);
			Float4x4& mat = ret;
			mat.r[0].x = cosine;
			mat.r[0].y = sine;
			mat.r[0].z = 0.0f;
			mat.r[0].w = 0.0f;
			mat.r[1].x = -sine;
			mat.r[1].y = cosine;
			mat.r[1].z = 0.0f;
			mat.r[1].w = 0.0f;
			mat.r[2].x = 0.0f;
			mat.r[2].y = 0.0f;
			mat.r[2].z = 1.0f;
			mat.r[2].w = 0.0f;
			mat.r[3].x = 0.0f;
			mat.r[3].y = 0.0f;
			mat.r[3].z = 0.0f;
			mat.r[3].w = 1.0f;
#endif
			return ret;
		}
		inline Float4x4 make_rotation_axis_angle(const Float3& axis, f32 angle)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 n = load_f4(axis.m);
			n = normalize3_f4(n);
			float4x4 m = transform3d_rotation_normal_angle_f4x4(n, angle);
			store_f4x4(ret.r[0].m, m);
#else
			Float3 norm = normalize(axis);
			f32    sine = sinf(angle);
			f32    cosine = cosf(angle);
			Float4 A(sine, cosine, 1.0f - cosine, 0.0f);
			Float4 C2(A.z, A.z, A.z, A.z);
			Float4 C1(A.y, A.y, A.y, A.y);
			Float4 C0(A.x, A.x, A.x, A.x);
			Float4 N0(norm.y, norm.z, norm.x, 0.0f);
			Float4 N1(norm.z, norm.x, norm.y, 0.0f);
			Float4 V0 = C2 * N0;
			V0 = V0 * N1;
			Float4 R0 = C2 * Float4(norm.x, norm.y, norm.z, 0.0f);
			R0 = (R0 * Float4(norm.x, norm.y, norm.z, 0.0f)) + C1;
			Float4 R1 = (C0 * Float4(norm.x, norm.y, norm.z, 0.0f)) + V0;
			Float4 R2 = C0 - (Float4(norm.x, norm.y, norm.z, 0.0f) * V0);
			V0 = Float4(R0.x, R0.y, R0.z, A.w);
			Float4 V1(R1.z, R2.y, R2.z, R1.x);
			Float4 V2(R1.y, R2.x, R1.y, R2.x);
			ret = Float4x4(
				V0.x, V1.x, V1.y, V0.w,
				V1.z, V0.y, V1.w, V0.w,
				V2.x, V2.y, V0.z, V0.w,
				0.0f, 0.0f, 0.0f, 1.0f
			);
#endif
			return ret;
		}
		inline Float4x4 make_rotation_euler_angles(const Float3& euler_angles)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = load_f4(euler_angles.m);
			float4x4 m = transform3d_rotation_euler_angles_f4x4(a);
			Float4x4 ret;
			store_f4x4(ret.r[0].m, m);
			return ret;
#else
			return make_rotation(Quaternion::from_euler_angles(euler_angles));
#endif
		}
		inline Float4x4 make_rotation_euler_angles(f32 pitch, f32 yaw, f32 roll)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = set_f4(pitch, yaw, roll, 0.0f);
			float4x4 m = transform3d_rotation_euler_angles_f4x4(a);
			Float4x4 ret;
			store_f4x4(ret.r[0].m, m);
			return ret;
#else
			return make_rotation(Quaternion::from_euler_angles(pitch, yaw, roll));
#endif
		}
		inline Float3x3 make_scaling(const Float2& scaling)
		{
			Float3x3 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = load_f2(scaling.m);
			float3x4 m = transform2d_scaling_f3x4(a);
			store_f3x4(ret.r[0].m, m);
#else
			ret = Float3x3(scaling.x, 0.0f, 0.0f, 0.0f, scaling.y, 0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline Float3x3 make_scaling(f32 scale_x, f32 scale_y)
		{
			Float3x3 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = set_f4(scale_x, scale_y, 0.0f, 0.0f);
			float3x4 m = transform2d_scaling_f3x4(a);
			store_f3x4(ret.r[0].m, m);
#else
			ret = Float3x3(scale_x, 0.0f, 0.0f, 0.0f, scale_y, 0.0f, 0.0f, 0.0f, 1.0f);
#endif
			return ret;
		}
		inline Float4x4 make_scaling(const Float3& scaling)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = load_f4(scaling.m);
			float4x4 m = transform3d_scaling_f4x4(a);
			store_f4x4(ret.r[0].m, m);
#else
			ret.r[0].x = scaling.x;
			ret.r[0].y = 0.0f;
			ret.r[0].z = 0.0f;
			ret.r[0].w = 0.0f;
			ret.r[1].x = 0.0f;
			ret.r[1].y = scaling.y;
			ret.r[1].z = 0.0f;
			ret.r[1].w = 0.0f;
			ret.r[2].x = 0.0f;
			ret.r[2].y = 0.0f;
			ret.r[2].z = scaling.z;
			ret.r[2].w = 0.0f;
			ret.r[3].x = 0.0f;
			ret.r[3].y = 0.0f;
			ret.r[3].z = 0.0f;
			ret.r[3].w = 1.0f;
#endif
			return ret;
		}
		inline Float4x4 make_scaling(f32 scale_x, f32 scale_y, f32 scale_z)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 a = set_f4(scale_x, scale_y, scale_z, 0.0f);
			float4x4 m = transform3d_scaling_f4x4(a);
			store_f4x4(ret.r[0].m, m);
#else
			ret.r[0].x = scale_x;
			ret.r[0].y = 0.0f;
			ret.r[0].z = 0.0f;
			ret.r[0].w = 0.0f;
			ret.r[1].x = 0.0f;
			ret.r[1].y = scale_y;
			ret.r[1].z = 0.0f;
			ret.r[1].w = 0.0f;
			ret.r[2].x = 0.0f;
			ret.r[2].y = 0.0f;
			ret.r[2].z = scale_z;
			ret.r[2].w = 0.0f;
			ret.r[3].x = 0.0f;
			ret.r[3].y = 0.0f;
			ret.r[3].z = 0.0f;
			ret.r[3].w = 1.0f;
#endif
			return ret;
		}
		inline Float4x4 make_look_at(const Float3& eye_pos, const Float3& target_pos, const Float3& up_dir)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 R0 = load_f4(eye_pos.m);
			float4 R1 = load_f4(target_pos.m);
			float4 R2 = load_f4(up_dir.m);
			R1 = sub_f4(R1, R0);
			R1 = normalize3_f4(R1);
			float4x4 m = transform3d_look_to_f4x4(R0, R1, R2);
			Float4x4 ret;
			store_f4x4(ret.r[0].m, m);
			return ret;
#else
			Float3 eye_dir = normalize(target_pos - eye_pos);
			return make_look_to(eye_pos, eye_dir, up_dir);
#endif	
		}
		inline Float4x4 make_look_to(const Float3& eye_pos, const Float3& eye_dir, const Float3& up_dir)
		{
			Float4x4 ret;
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 R0 = load_f4(eye_pos.m);
			float4 R1 = load_f4(eye_dir.m);
			float4 R2 = load_f4(up_dir.m);
			float4x4 m = transform3d_look_to_f4x4(R0, R1, R2);
			store_f4x4(ret.r[0].m, m);
#else
			Float3 RZ = eye_dir;
			Float3 RX = cross(up_dir, RZ);
			Float3 RY = cross(RZ, RX);
			Float3 neg_eye = -eye_pos;
			f32 TX = dot(RX, neg_eye);
			f32 TY = dot(RY, neg_eye);
			f32 TZ = dot(RZ, neg_eye);
			ret = Float4x4(
				RX.x, RY.x, RZ.x, 0.0f,
				RX.y, RY.y, RZ.y, 0.0f,
				RX.z, RY.z, RZ.z, 0.0f,
				TX, TY, TZ, 1.0f);
#endif
			return ret;
		}
	}

	namespace ProjectionMatrix
	{
		inline Float4x4 make_perspective(f32 near_width, f32 near_height, f32 near_z, f32 far_z)
		{
			f32 range = far_z / (far_z - near_z);
			f32 two_near_z = near_z + near_z;
			return Float4x4(
				two_near_z / near_width, 0, 0, 0,
				0, two_near_z / near_height, 0, 0,
				0, 0, range, 1.0f,
				0, 0, -range * near_z, 0
			);
		}
		inline Float4x4 make_perspective_fov(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z)
		{
			fov *= 0.5f;
			f32 diagonal = tanf(fov);
			f32 height = diagonal / sqrtf(1 + aspect_ratio * aspect_ratio);
			f32 width = height * aspect_ratio;
			f32 range = far_z / (far_z - near_z);
			return Float4x4(
				1.0f / width, 0, 0, 0,
				0, 1.0f / height, 0, 0,
				0, 0, range, 1.0f,
				0, 0, -range * near_z, 0
			);
		}
		inline Float4x4 make_perspective_fov_w(f32 fov_w, f32 aspect_ratio, f32 near_z, f32 far_z)
		{
			fov_w *= 0.5f;
			f32 width = tanf(fov_w);
			f32 height = width / aspect_ratio;
			f32 range = far_z / (far_z - near_z);
			return Float4x4(
				1.0f / width, 0, 0, 0,
				0, 1.0f / height, 0, 0,
				0, 0, range, 1.0f,
				0, 0, -range * near_z, 0
			);
		}
		inline Float4x4 make_perspective_fov_h(f32 fov_h, f32 aspect_ratio, f32 near_z, f32 far_z)
		{
			fov_h *= 0.5f;
			f32 height = tanf(fov_h);
			f32 width = height * aspect_ratio;
			f32 range = far_z / (far_z - near_z);
			return Float4x4(
				1.0f / width, 0, 0, 0,
				0, 1.0f / height, 0, 0,
				0, 0, range, 1.0f,
				0, 0, -range * near_z, 0
			);
		}
		inline Float4x4 make_perspective_off_center(f32 near_left, f32 near_right, f32 near_bottom, f32 near_top, f32 near_z, f32 far_z)
		{
			f32 two_near_z = near_z + near_z;
			f32 inv_width = 1.0f / (near_right - near_left);
			f32 inv_height = 1.0f / (near_top - near_bottom);
			f32 range = far_z / (far_z - near_z);
			return Float4x4(
				two_near_z * inv_width, 0, 0, 0,
				0, two_near_z * inv_height, 0, 0,
				-(near_left + near_right) * inv_width, -(near_top + near_bottom) * inv_height, range, 1.0f,
				0, 0, -range * near_z, 0
			);
		}
		inline Float4x4 make_orthographic(f32 width, f32 height, f32 near_z, f32 far_z)
		{
			f32 range = 1.0f / (far_z - near_z);
			return Float4x4(
				2.0f / width, 0, 0, 0,
				0, 2.0f / height, 0, 0,
				0, 0, range, 0,
				0, 0, -range * near_z, 1.0f
			);
		}
		inline Float4x4 make_orthographic_off_center(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z)
		{
			f32 inv_width = 1.0f / (right - left);
			f32 inv_height = 1.0f / (top - bottom);
			f32 range = 1.0f / (far_z - near_z);
			return Float4x4(
				inv_width + inv_width, 0, 0, 0,
				0, inv_height + inv_height, 0, 0,
				0, 0, range, 0,
				-(left + right) * inv_width, -(top + bottom) * inv_height, -range * near_z, 1.0f
			);
		}
	}
}
