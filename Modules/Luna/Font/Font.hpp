/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Font.hpp
* @author JXMaster
* @date 2019/10/7
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_FONT_API
#define LUNA_FONT_API
#endif

namespace Luna
{
	namespace Font
	{
		using glyph_t = i32;
		constexpr glyph_t INVALID_GLYPH = -1;

		constexpr i16 COMMAND_MOVE_TO = 1; // Starts a new contour. Two data points: x, y
		constexpr i16 COMMAND_LINE_TO = 2; // Line. Two data points: x, y
		constexpr i16 COMMAND_CURVE_TO = 3;// Quadratic Belzier curve. Four data points: cx, cy, x, y

		//! @interface IFontFile
		//! Represents a font file that may contain one or multiple fonts.
		struct IFontFile : virtual Interface
		{
			luiid("{989fe385-6d98-480d-89ab-6b7dd3ec5624}");

			//! Gets the data of the font file.
			virtual const Blob& get_data() = 0;

			//! Gets the number of fonts in the font file.
			virtual u32 count_fonts() = 0;

			//! Gets the glyph index of the specified character in specified font.
			virtual glyph_t find_glyph(u32 font_index, u32 codepoint) = 0;

			//! Computes a scale factor to produce a font whose "height" is 'pixels' tall.
			//! Height is measured as the distance from the highest ascender to the lowest
			//! descender; in other words, it's equivalent to calling get_vmetrics
			//! and computing:
			//!       scale = pixels / (ascent - descent)
			//! so if you prefer to measure height by the ascent only, use a similar calculation.
			virtual f32 scale_for_pixel_height(u32 font_index, f32 pixels) = 0;

			//! Gets the metrics information for a font in the vertical side.
			//! ascent is the coordinate above the baseline the font extends; descent
			//! is the coordinate below the baseline the font extends (i.e. it is typically negative)
			//! line_gap is the spacing between one row's descent and the next row's ascent...
			//! so you should advance the vertical position by "*ascent - *descent + *line_gap"
			//!   these are expressed in unscaled coordinates, so you must multiply by
			//!   the scale factor for a given size
			virtual void get_vmetrics(u32 font_index, i32* ascent, i32* descent, i32* line_gap) = 0;

			//! Gets the metrics information for a glyph in the horizontal side.
			//! left_side_bearing is the offset from the current horizontal position to the left edge of the character
			//! advance_width is the offset from the current horizontal position to the next horizontal position
			//!   these are expressed in unscaled coordinates.
			virtual void get_glyph_hmetrics(u32 font_index, glyph_t glyph, i32* advance_width, i32* left_side_bearing) = 0;

			//! Get an additional amount to add to the 'advance' value between ch1 and ch2.
			virtual i32 get_kern_advance(u32 font_index, glyph_t ch1, glyph_t ch2) = 0;

			//! Returns commands in order to draw the specified glyph in unscaled space.
			virtual Vector<i16> get_glyph_shape(u32 font_index, glyph_t glyph) = 0;

			//! Gets the bounding box of the visible part of the glyph, in unscaled coordinates.
			virtual RectI get_glyph_bounding_box(u32 font_index, glyph_t glyph) = 0;

			//! Gets the bounding box of the bitmap centered around the glyph origin; so the location to place the bitmap top left is (left_side_bearing * scale, offset_y).
			//! (Note that the bitmap uses y-increases-down, but the shape uses y-increases-up, so get_glyph_bitmap_box and get_glyph_bounding_box are inverted.)
			virtual RectI get_glyph_bitmap_box(u32 font_index, glyph_t glyph, f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y) = 0;

			//! Renders a bitmap of the specified glyph into the buffer specified by the `output`, where `out_w` and `out_h` is the width and height
			//! of region of the buffer to be used, and `out_row_pitch` is the stride size of one row of the buffer.
			//! `scale_x` and `scale_y` is the scale factor computed through `scale_for_pixel_height` call, and `shift_x`, `shify_y` is the shift of
			//! the glyph in pixels.
			//!
			//! The rendered bitmap is a one-channel bitmap with each pixel take one byte. The value ranges in 0-255.
			//! 
			//! This call uses CPU to rasterize the glyph, since the glyph bitmap is usually very small, this usually does not cause performance issue,
			//! but you should save the render result whenever possible.
			virtual void render_glyph_bitmap(u32 font_index, glyph_t glyph, void* output, i32 out_w, i32 out_h, i32 out_row_pitch,
				f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y) = 0;
		};

		enum class FontFileFormat : u32
		{
			ttf = 1,	// True Type Font, including .ttf and .ttc
		};

		//! Creates a font file object by parsing the provided font file data.
		//! To load font file, you should first creates a blob that owns the file data (.ttf or .ttc), then you pass the 
		//! data to the font system to create a font object for it. The font data will be referenced by font data and should
		//! not be changed during the font lifetime.
		//! @param[in] data The data of the font file.
		//! @param[in] data_size The data size of the font file.
		//! @param[in] format The file format of the data font.
		//! @return If succeeded, returns the created font file object which is initialized using the
		//! font data.
		LUNA_FONT_API R<Ref<IFontFile>> load_font_file(const byte_t* data, usize data_size, FontFileFormat format);

		//! Gets the default font object, which is embedded into the SDK and only supports for ASCII codepoint range.
		LUNA_FONT_API IFontFile* get_default_font();
	}

	struct Module;
	LUNA_FONT_API Module* module_font();
}