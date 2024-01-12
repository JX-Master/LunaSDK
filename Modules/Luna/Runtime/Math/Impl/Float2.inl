/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Float2.inl
* @author JXMaster
* @date 2022/3/16
 */
#pragma once
#include "../Vector.hpp"
#include "../Simd.hpp"
namespace Luna
{
	inline bool Float2::operator==(const Float2& vec) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = load_f2(vec.m);
		int4 cmp = cmpeq_f4(v1, v2);
		return maskint_i4(cmp) == 0xF;
#else
		return (vec.x == x) && (vec.y == y);
#endif
	}
	inline bool Float2::operator!=(const Float2& vec) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = load_f2(vec.m);
		int4 cmp = cmpeq_f4(v1, v2);
		return maskint_i4(cmp) != 0xF;
#else
		return (vec.x != x) || (vec.y != y);
#endif
	}
	inline Float2& Float2::operator+= (const Float2& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = load_f2(vec.m);
		float4 v3 = add_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x += vec.x;
		y += vec.y;
		return *this;
#endif
	}
	inline Float2& Float2::operator-= (const Float2& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = load_f2(vec.m);
		float4 v3 = sub_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x -= vec.x;
		y -= vec.y;
		return *this;
#endif
	}
	inline Float2& Float2::operator*= (const Float2& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = load_f2(vec.m);
		float4 v3 = mul_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x *= vec.x;
		y *= vec.y;
		return *this;
#endif
	}
	inline Float2& Float2::operator/=(const Float2& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = load_f2(vec.m);
		float4 v3 = div_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x /= vec.x;
		y /= vec.y;
		return *this;
#endif
	}
	inline Float2& Float2::operator+=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = dup_f4(s);
		float4 v3 = add_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x += s;
		y += s;
		return *this;
#endif
	}
	inline Float2& Float2::operator-=(f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = dup_f4(s);
		float4 v3 = sub_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x -= s;
		y -= s;
		return *this;
#endif
	}
	inline Float2& Float2::operator*= (f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = dup_f4(s);
		float4 v3 = mul_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x *= s;
		y *= s;
		return *this;
#endif
	}
	inline Float2& Float2::operator/= (f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(m);
		float4 v2 = dup_f4(1.0f / s);
		float4 v3 = mul_f4(v1, v2);
		store_f2(m, v3);
		return *this;
#else
		x /= s;
		y /= s;
		return *this;
#endif
	}
	inline Float2 operator+ (const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 vec3 = add_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v1.x + v2.x, v1.y + v2.y);
#endif
	}
	inline Float2 operator- (const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 vec3 = sub_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v1.x - v2.x, v1.y - v2.y);
#endif
	}
	inline Float2 operator* (const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 vec3 = mul_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v1.x * v2.x, v1.y * v2.y);
#endif
	}
	inline Float2 operator/ (const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 vec3 = div_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v1.x / v2.x, v1.y / v2.y);
#endif
	}
	inline Float2 operator+(const Float2& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v.m);
		float4 vec2 = dup_f4(s);
		float4 vec3 = add_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v.x + s, v.y + s);
#endif
	}
	inline Float2 operator+(f32 s, const Float2& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v.m);
		float4 vec2 = dup_f4(s);
		float4 vec3 = add_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v.x + s, v.y + s);
#endif
	}
	inline Float2 operator-(const Float2& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v.m);
		float4 vec2 = dup_f4(s);
		float4 vec3 = sub_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v.x - s, v.y - s);
#endif
	}
	inline Float2 operator-(f32 s, const Float2& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = dup_f4(s);
		float4 vec2 = load_f2(v.m);
		float4 vec3 = sub_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(s - v.x, s - v.y);
#endif
	}
	inline Float2 operator* (const Float2& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v.m);
		float4 vec2 = dup_f4(s);
		float4 vec3 = mul_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v.x * s, v.y * s);
#endif
	}
	inline Float2 operator* (f32 s, const Float2& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v.m);
		float4 vec2 = dup_f4(s);
		float4 vec3 = mul_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v.x * s, v.y * s);
#endif
	}
	inline Float2 operator/(const Float2& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v.m);
		float4 vec2 = dup_f4(1.0f / s);
		float4 vec3 = mul_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(v.x / s, v.y / s);
