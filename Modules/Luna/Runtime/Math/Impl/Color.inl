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
#include "../Color.hpp"
#include "../Simd.hpp"
namespace Luna
{
	namespace Color
	{
		inline constexpr u32 to_rgba8(const Float4& color)
		{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
			return
				(u32(color.x * 255.0f))  |
				(u32(color.y * 255.0f) << 8) |
				(u32(color.z * 255.0f) << 16) |
				(u32(color.w * 255.0f) << 24);
#else
			return
				(u32(color.x * 255.0f) << 24) |
				(u32(color.y * 255.0f) << 16) |
				(u32(color.z * 255.0f) << 8) |
				(u32(color.w * 255.0f));
#endif
		}
		inline constexpr u32 to_argb8(const Float4& color)
		{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
			return
				(u32(color.w * 255.0f))  |
				(u32(color.x * 255.0f) << 8) |
				(u32(color.y * 255.0f) << 16) |
				(u32(color.z * 255.0f) << 24);
#else
			return
				(u32(color.w * 255.0f) << 24) |
				(u32(color.x * 255.0f) << 16) |
				(u32(color.y * 255.0f) << 8) |
				(u32(color.z * 255.0f));
#endif
		}
		inline constexpr u32 to_abgr8(const Float4& color)
		{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
			return
				(u32(color.w * 255.0f))  |
				(u32(color.z * 255.0f) << 8) |
				(u32(color.y * 255.0f) << 16) |
				(u32(color.x * 255.0f) << 24);
#else
			return
				(u32(color.w * 255.0f) << 24) |
				(u32(color.z * 255.0f) << 16) |
				(u32(color.y * 255.0f) << 8) |
				(u32(color.x * 255.0f));
#endif
		}
		inline constexpr u32 to_bgra8(const Float4& color)
		{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
			return
				(u32(color.z * 255.0f))  |
				(u32(color.y * 255.0f) << 8) |
				(u32(color.x * 255.0f) << 16) |
				(u32(color.w * 255.0f) << 24);
#else
			return
				(u32(color.z * 255.0f) << 24) |
				(u32(color.y * 255.0f) << 16) |
				(u32(color.x * 255.0f) << 8) |
				(u32(color.w * 255.0f));
#endif
		}
		namespace Impl
		{
			inline constexpr f32 get_int_color_0(u32 c)
			{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
				return (f32)((c & 0x000000ff)) / 255.0f;
#else
				return (f32)((c & 0xff000000) >> 24) / 255.0f;
#endif
			}
			inline constexpr f32 get_int_color_1(u32 c)
			{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
				return (f32)((c & 0x0000ff00) >> 8) / 255.0f;
#else
				return (f32)((c & 0x00ff0000) >> 16) / 255.0f;
#endif
			}
			inline constexpr f32 get_int_color_2(u32 c)
			{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
				return (f32)((c & 0x00ff0000) >> 16) / 255.0f;
#else
				return (f32)((c & 0x0000ff00) >> 8) / 255.0f;
#endif
			}
			inline constexpr f32 get_int_color_3(u32 c)
			{
#ifdef LUNA_PLATFORM_LITTLE_ENDIAN
				return (f32)((c & 0xff000000) >> 24) / 255.0f;
#else
				return (f32)((c & 0x000000ff)) / 255.0f;
#endif
			}
		}
		inline constexpr Float4 from_rgba8(u32 c)
		{
			return Float4(
				Impl::get_int_color_0(c),
				Impl::get_int_color_1(c),
				Impl::get_int_color_2(c),
				Impl::get_int_color_3(c)
			);
		}
		inline constexpr Float4 from_argb8(u32 c)
		{
			return Float4(
				Impl::get_int_color_1(c),
				Impl::get_int_color_2(c),
				Impl::get_int_color_3(c),
				Impl::get_int_color_0(c)
			);
		}
		inline constexpr Float4 from_abgr8(u32 c)
		{
			return Float4(
				Impl::get_int_color_3(c),
				Impl::get_int_color_2(c),
				Impl::get_int_color_1(c),
				Impl::get_int_color_0(c)
			);
		}
		inline constexpr Float4 from_bgra8(u32 c)
		{
			return Float4(
				Impl::get_int_color_2(c),
				Impl::get_int_color_1(c),
				Impl::get_int_color_0(c),
				Impl::get_int_color_3(c)
			);
		}
		inline Float4 adjust_saturation(const Float4& c, f32 sat)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 clr = load_f4(c.m);
			float4 lum = dot3v_f4(clr, set_f4(0.2125f, 0.7154f, 0.0721f, 0.0f));
			float4 r = sub_f4(clr, lum);
			r = muladd_f4(r, dup_f4(sat), lum);
			setw_f4(r, c.w);
			Float4 ret;
			store_f4(ret.m, r);
			return ret;
#else
			constexpr Float4 gvLuminance(0.2125f, 0.7154f, 0.0721f, 0.0f);
			Float4 result;
			f32 lum = (c.x * gvLuminance.x) + (c.y * gvLuminance.y) + (c.z * gvLuminance.z);
			result.x = ((c.x - lum) * sat) + lum;
			result.y = ((c.y - lum) * sat) + lum;
			result.z = ((c.z - lum) * sat) + lum;
			result.w = c.w;
			return result;
#endif
		}
		inline Float4 adjust_contrast(const Float4& c, f32 contrast)
		{
#ifdef LUNA_SIMD
			using namespace Simd;
			float4 clr = load_f4(c.m);
			float4 half_one = dup_f4(0.5f);
			float4 r = sub_f4(clr, half_one);
			r = muladd_f4(r, dup_f4(contrast), half_one);
			setw_f4(r, c.w);
			Float4 ret;
			store_f4(ret.m, r);
			return ret;
#else
			Float4 result;
			result.x = ((c.x - 0.5f) * contrast) + 0.5f;
			result.y = ((c.y - 0.5f) * contrast) + 0.5f;
			result.z = ((c.z - 0.5f) * contrast) + 0.5f;
			result.w = c.w;
			return result;
#endif
		}
		inline Float4 negate(const Float4& c)
		{
	#ifdef LUNA_SSE2_INTRINSICS
			using namespace Simd;
			float4 clr = load_f4(c.m);
			clr = sub_f4(dup_f4(1.0f), clr);
			setw_f4(clr, c.w);
			Float4 ret;
			store_f4(ret.m, clr);
			return ret;
	#else
			Float4 result;
			result.x = 1.0f - c.x;
			result.y = 1.0f - c.y;
			result.z = 1.0f - c.z;
			result.w = c.w;
			return result;
	#endif
		}
	}
}
