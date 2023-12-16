/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Float3x3.inl
* @author JXMaster
* @date 2022/3/17
 */
#pragma once
#include "../Matrix.hpp"
#include "../Simd.hpp"
namespace Luna
{
	inline bool Float3x3::operator==(const Float3x3& m) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 x1 = load_f4(r[0].m);
		float4 x2 = load_f4(r[1].m);
		float4 x3 = load_f4(r[2].m);
		float4 y1 = load_f4(m.r[0].m);
		float4 y2 = load_f4(m.r[1].m);
		float4 y3 = load_f4(m.r[2].m);
		int4 r1 = cmpeq_f4(x1, y1);
		int4 r2 = cmpeq_f4(x2, y2);
		int4 r3 = cmpeq_f4(x3, y3);
		return (((maskint_i4(r1) & 7) == 7) != 0) &&
			(((maskint_i4(r2) & 7) == 7) != 0) && 
			(((maskint_i4(r3) & 7) == 7) != 0);
#else
		return ((r[0].x == m.r[0].x) && (r[0].y == m.r[0].y) && (r[0].z == m.r[0].z) &&
			(r[1].x == m.r[1].x) && (r[1].y == m.r[1].y) && (r[1].z == m.r[1].z) &&
			(r[2].x == m.r[2].x) && (r[2].y == m.r[2].y) && (r[2].z == m.r[2].z));
#endif
	}
	inline bool Float3x3::operator!=(const Float3x3& m) const
	{
		return !(*this == m);
	}
	inline Float3x3::Float3x3(const Float3& row1, const Float3& row2, const Float3& row3)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 r1 = load_f4(row1.m);
		float4 r2 = load_f4(row2.m);
		float4 r3 = load_f4(row3.m);
		store_f4(r[0].m, r1);
		store_f4(r[1].m, r2);
		store_f4(r[2].m, r3);
#else
		r[0].x = row1.x;
		r[0].y = row1.y;
		r[0].z = row1.z;
		r[1].x = row2.x;
		r[1].y = row2.y;
		r[1].z = row2.z;
		r[2].x = row3.x;
		r[2].y = row3.y;
		r[2].z = row3.z;
#endif
	}
	inline Float3x3 Float3x3::operator-() const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat = load_f3x4(r[0].m);
		mat = sub_f1_f3x4(0.0f, mat);
		Float3x3 r;
		store_f3x4(r.r[0].m, mat);
		return r;
#else
		Float3x3 ret;
        ret.r[0].x = -r[0].x;
        ret.r[0].y = -r[0].y;
        ret.r[0].z = -r[0].z;
        ret.r[1].x = -r[1].x;
        ret.r[1].y = -r[1].y;
        ret.r[1].z = -r[1].z;
        ret.r[2].x = -r[2].x;
        ret.r[2].y = -r[2].y;
        ret.r[2].z = -r[2].z;
		return ret;