#endif
	}
	inline Float2 operator/(f32 s, const Float2& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = dup_f4(s);
		float4 vec2 = load_f2(v.m);
		float4 vec3 = div_f4(vec1, vec2);
		Float2 R;
		store_f2(R.m, vec3);
		return R;
#else
		return Float2(s / v.x, s / v.y);
#endif
	}
	inline bool in_bounds(const Float2& point, const Float2& min_bound, const Float2& max_bound)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 p = load_f2(point.m);
		float4 minp = load_f2(min_bound.m);
		float4 maxp = load_f2(max_bound.m);
		// Test if less than or equal
		int4 t1 = cmple_f4(p, maxp);
		// Test if greater or equal
		int4 t2 = cmpge_f4(p, minp);
		// Blend answers
		t1 = and_i4(t1, t2);
		// x and y in bounds? (z and w are don't care)
		return (((maskint_i4(t1) & 0x3) == 0x3) != 0);
#else
		return (point.x <= max_bound.x && point.x >= min_bound.x) &&
			(point.y <= max_bound.y && point.y >= min_bound.y);
#endif
	}
	inline f32 length(const Float2& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(vec.m);
		float4 length = dot2v_f4(v1, v1);
		length = sqrt_f4(length);
		return getx_f4(length);
#else
		return sqrtf(vec.x * vec.x + vec.y * vec.y);
#endif
	}
	inline f32 length_squared(const Float2& vec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(vec.m);
		return dot2_f4(v1, v1);
#else
		return vec.x * vec.x + vec.y * vec.y;
#endif
	}
	inline f32 dot(const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		return dot2_f4(vec1, vec2);
#else
		return v1.x * v2.x + v1.y * v2.y;
#endif
	}
	inline Float2 cross(const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 res = cross2_f4(vec1, vec2);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		f32 cross = v1.x * v2.y - v1.y * v2.x;
		Float2 result;
		result.x = cross;
		result.y = cross;
		return result;
#endif
	}
	inline Float2 normalize(const Float2& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(v.m);
		float4 res = normalize2_f4(v1);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		f32 len = length(v);
        if (len > 0)
        {
            len = 1.0f / len;
        }
		Float2 result;
		result.x = v.x * len;
		result.y = v.y * len;
		return result;
#endif
	}
	inline Float2 clamp(const Float2& v, const Float2& vmin, const Float2& vmax)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f2(v.m);
		float4 v2 = load_f2(vmin.m);
		float4 v3 = load_f2(vmax.m);
		float4 res = max_f4(v2, v1);
		res = min_f4(v3, res);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		result.x = v.x > vmin.x ? v.x : vmin.x;
		result.x = result.x < vmax.x ? result.x : vmax.x;
		result.y = v.y > vmin.y ? v.y : vmin.y;
		result.y = result.y < vmax.y ? result.y : vmax.y;
		return result;
#endif
	}
	inline f32 distance(const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 sub = sub_f4(vec2, vec1);
		float4 length_sq = dot2v_f4(sub, sub);
		float4 length = sqrt_f4(length_sq);
		return getx_f4(length);
#else
		f32 dx = v1.x - v2.x;
		f32 dy = v1.y - v2.y;
		return sqrtf(dx * dx + dy * dy);
#endif
	}
	inline f32 distance_squared(const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 sub = sub_f4(vec2, vec1);
		return dot2_f4(sub, sub);
#else
		f32 dx = v1.x - v2.x;
		f32 dy = v1.y - v2.y;
		return dx * dx + dy * dy;
#endif
	}
	inline Float2 min(const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 res = min_f4(vec1, vec2);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		result.x = v1.x < v2.x ? v1.x : v2.x;
		result.y = v1.y < v2.y ? v1.y : v2.y;
		return result;
#endif
	}
	inline Float2 max(const Float2& v1, const Float2& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 res = max_f4(vec1, vec2);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		result.x = v1.x > v2.x ? v1.x : v2.x;
		result.y = v1.y > v2.y ? v1.y : v2.y;
		return result;
#endif
	}
	inline Float2 lerp(const Float2& v1, const Float2& v2, f32 t)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 res = lerp_f4(vec1, vec2, t);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		result.x = v1.x + t * (v2.x - v1.x);
		result.y = v1.y + t * (v2.y - v1.y);
		return result;
