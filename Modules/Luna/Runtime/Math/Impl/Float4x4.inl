/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Float4x4.inl
* @author JXMaster
* @date 2022/3/17
 */
#pragma once
#include "../Matrix.hpp"
#include "../Simd.hpp"
namespace Luna
{
	inline bool Float4x4::operator== (const Float4x4& rhs) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 x1 = load_f4(r[0].m);
		float4 x2 = load_f4(r[1].m);
		float4 x3 = load_f4(r[2].m);
		float4 x4 = load_f4(r[3].m);
		float4 y1 = load_f4(rhs.r[0].m);
		float4 y2 = load_f4(rhs.r[1].m);
		float4 y3 = load_f4(rhs.r[2].m);
		float4 y4 = load_f4(rhs.r[3].m);
		int4 r1 = cmpeq_f4(x1, y1);
		int4 r2 = cmpeq_f4(x2, y2);
		int4 r3 = cmpeq_f4(x3, y3);
		int4 r4 = cmpeq_f4(x4, y4);
		return (((maskint_i4(r1) == 0x0f) != 0) && ((maskint_i4(r2) == 0x0f) != 0) && 
			((maskint_i4(r3) == 0x0f) != 0) && ((maskint_i4(r4) == 0x0f) != 0));
#else
		return ((r[0].x == rhs.r[0].x) && (r[0].y == rhs.r[0].y) && (r[0].z == rhs.r[0].z) && (r[0].w == rhs.r[0].w) &&
			(r[1].x == rhs.r[1].x) && (r[1].y == rhs.r[1].y) && (r[1].z == rhs.r[1].z) && (r[1].w == rhs.r[1].w) &&
			(r[2].x == rhs.r[2].x) && (r[2].y == rhs.r[2].y) && (r[2].z == rhs.r[2].z) && (r[2].w == rhs.r[2].w) &&
			(r[3].x == rhs.r[3].x) && (r[3].y == rhs.r[3].y) && (r[3].z == rhs.r[3].z) && (r[3].w == rhs.r[3].w));
#endif
	}
	inline bool Float4x4::operator!= (const Float4x4& rhs) const
	{
		return !(*this == rhs);
	}
	inline Float4x4::Float4x4(const Float4& row1, const Float4& row2, const Float4& row3, const Float4& row4)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4 r1 = load_f4(row1.m);
		float4 r2 = load_f4(row2.m);
		float4 r3 = load_f4(row3.m);
		float4 r4 = load_f4(row4.m);
		store_f4(r[0].m, r1);
		store_f4(r[1].m, r2);
		store_f4(r[2].m, r3);
		store_f4(r[3].m, r4);
#else
		r[0].x = row1.x;
		r[0].y = row1.y;
		r[0].z = row1.z;
		r[0].w = row1.w;
		r[1].x = row2.x;
		r[1].y = row2.y;
		r[1].z = row2.z;
		r[1].w = row2.w;
		r[2].x = row3.x;
		r[2].y = row3.y;
		r[2].z = row3.z;
		r[2].w = row3.w;
		r[3].x = row4.x;
		r[3].y = row4.y;
		r[3].z = row4.z;
		r[3].w = row4.w;
#endif
	}
	inline Float4x4 Float4x4::rotation_matrix() const
	{
		Float3 scale = scale_factor();
		return Float4x4(
			r1() / scale.x,
			r2() / scale.y,
			r3() / scale.z,
			Float4(0.0f, 0.0f, 0.0f, 1.0f)
		);
	}
	inline Float3 Float4x4::euler_angles() const
	{
		// Roll, Pitch, Yaw (ZXY).
		Float3 v;
		if (r[2].y < 0.999f)
		{
			if (r[2].y > -0.999f)
			{
				v.x = -asinf(r[2].y);
				v.z = -atan2f(-r[0].y, r[1].y);
				v.y = -atan2f(-r[2].x, r[2].z);
			}
			else
			{
				v.x = PI / 2.0f;
				v.z = atan2f(r[0].z, r[0].x);
				v.y = 0.0f;
			}
		}
		else
		{
			v.x = -PI / 2.0f;
			v.z = -atan2f(r[0].z, r[0].x);
			v.y = 0.0f;
		}
		return v;
	}
	inline Quaternion Float4x4::quaternion() const
	{
		return Quaternion::from_euler_angles(euler_angles());
	}
	inline Float3 Float4x4::scale_factor() const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = set_f4(r[0].x, r[1].x, r[2].x, 0.0f);
		float4 v2 = set_f4(r[0].y, r[1].y, r[2].y, 0.0f);
		float4 v3 = set_f4(r[0].z, r[1].z, r[2].z, 0.0f);
		v1 = mul_f4(v1, v1);
		v2 = mul_f4(v2, v2);
		v3 = mul_f4(v3, v3);
		v1 = add_f4(v1, add_f4(v2, v3));
		v1 = sqrt_f4(v1);
		Float3 ret;
		store_f4(ret.m, v1);
		return ret;
#else
		return Float3(
			sqrtf(r[0].x * r[0].x + r[0].y * r[0].y + r[0].z * r[0].z),
			sqrtf(r[1].x * r[1].x + r[1].y * r[1].y + r[1].z * r[1].z),
			sqrtf(r[2].x * r[2].x + r[2].y * r[2].y + r[2].z * r[2].z)
		);
#endif
	}
	inline Float4x4 Float4x4::operator-() const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat = load_f4x4(r[0].m);
		mat = sub_f1_f4x4(0.0f, mat);
		Float4x4 r;
		store_f4x4(r.r[0].m, mat);
		return r;
#else
		Float4x4 ret;
        ret.r[0].x = -r[0].x;
        ret.r[0].y = -r[0].y;
        ret.r[0].z = -r[0].z;
        ret.r[0].w = -r[0].w;
        ret.r[1].x = -r[1].x;
        ret.r[1].y = -r[1].y;
		ret.r[1].z = -r[1].z;
		ret.r[1].w = -r[1].w;
		ret.r[2].x = -r[2].x;
        ret.r[2].y = -r[2].y;
        ret.r[2].z = -r[2].z;
        ret.r[2].w = -r[2].w;
        ret.r[3].x = -r[3].x;
        ret.r[3].y = -r[3].y;
        ret.r[3].z = -r[3].z;
        ret.r[3].w = -r[3].w;
		return ret;