#endif
	}
	inline Float3x3& Float3x3::operator+=(const Float3x3& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		float3x4 m2 = load_f3x4(rhs.r[0].m);
		float3x4 res = add_f3x4(m1, m2);
		store_f3x4(r[0].m, res);
		return *this;
#else
		r[0].x += rhs.r[0].x;
		r[0].y += rhs.r[0].y;
		r[0].z += rhs.r[0].z;
		r[1].x += rhs.r[1].x;
		r[1].y += rhs.r[1].y;
		r[1].z += rhs.r[1].z;
		r[2].x += rhs.r[2].x;
		r[2].y += rhs.r[2].y;
		r[2].z += rhs.r[2].z;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator-=(const Float3x3& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		float3x4 m2 = load_f3x4(rhs.r[0].m);
		float3x4 res = sub_f3x4(m1, m2);
		store_f3x4(r[0].m, res);
		return *this;
#else
		r[0].x -= rhs.r[0].x;
		r[0].y -= rhs.r[0].y;
		r[0].z -= rhs.r[0].z;
		r[1].x -= rhs.r[1].x;
		r[1].y -= rhs.r[1].y;
		r[1].z -= rhs.r[1].z;
		r[2].x -= rhs.r[2].x;
		r[2].y -= rhs.r[2].y;
		r[2].z -= rhs.r[2].z;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator*=(const Float3x3& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd; 
		float3x4 m1 = load_f3x4(r[0].m);
		float3x4 m2 = load_f3x4(rhs.r[0].m);
		float3x4 res = mul_f3x4(m1, m2);
		store_f3x4(r[0].m, res);
		return *this;
#else
		r[0].x *= rhs.r[0].x;
		r[0].y *= rhs.r[0].y;
		r[0].z *= rhs.r[0].z;
		r[1].x *= rhs.r[1].x;
		r[1].y *= rhs.r[1].y;
		r[1].z *= rhs.r[1].z;
		r[2].x *= rhs.r[2].x;
		r[2].y *= rhs.r[2].y;
		r[2].z *= rhs.r[2].z;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator/=(const Float3x3& rhs)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		float3x4 m2 = load_f3x4(rhs.r[0].m);
		float3x4 res = div_f3x4(m1, m2);
		store_f3x4(r[0].m, res);
		return *this;
#else
		r[0].x /= rhs.r[0].x;
		r[0].y /= rhs.r[0].y;
		r[0].z /= rhs.r[0].z;
		r[1].x /= rhs.r[1].x;
		r[1].y /= rhs.r[1].y;
		r[1].z /= rhs.r[1].z;
		r[2].x /= rhs.r[2].x;
		r[2].y /= rhs.r[2].y;
		r[2].z /= rhs.r[2].z;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator+=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		m1 = add_f3x4_f1(m1, s);
		store_f3x4(r[0].m, m1);
		return *this;
#else
		r[0].x += s;
		r[0].y += s;
		r[0].z += s;
		r[1].x += s;
		r[1].y += s;
		r[1].z += s;
		r[2].x += s;
		r[2].y += s;
		r[2].z += s;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator-=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		m1 = sub_f3x4_f1(m1, s);
		store_f3x4(r[0].m, m1);
		return *this;
#else
		r[0].x -= s;
		r[0].y -= s;
		r[0].z -= s;
		r[1].x -= s;
		r[1].y -= s;
		r[1].z -= s;
		r[2].x -= s;
		r[2].y -= s;
		r[2].z -= s;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator*=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		m1 = mul_f3x4_f1(m1, s);
		store_f3x4(r[0].m, m1);
		return *this;
#else
		r[0].x *= s;
		r[0].y *= s;
		r[0].z *= s;
		r[1].x *= s;
		r[1].y *= s;
		r[1].z *= s;
		r[2].x *= s;
		r[2].y *= s;
		r[2].z *= s;
		return *this;
#endif
	}
	inline Float3x3& Float3x3::operator/=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m1 = load_f3x4(r[0].m);
		s = 1.0f / s;
		m1 = mul_f3x4_f1(m1, s);
		store_f3x4(r[0].m, m1);
		return *this;
#else
		s = 1.0f / s;
		r[0].x *= s;
		r[0].y *= s;
		r[0].z *= s;
		r[1].x *= s;
		r[1].y *= s;
		r[1].z *= s;
		r[2].x *= s;
		r[2].y *= s;
		r[2].z *= s;
		return *this;
#endif
	}
	inline Float3x3 operator+(const Float3x3& m1, const Float3x3& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		float3x4 mat2 = load_f3x4(m2.r[0].m);
		mat1 = add_f3x4(mat1, mat2);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x + m2.r[0].x;
		result.r[0].y = m1.r[0].y + m2.r[0].y;
		result.r[0].z = m1.r[0].z + m2.r[0].z;
		result.r[1].x = m1.r[1].x + m2.r[1].x;
		result.r[1].y = m1.r[1].y + m2.r[1].y;
		result.r[1].z = m1.r[1].z + m2.r[1].z;
		result.r[2].x = m1.r[2].x + m2.r[2].x;
		result.r[2].y = m1.r[2].y + m2.r[2].y;
		result.r[2].z = m1.r[2].z + m2.r[2].z;
		return result;
#endif
	}
	inline Float3x3 operator+(const Float3x3& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		mat1 = add_f3x4_f1(mat1, s);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x + s;
		result.r[0].y = m1.r[0].y + s;
		result.r[0].z = m1.r[0].z + s;
		result.r[1].x = m1.r[1].x + s;
		result.r[1].y = m1.r[1].y + s;
		result.r[1].z = m1.r[1].z + s;
		result.r[2].x = m1.r[2].x + s;
		result.r[2].y = m1.r[2].y + s;
		result.r[2].z = m1.r[2].z + s;
		return result;
#endif
	}
	inline Float3x3 operator+(f32 s, const Float3x3& m1)
	{
		return m1 + s;
	}
	inline Float3x3 operator-(const Float3x3& m1, const Float3x3& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		float3x4 mat2 = load_f3x4(m2.r[0].m);
		mat1 = sub_f3x4(mat1, mat2);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x - m2.r[0].x;
		result.r[0].y = m1.r[0].y - m2.r[0].y;
		result.r[0].z = m1.r[0].z - m2.r[0].z;
		result.r[1].x = m1.r[1].x - m2.r[1].x;
		result.r[1].y = m1.r[1].y - m2.r[1].y;
		result.r[1].z = m1.r[1].z - m2.r[1].z;
		result.r[2].x = m1.r[2].x - m2.r[2].x;
		result.r[2].y = m1.r[2].y - m2.r[2].y;
		result.r[2].z = m1.r[2].z - m2.r[2].z;
		return result;
#endif
	}
	inline Float3x3 operator-(const Float3x3& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		mat1 = sub_f3x4_f1(mat1, s);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x - s;
		result.r[0].y = m1.r[0].y - s;
		result.r[0].z = m1.r[0].z - s;
		result.r[1].x = m1.r[1].x - s;
		result.r[1].y = m1.r[1].y - s;
		result.r[1].z = m1.r[1].z - s;
		result.r[2].x = m1.r[2].x - s;
		result.r[2].y = m1.r[2].y - s;
		result.r[2].z = m1.r[2].z - s;
		return result;
#endif
	}
	inline Float3x3 operator-(f32 s, const Float3x3& m1)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		mat1 = sub_f1_f3x4(s, mat1);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = s - m1.r[0].x;
		result.r[0].y = s - m1.r[0].y;
		result.r[0].z = s - m1.r[0].z;
		result.r[1].x = s - m1.r[1].x;
		result.r[1].y = s - m1.r[1].y;
		result.r[1].z = s - m1.r[1].z;
		result.r[2].x = s - m1.r[2].x;
		result.r[2].y = s - m1.r[2].y;
		result.r[2].z = s - m1.r[2].z;
		return result;
#endif
	}
	inline Float3x3 operator*(const Float3x3& m1, const Float3x3& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		float3x4 mat2 = load_f3x4(m2.r[0].m);
		mat1 = mul_f3x4(mat1, mat2);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x * m2.r[0].x;
		result.r[0].y = m1.r[0].y * m2.r[0].y;
		result.r[0].z = m1.r[0].z * m2.r[0].z;
		result.r[1].x = m1.r[1].x * m2.r[1].x;
		result.r[1].y = m1.r[1].y * m2.r[1].y;
		result.r[1].z = m1.r[1].z * m2.r[1].z;
		result.r[2].x = m1.r[2].x * m2.r[2].x;
		result.r[2].y = m1.r[2].y * m2.r[2].y;
		result.r[2].z = m1.r[2].z * m2.r[2].z;
		return result;
#endif
	}
	inline Float3x3 operator*(const Float3x3& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		mat1 = mul_f3x4_f1(mat1, s);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x * s;
		result.r[0].y = m1.r[0].y * s;
		result.r[0].z = m1.r[0].z * s;
		result.r[1].x = m1.r[1].x * s;
		result.r[1].y = m1.r[1].y * s;
		result.r[1].z = m1.r[1].z * s;
		result.r[2].x = m1.r[2].x * s;
		result.r[2].y = m1.r[2].y * s;
		result.r[2].z = m1.r[2].z * s;
		return result;
#endif
	}
	inline Float3x3 operator*(f32 s, const Float3x3& m1)
	{
		return m1 * s;
	}
	inline Float3x3 operator/(const Float3x3& m1, const Float3x3& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		float3x4 mat2 = load_f3x4(m2.r[0].m);
		mat1 = div_f3x4(mat1, mat2);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else

		Float3x3 result;
		result.r[0].x = m1.r[0].x / m2.r[0].x;
		result.r[0].y = m1.r[0].y / m2.r[0].y;
		result.r[0].z = m1.r[0].z / m2.r[0].z;
		result.r[1].x = m1.r[1].x / m2.r[1].x;
		result.r[1].y = m1.r[1].y / m2.r[1].y;
		result.r[1].z = m1.r[1].z / m2.r[1].z;
		result.r[2].x = m1.r[2].x / m2.r[2].x;
		result.r[2].y = m1.r[2].y / m2.r[2].y;
		result.r[2].z = m1.r[2].z / m2.r[2].z;
		return result;
#endif
	}
	inline Float3x3 operator/(const Float3x3& m1, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		s = 1.0f / s;
		mat1 = mul_f3x4_f1(mat1, s);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = m1.r[0].x / s;
		result.r[0].y = m1.r[0].y / s;
		result.r[0].z = m1.r[0].z / s;
		result.r[1].x = m1.r[1].x / s;
		result.r[1].y = m1.r[1].y / s;
		result.r[1].z = m1.r[1].z / s;
		result.r[2].x = m1.r[2].x / s;
		result.r[2].y = m1.r[2].y / s;
		result.r[2].z = m1.r[2].z / s;
		return result;
#endif
	}
	inline Float3x3 operator/(f32 s, const Float3x3& m1)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = dup_f3x4(s);
		float3x4 mat2 = load_f3x4(m1.r[0].m);
		mat1 = div_f3x4(mat1, mat2);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		Float3x3 result;
		result.r[0].x = s / m1.r[0].x;
		result.r[0].y = s / m1.r[0].y;
		result.r[0].z = s / m1.r[0].z;
		result.r[1].x = s / m1.r[1].x;
		result.r[1].y = s / m1.r[1].y;
		result.r[1].z = s / m1.r[1].z;
		result.r[2].x = s / m1.r[2].x;
		result.r[2].y = s / m1.r[2].y;
		result.r[2].z = s / m1.r[2].z;
		return result;
