/*!
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
	//! @addtogroup RuntimeMath
	//! @{
	//! @defgroup RuntimeColor Color operations
	//! @}

	//! @addtogroup RuntimeColor
	//! @{

	namespace Color
	{
		// Regularly used colors.
		// Reference : http://www.w3school.com.cn/tiy/color.asp

		//! Predefined color.
		constexpr Float4 alice_blue() { return Float4(0.941176534f, 0.972549081f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 antique_white() { return Float4(0.980392218f, 0.921568692f, 0.843137324f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 aqua() { return Float4(0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 aquamarine() { return Float4(0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 azure() { return Float4(0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 beige() { return Float4(0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 bisque() { return Float4(1.000000000f, 0.894117713f, 0.768627524f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 black() { return Float4(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 blanched_almond() { return Float4(1.000000000f, 0.921568692f, 0.803921640f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 blue() { return Float4(0.000000000f, 0.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 blue_violet() { return Float4(0.541176498f, 0.168627456f, 0.886274576f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 brown() { return Float4(0.647058845f, 0.164705887f, 0.164705887f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 burly_wood() { return Float4(0.870588303f, 0.721568644f, 0.529411793f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 cadet_blue() { return Float4(0.372549027f, 0.619607866f, 0.627451003f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 chartreuse() { return Float4(0.498039246f, 1.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 chocolate() { return Float4(0.823529482f, 0.411764741f, 0.117647067f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 coral() { return Float4(1.000000000f, 0.498039246f, 0.313725501f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 cornflower_blue() { return Float4(0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 cornsilk() { return Float4(1.000000000f, 0.972549081f, 0.862745166f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 crimson() { return Float4(0.862745166f, 0.078431375f, 0.235294133f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 cyan() { return Float4(0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_blue() { return Float4(0.000000000f, 0.000000000f, 0.545098066f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_cyan() { return Float4(0.000000000f, 0.545098066f, 0.545098066f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_golden_rod() { return Float4(0.721568644f, 0.525490224f, 0.043137256f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_gray() { return Float4(0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_green() { return Float4(0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_khaki() { return Float4(0.741176486f, 0.717647076f, 0.419607878f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_magenta() { return Float4(0.545098066f, 0.000000000f, 0.545098066f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_olive_green() { return Float4(0.333333343f, 0.419607878f, 0.184313729f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_orange() { return Float4(1.000000000f, 0.549019635f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_orchid() { return Float4(0.600000024f, 0.196078449f, 0.800000072f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_red() { return Float4(0.545098066f, 0.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_salmon() { return Float4(0.913725555f, 0.588235319f, 0.478431404f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_sea_green() { return Float4(0.560784340f, 0.737254918f, 0.545098066f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_slate_blue() { return Float4(0.282352954f, 0.239215702f, 0.545098066f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_slate_gray() { return Float4(0.184313729f, 0.309803933f, 0.309803933f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_turquoise() { return Float4(0.000000000f, 0.807843208f, 0.819607913f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dark_violet() { return Float4(0.580392182f, 0.000000000f, 0.827451050f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 deep_pink() { return Float4(1.000000000f, 0.078431375f, 0.576470613f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 deep_sky_blue() { return Float4(0.000000000f, 0.749019623f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dim_gray() { return Float4(0.411764741f, 0.411764741f, 0.411764741f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 dodger_blue() { return Float4(0.117647067f, 0.564705908f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 fire_brick() { return Float4(0.698039234f, 0.133333340f, 0.133333340f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 floral_white() { return Float4(1.000000000f, 0.980392218f, 0.941176534f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 forest_green() { return Float4(0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 fuchsia() { return Float4(1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 gainsboro() { return Float4(0.862745166f, 0.862745166f, 0.862745166f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 ghost_white() { return Float4(0.972549081f, 0.972549081f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 gold() { return Float4(1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 golden_rod() { return Float4(0.854902029f, 0.647058845f, 0.125490203f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 gray() { return Float4(0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 green() { return Float4(0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 green_yellow() { return Float4(0.678431392f, 1.000000000f, 0.184313729f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 honey_dew() { return Float4(0.941176534f, 1.000000000f, 0.941176534f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 hot_pink() { return Float4(1.000000000f, 0.411764741f, 0.705882370f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 indian_red() { return Float4(0.803921640f, 0.360784322f, 0.360784322f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 indigo() { return Float4(0.294117659f, 0.000000000f, 0.509803951f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 ivory() { return Float4(1.000000000f, 1.000000000f, 0.941176534f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 khaki() { return Float4(0.941176534f, 0.901960850f, 0.549019635f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 lavender() { return Float4(0.901960850f, 0.901960850f, 0.980392218f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 lavender_blush() { return Float4(1.000000000f, 0.941176534f, 0.960784376f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 lawn_green() { return Float4(0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 lemon_chiffon() { return Float4(1.000000000f, 0.980392218f, 0.803921640f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_blue() { return Float4(0.678431392f, 0.847058892f, 0.901960850f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_coral() { return Float4(0.941176534f, 0.501960814f, 0.501960814f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_cyan() { return Float4(0.878431439f, 1.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_golden_rod_yellow() { return Float4(0.980392218f, 0.980392218f, 0.823529482f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_gray() { return Float4(0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_green() { return Float4(0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_pink() { return Float4(1.000000000f, 0.713725507f, 0.756862819f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_salmon() { return Float4(1.000000000f, 0.627451003f, 0.478431404f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_sea_green() { return Float4(0.125490203f, 0.698039234f, 0.666666687f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_sky_blue() { return Float4(0.529411793f, 0.807843208f, 0.980392218f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_slate_gray() { return Float4(0.466666698f, 0.533333361f, 0.600000024f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_steel_blue() { return Float4(0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 light_yellow() { return Float4(1.000000000f, 1.000000000f, 0.878431439f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 lime() { return Float4(0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 lime_green() { return Float4(0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 linen() { return Float4(0.980392218f, 0.941176534f, 0.901960850f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 magenta() { return Float4(1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 maroon() { return Float4(0.501960814f, 0.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_aqua_marine() { return Float4(0.400000036f, 0.803921640f, 0.666666687f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_blue() { return Float4(0.000000000f, 0.000000000f, 0.803921640f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_orchid() { return Float4(0.729411781f, 0.333333343f, 0.827451050f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_purple() { return Float4(0.576470613f, 0.439215720f, 0.858823597f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_sea_green() { return Float4(0.235294133f, 0.701960802f, 0.443137288f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_slate_blue() { return Float4(0.482352972f, 0.407843173f, 0.933333397f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_spring_green() { return Float4(0.000000000f, 0.980392218f, 0.603921592f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_turquoise() { return Float4(0.282352954f, 0.819607913f, 0.800000072f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 medium_violet_red() { return Float4(0.780392230f, 0.082352944f, 0.521568656f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 midnight_blue() { return Float4(0.098039225f, 0.098039225f, 0.439215720f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 mint_cream() { return Float4(0.960784376f, 1.000000000f, 0.980392218f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 misty_rose() { return Float4(1.000000000f, 0.894117713f, 0.882353008f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 moccasin() { return Float4(1.000000000f, 0.894117713f, 0.709803939f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 navajo_white() { return Float4(1.000000000f, 0.870588303f, 0.678431392f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 navy() { return Float4(0.000000000f, 0.000000000f, 0.501960814f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 old_lace() { return Float4(0.992156923f, 0.960784376f, 0.901960850f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 olive() { return Float4(0.501960814f, 0.501960814f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 olive_drab() { return Float4(0.419607878f, 0.556862772f, 0.137254909f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 orange() { return Float4(1.000000000f, 0.647058845f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 orange_red() { return Float4(1.000000000f, 0.270588249f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 orchid() { return Float4(0.854902029f, 0.439215720f, 0.839215755f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 pale_golden_rod() { return Float4(0.933333397f, 0.909803987f, 0.666666687f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 pale_green() { return Float4(0.596078455f, 0.984313786f, 0.596078455f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 pale_turquoise() { return Float4(0.686274529f, 0.933333397f, 0.933333397f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 pale_violet_red() { return Float4(0.858823597f, 0.439215720f, 0.576470613f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 papaya_whip() { return Float4(1.000000000f, 0.937254965f, 0.835294187f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 peach_puff() { return Float4(1.000000000f, 0.854902029f, 0.725490212f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 peru() { return Float4(0.803921640f, 0.521568656f, 0.247058839f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 pink() { return Float4(1.000000000f, 0.752941251f, 0.796078503f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 plum() { return Float4(0.866666734f, 0.627451003f, 0.866666734f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 powder_blue() { return Float4(0.690196097f, 0.878431439f, 0.901960850f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 purple() { return Float4(0.501960814f, 0.000000000f, 0.501960814f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 red() { return Float4(1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 rosy_brown() { return Float4(0.737254918f, 0.560784340f, 0.560784340f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 royal_blue() { return Float4(0.254901975f, 0.411764741f, 0.882353008f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 saddle_brown() { return Float4(0.545098066f, 0.270588249f, 0.074509807f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 salmon() { return Float4(0.980392218f, 0.501960814f, 0.447058856f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 sandy_brown() { return Float4(0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 sea_green() { return Float4(0.180392161f, 0.545098066f, 0.341176480f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 sea_shell() { return Float4(1.000000000f, 0.960784376f, 0.933333397f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 sienna() { return Float4(0.627451003f, 0.321568638f, 0.176470593f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 silver() { return Float4(0.752941251f, 0.752941251f, 0.752941251f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 sky_blue() { return Float4(0.529411793f, 0.807843208f, 0.921568692f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 slate_blue() { return Float4(0.415686309f, 0.352941185f, 0.803921640f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 slate_gray() { return Float4(0.439215720f, 0.501960814f, 0.564705908f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 snow() { return Float4(1.000000000f, 0.980392218f, 0.980392218f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 spring_green() { return Float4(0.000000000f, 1.000000000f, 0.498039246f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 steel_blue() { return Float4(0.274509817f, 0.509803951f, 0.705882370f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 tan() { return Float4(0.823529482f, 0.705882370f, 0.549019635f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 teal() { return Float4(0.000000000f, 0.501960814f, 0.501960814f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 thistle() { return Float4(0.847058892f, 0.749019623f, 0.847058892f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 tomato() { return Float4(1.000000000f, 0.388235331f, 0.278431386f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 turquoise() { return Float4(0.250980407f, 0.878431439f, 0.815686345f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 violet() { return Float4(0.933333397f, 0.509803951f, 0.933333397f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 wheat() { return Float4(0.960784376f, 0.870588303f, 0.701960802f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 white() { return Float4(1.000000000f, 1.000000000f, 1.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 white_smoke() { return Float4(0.960784376f, 0.960784376f, 0.960784376f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 yellow() { return Float4(1.000000000f, 1.000000000f, 0.000000000f, 1.000000000f); }
		//! Predefined color.
		constexpr Float4 yellow_green() { return Float4(0.603921592f, 0.803921640f, 0.196078449f, 1.000000000f); }

		//! Converts one @ref Float4 linear color value to one 32-bit color value.
		//! Every color component takes 8 bits, and is arranged in RGBA order (R in least significant 8 bits, A in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr u32 to_rgba8(const Float4& color);
		//! Converts one @ref Float4 linear color value to one 32-bit color value.
		//! Every color component takes 8 bits, and is arranged in ARGB order (A in least significant 8 bits, B in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr u32 to_argb8(const Float4& color);
		//! Converts one @ref Float4 linear color value to one 32-bit color value.
		//! Every color component takes 8 bits, and is arranged in ABGR order (A in least significant 8 bits, R in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr u32 to_abgr8(const Float4& color);
		//! Converts one @ref Float4 linear color value to one 32-bit color value.
		//! Every color component takes 8 bits, and is arranged in BGRA order (B in least significant 8 bits, A in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr u32 to_bgra8(const Float4& color);
		//! Converts one 32-bit color value to one @ref Float4 linear color.
		//! The source color is interpreted in RGBA order (R in least significant 8 bits, A in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr Float4 from_rgba8(u32 color);
		//! Converts one 32-bit color value to one @ref Float4 linear color.
		//! The source color is interpreted in ARGB order (A in least significant 8 bits, B in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr Float4 from_argb8(u32 color);
		//! Converts one 32-bit color value to one @ref Float4 linear color.
		//! The source color is interpreted in ABGR order (A in least significant 8 bits, R in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr Float4 from_abgr8(u32 color);
		//! Converts one 32-bit color value to one @ref Float4 linear color.
		//! The source color is interpreted in BGRA order (B in least significant 8 bits, A in most significant 8 bits).
		//! @param[in] color The color to convert.
		//! @return Returns the converted color.
		constexpr Float4 from_bgra8(u32 color);
		//! Adjusts color saturation.
		//! @details The color saturation is adjusted as follows:
		//! ```
		//! f32 lum = dot(color, Float4(0.2125f, 0.7154f, 0.0721f, 0.0f));
		//! Float4 ret = (color - lum) * sat + lum;
		//! ret.w = color.w;
		//! return ret;
		//! ``` 
		//! @param[in] color The color to adjust.
		//! @param[in] sat The saturation scale.
		//! @return Returns the adjusted color.
		Float4 adjust_saturation(const Float4& color, f32 sat);
		//! Adjusts color contrast.
		//! @details The color contrast is adjusted as follows:
		//! ```
		//! Float4 ret = ((color - 0.5f) * contrast) + 0.5f;
		//! ret.w = color.w;
		//! return ret;
		//! ``` 
		//! @param[in] color The color to adjust.
		//! @param[in] contrast The contrast scale.
		//! @return Returns the adjusted color.
		Float4 adjust_contrast(const Float4& color, f32 contrast);
		//! Inverts a color value.
		//! @details The color is inverted as follows:
		//! ```
		//! Float4 ret = 1.0f - color;
		//! ret.w = color.w;
		//! return ret;
		//! ``` 
		//! @param[in] color The color to invert.
		//! @return Returns the inverted color.
		Float4 negate(const Float4& color);
	}

	//! @}
}

#include "Impl/Color.inl"