#endif
	}
	inline Float4x4& Float4x4::operator+=(const Float4x4& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat1 = load_f4x4(r[0].m);
		float4x4 mat2 = load_f4x4(rhs.r[0].m);
		mat1 = add_f4x4(mat1, mat2);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x += rhs.r[0].x;
		r[0].y += rhs.r[0].y;
		r[0].z += rhs.r[0].z;
		r[0].w += rhs.r[0].w;
		r[1].x += rhs.r[1].x;
		r[1].y += rhs.r[1].y;
		r[1].z += rhs.r[1].z;
		r[1].w += rhs.r[1].w;
		r[2].x += rhs.r[2].x;
		r[2].y += rhs.r[2].y;
		r[2].z += rhs.r[2].z;
		r[2].w += rhs.r[2].w;
		r[3].x += rhs.r[3].x;
		r[3].y += rhs.r[3].y;
		r[3].z += rhs.r[3].z;
		r[3].w += rhs.r[3].w;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator-=(const Float4x4& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		float4x4 mat2 = load_f4x4(rhs.r[0].m);
		mat1 = sub_f4x4(mat1, mat2);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x -= rhs.r[0].x;
		r[0].y -= rhs.r[0].y;
		r[0].z -= rhs.r[0].z;
		r[0].w -= rhs.r[0].w;
		r[1].x -= rhs.r[1].x;
		r[1].y -= rhs.r[1].y;
		r[1].z -= rhs.r[1].z;
		r[1].w -= rhs.r[1].w;
		r[2].x -= rhs.r[2].x;
		r[2].y -= rhs.r[2].y;
		r[2].z -= rhs.r[2].z;
		r[2].w -= rhs.r[2].w;
		r[3].x -= rhs.r[3].x;
		r[3].y -= rhs.r[3].y;
		r[3].z -= rhs.r[3].z;
		r[3].w -= rhs.r[3].w;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator*=(const Float4x4& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		float4x4 mat2 = load_f4x4(rhs.r[0].m);
		mat1 = mul_f4x4(mat1, mat2);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x *= rhs.r[0].x;
		r[0].y *= rhs.r[0].y;
		r[0].z *= rhs.r[0].z;
		r[0].w *= rhs.r[0].w;
		r[1].x *= rhs.r[1].x;
		r[1].y *= rhs.r[1].y;
		r[1].z *= rhs.r[1].z;
		r[1].w *= rhs.r[1].w;
		r[2].x *= rhs.r[2].x;
		r[2].y *= rhs.r[2].y;
		r[2].z *= rhs.r[2].z;
		r[2].w *= rhs.r[2].w;
		r[3].x *= rhs.r[3].x;
		r[3].y *= rhs.r[3].y;
		r[3].z *= rhs.r[3].z;
		r[3].w *= rhs.r[3].w;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator/=(const Float4x4& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		float4x4 mat2 = load_f4x4(rhs.r[0].m);
		mat1 = div_f4x4(mat1, mat2);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x /= rhs.r[0].x;
		r[0].y /= rhs.r[0].y;
		r[0].z /= rhs.r[0].z;
		r[0].w /= rhs.r[0].w;
		r[1].x /= rhs.r[1].x;
		r[1].y /= rhs.r[1].y;
		r[1].z /= rhs.r[1].z;
		r[1].w /= rhs.r[1].w;
		r[2].x /= rhs.r[2].x;
		r[2].y /= rhs.r[2].y;
		r[2].z /= rhs.r[2].z;
		r[2].w /= rhs.r[2].w;
		r[3].x /= rhs.r[3].x;
		r[3].y /= rhs.r[3].y;
		r[3].z /= rhs.r[3].z;
		r[3].w /= rhs.r[3].w;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator+=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		mat1 = add_f4x4_f1(mat1, s);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x += s;
		r[0].y += s;
		r[0].z += s;
		r[0].w += s;
		r[1].x += s;
		r[1].y += s;
		r[1].z += s;
		r[1].w += s;
		r[2].x += s;
		r[2].y += s;
		r[2].z += s;
		r[2].w += s;
		r[3].x += s;
		r[3].y += s;
		r[3].z += s;
		r[3].w += s;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator-=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		mat1 = sub_f4x4_f1(mat1, s);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x -= s;
		r[0].y -= s;
		r[0].z -= s;
		r[0].w -= s;
		r[1].x -= s;
		r[1].y -= s;
		r[1].z -= s;
		r[1].w -= s;
		r[2].x -= s;
		r[2].y -= s;
		r[2].z -= s;
		r[2].w -= s;
		r[3].x -= s;
		r[3].y -= s;
		r[3].z -= s;
		r[3].w -= s;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator*=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		mat1 = mul_f4x4_f1(mat1, s);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x *= s;
		r[0].y *= s;
		r[0].z *= s;
		r[0].w *= s;
		r[1].x *= s;
		r[1].y *= s;
		r[1].z *= s;
		r[1].w *= s;
		r[2].x *= s;
		r[2].y *= s;
		r[2].z *= s;
		r[2].w *= s;
		r[3].x *= s;
		r[3].y *= s;
		r[3].z *= s;
		r[3].w *= s;
		return *this;
#endif
	}
	inline Float4x4& Float4x4::operator/=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(r[0].m);
		s = 1.0f / s;
		mat1 = mul_f4x4_f1(mat1, s);
		store_f4x4(r[0].m, mat1);
		return *this;
#else
		r[0].x /= s;
		r[0].y /= s;
		r[0].z /= s;
		r[0].w /= s;
		r[1].x /= s;
		r[1].y /= s;
		r[1].z /= s;
		r[1].w /= s;
		r[2].x /= s;
		r[2].y /= s;
		r[2].z /= s;
		r[2].w /= s;
		r[3].x /= s;
		r[3].y /= s;
		r[3].z /= s;
		r[3].w /= s;
		return *this;
#endif
	}
	inline Float4x4 operator+(const Float4x4& m1, const Float4x4& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		float4x4 mat2 = load_f4x4(m2.r[0].m);
		mat1 = add_f4x4(mat1, mat2);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x + m2.r[0].x;
		result.r[0].y = m1.r[0].y + m2.r[0].y;
		result.r[0].z = m1.r[0].z + m2.r[0].z;
		result.r[0].w = m1.r[0].w + m2.r[0].w;
		result.r[1].x = m1.r[1].x + m2.r[1].x;
		result.r[1].y = m1.r[1].y + m2.r[1].y;
		result.r[1].z = m1.r[1].z + m2.r[1].z;
		result.r[1].w = m1.r[1].w + m2.r[1].w;
		result.r[2].x = m1.r[2].x + m2.r[2].x;
		result.r[2].y = m1.r[2].y + m2.r[2].y;
		result.r[2].z = m1.r[2].z + m2.r[2].z;
		result.r[2].w = m1.r[2].w + m2.r[2].w;
		result.r[3].x = m1.r[3].x + m2.r[3].x;
		result.r[3].y = m1.r[3].y + m2.r[3].y;
		result.r[3].z = m1.r[3].z + m2.r[3].z;
		result.r[3].w = m1.r[3].w + m2.r[3].w;
		return result;
#endif
	}
	inline Float4x4 operator+(const Float4x4& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		mat1 = add_f4x4_f1(mat1, s);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x + s;
		result.r[0].y = m1.r[0].y + s;
		result.r[0].z = m1.r[0].z + s;
		result.r[0].w = m1.r[0].w + s;
		result.r[1].x = m1.r[1].x + s;
		result.r[1].y = m1.r[1].y + s;
		result.r[1].z = m1.r[1].z + s;
		result.r[1].w = m1.r[1].w + s;
		result.r[2].x = m1.r[2].x + s;
		result.r[2].y = m1.r[2].y + s;
		result.r[2].z = m1.r[2].z + s;
		result.r[2].w = m1.r[2].w + s;
		result.r[3].x = m1.r[3].x + s;
		result.r[3].y = m1.r[3].y + s;
		result.r[3].z = m1.r[3].z + s;
		result.r[3].w = m1.r[3].w + s;
		return result;
#endif
	}
	inline Float4x4 operator+(f32 s, const Float4x4& m1)
	{
		return m1 + s;
	}
	inline Float4x4 operator-(const Float4x4& m1, const Float4x4& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		float4x4 mat2 = load_f4x4(m2.r[0].m);
		mat1 = sub_f4x4(mat1, mat2);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x - m2.r[0].x;
		result.r[0].y = m1.r[0].y - m2.r[0].y;
		result.r[0].z = m1.r[0].z - m2.r[0].z;
		result.r[0].w = m1.r[0].w - m2.r[0].w;
		result.r[1].x = m1.r[1].x - m2.r[1].x;
		result.r[1].y = m1.r[1].y - m2.r[1].y;
		result.r[1].z = m1.r[1].z - m2.r[1].z;
		result.r[1].w = m1.r[1].w - m2.r[1].w;
		result.r[2].x = m1.r[2].x - m2.r[2].x;
		result.r[2].y = m1.r[2].y - m2.r[2].y;
		result.r[2].z = m1.r[2].z - m2.r[2].z;
		result.r[2].w = m1.r[2].w - m2.r[2].w;
		result.r[3].x = m1.r[3].x - m2.r[3].x;
		result.r[3].y = m1.r[3].y - m2.r[3].y;
		result.r[3].z = m1.r[3].z - m2.r[3].z;
		result.r[3].w = m1.r[3].w - m2.r[3].w;
		return result;
#endif
	}
	inline Float4x4 operator-(const Float4x4& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		mat1 = sub_f4x4_f1(mat1, s);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x - s;
		result.r[0].y = m1.r[0].y - s;
		result.r[0].z = m1.r[0].z - s;
		result.r[0].w = m1.r[0].w - s;
		result.r[1].x = m1.r[1].x - s;
		result.r[1].y = m1.r[1].y - s;
		result.r[1].z = m1.r[1].z - s;
		result.r[1].w = m1.r[1].w - s;
		result.r[2].x = m1.r[2].x - s;
		result.r[2].y = m1.r[2].y - s;
		result.r[2].z = m1.r[2].z - s;
		result.r[2].w = m1.r[2].w - s;
		result.r[3].x = m1.r[3].x - s;
		result.r[3].y = m1.r[3].y - s;
		result.r[3].z = m1.r[3].z - s;
		result.r[3].w = m1.r[3].w - s;
		return result;
#endif
	}
	inline Float4x4 operator-(f32 s, const Float4x4& m1)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		mat1 = sub_f1_f4x4(s, mat1);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = s - m1.r[0].x;
		result.r[0].y = s - m1.r[0].y;
		result.r[0].z = s - m1.r[0].z;
		result.r[0].w = s - m1.r[0].w;
		result.r[1].x = s - m1.r[1].x;
		result.r[1].y = s - m1.r[1].y;
		result.r[1].z = s - m1.r[1].z;
		result.r[1].w = s - m1.r[1].w;
		result.r[2].x = s - m1.r[2].x;
		result.r[2].y = s - m1.r[2].y;
		result.r[2].z = s - m1.r[2].z;
		result.r[2].w = s - m1.r[2].w;
		result.r[3].x = s - m1.r[3].x;
		result.r[3].y = s - m1.r[3].y;
		result.r[3].z = s - m1.r[3].z;
		result.r[3].w = s - m1.r[3].w;
		return result;
#endif
	}
	inline Float4x4 operator*(const Float4x4& m1, const Float4x4& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		float4x4 mat2 = load_f4x4(m2.r[0].m);
		mat1 = mul_f4x4(mat1, mat2);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x * m2.r[0].x;
		result.r[0].y = m1.r[0].y * m2.r[0].y;
		result.r[0].z = m1.r[0].z * m2.r[0].z;
		result.r[0].w = m1.r[0].w * m2.r[0].w;
		result.r[1].x = m1.r[1].x * m2.r[1].x;
		result.r[1].y = m1.r[1].y * m2.r[1].y;
		result.r[1].z = m1.r[1].z * m2.r[1].z;
		result.r[1].w = m1.r[1].w * m2.r[1].w;
		result.r[2].x = m1.r[2].x * m2.r[2].x;
		result.r[2].y = m1.r[2].y * m2.r[2].y;
		result.r[2].z = m1.r[2].z * m2.r[2].z;
		result.r[2].w = m1.r[2].w * m2.r[2].w;
		result.r[3].x = m1.r[3].x * m2.r[3].x;
		result.r[3].y = m1.r[3].y * m2.r[3].y;
		result.r[3].z = m1.r[3].z * m2.r[3].z;
		result.r[3].w = m1.r[3].w * m2.r[3].w;
		return result;
#endif
	}
	inline Float4x4 operator*(const Float4x4& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		mat1 = mul_f4x4_f1(mat1, s);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x * s;
		result.r[0].y = m1.r[0].y * s;
		result.r[0].z = m1.r[0].z * s;
		result.r[0].w = m1.r[0].w * s;
		result.r[1].x = m1.r[1].x * s;
		result.r[1].y = m1.r[1].y * s;
		result.r[1].z = m1.r[1].z * s;
		result.r[1].w = m1.r[1].w * s;
		result.r[2].x = m1.r[2].x * s;
		result.r[2].y = m1.r[2].y * s;
		result.r[2].z = m1.r[2].z * s;
		result.r[2].w = m1.r[2].w * s;
		result.r[3].x = m1.r[3].x * s;
		result.r[3].y = m1.r[3].y * s;
		result.r[3].z = m1.r[3].z * s;
		result.r[3].w = m1.r[3].w * s;
		return result;
#endif
	}
	inline Float4x4 operator*(f32 s, const Float4x4& m1)
	{
		return m1 * s;
	}
	inline Float4x4 operator/(const Float4x4& m1, const Float4x4& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		float4x4 mat2 = load_f4x4(m2.r[0].m);
		mat1 = div_f4x4(mat1, mat2);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x / m2.r[0].x;
		result.r[0].y = m1.r[0].y / m2.r[0].y;
		result.r[0].z = m1.r[0].z / m2.r[0].z;
		result.r[0].w = m1.r[0].w / m2.r[0].w;
		result.r[1].x = m1.r[1].x / m2.r[1].x;
		result.r[1].y = m1.r[1].y / m2.r[1].y;
		result.r[1].z = m1.r[1].z / m2.r[1].z;
		result.r[1].w = m1.r[1].w / m2.r[1].w;
		result.r[2].x = m1.r[2].x / m2.r[2].x;
		result.r[2].y = m1.r[2].y / m2.r[2].y;
		result.r[2].z = m1.r[2].z / m2.r[2].z;
		result.r[2].w = m1.r[2].w / m2.r[2].w;
		result.r[3].x = m1.r[3].x / m2.r[3].x;
		result.r[3].y = m1.r[3].y / m2.r[3].y;
		result.r[3].z = m1.r[3].z / m2.r[3].z;
		result.r[3].w = m1.r[3].w / m2.r[3].w;
		return result;
#endif
	}
	inline Float4x4 operator/(const Float4x4& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		s = 1.0f / s;
		mat1 = mul_f4x4_f1(mat1, s);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = m1.r[0].x / s;
		result.r[0].y = m1.r[0].y / s;
		result.r[0].z = m1.r[0].z / s;
		result.r[0].w = m1.r[0].w / s;
		result.r[1].x = m1.r[1].x / s;
		result.r[1].y = m1.r[1].y / s;
		result.r[1].z = m1.r[1].z / s;
		result.r[1].w = m1.r[1].w / s;
		result.r[2].x = m1.r[2].x / s;
		result.r[2].y = m1.r[2].y / s;
		result.r[2].z = m1.r[2].z / s;
		result.r[2].w = m1.r[2].w / s;
		result.r[3].x = m1.r[3].x / s;
		result.r[3].y = m1.r[3].y / s;
		result.r[3].z = m1.r[3].z / s;
		result.r[3].w = m1.r[3].w / s;
		return result;
#endif
	}
	inline Float4x4 operator/(f32 s, const Float4x4& m1)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float4x4 mat1 = dup_f4x4(s);
		float4x4 mat2 = load_f4x4(m1.r[0].m);
		mat1 = div_f4x4(mat1, mat2);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		Float4x4 result;
		result.r[0].x = s / m1.r[0].x;
		result.r[0].y = s / m1.r[0].y;
		result.r[0].z = s / m1.r[0].z;
		result.r[0].w = s / m1.r[0].w;
		result.r[1].x = s / m1.r[1].x;
		result.r[1].y = s / m1.r[1].y;
		result.r[1].z = s / m1.r[1].z;
		result.r[1].w = s / m1.r[1].w;
		result.r[2].x = s / m1.r[2].x;
		result.r[2].y = s / m1.r[2].y;
		result.r[2].z = s / m1.r[2].z;
		result.r[2].w = s / m1.r[2].w;
		result.r[3].x = s / m1.r[3].x;
		result.r[3].y = s / m1.r[3].y;
		result.r[3].z = s / m1.r[3].z;
		result.r[3].w = s / m1.r[3].w;
		return result;
#endif
	}
	inline Float4 mul(const Float4& vec, const Float4x4& mat)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 s = dup_f4(vec.x);
		float4 m = load_f4(&(mat.r[0].x));
		float4 r = mul_f4(s, m);
		s = dup_f4(vec.y);
		m = load_f4(&(mat.r[1].x));
		r = muladd_f4(s, m, r);
		s = dup_f4(vec.z);
		m = load_f4(&(mat.r[2].x));
		r = muladd_f4(s, m, r);
		s = dup_f4(vec.w);
		m = load_f4(&(mat.r[3].x));
		r = muladd_f4(s, m, r);
		Float4 result;
		store_f4(result.m, r);
		return result;
