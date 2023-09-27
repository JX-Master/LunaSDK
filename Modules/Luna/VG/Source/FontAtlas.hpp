/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontAtlas.hpp
* @author JXMaster
* @date 2022/4/27
*/
#pragma once
#include "../FontAtlas.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/RHI/Device.hpp>

namespace Luna
{
	namespace VG
	{
		struct FontAtlas : IFontAtlas
		{
			lustruct("VG::FontAtlas", "{E25DC74A-20B6-4207-B0C1-3E4F8CDB45A2}");
			luiimpl();
			lutsassert_lock();

			struct ShapeDesc
			{
				usize first_shape_point;
				usize num_shape_points;
				RectF bounding_rect;
			};

			struct GlyphData
			{
				i32 m_advance_width;
				i32 m_left_side_bearing;
				Font::glyph_t m_glyph;
				usize m_shape_index;
			};

			Ref<RHI::IDevice> m_device;
			Ref<Font::IFontFile> m_font;
			u32 m_font_index;
			Vector<f32> m_shape_points;
			Vector<ShapeDesc> m_shapes;
			HashMap<u64, GlyphData> m_shape_map;

			Ref<RHI::IBuffer> m_shape_buffer;
			usize m_shape_buffer_capacity;
			bool m_shape_buffer_dirty;

			i32 m_ascent;
			i32 m_descent;
			i32 m_line_gap;

			FontAtlas() :
				m_shape_buffer_capacity(0),
				m_shape_buffer_dirty(false) {}

			usize add_shape(Span<const f32> points, const RectF* bounding_rect);
			void load_default_glyph();
			bool load_glyph(u32 codepoint);
			usize get_glyph_shape_index(u32 codepoint);
			RV recreate_buffer();

			virtual void clear() override
			{
				lutsassert();
				m_shape_points.clear();
				m_shapes.clear();
				m_shape_buffer_dirty = false;
				m_shape_map.clear();
				load_default_glyph();
			}
			virtual Font::IFontFile* get_font(u32* index) override
			{
				lutsassert();
				if (index) *index = m_font_index;
				return m_font;
			}
			virtual void set_font(Font::IFontFile* font, u32 index) override
			{
				lutsassert();
				m_font = font;
				m_font_index = index;
				font->get_vmetrics(index, &(m_ascent), &(m_descent), &(m_line_gap));
				clear();
			}
			virtual void get_glyph_hmetrics(u32 codepoint, i32* advance_width, i32* left_side_bearing) override;
			virtual f32 scale_for_pixel_height(f32 pixels) override
			{
				lutsassert();
				return m_font->scale_for_pixel_height(m_font_index, pixels);
			}
			virtual void get_vmetrics(i32* ascent, i32* descent, i32* line_gap) override
			{
				lutsassert();
				if (ascent) *ascent = m_ascent;
				if (descent) *descent = m_descent;
				if (line_gap) *line_gap = m_line_gap;
			}
			virtual i32 get_kern_advance(u32 ch1, u32 ch2) override;
			virtual R<RHI::IBuffer*> get_shape_buffer() override;
			virtual Span<const f32> get_shape_points() override;
			virtual void get_glyph(usize codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect) override;
		};
	}
}