/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontAtlas.hpp
* @author JXMaster
* @date 2022/4/27
*/
#pragma once
#include "../VG.hpp"
#include <Runtime/HashMap.hpp>

namespace Luna
{
	namespace VG
	{
		struct FontAtlas : IFontAtlas
		{
			lustruct("VG::FontAtlas", "{E25DC74A-20B6-4207-B0C1-3E4F8CDB45A2}");
			luiimpl();

			struct GlyphData
			{
				i32 m_advance_width;
				i32 m_left_side_bearing;
				Font::glyph_t m_glyph;
				usize m_shape_index;
			};

			Ref<Font::IFontFile> m_font;
			u32 m_font_index;
			Ref<IShapeAtlas> m_shape_atlas;
			HashMap<u64, GlyphData> m_shape_map;

			i32 m_ascent;
			i32 m_descent;
			i32 m_line_gap;

			FontAtlas() {}

			void load_default_glyph();

			bool load_glyph(u32 codepoint);

			RV recreate_buffer();

			void clear()
			{
				m_shape_atlas->clear();
				m_shape_map.clear();
				load_default_glyph();
			}
			IShapeAtlas* get_shape_atlas()
			{
				return m_shape_atlas;
			}
			Font::IFontFile* get_font(u32* index)
			{
				if (index) *index = m_font_index;
				return m_font;
			}
			void set_font(Font::IFontFile* font, u32 index)
			{
				m_font = font;
				m_font_index = index;
				font->get_vmetrics(index, &(m_ascent), &(m_descent), &(m_line_gap));
				clear();
			}
			usize get_glyph_shape_index(u32 codepoint);
			void get_glyph_hmetrics(u32 codepoint, i32* advance_width, i32* left_side_bearing);
			f32 scale_for_pixel_height(f32 pixels)
			{
				return m_font->scale_for_pixel_height(m_font_index, pixels);
			}
			void get_vmetrics(i32* ascent, i32* descent, i32* line_gap)
			{
				if (ascent) *ascent = m_ascent;
				if (descent) *descent = m_descent;
				if (line_gap) *line_gap = m_line_gap;
			}
			i32 get_kern_advance(u32 ch1, u32 ch2);
		};
	}
}