#else
		return Float4(
			vec.x * mat.r[0].x + vec.y * mat.r[1].x + vec.z * mat.r[2].x + vec.w * mat.r[3].x,
			vec.x * mat.r[0].y + vec.y * mat.r[1].y + vec.z * mat.r[2].y + vec.w * mat.r[3].y,
			vec.x * mat.r[0].z + vec.y * mat.r[1].z + vec.z * mat.r[2].z + vec.w * mat.r[3].z,
			vec.x * mat.r[0].w + vec.y * mat.r[1].w + vec.z * mat.r[2].w + vec.w * mat.r[3].w);
#endif
	}
	inline Float4 mul(const Float4x4& mat, const Float4& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		Float4 t;
		Float4 result;
		float4 s = load_f4(vec.m);
		float4 m = load_f4(&(mat.r[0].x));
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.x = t.x + t.y + t.z + t.w;
		m = load_f4(&(mat.r[1].x));
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.y = t.x + t.y + t.z + t.w;
		m = load_f4(&(mat.r[2].x));
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.z = t.x + t.y + t.z + t.w;
		m = load_f4(&(mat.r[3].x));
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.w = t.x + t.y + t.z + t.w;
		return result;
#else
		return Float4(
			vec.x * mat.r[0].x + vec.y * mat.r[0].y + vec.z * mat.r[0].z + vec.w * mat.r[0].w,
			vec.x * mat.r[1].x + vec.y * mat.r[1].y + vec.z * mat.r[1].z + vec.w * mat.r[1].w,
			vec.x * mat.r[2].x + vec.y * mat.r[2].y + vec.z * mat.r[2].z + vec.w * mat.r[2].w,
			vec.x * mat.r[3].x + vec.y * mat.r[3].y + vec.z * mat.r[3].z + vec.w * mat.r[3].w);
#endif
	}
	inline Float4x4 mul(const Float4x4& m1, const Float4x4& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat1 = load_f4x4(m1.r[0].m);
		float4x4 mat2 = load_f4x4(m2.r[0].m);
		mat1 = matmul_f4x4(mat1, mat2);
		Float4x4 result;
		store_f4x4(result.r[0].m, mat1);
		return result;
#else
		return Float4x4(
			m1.r[0].x * m2.r[0].x + m1.r[0].y * m2.r[1].x + m1.r[0].z * m2.r[2].x + m1.r[0].w * m2.r[3].x,
			m1.r[0].x * m2.r[0].y + m1.r[0].y * m2.r[1].y + m1.r[0].z * m2.r[2].y + m1.r[0].w * m2.r[3].y,
			m1.r[0].x * m2.r[0].z + m1.r[0].y * m2.r[1].z + m1.r[0].z * m2.r[2].z + m1.r[0].w * m2.r[3].z,
			m1.r[0].x * m2.r[0].w + m1.r[0].y * m2.r[1].w + m1.r[0].z * m2.r[2].w + m1.r[0].w * m2.r[3].w,
			m1.r[1].x * m2.r[0].x + m1.r[1].y * m2.r[1].x + m1.r[1].z * m2.r[2].x + m1.r[1].w * m2.r[3].x,
			m1.r[1].x * m2.r[0].y + m1.r[1].y * m2.r[1].y + m1.r[1].z * m2.r[2].y + m1.r[1].w * m2.r[3].y,
			m1.r[1].x * m2.r[0].z + m1.r[1].y * m2.r[1].z + m1.r[1].z * m2.r[2].z + m1.r[1].w * m2.r[3].z,
			m1.r[1].x * m2.r[0].w + m1.r[1].y * m2.r[1].w + m1.r[1].z * m2.r[2].w + m1.r[1].w * m2.r[3].w,
			m1.r[2].x * m2.r[0].x + m1.r[2].y * m2.r[1].x + m1.r[2].z * m2.r[2].x + m1.r[2].w * m2.r[3].x,
			m1.r[2].x * m2.r[0].y + m1.r[2].y * m2.r[1].y + m1.r[2].z * m2.r[2].y + m1.r[2].w * m2.r[3].y,
			m1.r[2].x * m2.r[0].z + m1.r[2].y * m2.r[1].z + m1.r[2].z * m2.r[2].z + m1.r[2].w * m2.r[3].z,
			m1.r[2].x * m2.r[0].w + m1.r[2].y * m2.r[1].w + m1.r[2].z * m2.r[2].w + m1.r[2].w * m2.r[3].w,
			m1.r[3].x * m2.r[0].x + m1.r[3].y * m2.r[1].x + m1.r[3].z * m2.r[2].x + m1.r[3].w * m2.r[3].x,
			m1.r[3].x * m2.r[0].y + m1.r[3].y * m2.r[1].y + m1.r[3].z * m2.r[2].y + m1.r[3].w * m2.r[3].y,
			m1.r[3].x * m2.r[0].z + m1.r[3].y * m2.r[1].z + m1.r[3].z * m2.r[2].z + m1.r[3].w * m2.r[3].z,
			m1.r[3].x * m2.r[0].w + m1.r[3].y * m2.r[1].w + m1.r[3].z * m2.r[2].w + m1.r[3].w * m2.r[3].w);
#endif
	}
	inline f32 determinant(const Float4x4& m)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat = load_f4x4(m.r[0].m);
		return determinant_f4x4(mat);
