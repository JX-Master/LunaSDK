/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Float3.inl
* @author JXMaster
* @date 2022/3/17
 */
#pragma once
#include "../Vector.hpp"
#include "../Simd.hpp"
namespace Luna
{
	inline bool Float3::operator==(const Float3& v) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = casti_f4(and_i4(castf_i4(load_f4(m)), set_i4(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
		float4 v2 = casti_f4(and_i4(castf_i4(load_f4(v.m)), set_i4(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
		int4 temp = cmpeq_f4(v1, v2);
		return (((maskint_i4(temp) & 7) == 7) != 0);
#else
		return (x == v.x) && (y == v.y) && (z == v.z);
#endif
	}
	inline bool Float3::operator!=(const Float3& v) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = casti_f4(and_i4(castf_i4(load_f4(m)), set_i4(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
		float4 v2 = casti_f4(and_i4(castf_i4(load_f4(v.m)), set_i4(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF)));
		int4 temp = cmpeq_f4(v1, v2);
		return (((maskint_i4(temp) & 7) != 7) != 0);
#else
		return (x != v.x) || (y != v.y) || (z != v.z);
#endif
	}
	inline Float3& Float3::operator+=(const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = add_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
#endif
	}
	inline Float3& Float3::operator-=(const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = sub_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
#endif
	}
	inline Float3& Float3::operator*= (const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = mul_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
#endif
	}
	inline Float3& Float3::operator/=(const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = div_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
#endif
	}
	inline Float3& Float3::operator+=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(s);
		float4 res = add_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x += s;
		y += s;
		z += s;
		return *this;
#endif
	}
	inline Float3& Float3::operator-=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(s);
		float4 res = sub_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x -= s;
		y -= s;
		z -= s;
		return *this;
#endif
	}
	inline Float3& Float3::operator*=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(s);
		float4 res = mul_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x *= s;
		y *= s;
		z *= s;
		return *this;
#endif
	}
	inline Float3& Float3::operator/=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(1.0f / s);
		float4 res = mul_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		x /= s;
		y /= s;
		z /= s;
		return *this;
#endif
	}
	inline Float3 Float3::operator-() const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 res = sub_f4(setzero_f4(), v1);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(-x, -y, -z);
#endif
	}
	inline Float3 operator+(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = add_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
#endif
	}
	inline Float3 operator-(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = sub_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
#endif
	}
	inline Float3 operator*(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = mul_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
#endif
	}
	inline Float3 operator/(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = div_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
#endif
	}
	inline Float3 operator+(const Float3& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = add_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v.x + s, v.y + s, v.z + s);
#endif
	}
	inline Float3 operator+(f32 s, const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = add_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v.x + s, v.y + s, v.z + s);
#endif
	}
	inline Float3 operator-(const Float3& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = sub_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v.x - s, v.y - s, v.z - s);
#endif
	}
	inline Float3 operator-(f32 s, const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = dup_f4(s); 
		float4 v2 = load_f4(v.m);
		float4 res = sub_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(s - v.x, s - v.y, s - v.z);
#endif
	}
	inline Float3 operator*(const Float3& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = mul_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v.x * s, v.y * s, v.z * s);
#endif
	}
	inline Float3 operator*(f32 s, const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = mul_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v.x * s, v.y * s, v.z * s);
