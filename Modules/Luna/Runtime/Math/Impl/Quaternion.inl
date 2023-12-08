/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Quaternion.inl
* @author JXMaster
* @date 2022/3/17
 */
#pragma once
#include "../Quaternion.hpp"
#include "../SimdQuaternion.hpp"
namespace Luna
{
	inline bool Quaternion::operator == (const Quaternion& q) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(this->m);
		float4 q2 = load_f4(q.m);
		return maskint_i4(cmpeq_f4(q1, q2)) == 0x0F;
#else
		return (x == q.x) && (y == q.y) && (z == q.z) && (w == q.w);
#endif
	}
	inline bool Quaternion::operator != (const Quaternion& q) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(this->m);
		float4 q2 = load_f4(q.m);
		return maskint_i4(cmpeq_f4(q1, q2)) != 0x0F;
#else
		return (x != q.x) || (y != q.y) || (z != q.z) || (w != q.w);
#endif
	}
	inline Quaternion& Quaternion::operator+= (const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(this->m);
		float4 q2 = load_f4(q.m);
		store_f4(this->m, add_f4(q1, q2));
		return *this;
#else
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
		return *this;
#endif
	}
	inline Quaternion& Quaternion::operator-= (const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(this->m);
		float4 q2 = load_f4(q.m);
		store_f4(this->m, sub_f4(q1, q2));
		return *this;
#else
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;
		return *this;
#endif
	}
	inline Quaternion& Quaternion::operator*= (const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(this->m);
		float4 q2 = load_f4(q.m);
		store_f4(this->m, mulquat_f4(q1, q2));
		return *this;
#else
		x = q.w * x + q.x * w + q.y * z - q.z * y;
		y = q.w * y - q.x * z + q.y * w + q.z * x;
		z = q.w * z + q.x * y - q.y * x + q.z * w;
		w = q.w * w - q.x * x - q.y * y - q.z * z;
#endif
	}
	inline Quaternion& Quaternion::operator*= (f32 S)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q = load_f4(this->m);
		store_f4(this->m, mul_f4(q, dup_f4(S)));
		return *this;
#else
		x *= S;
		y *= S;
		z *= S;
		w *= S;
#endif
	}
	inline Quaternion& Quaternion::operator/= (const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(this->m);
		float4 q2 = load_f4(q.m);
		q2 = quatinverse_f4(q2);
		store_f4(this->m, mulquat_f4(q1, q2));
		return *this;
#else
		Quaternion inv = inverse(q);
		*this = *this * inv;
#endif
	}
	inline Quaternion Quaternion::operator- () const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q = load_f4(this->m);
		Quaternion R;
		store_f4(R.m, sub_f4(setzero_f4(), q));
		return R;
#else
		return Quaternion(-x, -y, -z, -w);
#endif
	}
	inline Quaternion operator+ (const Quaternion& Q1, const Quaternion& Q2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(Q1.m);
		float4 q2 = load_f4(Q2.m);
		Quaternion R;
		store_f4(R.m, add_f4(q1, q2));
		return R;
#else
		return Quaternion(Q1.x + Q2.x, Q1.y + Q2.y, Q1.z + Q2.z, Q1.w + Q2.w);
#endif
	}
	inline Quaternion operator- (const Quaternion& Q1, const Quaternion& Q2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(Q1.m);
		float4 q2 = load_f4(Q2.m);
		Quaternion R;
		store_f4(R.m, sub_f4(q1, q2));
		return R;
#else
		return Quaternion(Q1.x - Q2.x, Q1.y - Q2.y, Q1.z - Q2.z, Q1.w - Q2.w);
#endif
	}
	inline Quaternion operator* (const Quaternion& Q1, const Quaternion& Q2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(Q1.m);
		float4 q2 = load_f4(Q2.m);
		Quaternion R;
		store_f4(R.m, mulquat_f4(q1, q2));
		return R;
#else
		return Quaternion(
			Q2.w * Q1.x + Q2.x * Q1.w + Q2.y * Q1.z - Q2.z * Q1.y,
			Q2.w * Q1.y - Q2.x * Q1.z + Q2.y * Q1.w + Q2.z * Q1.x,
			Q2.w * Q1.z + Q2.x * Q1.y - Q2.y * Q1.x + Q2.z * Q1.w,
			Q2.w * Q1.w - Q2.x * Q1.x - Q2.y * Q1.y - Q2.z * Q1.z);
#endif
	}
	inline Quaternion operator* (const Quaternion& Q, f32 S)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q = load_f4(Q.m);
		Quaternion R;
		store_f4(R.m, mul_f4(q, dup_f4(S)));
		return R;
#else
		return Quaternion(Q.x * S, Q.y * S, Q.z * S, Q.w * S);