#else
		return
			 m.r[0].x * (m.r[1].y * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[1].z * (m.r[2].w * m.r[3].y - m.r[2].y * m.r[3].w) + m.r[1].w * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y))
			-m.r[0].y * (m.r[1].x * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[1].z * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[1].w * (m.r[2].x * m.r[3].z - m.r[2].z * m.r[3].x))
			+m.r[0].z * (m.r[1].x * (m.r[2].y * m.r[3].w - m.r[2].w * m.r[3].y) + m.r[1].y * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[1].w * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x))
			-m.r[0].w * (m.r[1].x * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y) + m.r[1].y * (m.r[2].z * m.r[3].x - m.r[2].x * m.r[3].z) + m.r[1].z * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x));
#endif
	}
	inline Float4x4 transpose(const Float4x4& m)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat = load_f4x4(m.r[0].m);
		mat = transpose_f4x4(mat);
		Float4x4 res;
		store_f4x4(res.r[0].m, mat);
		return res;
#else
		return Float4x4(
			m.r[0].x, m.r[1].x, m.r[2].x, m.r[3].x,
			m.r[0].y, m.r[1].y, m.r[2].y, m.r[3].y,
			m.r[0].z, m.r[1].z, m.r[2].z, m.r[3].z,
			m.r[0].w, m.r[1].w, m.r[2].w, m.r[3].w
		);
#endif
	}
	inline Float4x4 inverse(const Float4x4& m, f32* out_determinant)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat = load_f4x4(m.r[0].m);
		mat = inverse_f4x4(mat, out_determinant);
		Float4x4 res;
		store_f4x4(res.r[0].m, mat);
		return res;
#else
		f32 det = determinant(m);
		if (out_determinant)
		{
			*out_determinant = det;
		}
		if (det > -F32_EPSILON && det < F32_EPSILON)
		{
			det = F32_EPSILON;
		}
		f32 det_inv = 1.0f / det;
		Float4x4 r;
		r.r[0].x =  det_inv * (m.r[1].y * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[1].z * (m.r[2].w * m.r[3].y - m.r[2].y * m.r[3].w) + m.r[1].w * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y));
		r.r[1].x = -det_inv * (m.r[1].x * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[1].z * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[1].w * (m.r[2].x * m.r[3].z - m.r[2].z * m.r[3].x));
		r.r[2].x =  det_inv * (m.r[1].x * (m.r[2].y * m.r[3].w - m.r[2].w * m.r[3].y) + m.r[1].y * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[1].w * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x));
		r.r[3].x = -det_inv * (m.r[1].x * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y) + m.r[1].y * (m.r[2].z * m.r[3].x - m.r[2].x * m.r[3].z) + m.r[1].z * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x));

		r.r[0].y = -det_inv * (m.r[0].y * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[0].z * (m.r[2].w * m.r[3].y - m.r[2].y * m.r[3].w) + m.r[0].w * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y));
		r.r[1].y =  det_inv * (m.r[0].x * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[0].z * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[0].w * (m.r[2].x * m.r[3].z - m.r[2].z * m.r[3].x));
		r.r[2].y = -det_inv * (m.r[0].x * (m.r[2].y * m.r[3].w - m.r[2].w * m.r[3].y) + m.r[0].y * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[0].w * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x));
		r.r[3].y =  det_inv * (m.r[0].x * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y) + m.r[0].y * (m.r[2].z * m.r[3].x - m.r[2].x * m.r[3].z) + m.r[0].z * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x));

		r.r[0].z =  det_inv * (m.r[3].w * (m.r[0].y * m.r[1].z - m.r[0].z * m.r[1].y) + m.r[3].z * (m.r[0].w * m.r[1].y - m.r[0].y * m.r[1].w) + m.r[3].y * (m.r[0].z * m.r[1].w - m.r[0].w * m.r[1].z));
		r.r[1].z = -det_inv * (m.r[3].w * (m.r[0].x * m.r[1].z - m.r[0].z * m.r[1].x) + m.r[3].z * (m.r[0].w * m.r[1].x - m.r[0].x * m.r[1].w) + m.r[3].x * (m.r[0].z * m.r[1].w - m.r[0].w * m.r[1].z));
		r.r[2].z =  det_inv * (m.r[3].w * (m.r[0].x * m.r[1].y - m.r[0].y * m.r[1].x) + m.r[3].y * (m.r[0].w * m.r[1].x - m.r[0].x * m.r[1].w) + m.r[3].x * (m.r[0].y * m.r[1].w - m.r[0].w * m.r[1].y));
		r.r[3].z = -det_inv * (m.r[3].z * (m.r[0].x * m.r[1].y - m.r[0].y * m.r[1].x) + m.r[3].y * (m.r[0].z * m.r[1].x - m.r[0].x * m.r[1].z) + m.r[3].x * (m.r[0].y * m.r[1].z - m.r[0].z * m.r[1].y));
		
		r.r[0].w = -det_inv * (m.r[2].w * (m.r[0].y * m.r[1].z - m.r[0].z * m.r[1].y) + m.r[2].z * (m.r[0].w * m.r[1].y - m.r[0].y * m.r[1].w) + m.r[2].y * (m.r[0].z * m.r[1].w - m.r[0].w * m.r[1].z));
		r.r[1].w =  det_inv * (m.r[2].w * (m.r[0].x * m.r[1].z - m.r[0].z * m.r[1].x) + m.r[2].z * (m.r[0].w * m.r[1].x - m.r[0].x * m.r[1].w) + m.r[2].x * (m.r[0].z * m.r[1].w - m.r[0].w * m.r[1].z));
		r.r[2].w = -det_inv * (m.r[2].w * (m.r[0].x * m.r[1].y - m.r[0].y * m.r[1].x) + m.r[2].y * (m.r[0].w * m.r[1].x - m.r[0].x * m.r[1].w) + m.r[2].x * (m.r[0].y * m.r[1].w - m.r[0].w * m.r[1].y));
		r.r[3].w =  det_inv * (m.r[2].z * (m.r[0].x * m.r[1].y - m.r[0].y * m.r[1].x) + m.r[2].y * (m.r[0].z * m.r[1].x - m.r[0].x * m.r[1].z) + m.r[2].x * (m.r[0].y * m.r[1].z - m.r[0].z * m.r[1].y));
		return r;
#endif
	}