#endif
	}
	inline Float3 mul(const Float3& vec, const Float3x3& mat)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 s = dup_f4(vec.x);
		float4 m = load_f4(mat.r[0].m);
		float4 r = mul_f4(s, m);
		s = dup_f4(vec.y);
		m = load_f4(mat.r[1].m);
		r = muladd_f4(s, m, r);
		s = dup_f4(vec.z);
		m = load_f4(mat.r[2].m);
		r = muladd_f4(s, m, r);
		Float3 result;
		store_f4(result.m, r);
		return result;
#else
		return Float3(
			vec.x * mat.r[0].x + vec.y * mat.r[1].x + vec.z * mat.r[2].x,
			vec.x * mat.r[0].y + vec.y * mat.r[1].y + vec.z * mat.r[2].y,
			vec.x * mat.r[0].z + vec.y * mat.r[1].z + vec.z * mat.r[2].z);
#endif
	}
	inline Float3 mul(const Float3x3& mat, const Float3& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		Float3 t;
		Float3 result;
		float4 s = load_f4(vec.m);
		float4 m = load_f4(mat.r[0].m);
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.x = t.x + t.y + t.z;
		m = load_f4(mat.r[1].m);
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.y = t.x + t.y + t.z;
		m = load_f4(mat.r[2].m);
		m = mul_f4(m, s);
		store_f4(t.m, m);
		result.z = t.x + t.y + t.z;
		return result;
#else
		return Float3(
			vec.x * mat.r[0].x + vec.y * mat.r[0].y + vec.z * mat.r[0].z,
			vec.x * mat.r[1].x + vec.y * mat.r[1].y + vec.z * mat.r[1].z,
			vec.x * mat.r[2].x + vec.y * mat.r[2].y + vec.z * mat.r[2].z);
#endif
	}
	inline Float3x3 mul(const Float3x3& m1, const Float3x3& m2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat1 = load_f3x4(m1.r[0].m);
		float3x4 mat2 = load_f3x4(m2.r[0].m);
		mat1 = matmul_f3x3(mat1, mat2);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat1);
		return result;