#endif
	}
	inline Float3 operator/(const Float3& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(1.0f / s);
		float4 res = mul_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(v.x / s, v.y / s, v.z / s);
#endif
	}
	inline Float3 operator/(f32 s, const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = dup_f4(s); 
		float4 v2 = load_f4(v.m);
		float4 res = div_f4(v1, v2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return Float3(s / v.x, s / v.y, s / v.z);
#endif
	}
	inline bool in_bounds(const Float3& point, const Float3& min_bound, const Float3& max_bound)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 p = load_f4(point.m);
		float4 minp = load_f4(min_bound.m);
		float4 maxp = load_f4(max_bound.m);
		// Test if less than or equal
		int4 t1 = cmple_f4(p, maxp);
		// Test if greater or equal
		int4 t2 = cmpge_f4(p, minp);
		// Blend answers
		t1 = and_i4(t1, t2);
		// x,y and z in bounds? (w is don't care)
		return (((maskint_i4(t1) & 0x7) == 0x7) != 0);
#else
		return (point.x <= max_bound.x && point.x >= min_bound.x) &&
			(point.y <= max_bound.y && point.y >= min_bound.y) &&
			(point.z <= max_bound.z && point.z >= min_bound.z);
#endif
	}
	inline f32 length(const Float3& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(vec.m);
		float4 length_sq = dot3v_f4(v1, v1);
		float4 length = sqrt_f4(length_sq);
		return getx_f4(length);
#else
		return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
#endif
	}
	inline f32 length_squared(const Float3& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(vec.m);
		return dot3_f4(v1, v1);
#else
		return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
#endif
	}
	inline f32 dot(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		return dot3_f4(vec1, vec2);
#else
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
#endif
	}
	inline Float3 cross(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = cross3_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
		return result;
#endif
	}
	inline Float3 normalize(const Float3& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 res = normalize3_f4(v1);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		f32 len = length(v);
		if (len > 0)
		{
			len = 1.0f / len;
		}
		Float3 result;
		result.x = v.x * len;
		result.y = v.y * len;
		result.z = v.z * len;
		return result;
#endif
	}
	inline Float3 clamp(const Float3& v, const Float3& vmin, const Float3& vmax)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = load_f4(vmin.m);
		float4 v3 = load_f4(vmax.m);
		float4 res = max_f4(v2, v1);
		res = min_f4(v3, res);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = v.x > vmin.x ? v.x : vmin.x;
		result.x = result.x < vmax.x ? result.x : vmax.x;
		result.y = v.y > vmin.y ? v.y : vmin.y;
		result.y = result.y < vmax.y ? result.y : vmax.y;
		result.z = v.z > vmin.z ? v.z : vmin.z;
		result.z = result.z < vmax.z ? result.z : vmax.z;
		return result;
#endif
	}
	inline f32 distance(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 sub = sub_f4(vec2, vec1);
		float4 length_sq = dot3v_f4(sub, sub);
		float4 length = sqrt_f4(length_sq);
		return getx_f4(length);
#else
		f32 delX = v1.x - v2.x;
		f32 delY = v1.y - v2.y;
		f32 delZ = v1.z - v2.z;
		return sqrtf(delX * delX + delY * delY + delZ * delZ);
#endif
	}
	inline f32 distance_squared(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 sub = sub_f4(vec2, vec1);
		return dot3_f4(sub, sub);
#else
		f32 delX = v1.x - v2.x;
		f32 delY = v1.y - v2.y;
		f32 delZ = v1.z - v2.z;
		return delX * delX + delY * delY + delZ * delZ;
#endif
	}
	inline Float3 min(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = min_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = v1.x < v2.x ? v1.x : v2.x;
		result.y = v1.y < v2.y ? v1.y : v2.y;
		result.z = v1.z < v2.z ? v1.z : v2.z;
		return result;
#endif
	}
	inline Float3 max(const Float3& v1, const Float3& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = max_f4(vec1, vec2);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = v1.x > v2.x ? v1.x : v2.x;
		result.y = v1.y > v2.y ? v1.y : v2.y;
		result.z = v1.z > v2.z ? v1.z : v2.z;
		return result;
#endif
	}
	inline Float3 lerp(const Float3& v1, const Float3& v2, f32 t)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = lerp_f4(vec1, vec2, t);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = v1.x + t * (v2.x - v1.x);
		result.y = v1.y + t * (v2.y - v1.y);
		result.z = v1.z + t * (v2.z - v1.z);
		return result;
#endif
	}
	inline Float3 smoothstep(const Float3& v1, const Float3& v2, f32 t)
	{
		t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
		t = t * t * (3.f - 2.f * t);
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = lerp_f4(vec1, vec2, t);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		return lerp(v1, v2, t);
#endif
	}
	inline Float3 barycentric(const Float3& v1, const Float3& v2, const Float3& v3, f32 f, f32 g)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 vec3 = load_f4(v3.m);
		float4 res = barycentric_f4(vec1, vec2, vec3, f, g);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = v1.x + (v2.x - v1.x) * f + (v3.x - v1.x) * g;
		result.y = v1.y + (v2.y - v1.y) * f + (v3.y - v1.y) * g;
		result.z = v1.z + (v2.z - v1.z) * f + (v3.z - v1.z) * g;
		return result;
