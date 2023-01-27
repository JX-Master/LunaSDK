/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Color.hpp
* @author JXMaster
* @date 2019/1/5
*/
#pragma once
#include "Vector.hpp"

namespace Luna
{
	//! u32 color format, each 8-bit component represents a color channel.
	using color_u32 = u32;

	//! Linear color.
	struct alignas(Float4) Color
	{
		lustruct("Color", "{15377FB8-CA20-4704-A31E-44EE086DEEC4}");

		union
		{
			struct
			{
				f32 r;
				f32 g;
				f32 b;
				f32 a;
			};
			f32 m[4];
		};

		Color() = default;
		Color(const Color&) = default;
		Color& operator=(const Color&) = default;
		Color(Color&&) = default;
		Color& operator=(Color&&) = default;

		constexpr Color(f32 _r, f32 _g, f32 _b) :
			r(_r), g(_g), b(_b), a(1.0f) {}
		constexpr Color(f32 _r, f32 _g, f32 _b, f32 _a) :
			r(_r), g(_g), b(_b), a(_a) {}
		constexpr Color(const Float3& rhs) :
			r(rhs.x), g(rhs.y), b(rhs.z), a(1.0f) {}
		constexpr Color(const Float4& rhs) :
			r(rhs.x), g(rhs.y), b(rhs.z), a(rhs.w) {}
		constexpr Color(const Float3U& rhs) :
			r(rhs.x), g(rhs.y), b(rhs.z), a(1.0f) {}
		constexpr Color(const Float4U& rhs) :
			r(rhs.x), g(rhs.y), b(rhs.z), a(rhs.w) {}

		bool operator==(const Color& rhs) const;
		bool operator!=(const Color& rhs) const;

		Color& operator+= (const Color& v);
		Color& operator-= (const Color& v);
		Color& operator*= (const Color& v);
		Color& operator/= (const Color& v);
		Color& operator+= (f32 s);
		Color& operator-= (f32 s);
		Color& operator*= (f32 s);
		Color& operator/= (f32 s);

		Color operator+ () const { return *this; }
		Color operator- () const;

		color_u32 rgba8() const;
		color_u32 argb8() const;
		color_u32 abgr8() const;

		operator Float4() const { return Float4(r, g, b, a); }
		operator Float4U() const { return Float4U(r, g, b, a); }

		static Color from_rgba8(color_u32 c);
		static Color from_argb8(color_u32 c);
		static Color from_abgr8(color_u32 c);

