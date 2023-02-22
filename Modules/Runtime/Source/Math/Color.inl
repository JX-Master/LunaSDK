/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Color.inl
* @author JXMaster
* @date 2022/3/17
 */
#pragma once
#include "../../Math/Color.hpp"
#include "../../Math/Simd.hpp"
namespace Luna
{
	inline bool Color::operator==(const Color& v) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		int4 temp = cmpeq_f4(v1, v2);
		return ((maskint_i4(temp) == 0x0f) != 0);
#else
		return (r == v.r) && (g == v.g) && (b == v.b) && (a == v.a);
#endif
	}
	inline bool Color::operator!=(const Color& v) const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		int4 temp = cmpneq_f4(v1, v2);
		return ((maskint_i4(temp)) != 0);
#else
		return (r != v.r) || (g != v.g) || (b != v.b) || (a != v.a);
#endif
	}
	inline Color& Color::operator+= (const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = add_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r += v.r;
		g += v.g;
		b += v.b;
		a += v.a;
		return *this;
#endif
	}
	inline Color& Color::operator-= (const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = sub_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r -= v.r;
		g -= v.g;
		b -= v.b;
		a -= v.a;
		return *this;
#endif
	}
	inline Color& Color::operator*= (const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = mul_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r *= v.r;
		g *= v.g;
		b *= v.b;
		a *= v.a;
		return *this;
#endif
	}
	inline Color& Color::operator/= (const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = load_f4(v.m);
		float4 res = div_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r /= v.r;
		g /= v.g;
		b /= v.b;
		a /= v.a;
		return *this;
#endif
	}
	inline Color& Color::operator+= (f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(s);
		float4 res = add_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r += s;
		g += s;
		b += s;
		a += s;
		return *this;
#endif
	}
	inline Color& Color::operator-= (f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(s);
		float4 res = sub_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r -= s;
		g -= s;
		b -= s;
		a -= s;
		return *this;
#endif
	}
	inline Color& Color::operator*= (f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(s);
		float4 res = mul_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r *= s;
		g *= s;
		b *= s;
		a *= s;
		return *this;
#endif
	}
	inline Color& Color::operator/= (f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 v2 = dup_f4(1.0f / s);
		float4 res = mul_f4(v1, v2);
		store_f4(m, res);
		return *this;
#else
		r /= s;
		g /= s;
		b /= s;
		a /= s;
		return *this;
#endif
	}
	inline Color Color::operator- () const
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(m);
		float4 res = sub_f4(setzero_f4(), v1);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(-r, -g, -b, -a);
#endif
	}

	inline color_u32 Color::rgba8() const
	{
		return
			u32(r * 255.0f) << 24 |
			u32(g * 255.0f) << 16 |
			u32(b * 255.0f) << 8 |
			u32(a * 255.0f);
	}
	inline color_u32 Color::argb8() const
	{
		return
			u32(a * 255.0f) << 24 |
			u32(r * 255.0f) << 16 |
			u32(g * 255.0f) << 8 |
			u32(b * 255.0f);
	}
	inline color_u32 Color::abgr8() const
	{
		return
			u32(a * 255.0f) << 24 |
			u32(b * 255.0f) << 16 |
			u32(g * 255.0f) << 8 |
			u32(r * 255.0f);
	}
	inline Color Color::from_rgba8(color_u32 c)
	{
		Color re;
		re.r = (f32)((c & 0xff000000) >> 24);
		re.g = (f32)((c & 0x00ff0000) >> 16);
		re.b = (f32)((c & 0x0000ff00) >> 8);
		re.a = (f32)((c & 0x000000ff));
		re /= 255.0f;
		return re;
	}
	inline Color Color::from_argb8(color_u32 c)
	{
		Color re;
		re.a = (f32)((c & 0xff000000) >> 24);
		re.r = (f32)((c & 0x00ff0000) >> 16);
		re.g = (f32)((c & 0x0000ff00) >> 8);
		re.b = (f32)((c & 0x000000ff));
		re /= 255.0f;
		return re;
	}
	inline Color Color::from_abgr8(color_u32 c)
	{
		Color re;
		re.a = (f32)((c & 0xff000000) >> 24);
		re.b = (f32)((c & 0x00ff0000) >> 16);
		re.g = (f32)((c & 0x0000ff00) >> 8);
		re.r = (f32)((c & 0x000000ff));
		re /= 255.0f;
		return re;
	}
	inline Color operator+ (const Color& v1, const Color& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(v1.m);
		float4 n = load_f4(v2.m);
		float4 res = add_f4(i, n);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v1.r + v2.r, v1.g + v2.g, v1.b + v2.b, v1.a + v2.a);