#endif
	}
	inline Float2 smoothstep(const Float2& v1, const Float2& v2, f32 t)
	{
		t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
		t = t * t * (3.f - 2.f * t);
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 res = lerp_f4(vec1, vec2, t);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		return lerp(v1, v2, t);
#endif
	}
	inline Float2 barycentric(const Float2& v1, const Float2& v2, const Float2& v3, f32 f, f32 g)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 vec3 = load_f2(v3.m);
		float4 res = barycentric_f4(vec1, vec2, vec3, f, g);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		result.x = v1.x + (v2.x - v1.x) * f + (v3.x - v1.x) * g;
		result.y = v1.y + (v2.y - v1.y) * f + (v3.y - v1.y) * g;
		return result;
#endif
	}
	inline Float2 catmull_rom(const Float2& v1, const Float2& v2, const Float2& v3, const Float2& v4, f32 t)
	{
		// Result = ((-t^3 + 2 * t^2 - t) * Position0 +
		//           (3 * t^3 - 5 * t^2 + 2) * Position1 +
		//           (-3 * t^3 + 4 * t^2 + t) * Position2 +
		//           (t^3 - t^2) * Position3) * 0.5
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(v2.m);
		float4 vec3 = load_f2(v3.m);;
		float4 vec4 = load_f2(v4.m);;
		float4 res = catmull_rom_f4(vec1, vec2, vec3, vec4, t);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		f32 t2 = t * t;
		f32 t3 = t2 * t;
		f32 f1 = -t3 + 2.0f * t2 - t;
		f32 f2 = 3.0f * t3 - 5.0f * t2 + 2.0f;
		f32 f3 = -3.0f * t3 + 4.0f * t2 + t;
		f32 f4 = t3 - t2;
		result.x = (
			f1 * v1.x +
			f2 * v2.x +
			f3 * v3.x +
			f4 * v4.x) * 0.5f;
		result.y = (
			f1 * v1.y +
			f2 * v2.y +
			f3 * v3.y +
			f4 * v4.y) * 0.5f;
		return result;
#endif
	}
	inline Float2 hermite(const Float2& v1, const Float2& t1, const Float2& v2, const Float2& t2, f32 t)
	{
		// Result = (2 * t^3 - 3 * t^2 + 1) * Position0 +
		//          (t^3 - 2 * t^2 + t) * Tangent0 +
		//          (-2 * t^3 + 3 * t^2) * Position1 +
		//          (t^3 - t^2) * Tangent1
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f2(v1.m);
		float4 vec2 = load_f2(t1.m);
		float4 vec3 = load_f2(v2.m);
		float4 vec4 = load_f2(t2.m);
		float4 res = hermite_f4(vec1, vec2, vec3, vec4, t);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		Float2 result;
		f32 t2 = t * t;
		f32 t3 = t2 * t;
		f32 f1 = 2 * t3 - 3 * t2 + 1;
		f32 f2 = t3 - 2 * t2 + t;
		f32 f3 = -2 * t3 + 3 * t2;
		f32 f4 = t3 - t2;
		result.x = 
			f1 * v1.x +
			f2 * t1.x +
			f3 * v2.x +
			f4 * t2.x;
		result.y = 
			f1 * v1.y +
			f2 * t1.y +
			f3 * v2.y +
			f4 * t2.y;
		return result;
#endif
	}
	inline Float2 reflect(const Float2& ivec, const Float2& nvec)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f2(ivec.m);
		float4 n = load_f2(nvec.m);
		float4 res = reflect2_f4(i, n);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		return ivec - (2 * dot(ivec, nvec) * nvec);
#endif
	}
	inline Float2 refract(const Float2& ivec, const Float2& nvec, f32 refraction_index)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f2(ivec.m);
		float4 n = load_f2(nvec.m);
		float4 res = refract2_f4(i, n, refraction_index);
		Float2 result;
		store_f2(result.m, res);
		return result;
#else
		f32 proj = dot(ivec, nvec);
		f32 deter = 1.0f - refraction_index * refraction_index * (1.0f - proj * proj);
		if (deter >= 0.0f)
		{
			return ivec * refraction_index - nvec * (refraction_index * proj + sqrtf(deter));
		}
		return Float2(0.0f, 0.0f);
#endif
	}
}