#else
		return Float3x3(
			m1.r[0].x * m2.r[0].x + m1.r[0].y * m2.r[1].x + m1.r[0].z * m2.r[2].x,
			m1.r[0].x * m2.r[0].y + m1.r[0].y * m2.r[1].y + m1.r[0].z * m2.r[2].y,
			m1.r[0].x * m2.r[0].z + m1.r[0].y * m2.r[1].z + m1.r[0].z * m2.r[2].z,

			m1.r[1].x * m2.r[0].x + m1.r[1].y * m2.r[1].x + m1.r[1].z * m2.r[2].x,
			m1.r[1].x * m2.r[0].y + m1.r[1].y * m2.r[1].y + m1.r[1].z * m2.r[2].y,
			m1.r[1].x * m2.r[0].z + m1.r[1].y * m2.r[1].z + m1.r[1].z * m2.r[2].z,

			m1.r[2].x * m2.r[0].x + m1.r[2].y * m2.r[1].x + m1.r[2].z * m2.r[2].x,
			m1.r[2].x * m2.r[0].y + m1.r[2].y * m2.r[1].y + m1.r[2].z * m2.r[2].y,
			m1.r[2].x * m2.r[0].z + m1.r[2].y * m2.r[1].z + m1.r[2].z * m2.r[2].z);