#endif
	}
	inline Color operator- (const Color& v1, const Color& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(v1.m);
		float4 n = load_f4(v2.m);
		float4 res = sub_f4(i, n);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v1.r - v2.r, v1.g - v2.g, v1.b - v2.b, v1.a - v2.a);
#endif
	}
	inline Color operator* (const Color& v1, const Color& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(v1.m);
		float4 n = load_f4(v2.m);
		float4 res = mul_f4(i, n);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v1.r * v2.r, v1.g * v2.g, v1.b * v2.b, v1.a * v2.a);
#endif
	}
	inline Color operator/ (const Color& v1, const Color& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(v1.m);
		float4 n = load_f4(v2.m);
		float4 res = div_f4(i, n);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v1.r / v2.r, v1.g / v2.g, v1.b / v2.b, v1.a / v2.a);
#endif
	}
	inline Color operator+ (const Color& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = add_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v.r + s, v.g + s, v.b + s, v.a + s);
#endif
	}
	inline Color operator+ (f32 s, const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = add_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v.r + s, v.g + s, v.b + s, v.a + s);
#endif
	}
	inline Color operator- (const Color& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = sub_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v.r - s, v.g - s, v.b - s, v.a - s);
#endif
	}
	inline Color operator- (f32 s, const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = dup_f4(s);
		float4 v2 = load_f4(v.m);
		float4 res = sub_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(s - v.r, s - v.g, s - v.b, s - v.a);
#endif
	}
	inline Color operator* (const Color& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = mul_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v.r * s, v.g * s, v.b * s, v.a * s);
#endif
	}
	inline Color operator* (f32 s, const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(s);
		float4 res = mul_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v.r * s, v.g * s, v.b * s, v.a * s);
#endif
	}
	inline Color operator/ (const Color& v, f32 s)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = dup_f4(1.0f / s);
		float4 res = mul_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(v.r / s, v.g / s, v.b / s, v.a / s);
#endif
	}
	inline Color operator/ (f32 s, const Color& v)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = dup_f4(s);
		float4 v2 = load_f4(v.m);
		float4 res = div_f4(v1, v2);
		Color r;
		store_f4(r.m, res);
		return r;
#else
		return Color(s / v.r, s / v.g, s / v.b, s / v.a);
#endif
	}
	inline Color adjust_saturation(const Color& c, f32 sat)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 clr = load_f4(c.m);
		float4 lum = dot3v_f4(clr, set_f4(0.2125f, 0.7154f, 0.0721f, 0.0f));
		float4 r = sub_f4(clr, lum);
		r = muladd_f4(r, dup_f4(sat), lum);
		setw_f4(r, c.a);
		Color ret;
		store_f4(ret.m, r);
		return ret;
#else
		constexpr Color gvLuminance(0.2125f, 0.7154f, 0.0721f, 0.0f);
		Color result;
		f32 lum = (c.r * gvLuminance.r) + (c.g * gvLuminance.g) + (c.b * gvLuminance.b);
		result.r = ((c.r - lum) * sat) + lum;
		result.g = ((c.g - lum) * sat) + lum;
		result.b = ((c.b - lum) * sat) + lum;
		result.a = c.a;
		return result;
#endif
	}

	inline Color adjust_contrast(const Color& c, f32 contrast)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 clr = load_f4(c.m);
		float4 half_one = dup_f4(0.5f);
		float4 r = sub_f4(clr, half_one);
		r = muladd_f4(r, dup_f4(contrast), half_one);
		setw_f4(r, c.a);
		Color ret;
		store_f4(ret.m, r);
		return ret;
#else
		Color result;
		result.r = ((c.r - 0.5f) * contrast) + 0.5f;
		result.g = ((c.g - 0.5f) * contrast) + 0.5f;
		result.b = ((c.b - 0.5f) * contrast) + 0.5f;
		result.a = c.a;
		return result;