#endif
	}
	inline Quaternion operator/ (const Quaternion& Q1, const Quaternion& Q2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(Q1.m);
		float4 q2 = load_f4(Q2.m);
		q2 = quatinverse_f4(q2);
		Quaternion R;
		store_f4(R.m, mulquat_f4(q1, q2));
		return R;
#else
		Quaternion inv = inverse(Q2);
		return Q1 * inv;
#endif
	}
	inline Quaternion operator* (f32 S, const Quaternion& Q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q1 = load_f4(Q.m);
		Quaternion R;
		store_f4(R.m, mul_f4(q1, dup_f4(S)));
		return R;
#else
		return Quaternion(Q.x * S, Q.y * S, Q.z * S, Q.w * S);
#endif
	}
	inline f32 length(Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q_vec = load_f4(q.m);
		return getx_f4(sqrt_f4(dot4v_f4(q_vec, q_vec)));
#else
		return sqrtf((q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w));
#endif
	}
	inline f32 length_squared(Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 q_vec = load_f4(q.m);
		return dot4_f4(q_vec, q_vec);
#else
		return (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
#endif
	}
	inline Quaternion normalize(const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vecq = load_f4(q.m);
		Quaternion result;
		store_f4(result.m, normalize4_f4(vecq));
		return result;
#else
		Quaternion result;
		Float4 res(q.x, q.y, q.z, q.w);
		res = normalize(res);
		result.x = res.x;
		result.y = res.y;
		result.z = res.z;
		result.w = res.w;
		return result;
#endif
	}
	inline Quaternion conjugate(const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vecq = load_f4(q.m);
		Quaternion result;
		store_f4(result.m, mul_f4(set_f4(-1.0f, -1.0f, -1.0f, 1.0f), vecq));
		return result;
#else
		Quaternion result;
		result.x = -q.x;
		result.y = -q.y;
		result.z = -q.z;
		result.w = q.w;
		return result;
#endif
	}
	inline Quaternion inverse(const Quaternion& q)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vecq = load_f4(q.m);
		Quaternion result;
		store_f4(result.m, quatinverse_f4(vecq));
		return result;
#else
		f32 l = length_squared(q);
		Quaternion conjugated = conjugate(q);
		Quaternion result;
		if (l <= 1.192092896e-7f)
		{
			result.x = 0.0f;
			result.y = 0.0f;
			result.z = 0.0f;
			result.w = 0.0f;
		}
		else
		{
			result.x = conjugated.x / l;
			result.y = conjugated.y / l;
			result.z = conjugated.z / l;
			result.w = conjugated.w / l;
		}
		return result;
#endif
	}
	inline f32 dot(const Quaternion& q1, const Quaternion& q2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 qua1 = load_f4(q1.m);
		float4 qua2 = load_f4(q2.m);
		return dot4_f4(qua1, qua2);
#else
		Float4 d1(q1.x, q1.y, q1.z, q1.w);
		Float4 d2(q2.x, q2.y, q2.z, q2.w);
		return dot(d1, d2);
#endif
	}
	inline Quaternion Quaternion::from_axis_angle(const Float3& axis, f32 angle)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 n = load_f4(axis.m);
		n = normalize3_f4(n);
		n = quatnormalangle_f4(n, angle);
		Quaternion result;
		store_f4(result.m, n);
		return result;
#else
		Float3 normal = normalize(axis);
		f32 sinV = sinf(0.5f * angle);
		f32 cosV = cosf(0.5f * angle);
		Quaternion result;
		result.x = normal.x * sinV;
		result.y = normal.y * sinV;
		result.z = normal.z * sinV;
		result.w = cosV;
		return result;
#endif
	}
	inline Quaternion Quaternion::from_euler_angles(const Float3& euler_angles)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 angles = load_f4(euler_angles.m);
		float4 Q = quateulerangles_f4(angles);
		Quaternion result;
		store_f4(result.m, Q);
		return result;
#else
		f32 pitch = euler_angles.x * 0.5f;
		f32 yaw = euler_angles.y * 0.5f;
		f32 roll = euler_angles.z * 0.5f;
		f32 SP = sinf(pitch);
		f32 CP = cosf(pitch);
		f32 SY = sinf(yaw);
		f32 CY = cosf(yaw);
		f32 SR = sinf(roll);
		f32 CR = cosf(roll);
		Quaternion result;
		result.x = CR * SP * CY + SR * CP * SY;
		result.y = CR * CP * SY - SR * SP * CY;
		result.z = SR * CP * CY - CR * SP * SY;
		result.w = CR * CP * CY + SR * SP * SY;
		return result;
#endif
	}
	inline Quaternion Quaternion::from_euler_angles(f32 pitch, f32 yaw, f32 roll)
	{
		return Quaternion::from_euler_angles(Float3(pitch, yaw, roll));
	}
	inline Quaternion lerp(const Quaternion& q1, const Quaternion& q2, f32 t)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 a = load_f4(q1.m);
		float4 b = load_f4(q2.m);
		float4 res = quatlerp_f4(a, b, t);
		Quaternion result;
		store_f4(result.m, res);
		return result;
#else
		Quaternion result;
		f32 cos_omega = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		if (cos_omega >= 0)
		{
			result.x = q1.x + t * (q2.x - q1.x);
			result.y = q1.y + t * (q2.y - q1.y);
			result.z = q1.z + t * (q2.z - q1.z);
			result.w = q1.w + t * (q2.w - q1.w);
		}
		else
		{
			result.x = q1.x * (1.0f - t) - q2.x * t;
			result.y = q1.y * (1.0f - t) - q2.y * t;
			result.z = q1.z * (1.0f - t) - q2.z * t;
			result.w = q1.w * (1.0f - t) - q2.w * t;
		}
		return result;
#endif
	}
	inline Quaternion slerp(const Quaternion& q1, const Quaternion& q2, f32 t)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 a = load_f4(q1.m);
		float4 b = load_f4(q2.m);
		float4 res = quatslerp_f4(a, b, t);
		Quaternion r;
		store_f4(r.m, res);
		return r;
#else
		f32 cos_omega = dot(q1, q2);
		cos_omega = cos_omega < 0 ? -cos_omega : cos_omega;
		f32 sin_omega = sqrtf(1.0f - cos_omega * cos_omega);
		f32 omega = atan2f(sin_omega, cos_omega);
		// Result = Q0 * sin((1.0 - t) * omega) / sin(omega) + Q1 * sin(t * omega) / sin(omega)
		f32 wa = sinf((1.0 - t) * omega) / sin_omega;
		f32 wb = sinf(t * omega) / sin_omega;
		Quaternion result = q1 * wa + q2 * wb;
		return result;
#endif
	}
}