#endif
	}
	inline f32 determinant(const Float3x3& mat)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 m = load_f3x4(mat.r[0].m);
		return determinant_f3x3(m);
#else
		return
			mat.r[0].x * (mat.r[1].y * mat.r[2].z - mat.r[1].z * mat.r[2].y) +
			mat.r[0].y * (mat.r[1].z * mat.r[2].x - mat.r[1].x * mat.r[2].z) +
			mat.r[0].z * (mat.r[1].x * mat.r[2].y - mat.r[1].y * mat.r[2].x);
#endif
	}
	inline Float3x3 transpose(const Float3x3& m)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4x4 mat = castf3x4_f4x4(load_f3x4(m.r[0].m));
		mat = transpose_f4x4(mat);
		Float3x3 result;
		store_f3x4(result.r[0].m, castf4x4_f3x4(mat));
		return result;
#else
		return Float3x3(
			m.r[0].x, m.r[1].x, m.r[2].x,
			m.r[0].y, m.r[1].y, m.r[2].y,
			m.r[0].z, m.r[1].z, m.r[2].z
		);
#endif
	}
	inline Float3x3 inverse(const Float3x3& m, f32* out_determinant)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float3x4 mat = load_f3x4(m.r[0].m);
		mat = inverse_f3x3(mat, out_determinant);
		Float3x3 result;
		store_f3x4(result.r[0].m, mat);
		return result;
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
		Float3x3 r;
		r.r[0].x = det_inv * (m.r[1].y * m.r[2].z - m.r[1].z * m.r[2].y);
		r.r[1].x = det_inv * (m.r[1].z * m.r[2].x - m.r[1].x * m.r[2].z);
		r.r[2].x = det_inv * (m.r[1].x * m.r[2].y - m.r[1].y * m.r[2].x);
		r.r[0].y = det_inv * (m.r[0].z * m.r[2].y - m.r[0].y * m.r[2].z);
		r.r[1].y = det_inv * (m.r[0].x * m.r[2].z - m.r[0].z * m.r[2].x);
		r.r[2].y = det_inv * (m.r[0].y * m.r[2].x - m.r[0].x * m.r[2].y);
		r.r[0].z = det_inv * (m.r[0].y * m.r[1].z - m.r[0].z * m.r[1].y);
		r.r[1].z = det_inv * (m.r[0].z * m.r[1].x - m.r[0].x * m.r[1].z);
		r.r[2].z = det_inv * (m.r[0].x * m.r[1].y - m.r[0].y * m.r[1].x);
		return r;
#endif
	}
}