#endif
	}

	inline Color negate(const Color& c)
	{
#ifdef LUNA_SSE2_INTRINSICS
		using namespace Simd;
		float4 clr = load_f4(c.m);
		clr = sub_f4(dup_f4(1.0f), clr);
		setw_f4(clr, c.a);
		Color ret;
		store_f4(ret.m, clr);
		return ret;
#else
		Color result;
		result.r = 1.0f - c.r;
		result.g = 1.0f - c.g;
		result.b = 1.0f - c.b;
		result.a = c.a;
		return result;
#endif
	}

	inline Color clamp(const Color& v, const Color& vmin, const Color& vmax)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 v1 = load_f4(v.m);
		float4 v2 = load_f4(vmin.m);
		float4 v3 = load_f4(vmax.m);
		float4 res = max_f4(v2, v1);
		res = min_f4(v3, res);
		Float4 r;
		store_f4(r.m, res);
		return r;
#else
		Float4 result;
		result.r = v.r > vmin.r ? v.r : vmin.r;
		result.r = result.r < vmax.r ? result.r : vmax.r;
		result.g = v.g > vmin.g ? v.g : vmin.g;
		result.g = result.g < vmax.g ? result.g : vmax.g;
		result.b = v.b > vmin.b ? v.b : vmin.b;
		result.b = result.b < vmax.b ? result.b : vmax.b;
		result.a = v.a > vmin.a ? v.a : vmin.a;
		result.a = result.a < vmax.a ? result.a : vmax.a;
		return result;
#endif
	}
	inline Color min(const Color& v1, const Color& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(v1.m);
		float4 n = load_f4(v2.m);
		float4 res = min_f4(i, n);
		Float4 r;
		store_f4(r.m, res);
		return r;
#else
		Float4 result;
		result.r = v1.r < v2.r ? v1.r : v2.r;
		result.g = v1.g < v2.g ? v1.g : v2.g;
		result.b = v1.b < v2.b ? v1.b : v2.b;
		result.a = v1.a < v2.a ? v1.a : v2.a;
		return result;
#endif
	}
	inline Color max(const Color& v1, const Color& v2)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 i = load_f4(v1.m);
		float4 n = load_f4(v2.m);
		float4 res = max_f4(i, n);
		Float4 r;
		store_f4(r.m, res);
		return r;
#else
		Float4 result;
		result.r = v1.r > v2.r ? v1.r : v2.r;
		result.g = v1.g > v2.g ? v1.g : v2.g;
		result.b = v1.b > v2.b ? v1.b : v2.b;
		result.a = v1.a > v2.a ? v1.a : v2.a;
		return result;
#endif
	}
	inline Color lerp(const Color& v1, const Color& v2, f32 t)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = lerp_f4(vec1, vec2, t);
		Float4 r;
		store_f4(r.m, res);
		return r;
#else
		Float4 result;
		result.r = v1.r + t * (v2.r - v1.r);
		result.g = v1.g + t * (v2.g - v1.g);
		result.b = v1.b + t * (v2.b - v1.b);
		result.a = v1.a + t * (v2.a - v1.a);
		return result;
#endif
	}
	inline Color smoothstep(const Color& v1, const Color& v2, f32 t)
	{
		t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
		t = t * t * (3.f - 2.f * t);
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 res = lerp_f4(vec1, vec2, t);
		Float4 r;
		store_f4(r.m, res);
		return r;
#else
		return lerp(v1, v2, t);
#endif
	}
	inline Color barycentric(const Color& v1, const Color& v2, const Color& v3, f32 f, f32 g)
	{
#ifdef LUNA_SIMD
		using namespace Simd;
		float4 vec1 = load_f4(v1.m);
		float4 vec2 = load_f4(v2.m);
		float4 vec3 = load_f4(v3.m);
		float4 res = barycentric_f4(vec1, vec2, vec3, f, g);
		Float4 r;
		store_f4(r.m, res);
		return r;
#else
		Float4 result;
		result.r = v1.r + (v2.r - v1.r) * f + (v3.r - v1.r) * g;
		result.g = v1.g + (v2.g - v1.g) * f + (v3.g - v1.g) * g;
		result.b = v1.b + (v2.b - v1.b) * f + (v3.b - v1.b) * g;
		result.a = v1.a + (v2.a - v1.a) * f + (v3.a - v1.a) * g;
		return result;
#endif
	}
}