//	inline Float4x4 Float4x4::make_billboard(const Float3& object_pos, const Float3& camera_pos,
//		const Float3& camera_up, const Float3& camera_forward)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		float4 O = load_f4(object_pos.m);
//		float4 C = load_f4(camera_pos.m);
//		float4 Z = sub_f4(O, C);
//		float4 N = dot3v_f4(Z, Z);
//		if (_mm_cmplt3_ps(N, _mm_setepsilon()))
//		{
//			float4 F = load_f4(camera_forward.m);
//			Z = _mm_neg_ps(F);
//		}
//		else
//		{
//			Z = _mm_normal3_ps(Z);
//		}
//		float4 up = load_f4(camera_up.m);
//		float4 X = _mm_cross3_ps(up, Z);
//		X = _mm_normal3_ps(X);
//		float4 Y = _mm_cross3_ps(Z, X);
//		Float4x4 R;
//		store_f4(&R.r[0].x, X);
//		store_f4(&R.r[1].x, Y);
//		store_f4(&R.r[2].x, Z);
//		store_f4(&R.r[3].x, _mm_setw_ps(O, 1.f));
//		return R;
//#else
//		Float3 Z = object_pos - camera_pos;
//
//		f32 N = length_squared(Z);
//		if (N < 1.192092896e-7f)
//		{
//			Z = -camera_forward;
//		}
//		else
//		{
//			Z = normalize(Z);
//		}
//
//		Float3 X = cross(camera_up, Z);
//		X = normalize(X);
//
//		Float3 Y = cross(Z, X);
//
//		Float4x4 M(
//			X.x, X.y, X.z, 0.0f,
//			Y.x, Y.y, Y.z, 0.0f,
//			Z.x, Z.y, Z.z, 0.0f,
//			object_pos.x, object_pos.y, object_pos.z, 1.0f
//		);
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_constrained_billboard(const Float3& object_pos, const Float3& camera_pos,
//		const Float3& rotate_axis, const Float3& camera_forward, const Float3& object_forward)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		constexpr Float4 s_minAngle = { 0.99825467075f, 0.99825467075f, 0.99825467075f, 0.99825467075f };// 1.0 - ConvertToRadians( 0.1f );
//		float4 min_angle = load_f4(s_minAngle.m);
//		float4 O = load_f4(object_pos.m);
//		float4 C = load_f4(camera_pos.m);
//		float4 faceDir = sub_f4(O, C);
//
//		float4 N = dot3v_f4(faceDir, faceDir);
//		if (_mm_cmplt3_ps(N, _mm_setepsilon()))
//		{
//			float4 F = load_f4(camera_forward.m);
//			faceDir = _mm_neg_ps(F);
//		}
//		else
//		{
//			faceDir = _mm_normal3_ps(faceDir);
//		}
//
//		float4 Y = load_f4(rotate_axis.m);
//		float4 X, Z;
//
//		float4 dot = _mm_abs_ps(dot3v_f4(Y, faceDir));
//		if (_mm_cmpgt3_ps(dot, min_angle))
//		{
//			Z = load_f4(object_forward.m);
//			dot = _mm_abs_ps(dot3v_f4(Y, Z));
//			if (_mm_cmpgt3_ps(dot, min_angle))
//			{
//				dot = _mm_abs_ps(dot3v_f4(Y, _mm_set_f4(0.0f, 0.0f, -1.0f, 0.0f)));
//				Z = (_mm_cmpgt3_ps(dot, min_angle)) ? _mm_set_f4(1.0f, 0.0f, 0.0f, 0.0f) : _mm_set_f4(0.0f, 0.0f, -1.0f, 0.0f);
//			}
//
//			X = _mm_cross3_ps(Y, Z);
//			X = _mm_normal3_ps(X);
//
//			Z = _mm_cross3_ps(X, Y);
//			Z = _mm_normal3_ps(Z);
//		}
//		else
//		{
//			X = _mm_cross3_ps(Y, faceDir);
//			X = _mm_normal3_ps(X);
//
//			Z = _mm_cross3_ps(X, Y);
//			Z = _mm_normal3_ps(Z);
//		}
//		Float4x4 R;
//		store_f4(&R.r[0].x, X);
//		store_f4(&R.r[1].x, Y);
//		store_f4(&R.r[2].x, Z);
//		store_f4(&R.r[3].x, _mm_setw_ps(O, 1.f));
//		return R;
//#else
//		static const f32 s_minAngle{ 0.99825467075f }; // 1.0 - ConvertToRadians( 0.1f );
//
//		//float4 O = LoadFloat3(object.m);
//		//float4 C = LoadFloat3(cameraPosition.m);
//		Float3 faceDir = object_pos - camera_pos;
//
//		f32 N = length_squared(faceDir);
//		if (N < 1.192092896e-7f)
//		{
//			faceDir = -camera_forward;
//		}
//		else
//		{
//			faceDir = normalize(faceDir);
//		}
//
//		Float3 Y = rotate_axis;
//		Float3 X, Z;
//
//		f32 d = fabsf(dot(Y, faceDir));
//		if (d > s_minAngle)
//		{
//			Z = object_forward;
//			d = fabsf(dot(Y, Z));
//			if (d > s_minAngle)
//			{
//				d = fabsf(dot(Y, Float3(0.0f, 0.0f, -1.0f)));
//				Z = (d > s_minAngle) ? Float3(1.0f, 0.0f, 0.0f) : Float3(0.0f, 0.0f, -1.0f);
//			}
//
//			X = cross(Y, Z);
//			X = normalize(X);
//
//			Z = cross(X, Y);
//			Z = normalize(Z);
//		}
//		else
//		{
//			X = cross(Y, faceDir);
//			X = normalize(X);
//
//			Z = cross(X, Y);
//			Z = normalize(Z);
//		}
//
//		Float4x4 M(
//			X.x, X.y, X.z, 0.0f,
//			Y.x, Y.y, Y.z, 0.0f,
//			Z.x, Z.y, Z.z, 0.0f,
//			object_pos.x, object_pos.y, object_pos.z, 1.0f
//		);
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_translation(const Float3& position)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		Float4x4 R;
//		store_f4(&R.r[0].x, set_f4(0.0f, 0.0f, 0.0f, 1.0f));
//		store_f4(&R.r[1].x, set_f4(0.0f, 0.0f, 1.0f, 0.0f));
//		store_f4(&R.r[2].x, set_f4(0.0f, 1.0f, 0.0f, 0.0f));
//		store_f4(&R.r[3].x, set_f4(1.0f, position.z, position.y, position.x));
//		return R;
//#else
//		Float4x4 M;
//		M.m[0][0] = 1.0f;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = 1.0f;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = 1.0f;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = position.x;
//		M.m[3][1] = position.y;
//		M.m[3][2] = position.z;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_scale(const Float3& scales)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		Float4x4 R;
//		store_f4(&R.r[0].x, set_f4(0.0f, 0.0f, 0.0f, scales.x));
//		store_f4(&R.r[1].x, set_f4(0.0f, 0.0f, scales.y, 0.0f));
//		store_f4(&R.r[2].x, set_f4(0.0f, scales.z, 0.0f, 0.0f));
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//		return R;
//#else
//		Float4x4 M;
//		M.m[0][0] = scales.x;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = scales.y;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = scales.z;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = 0.0f;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	namespace Impl
//	{
//		inline void ScalarSinCos
//		(
//			f32* pSin,
//			f32* pCos,
//			f32  Value
//		)
//		{
//			// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
//			f32 quotient = ONE_DIV_TWO_PI * Value;
//			if (Value >= 0.0f)
//			{
//				quotient = static_cast<f32>(static_cast<int>(quotient + 0.5f));
//			}
//			else
//			{
//				quotient = static_cast<f32>(static_cast<int>(quotient - 0.5f));
//			}
//			f32 y = Value - TWO_PI * quotient;
//
//			// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
//			f32 sign;
//			if (y > PI_DIV_TWO)
//			{
//				y = PI - y;
//				sign = -1.0f;
//			}
//			else if (y < -PI_DIV_TWO)
//			{
//				y = -PI - y;
//				sign = -1.0f;
//			}
//			else
//			{
//				sign = +1.0f;
//			}
//
//			f32 y2 = y * y;
//
//			// 11-degree minimax approximation
//			*pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;
//
//			// 10-degree minimax approximation
//			f32 p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
//			*pCos = sign * p;
//		}
//	}
//	inline Float4x4 Float4x4::make_rotation_x(f32 radians)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32    SinAngle;
//		f32    CosAngle;
//		Luna::Impl::ScalarSinCos(&SinAngle, &CosAngle, radians);
//		float4 vSin = _mm_set_ss(SinAngle);
//		float4 vCos = _mm_set_ss(CosAngle);
//		// x = 0,y = cos,z = sin, w = 0
//		vCos = _mm_shuffle_ps(vCos, vSin, _MM_SHUFFLE(3, 0, 0, 3));
//		Float4x4 R;
//		store_f4(&R.r[0].x, set_f4(0.0f, 0.0f, 0.0f, 1.0f));
//		store_f4(&R.r[1].x, vCos);
//		// x = 0,y = sin,z = cos, w = 0
//		vCos = permute_f4<0, 2, 1, 3>(vCos);
//		// x = 0,y = -sin,z = cos, w = 0
//		vCos = mul_f4(vCos, set_f4(1.0f, 1.0f, -1.0f, 1.0f));
//		store_f4(&R.r[2].x, vCos);
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//		return R;
//#else
//		f32    fSinAngle = sinf(radians);
//		f32    fCosAngle = cosf(radians);
//		Float4x4 M;
//		M.m[0][0] = 1.0f;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = fCosAngle;
//		M.m[1][2] = fSinAngle;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = -fSinAngle;
//		M.m[2][2] = fCosAngle;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = 0.0f;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_rotation_y(f32 radians)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32    SinAngle;
//		f32    CosAngle;
//		Luna::Impl::ScalarSinCos(&SinAngle, &CosAngle, radians);
//		float4 vSin = _mm_set_ss(SinAngle);
//		float4 vCos = _mm_set_ss(CosAngle);
//		// x = sin,y = 0,z = cos, w = 0
//		vSin = _mm_shuffle_ps(vSin, vCos, _MM_SHUFFLE(3, 0, 3, 0));
//		Float4x4 R;
//		store_f4(&R.r[2].x, vSin);
//		store_f4(&R.r[1].x, set_f4(0.0f, 0.0f, 1.0f, 0.0f));
//		// x = cos,y = 0,z = sin, w = 0
//		vSin = _mm_permute_ps(vSin, _MM_SHUFFLE(3, 0, 1, 2));
//		// x = cos,y = 0,z = -sin, w = 0
//		vSin = mul_f4(vSin, set_f4(1.0f, -1.0f, 1.0f, 1.0f));
//		store_f4(&R.r[0].x, vSin);
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//		return R;
//#else
//		f32    fSinAngle = sinf(radians);
//		f32    fCosAngle = cosf(radians);
//		Float4x4 M;
//		M.m[0][0] = fCosAngle;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = -fSinAngle;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = 1.0f;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = fSinAngle;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = fCosAngle;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = 0.0f;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_rotation_z(f32 radians)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32    SinAngle;
//		f32    CosAngle;
//		Luna::Impl::ScalarSinCos(&SinAngle, &CosAngle, radians);
//		float4 vSin = _mm_set_ss(SinAngle);
//		float4 vCos = _mm_set_ss(CosAngle);
//		// x = cos,y = sin,z = 0, w = 0
//		vCos = _mm_unpacklo_ps(vCos, vSin);
//		Float4x4 R;
//		store_f4(&R.r[0].x, vCos);
//		// x = sin,y = cos,z = 0, w = 0
//		vCos = _mm_permute_ps(vCos, _MM_SHUFFLE(3, 2, 0, 1));
//		// x = cos,y = -sin,z = 0, w = 0
//		vCos = mul_f4(vCos, set_f4(1.0f, 1.0f, 1.0f, -1.0f));
//		store_f4(&R.r[1].x, vCos);
//		store_f4(&R.r[2].x, set_f4(0.0f, 1.0f, 0.0f, 0.0f));
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//
//#else
//		f32    fSinAngle = sinf(radians);
//		f32    fCosAngle = cosf(radians);
//		Float4x4 M;
//		M.m[0][0] = fCosAngle;
//		M.m[0][1] = fSinAngle;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = -fSinAngle;
//		M.m[1][1] = fCosAngle;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = 1.0f;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = 0.0f;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_rotation_quaternion(const Quaternion& quaternion)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		float4 quat = load_f4(quaternion.m);
//		float4 Q0 = add_f4(quat, quat);
//		float4 Q1 = mul_f4(quat, Q0);
//		float4 V0 = _mm_permute_ps(Q1, _MM_SHUFFLE(3, 0, 0, 1));
//		V0 = _mm_and_ps(V0, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
//		float4 V1 = _mm_permute_ps(Q1, _MM_SHUFFLE(3, 1, 2, 2));
//		V1 = _mm_and_ps(V1, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
//		float4 R0 = sub_f4(set_f4(0.0f, 1.0f, 1.0f, 1.0f), V0);
//		R0 = sub_f4(R0, V1);
//		V0 = _mm_permute_ps(quat, _MM_SHUFFLE(3, 1, 0, 0));
//		V1 = _mm_permute_ps(Q0, _MM_SHUFFLE(3, 2, 1, 2));
//		V0 = mul_f4(V0, V1);
//		V1 = _mm_permute_ps(quat, _MM_SHUFFLE(3, 3, 3, 3));
//		float4 V2 = _mm_permute_ps(Q0, _MM_SHUFFLE(3, 0, 2, 1));
//		V1 = mul_f4(V1, V2);
//		float4 R1 = add_f4(V0, V1);
//		float4 R2 = sub_f4(V0, V1);
//		V0 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(1, 0, 2, 1));
//		V0 = _mm_permute_ps(V0, _MM_SHUFFLE(1, 3, 2, 0));
//		V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 2, 0, 0));
//		V1 = _mm_permute_ps(V1, _MM_SHUFFLE(2, 0, 2, 0));
//		Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(1, 0, 3, 0));
//		Q1 = _mm_permute_ps(Q1, _MM_SHUFFLE(1, 3, 2, 0));
//		Float4x4 R;
//		store_f4(&R.r[0].x, Q1);
//		Q1 = _mm_shuffle_ps(R0, V0, _MM_SHUFFLE(3, 2, 3, 1));
//		Q1 = _mm_permute_ps(Q1, _MM_SHUFFLE(1, 3, 0, 2));
//		store_f4(&R.r[1].x, Q1);
//		Q1 = _mm_shuffle_ps(V1, R0, _MM_SHUFFLE(3, 2, 1, 0));
//		store_f4(&R.r[2].x, Q1);
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//		return R;
//#else
//		f32 qx = quaternion.x;
//		f32 qxx = qx * qx;
//		f32 qy = quaternion.y;
//		f32 qyy = qy * qy;
//		f32 qz = quaternion.z;
//		f32 qzz = qz * qz;
//		f32 qw = quaternion.w;
//		Float4x4 M;
//		M.m[0][0] = 1.f - 2.f * qyy - 2.f * qzz;
//		M.m[0][1] = 2.f * qx * qy + 2.f * qz * qw;
//		M.m[0][2] = 2.f * qx * qz - 2.f * qy * qw;
//		M.m[0][3] = 0.f;
//		M.m[1][0] = 2.f * qx * qy - 2.f * qz * qw;
//		M.m[1][1] = 1.f - 2.f * qxx - 2.f * qzz;
//		M.m[1][2] = 2.f * qy * qz + 2.f * qx * qw;
//		M.m[1][3] = 0.f;
//		M.m[2][0] = 2.f * qx * qz + 2.f * qy * qw;
//		M.m[2][1] = 2.f * qy * qz - 2.f * qx * qw;
//		M.m[2][2] = 1.f - 2.f * qxx - 2.f * qyy;
//		M.m[2][3] = 0.f;
//		M.m[3][0] = 0.f;
//		M.m[3][1] = 0.f;
//		M.m[3][2] = 0.f;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_from_axis_angle(const Float3& axis, f32 angle)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		float4 a = load_f4(axis.m);
//		float4 normal = _mm_normal3_ps(a);
//		f32    fSinAngle;
//		f32    fCosAngle;
//		Luna::Impl::ScalarSinCos(&fSinAngle, &fCosAngle, angle);
//		float4 C2 = dup_f4(1.0f - fCosAngle);
//		float4 C1 = dup_f4(fCosAngle);
//		float4 C0 = dup_f4(fSinAngle);
//		float4 N0 = _mm_permute_ps(normal, _MM_SHUFFLE(3, 0, 2, 1));
//		float4 N1 = _mm_permute_ps(normal, _MM_SHUFFLE(3, 1, 0, 2));
//		float4 V0 = mul_f4(C2, N0);
//		V0 = mul_f4(V0, N1);
//		float4 R0 = mul_f4(C2, normal);
//		R0 = mul_f4(R0, normal);
//		R0 = add_f4(R0, C1);
//		float4 R1 = mul_f4(C0, normal);
//		R1 = add_f4(R1, V0);
//		float4 R2 = mul_f4(C0, normal);
//		R2 = sub_f4(V0, R2);
//		V0 = _mm_and_ps(R0, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
//		float4 V1 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(2, 1, 2, 0));
//		V1 = _mm_permute_ps(V1, _MM_SHUFFLE(0, 3, 2, 1));
//		float4 V2 = _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(0, 0, 1, 1));
//		V2 = _mm_permute_ps(V2, _MM_SHUFFLE(2, 0, 2, 0));
//		R2 = _mm_shuffle_ps(V0, V1, _MM_SHUFFLE(1, 0, 3, 0));
//		R2 = _mm_permute_ps(R2, _MM_SHUFFLE(1, 3, 2, 0));
//		Float4x4 R;
//		store_f4(&R.r[0].x, R2);
//		R2 = _mm_shuffle_ps(V0, V1, _MM_SHUFFLE(3, 2, 3, 1));
//		R2 = _mm_permute_ps(R2, _MM_SHUFFLE(1, 3, 0, 2));
//		store_f4(&R.r[1].x, R2);
//		V2 = _mm_shuffle_ps(V2, V0, _MM_SHUFFLE(3, 2, 1, 0));
//		store_f4(&R.r[2].x, V2);
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//		return R;
//#else
//		Float3 norm = normalize(axis);
//		f32    fSinAngle = sinf(angle);
//		f32    fCosAngle = cosf(angle);
//		Float4 A(fSinAngle, fCosAngle, 1.0f - fCosAngle, 0.0f);
//		Float4 C2(A.z, A.z, A.z, A.z);
//		Float4 C1(A.y, A.y, A.y, A.y);
//		Float4 C0(A.x, A.x, A.x, A.x);
//		Float4 N0(norm.y, norm.z, norm.x, 0.0f);
//		Float4 N1(norm.z, norm.x, norm.y, 0.0f);
//		Float4 V0 = C2 * N0;
//		V0 = V0 * N1;
//		Float4 R0 = C2 * Float4(norm.x, norm.y, norm.z, 0.0f);
//		R0 = (R0 * Float4(norm.x, norm.y, norm.z, 0.0f)) + C1;
//		Float4 R1 = (C0 * Float4(norm.x, norm.y, norm.z, 0.0f)) + V0;
//		Float4 R2 = C0 - (Float4(norm.x, norm.y, norm.z, 0.0f) * V0);
//		V0 = Float4(R0.x, R0.y, R0.z, A.w);
//		Float4 V1(R1.z, R2.y, R2.z, R1.x);
//		Float4 V2(R1.y, R2.x, R1.y, R2.x);
//		Float4x4 M(
//			V0.x, V1.x, V1.y, V0.w,
//			V1.z, V0.y, V1.w, V0.w,
//			V2.x, V2.y, V0.z, V0.w,
//			0.0f, 0.0f, 0.0f, 1.0f
//		);
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_perspective_field_of_view(f32 fov, f32 aspect_ratio,
//		f32 near_plane_distance, f32 far_plance_distance)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32    SinFov;
//		f32    CosFov;
//		Luna::Impl::ScalarSinCos(&SinFov, &CosFov, 0.5f * fov);
//
//		f32 fRange = far_plance_distance / (far_plance_distance - near_plane_distance);
//		// Note: This is recorded on the stack
//		f32 Height = CosFov / SinFov;
//		float4 rMem = {
//			Height / aspect_ratio,
//			Height,
//			fRange,
//			-fRange * near_plane_distance
//		};
//		// Copy from memory to SSE register
//		float4 vValues = rMem;
//		float4 vTemp = _mm_setzero_ps();
//		// Copy x only
//		vTemp = _mm_move_ss(vTemp, vValues);
//		// CosFov / SinFov,0,0,0
//		Float4x4 R;
//		store_f4(&R.r[0].x, vTemp);
//		// 0,Height / AspectRatio,0,0
//		vTemp = vValues;
//		vTemp = _mm_and_ps(vTemp, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000)));
//		store_f4(&R.r[1].x, vTemp);
//		// x=fRange,y=-fRange * NearZ,0,1.0f
//		vTemp = _mm_setzero_ps();
//		vValues = _mm_shuffle_ps(vValues, set_f4(1.0f, 0.0f, 0.0f, 0.0f), _MM_SHUFFLE(3, 2, 3, 2));
//		// 0,0,fRange,1.0f
//		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 0, 0, 0));
//		store_f4(&R.r[2].x, vTemp);
//		// 0,0,-fRange * NearZ,0.0f
//		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 1, 0, 0));
//		store_f4(&R.r[3].x, vTemp);
//		return R;
//#else
//		f32    SinFov = sinf(0.5f * fov);
//		f32    CosFov = cosf(0.5f * fov);
//		f32 Height = CosFov / SinFov;
//		f32 Width = Height / aspect_ratio;
//		f32 fRange = far_plance_distance / (far_plance_distance - near_plane_distance);
//		Float4x4 M;
//		M.m[0][0] = Width;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = Height;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = fRange;
//		M.m[2][3] = 1.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = -fRange * near_plane_distance;
//		M.m[3][3] = 0.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_perspective(f32 width, f32 height, f32 near_plane_distance,
//		f32 far_plane_distance)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32 TwoNearZ = near_plane_distance + near_plane_distance;
//		f32 fRange = far_plane_distance / (far_plane_distance - near_plane_distance);
//		// Note: This is recorded on the stack
//		float4 rMem = {
//			TwoNearZ / width,
//			TwoNearZ / height,
//			fRange,
//			-fRange * near_plane_distance
//		};
//		// Copy from memory to SSE register
//		float4 vValues = rMem;
//		float4 vTemp = _mm_setzero_ps();
//		// Copy x only
//		vTemp = _mm_move_ss(vTemp, vValues);
//		Float4x4 R;
//		// TwoNearZ / ViewWidth,0,0,0
//		store_f4(&R.r[0].x, vTemp);
//		// 0,TwoNearZ / ViewHeight,0,0
//		vTemp = vValues;
//		vTemp = _mm_and_ps(vTemp, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000)));
//		store_f4(&R.r[1].x, vTemp);
//		// x=fRange,y=-fRange * NearZ,0,1.0f
//		vValues = _mm_shuffle_ps(vValues, set_f4(1.0f, 0.0f, 0.0f, 0.0f), _MM_SHUFFLE(3, 2, 3, 2));
//		// 0,0,fRange,1.0f
//		vTemp = _mm_setzero_ps();
//		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 0, 0, 0));
//		store_f4(&R.r[2].x, vTemp);
//		// 0,0,-fRange * NearZ,0
//		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 1, 0, 0));
//		store_f4(&R.r[3].x, vTemp);
//		return R;
//#else
//		f32 TwoNearZ = near_plane_distance + near_plane_distance;
//		f32 fRange = far_plane_distance / (far_plane_distance - near_plane_distance);
//		Float4x4 M;
//		M.m[0][0] = TwoNearZ / width;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = TwoNearZ / height;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = fRange;
//		M.m[2][3] = 1.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = -fRange * near_plane_distance;
//		M.m[3][3] = 0.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_perspective_off_center(f32 left, f32 right, f32 bottom,
//		f32 top, f32 near_plane_distance, f32 far_plance_distance)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32 TwoNearZ = near_plane_distance + near_plane_distance;
//		f32 ReciprocalWidth = 1.0f / (right - left);
//		f32 ReciprocalHeight = 1.0f / (top - bottom);
//		f32 fRange = far_plance_distance / (far_plance_distance - near_plane_distance);
//		// Note: This is recorded on the stack
//		float4 rMem = {
//			TwoNearZ* ReciprocalWidth,
//			TwoNearZ* ReciprocalHeight,
//			-fRange * near_plane_distance,
//			0,
//		};
//		// Copy from memory to SSE register
//		float4 vValues = rMem;
//		float4 vTemp = _mm_setzero_ps();
//		// Copy x only
//		vTemp = _mm_move_ss(vTemp, vValues);
//		Float4x4 R;
//		// TwoNearZ*ReciprocalWidth,0,0,0
//		store_f4(&R.r[0].x, vTemp);
//		// 0,TwoNearZ*ReciprocalHeight,0,0
//		vTemp = vValues;
//		vTemp = _mm_and_ps(vTemp, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000)));
//		store_f4(&R.r[1].x, vTemp);
//		// 0,0,fRange,1.0f
//		store_f4(&R.r[2].x, set_f4(1.0f, fRange,
//			-(top + bottom) * ReciprocalHeight,
//			-(left + right) * ReciprocalWidth));
//		// 0,0,-fRange * NearZ,0.0f
//		vValues = _mm_and_ps(vValues, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000)));
//		store_f4(&R.r[3].x, vValues);
//		return R;
//#else
//		f32 TwoNearZ = near_plane_distance + near_plane_distance;
//		f32 ReciprocalWidth = 1.0f / (right - left);
//		f32 ReciprocalHeight = 1.0f / (top - bottom);
//		f32 fRange = far_plance_distance / (near_plane_distance - far_plance_distance);
//		Float4x4 M;
//		M.m[0][0] = TwoNearZ * ReciprocalWidth;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = TwoNearZ * ReciprocalHeight;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = (left + right) * ReciprocalWidth;
//		M.m[2][1] = (top + bottom) * ReciprocalHeight;
//		M.m[2][2] = fRange;
//		M.m[2][3] = -1.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = fRange * near_plane_distance;
//		M.m[3][3] = 0.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_orthographic(f32 width, f32 height, f32 z_near_place_distance,
//		f32 z_far_plane_distance)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32 fRange = 1.0f / (z_far_plane_distance - z_near_place_distance);
//		// Note: This is recorded on the stack
//		float4 rMem = {
//			2.0f / width,
//			2.0f / height,
//			fRange,
//			-fRange * z_near_place_distance
//		};
//		// Copy from memory to SSE register
//		float4 vValues = rMem;
//		float4 vTemp = _mm_setzero_ps();
//		// Copy x only
//		vTemp = _mm_move_ss(vTemp, vValues);
//		Float4x4 R;
//		// 2.0f / ViewWidth,0,0,0
//		store_f4(&R.r[0].x, vTemp);
//		// 0,2.0f / ViewHeight,0,0
//		vTemp = vValues;
//		vTemp = _mm_and_ps(vTemp, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000)));
//		store_f4(&R.r[1].x, vTemp);
//		// x=fRange,y=-fRange * NearZ,0,1.0f
//		vTemp = _mm_setzero_ps();
//		vValues = _mm_shuffle_ps(vValues, set_f4(1.0f, 0.0f, 0.0f, 0.0f), _MM_SHUFFLE(3, 2, 3, 2));
//		// 0,0,fRange,0.0f
//		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(2, 0, 0, 0));
//		store_f4(&R.r[2].x, vTemp);
//		// 0,0,-fRange * NearZ,1.0f
//		vTemp = _mm_shuffle_ps(vTemp, vValues, _MM_SHUFFLE(3, 1, 0, 0));
//		store_f4(&R.r[3].x, vTemp);
//		return R;
//#else
//		f32 fRange = 1.0f / (z_far_plane_distance - z_near_place_distance);
//		Float4x4 M;
//		M.m[0][0] = 2.0f / width;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = 2.0f / height;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = fRange;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = 0.0f;
//		M.m[3][1] = 0.0f;
//		M.m[3][2] = -fRange * z_near_place_distance;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_orthographic_off_center(f32 left, f32 right, f32 bottom,
//		f32 top, f32 near_plane_distance, f32 far_plance_distance)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		f32 fReciprocalWidth = 1.0f / (right - left);
//		f32 fReciprocalHeight = 1.0f / (top - bottom);
//		f32 fRange = 1.0f / (far_plance_distance - near_plane_distance);
//		// Note: This is recorded on the stack
//		float4 rMem = {
//			fReciprocalWidth,
//			fReciprocalHeight,
//			fRange,
//			1.0f
//		};
//		float4 rMem2 = {
//			-(left + right),
//			-(top + bottom),
//			-near_plane_distance,
//			1.0f
//		};
//		// Copy from memory to SSE register
//		float4 vValues = rMem;
//		float4 vTemp = _mm_setzero_ps();
//		// Copy x only
//		vTemp = _mm_move_ss(vTemp, vValues);
//		Float4x4 R;
//		// fReciprocalWidth*2,0,0,0
//		vTemp = _mm_add_ss(vTemp, vTemp);
//		store_f4(&R.r[0].x, vTemp);
//		// 0,fReciprocalHeight*2,0,0
//		vTemp = vValues;
//		vTemp = _mm_and_ps(vTemp, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000)));
//		vTemp = add_f4(vTemp, vTemp);
//		store_f4(&R.r[1].x, vTemp);
//		// 0,0,fRange,0.0f
//		vTemp = vValues;
//		vTemp = _mm_and_ps(vTemp, _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000)));
//		store_f4(&R.r[2].x, vTemp);
//		// -(ViewLeft + ViewRight)*fReciprocalWidth,-(ViewTop + ViewBottom)*fReciprocalHeight,fRange*-NearZ,1.0f
//		vValues = mul_f4(vValues, rMem2);
//		store_f4(&R.r[3].x, vValues);
//		return R;
//#else
//		f32 ReciprocalWidth = 1.0f / (right - left);
//		f32 ReciprocalHeight = 1.0f / (top - bottom);
//		f32 fRange = 1.0f / (far_plance_distance - near_plane_distance);
//		Float4x4 M;
//		M.m[0][0] = ReciprocalWidth + ReciprocalWidth;
//		M.m[0][1] = 0.0f;
//		M.m[0][2] = 0.0f;
//		M.m[0][3] = 0.0f;
//		M.m[1][0] = 0.0f;
//		M.m[1][1] = ReciprocalHeight + ReciprocalHeight;
//		M.m[1][2] = 0.0f;
//		M.m[1][3] = 0.0f;
//		M.m[2][0] = 0.0f;
//		M.m[2][1] = 0.0f;
//		M.m[2][2] = fRange;
//		M.m[2][3] = 0.0f;
//		M.m[3][0] = -(left + right) * ReciprocalWidth;
//		M.m[3][1] = -(top + bottom) * ReciprocalHeight;
//		M.m[3][2] = -fRange * near_plane_distance;
//		M.m[3][3] = 1.0f;
//		return M;
//#endif
//	}
//	inline Float4x4 Float4x4::make_look_at(const Float3& camera_position, const Float3& target_position, const Float3& up_dir)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		float4 eyev = load_f4(camera_position.m);
//		float4 targetv = load_f4(target_position.m);
//		float4 upv = load_f4(up_dir.m);
//		float4 eye_dir = sub_f4(targetv, eyev);
//
//		float4 R2 = _mm_normal3_ps(eye_dir);
//
//		float4 R0 = _mm_cross3_ps(upv, R2);
//		R0 = _mm_normal3_ps(R0);
//
//		float4 R1 = _mm_cross3_ps(R2, R0);
//
//		float4 NegEyePosition = sub_f4(_mm_setzero_ps(), eyev);
//
//		float4 D0 = dot3v_f4(R0, NegEyePosition);
//		float4 D1 = dot3v_f4(R1, NegEyePosition);
//		float4 D2 = dot3v_f4(R2, NegEyePosition);
//
//		Float4x4 R;
//		float4 select1110 = _mm_castsi128_ps(_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
//		store_f4(&R.r[0].x, _mm_or_ps(_mm_andnot_ps(select1110, D0), _mm_and_ps(R0, select1110)));
//		store_f4(&R.r[1].x, _mm_or_ps(_mm_andnot_ps(select1110, D1), _mm_and_ps(R1, select1110)));
//		store_f4(&R.r[2].x, _mm_or_ps(_mm_andnot_ps(select1110, D2), _mm_and_ps(R2, select1110)));
//		store_f4(&R.r[3].x, set_f4(1.0f, 0.0f, 0.0f, 0.0f));
//		return transpose(R);
//#else
//		Float3 EyeDirection = target_position - camera_position;
//		Float3 R2 = normalize(EyeDirection);
//		Float3 R0 = normalize(cross(up_dir, R2));
//		Float3 R1 = cross(R2, R0);
//		Float3 NegEyePosition = -camera_position;
//		f32 D0 = dot(R0, NegEyePosition);
//		f32 D1 = dot(R1, NegEyePosition);
//		f32 D2 = dot(R2, NegEyePosition);
//		Float4x4 M(
//			R0.x, R0.y, R0.z, D0,
//			R1.x, R1.y, R1.z, D1,
//			R2.x, R2.y, R2.z, D2,
//			0.0f, 0.0f, 0.0f, 1.0f
//		);
//		return transpose(M);
//#endif
//	}
//	inline Float4x4 Float4x4::make_world(const Float3& position, const Float3& forward, const Float3& up)
//	{
//#ifdef LUNA_SIMD
//		using namespace Simd;
//		float4 zaxis = sub_f4(_mm_setzero_ps(), load_f4(forward.m));
//		zaxis = _mm_normal3_ps(zaxis);
//		float4 yaxis = load_f4(up.m);
//
//		float4 xaxis;
//		xaxis = _mm_cross3_ps(yaxis, zaxis);
//		xaxis = _mm_normal3_ps(xaxis);
//
//		yaxis = _mm_cross3_ps(zaxis, xaxis);
//		Float4x4 R;
//		store_f4(reinterpret_cast<f32*>(&R.r[0].x), xaxis);
//		store_f4(reinterpret_cast<f32*>(&R.r[1].x), yaxis);
//		store_f4(reinterpret_cast<f32*>(&R.r[2].x), zaxis);
//		R.r[0].w = R.r[1].w = R.r[2].w = 0.f;
//		R.r[3].x = position.x; R.r[3].y = position.y; R.r[3].z = position.z;
//		R.r[3].w = 1.f;
//		return R;
//#else
//		Float3 zaxis = normalize(-forward);
//		Float3 yaxis = up;
//		Float3 xaxis = normalize(cross(yaxis, zaxis));
//		yaxis = cross(zaxis, xaxis);
//		Float4x4 R(
//			xaxis.x, xaxis.y, xaxis.z, 0.0f,
//			yaxis.x, yaxis.y, yaxis.z, 0.0f,
//			zaxis.x, zaxis.y, zaxis.z, 0.0f,
//			position.x, position.y, position.z, 1.0f
//		);
//		return R;
//#endif
//	}
//	inline Float4x4 Float4x4::make_affine_position_rotation_scale(const Float3& position, const Quaternion& rotation, const Float3& scale)
//	{
//		// Apply scale.
//		Float4x4 ret = Float4x4::make_scale(scale);
//		// Apply rotation.
//		ret = mul(ret, Float4x4::make_rotation_quaternion(rotation));
//		// Apply translation.
//		ret.r[3].x += position.x;
//		ret.r[3].y += position.y;
//		ret.r[3].z += position.z;
//		return ret;
//	}
//	inline Float4x4 Float4x4::make_transform3d(const Tranform3D& transform)
//	{
//		return make_affine_position_rotation_scale(transform.position, transform.rotation, transform.scale);
//	}
}