		// Regularly used colors.
		// Reference : http://www.w3school.com.cn/tiy/color.asp
		static constexpr Color alice_blue() { return Color(0.941176534f, 0.972549081f, 1.000000000f, 1.000000000f); }
		static constexpr Color antique_white() { return Color(0.980392218f, 0.921568692f, 0.843137324f, 1.000000000f); }
		static constexpr Color aqua() { return Color(0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color aquamarine() { return Color(0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f); }
		static constexpr Color azure() { return Color(0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f); }
		static constexpr Color beige() { return Color(0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f); }
		static constexpr Color bisque() { return Color(1.000000000f, 0.894117713f, 0.768627524f, 1.000000000f); }
		static constexpr Color black() { return Color(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color blanched_almond() { return Color(1.000000000f, 0.921568692f, 0.803921640f, 1.000000000f); }
		static constexpr Color blue() { return Color(0.000000000f, 0.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color blue_violet() { return Color(0.541176498f, 0.168627456f, 0.886274576f, 1.000000000f); }
		static constexpr Color brown() { return Color(0.647058845f, 0.164705887f, 0.164705887f, 1.000000000f); }
		static constexpr Color burly_wood() { return Color(0.870588303f, 0.721568644f, 0.529411793f, 1.000000000f); }
		static constexpr Color cadet_blue() { return Color(0.372549027f, 0.619607866f, 0.627451003f, 1.000000000f); }
		static constexpr Color chartreuse() { return Color(0.498039246f, 1.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color chocolate() { return Color(0.823529482f, 0.411764741f, 0.117647067f, 1.000000000f); }
		static constexpr Color coral() { return Color(1.000000000f, 0.498039246f, 0.313725501f, 1.000000000f); }
		static constexpr Color cornflower_blue() { return Color(0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f); }
		static constexpr Color cornsilk() { return Color(1.000000000f, 0.972549081f, 0.862745166f, 1.000000000f); }
		static constexpr Color crimson() { return Color(0.862745166f, 0.078431375f, 0.235294133f, 1.000000000f); }
		static constexpr Color cyan() { return Color(0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color dark_blue() { return Color(0.000000000f, 0.000000000f, 0.545098066f, 1.000000000f); }
		static constexpr Color dark_cyan() { return Color(0.000000000f, 0.545098066f, 0.545098066f, 1.000000000f); }
		static constexpr Color dark_golden_rod() { return Color(0.721568644f, 0.525490224f, 0.043137256f, 1.000000000f); }
		static constexpr Color dark_gray() { return Color(0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f); }
		static constexpr Color dark_green() { return Color(0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f); }
		static constexpr Color dark_khaki() { return Color(0.741176486f, 0.717647076f, 0.419607878f, 1.000000000f); }
		static constexpr Color dark_magenta() { return Color(0.545098066f, 0.000000000f, 0.545098066f, 1.000000000f); }
		static constexpr Color dark_olive_green() { return Color(0.333333343f, 0.419607878f, 0.184313729f, 1.000000000f); }
		static constexpr Color dark_orange() { return Color(1.000000000f, 0.549019635f, 0.000000000f, 1.000000000f); }
		static constexpr Color dark_orchid() { return Color(0.600000024f, 0.196078449f, 0.800000072f, 1.000000000f); }
		static constexpr Color dark_red() { return Color(0.545098066f, 0.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color dark_salmon() { return Color(0.913725555f, 0.588235319f, 0.478431404f, 1.000000000f); }
		static constexpr Color dark_sea_green() { return Color(0.560784340f, 0.737254918f, 0.545098066f, 1.000000000f); }
		static constexpr Color dark_slate_blue() { return Color(0.282352954f, 0.239215702f, 0.545098066f, 1.000000000f); }
		static constexpr Color dark_slate_gray() { return Color(0.184313729f, 0.309803933f, 0.309803933f, 1.000000000f); }
		static constexpr Color dark_turquoise() { return Color(0.000000000f, 0.807843208f, 0.819607913f, 1.000000000f); }
		static constexpr Color dark_violet() { return Color(0.580392182f, 0.000000000f, 0.827451050f, 1.000000000f); }
		static constexpr Color deep_pink() { return Color(1.000000000f, 0.078431375f, 0.576470613f, 1.000000000f); }
		static constexpr Color deep_sky_blue() { return Color(0.000000000f, 0.749019623f, 1.000000000f, 1.000000000f); }
		static constexpr Color dim_gray() { return Color(0.411764741f, 0.411764741f, 0.411764741f, 1.000000000f); }
		static constexpr Color dodger_blue() { return Color(0.117647067f, 0.564705908f, 1.000000000f, 1.000000000f); }
		static constexpr Color fire_brick() { return Color(0.698039234f, 0.133333340f, 0.133333340f, 1.000000000f); }
		static constexpr Color floral_white() { return Color(1.000000000f, 0.980392218f, 0.941176534f, 1.000000000f); }
		static constexpr Color forest_green() { return Color(0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f); }
		static constexpr Color fuchsia() { return Color(1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color gainsboro() { return Color(0.862745166f, 0.862745166f, 0.862745166f, 1.000000000f); }
		static constexpr Color ghost_white() { return Color(0.972549081f, 0.972549081f, 1.000000000f, 1.000000000f); }
		static constexpr Color gold() { return Color(1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f); }
		static constexpr Color golden_rod() { return Color(0.854902029f, 0.647058845f, 0.125490203f, 1.000000000f); }
		static constexpr Color gray() { return Color(0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f); }
		static constexpr Color green() { return Color(0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f); }
		static constexpr Color green_yellow() { return Color(0.678431392f, 1.000000000f, 0.184313729f, 1.000000000f); }
		static constexpr Color honey_dew() { return Color(0.941176534f, 1.000000000f, 0.941176534f, 1.000000000f); }
		static constexpr Color hot_pink() { return Color(1.000000000f, 0.411764741f, 0.705882370f, 1.000000000f); }
		static constexpr Color indian_red() { return Color(0.803921640f, 0.360784322f, 0.360784322f, 1.000000000f); }
		static constexpr Color indigo() { return Color(0.294117659f, 0.000000000f, 0.509803951f, 1.000000000f); }
		static constexpr Color ivory() { return Color(1.000000000f, 1.000000000f, 0.941176534f, 1.000000000f); }
		static constexpr Color khaki() { return Color(0.941176534f, 0.901960850f, 0.549019635f, 1.000000000f); }
		static constexpr Color lavender() { return Color(0.901960850f, 0.901960850f, 0.980392218f, 1.000000000f); }
		static constexpr Color lavender_blush() { return Color(1.000000000f, 0.941176534f, 0.960784376f, 1.000000000f); }
		static constexpr Color lawn_green() { return Color(0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f); }
		static constexpr Color lemon_chiffon() { return Color(1.000000000f, 0.980392218f, 0.803921640f, 1.000000000f); }
		static constexpr Color light_blue() { return Color(0.678431392f, 0.847058892f, 0.901960850f, 1.000000000f); }
		static constexpr Color light_coral() { return Color(0.941176534f, 0.501960814f, 0.501960814f, 1.000000000f); }
		static constexpr Color light_cyan() { return Color(0.878431439f, 1.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color light_golden_rod_yellow() { return Color(0.980392218f, 0.980392218f, 0.823529482f, 1.000000000f); }
		static constexpr Color light_gray() { return Color(0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f); }
		static constexpr Color light_green() { return Color(0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f); }
		static constexpr Color light_pink() { return Color(1.000000000f, 0.713725507f, 0.756862819f, 1.000000000f); }
		static constexpr Color light_salmon() { return Color(1.000000000f, 0.627451003f, 0.478431404f, 1.000000000f); }
		static constexpr Color light_sea_green() { return Color(0.125490203f, 0.698039234f, 0.666666687f, 1.000000000f); }
		static constexpr Color light_sky_blue() { return Color(0.529411793f, 0.807843208f, 0.980392218f, 1.000000000f); }
		static constexpr Color light_slate_gray() { return Color(0.466666698f, 0.533333361f, 0.600000024f, 1.000000000f); }
		static constexpr Color light_steel_blue() { return Color(0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f); }
		static constexpr Color light_yellow() { return Color(1.000000000f, 1.000000000f, 0.878431439f, 1.000000000f); }
		static constexpr Color lime() { return Color(0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color lime_green() { return Color(0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f); }
		static constexpr Color linen() { return Color(0.980392218f, 0.941176534f, 0.901960850f, 1.000000000f); }
		static constexpr Color magenta() { return Color(1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color maroon() { return Color(0.501960814f, 0.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color medium_aqua_marine() { return Color(0.400000036f, 0.803921640f, 0.666666687f, 1.000000000f); }
		static constexpr Color medium_blue() { return Color(0.000000000f, 0.000000000f, 0.803921640f, 1.000000000f); }
		static constexpr Color medium_orchid() { return Color(0.729411781f, 0.333333343f, 0.827451050f, 1.000000000f); }
		static constexpr Color medium_purple() { return Color(0.576470613f, 0.439215720f, 0.858823597f, 1.000000000f); }
		static constexpr Color medium_sea_green() { return Color(0.235294133f, 0.701960802f, 0.443137288f, 1.000000000f); }
		static constexpr Color medium_slate_blue() { return Color(0.482352972f, 0.407843173f, 0.933333397f, 1.000000000f); }
		static constexpr Color medium_spring_green() { return Color(0.000000000f, 0.980392218f, 0.603921592f, 1.000000000f); }
		static constexpr Color medium_turquoise() { return Color(0.282352954f, 0.819607913f, 0.800000072f, 1.000000000f); }
		static constexpr Color medium_violet_red() { return Color(0.780392230f, 0.082352944f, 0.521568656f, 1.000000000f); }
		static constexpr Color midnight_blue() { return Color(0.098039225f, 0.098039225f, 0.439215720f, 1.000000000f); }
		static constexpr Color mint_cream() { return Color(0.960784376f, 1.000000000f, 0.980392218f, 1.000000000f); }
		static constexpr Color misty_rose() { return Color(1.000000000f, 0.894117713f, 0.882353008f, 1.000000000f); }
		static constexpr Color moccasin() { return Color(1.000000000f, 0.894117713f, 0.709803939f, 1.000000000f); }
		static constexpr Color navajo_white() { return Color(1.000000000f, 0.870588303f, 0.678431392f, 1.000000000f); }
		static constexpr Color navy() { return Color(0.000000000f, 0.000000000f, 0.501960814f, 1.000000000f); }
		static constexpr Color old_lace() { return Color(0.992156923f, 0.960784376f, 0.901960850f, 1.000000000f); }
		static constexpr Color olive() { return Color(0.501960814f, 0.501960814f, 0.000000000f, 1.000000000f); }
		static constexpr Color olive_drab() { return Color(0.419607878f, 0.556862772f, 0.137254909f, 1.000000000f); }
		static constexpr Color orange() { return Color(1.000000000f, 0.647058845f, 0.000000000f, 1.000000000f); }
		static constexpr Color orange_red() { return Color(1.000000000f, 0.270588249f, 0.000000000f, 1.000000000f); }
		static constexpr Color orchid() { return Color(0.854902029f, 0.439215720f, 0.839215755f, 1.000000000f); }
		static constexpr Color pale_golden_rod() { return Color(0.933333397f, 0.909803987f, 0.666666687f, 1.000000000f); }
		static constexpr Color pale_green() { return Color(0.596078455f, 0.984313786f, 0.596078455f, 1.000000000f); }
		static constexpr Color pale_turquoise() { return Color(0.686274529f, 0.933333397f, 0.933333397f, 1.000000000f); }
		static constexpr Color pale_violet_red() { return Color(0.858823597f, 0.439215720f, 0.576470613f, 1.000000000f); }
		static constexpr Color papaya_whip() { return Color(1.000000000f, 0.937254965f, 0.835294187f, 1.000000000f); }
		static constexpr Color peach_puff() { return Color(1.000000000f, 0.854902029f, 0.725490212f, 1.000000000f); }
		static constexpr Color peru() { return Color(0.803921640f, 0.521568656f, 0.247058839f, 1.000000000f); }
		static constexpr Color pink() { return Color(1.000000000f, 0.752941251f, 0.796078503f, 1.000000000f); }
		static constexpr Color plum() { return Color(0.866666734f, 0.627451003f, 0.866666734f, 1.000000000f); }
		static constexpr Color powder_blue() { return Color(0.690196097f, 0.878431439f, 0.901960850f, 1.000000000f); }
		static constexpr Color purple() { return Color(0.501960814f, 0.000000000f, 0.501960814f, 1.000000000f); }
		static constexpr Color red() { return Color(1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color rosy_brown() { return Color(0.737254918f, 0.560784340f, 0.560784340f, 1.000000000f); }
		static constexpr Color royal_blue() { return Color(0.254901975f, 0.411764741f, 0.882353008f, 1.000000000f); }
		static constexpr Color saddle_brown() { return Color(0.545098066f, 0.270588249f, 0.074509807f, 1.000000000f); }
		static constexpr Color salmon() { return Color(0.980392218f, 0.501960814f, 0.447058856f, 1.000000000f); }
		static constexpr Color sandy_brown() { return Color(0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f); }
		static constexpr Color sea_green() { return Color(0.180392161f, 0.545098066f, 0.341176480f, 1.000000000f); }
		static constexpr Color sea_shell() { return Color(1.000000000f, 0.960784376f, 0.933333397f, 1.000000000f); }
		static constexpr Color sienna() { return Color(0.627451003f, 0.321568638f, 0.176470593f, 1.000000000f); }
		static constexpr Color silver() { return Color(0.752941251f, 0.752941251f, 0.752941251f, 1.000000000f); }
		static constexpr Color sky_blue() { return Color(0.529411793f, 0.807843208f, 0.921568692f, 1.000000000f); }
		static constexpr Color slate_blue() { return Color(0.415686309f, 0.352941185f, 0.803921640f, 1.000000000f); }
		static constexpr Color slate_gray() { return Color(0.439215720f, 0.501960814f, 0.564705908f, 1.000000000f); }
		static constexpr Color snow() { return Color(1.000000000f, 0.980392218f, 0.980392218f, 1.000000000f); }
		static constexpr Color spring_green() { return Color(0.000000000f, 1.000000000f, 0.498039246f, 1.000000000f); }
		static constexpr Color steel_blue() { return Color(0.274509817f, 0.509803951f, 0.705882370f, 1.000000000f); }
		static constexpr Color tan() { return Color(0.823529482f, 0.705882370f, 0.549019635f, 1.000000000f); }
		static constexpr Color teal() { return Color(0.000000000f, 0.501960814f, 0.501960814f, 1.000000000f); }
		static constexpr Color thistle() { return Color(0.847058892f, 0.749019623f, 0.847058892f, 1.000000000f); }
		static constexpr Color tomato() { return Color(1.000000000f, 0.388235331f, 0.278431386f, 1.000000000f); }
		static constexpr Color turquoise() { return Color(0.250980407f, 0.878431439f, 0.815686345f, 1.000000000f); }
		static constexpr Color violet() { return Color(0.933333397f, 0.509803951f, 0.933333397f, 1.000000000f); }
		static constexpr Color wheat() { return Color(0.960784376f, 0.870588303f, 0.701960802f, 1.000000000f); }
		static constexpr Color white() { return Color(1.000000000f, 1.000000000f, 1.000000000f, 1.000000000f); }
		static constexpr Color white_smoke() { return Color(0.960784376f, 0.960784376f, 0.960784376f, 1.000000000f); }
		static constexpr Color yellow() { return Color(1.000000000f, 1.000000000f, 0.000000000f, 1.000000000f); }
		static constexpr Color yellow_green() { return Color(0.603921592f, 0.803921640f, 0.196078449f, 1.000000000f); }
	};

	Color operator+ (const Color& v1, const Color& v2);
	Color operator- (const Color& v1, const Color& v2);
	Color operator* (const Color& v1, const Color& v2);
	Color operator/ (const Color& v1, const Color& v2);
	Color operator+ (const Color& v, f32 s);
	Color operator+ (f32 s, const Color& v);
	Color operator- (const Color& v, f32 s);
	Color operator- (f32 s, const Color& v);
	Color operator* (const Color& v, f32 s);
	Color operator* (f32 s, const Color& v);
	Color operator/ (const Color& v, f32 s);
	Color operator/ (f32 s, const Color& v);

	//! Adjusts color saturation.
	Color adjust_saturation(const Color& c, f32 sat);
	//! Adjusts color contrast.
	Color adjust_contrast(const Color& c, f32 contrast);
	//! Inverts a color value.
	Color negate(const Color& c);

	Color clamp(const Color& clr, const Color& clr_min, const Color& clr_max);
	Color min(const Color& a, const Color& b);
	Color max(const Color& a, const Color& b);
	Color lerp(const Color& a, const Color& b, f32 t);
	Color smoothstep(const Color& a, const Color& b, f32 t);
	Color barycentric(const Color& a, const Color& b, const Float4& c, f32 f, f32 g);

	LUNA_RUNTIME_API typeinfo_t color_type();
	template <> struct typeof_t<Color> { typeinfo_t operator()() const { return color_type(); } };
}

#include "../Source/Math/Color.inl"