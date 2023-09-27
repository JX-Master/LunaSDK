/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontAtlas.hpp
* @author JXMaster
* @date 2023/9/27
*/
#pragma once
#include <Luna/Font/Font.hpp>
#include <Luna/RHI/Buffer.hpp>

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        struct IFontAtlas : virtual Interface
		{
			luiid("{FCDB9053-448B-4E7D-BC94-B67A7E81081A}");

			virtual void clear() = 0;

			//! Gets the font bound to this font atlas.
			virtual Font::IFontFile* get_font(u32* index) = 0;

			//! Sets the font bound to this font atlas. The will reset the font atlas.
			virtual void set_font(Font::IFontFile* font, u32 index) = 0;

			virtual void get_glyph_hmetrics(u32 codepoint, i32* advance_width, i32* left_side_bearing) = 0;
		
			virtual f32 scale_for_pixel_height(f32 pixels) = 0;

			virtual void get_vmetrics(i32* ascent, i32* descent, i32* line_gap) = 0;
		
			virtual i32 get_kern_advance(u32 ch1, u32 ch2) = 0;

			//! Gets the shape buffer buffer. This call flushes shape commands so that they will be uploaded to GPU before this call returns.
			virtual R<RHI::IBuffer*> get_shape_buffer() = 0;

			//! Gets the shape points data.
			virtual Span<const f32> get_shape_points() = 0;

			//! Queries the information about the specified glyph.
			//! @param[in] codepoint The codepoint of the shape.
			//! @param[out] first_shape_point If not `nullptr`, returns the offset of the first point of the shape in the shape buffer.
			//! @param[out] num_shape_points If not `nullptr`, returns the number of points of the shape data.
			//! @param[out] bounding_rect If not `nullptr`, returns the bounding rect of the glyph.
			virtual void get_glyph(usize codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect) = 0;
		};

		LUNA_VG_API Ref<IFontAtlas> new_font_atlas(Font::IFontFile* font, u32 index, RHI::IDevice* device = nullptr);
    }
}