#endif
	}
	inline Float3 catmull_rom(const Float3& v1, const Float3& v2, const Float3& v3, const Float3& v4, f32 t)
	{
		// Result = ((-t^3 + 2 * t^2 - t) * Position0 +
			//           (3 * t^3 - 5 * t^2 + 2) * Position1 +
			//           (-3 * t^3 + 4 * t^2 + t) * Position2 +
			//           (t^3 - t^2) * Position3) * 0.5
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 vec3 = load_f4(v3.m);
		float4 vec4 = load_f4(v4.m);
		float4 res = catmull_rom_f4(vec1, vec2, vec3, vec4, t);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = ((-(t * t * t) + 2.0f * (t * t) - t) * v1.x
			+ (3.0f * (t * t * t) - 5.0f * (t * t) + 2.0f) * v2.x
			+ (-3.0f * (t * t * t) + 4.0f * (t * t) + t) * v3.x
			+ ((t * t * t) - (t * t)) * v4.x) * 0.5f;
		result.y = ((-(t * t * t) + 2 * (t * t) - t) * v1.y
			+ (3.0f * (t * t * t) - 5.0f * (t * t) + 2.0f) * v2.y
			+ (-3.0f * (t * t * t) + 4 * (t * t) + t) * v3.y
			+ ((t * t * t) - (t * t)) * v4.y) * 0.5f;
		result.z = ((-(t * t * t) + 2.0f * (t * t) - t) * v1.z
			+ (3.0f * (t * t * t) - 5.0f * (t * t) + 2.0f) * v2.z
			+ (-3.0f * (t * t * t) + 4.0f * (t * t) + t) * v3.z
			+ ((t * t * t) - (t * t)) * v4.z) * 0.5f;
		return result;
#endif
	}
	inline Float3 hermite(const Float3& v1, const Float3& t1, const Float3& v2, const Float3& t2, f32 t)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(t1.m);
		float4 vec3 = load_f4(v2.m);
		float4 vec4 = load_f4(t2.m);
		float4 res = hermite_f4(vec1, vec2, vec3, vec4, t);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		result.x = (2 * (t * t * t) - 3 * (t * t) + 1) * v1.x +
			((t * t * t) - 2 * (t * t) + t) * t1.x +
			(-2 * (t * t * t) + 3 * (t * t)) * v2.x +
			((t * t * t) - (t * t)) * t2.x;
		result.y = (2 * (t * t * t) - 3 * (t * t) + 1) * v1.y +
			((t * t * t) - 2 * (t * t) + t) * t1.y +
			(-2 * (t * t * t) + 3 * (t * t)) * v2.y +
			((t * t * t) - (t * t)) * t2.y;
		result.z = (2 * (t * t * t) - 3 * (t * t) + 1) * v1.z +
			((t * t * t) - 2 * (t * t) + t) * t1.z +
			(-2 * (t * t * t) + 3 * (t * t)) * v2.z +
			((t * t * t) - (t * t)) * t2.z;
		return result;
#endif
	}
	inline Float3 reflect(const Float3& ivec, const Float3& nvec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(ivec.m);
		float4 n = load_f4(nvec.m);
		float4 res = reflect3_f4(i, n);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		Float3 result;
		f32 s = 2.0f * (ivec.x * nvec.x + ivec.y * nvec.y + ivec.z * nvec.z);
		result.x = ivec.x - s * nvec.x;
		result.y = ivec.y - s * nvec.y;
		result.z = ivec.z - s * nvec.z;
		return result;
#endif
	}
	inline Float3 refract(const Float3& ivec, const Float3& nvec, f32 refraction_index)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(ivec.m);
		float4 n = load_f4(nvec.m);
		float4 res = refract3_f4(i, n, refraction_index);
		Float3 r;
		store_f4(r.m, res);
		return r;
#else
		f32 t = (ivec.x * nvec.x + ivec.y * nvec.y + ivec.z * nvec.z);
		f32 r = 1.0f - refraction_index * refraction_index * (1.0f - t * t);
		Float3 result;
		if (r < 0.0f) // Total internal reflection
		{
			result.x = 0.0f;
			result.y = 0.0f;
			result.z = 0.0f;
		}
		else
		{
			f32 s = refraction_index * t + sqrtf(r);
			result.x = refraction_index * ivec.x - s * nvec.x;
			result.y = refraction_index * ivec.y - s * nvec.y;
			result.z = refraction_index * ivec.z - s * nvec.z;
		}
		return result;
#endif
	}
}
