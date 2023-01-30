/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontAtlas.cpp
* @author JXMaster
* @date 2022/4/27
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "FontAtlas.hpp"

namespace Luna
{
	namespace VG
	{
		void FontAtlas::load_default_glyph()
		{
			f32 points[] = {
				COMMAND_MOVE_TO, 0.0f, 0.0f,
				COMMAND_LINE_TO, 0.0f, 10.0f,
				COMMAND_LINE_TO, 5.0f, 10.0f,
				COMMAND_LINE_TO, 5.0f, 0.0f,
				COMMAND_LINE_TO, 0.0f, 0.0f,
			};
			usize index = m_shape_atlas->add_shape(points, 15, &RectF(0.0f, 0.0f, 5.0f, 10.0f));
			GlyphData data;
			data.m_glyph = Font::INVALID_GLYPH;
			data.m_shape_index = index;
			data.m_advance_width = 5;
			data.m_left_side_bearing = 0;
			m_shape_map.insert(make_pair(0, data));
		}
		bool FontAtlas::load_glyph(u32 codepoint)
		{
			auto glyph = m_font->find_glyph(m_font_index, codepoint);
			if (glyph == Font::INVALID_GLYPH)
			{
				return false;
			}
			auto font_shape = m_font->get_glyph_shape(m_font_index, glyph);
			usize i = 0;
			Vector<f32> font_data;
			while (i < font_shape.size())
			{
				auto command = font_shape[i];
				switch (command)
				{
				case Font::COMMAND_MOVE_TO:
				{
					f32 p0 = f32(font_shape[i + 1]);
					f32 p1 = f32(font_shape[i + 2]);
					font_data.insert(font_data.end(), { VG::COMMAND_MOVE_TO,
						p0, p1 });
					i += 3;
				}
				break;
				case Font::COMMAND_LINE_TO:
				{
					f32 p0 = f32(font_shape[i + 1]);
					f32 p1 = f32(font_shape[i + 2]);
					font_data.insert(font_data.end(), { VG::COMMAND_LINE_TO,
						p0, p1 });
					i += 3;
				}
				break;
				case Font::COMMAND_CURVE_TO:
				{
					f32 p0 = f32(font_shape[i + 1]);
					f32 p1 = f32(font_shape[i + 2]);
					f32 p2 = f32(font_shape[i + 3]);
					f32 p3 = f32(font_shape[i + 4]);
					font_data.insert(font_data.end(), { VG::COMMAND_CURVE_TO,
						p0, p1, p2, p3 });
					i += 5;
				}
				break;
				default: lupanic();
				}
			}
			RectI rect = m_font->get_glyph_bounding_box(m_font_index, glyph);
			usize shape_index = m_shape_atlas->add_shape(font_data.data(), font_data.size(), &RectF((f32)rect.offset_x, (f32)rect.offset_y, (f32)rect.width, (f32)rect.height));
			GlyphData data;
			data.m_shape_index = shape_index;
			data.m_glyph = glyph;
			m_font->get_glyph_hmetrics(m_font_index, glyph, &data.m_advance_width, &data.m_left_side_bearing);
			m_shape_map.insert(make_pair(codepoint, data));
			return true;
		}
		usize FontAtlas::get_glyph_shape_index(u32 codepoint)
		{
			auto iter = m_shape_map.find(codepoint);
			if (iter == m_shape_map.end())
			{
				auto loaded = load_glyph(codepoint);
				if (loaded) iter = m_shape_map.find(codepoint);
				else iter = m_shape_map.find(0);
			}
			return iter->second.m_shape_index;
		}
		void FontAtlas::get_glyph_hmetrics(u32 codepoint, i32* advance_width, i32* left_side_bearing)
		{
			auto iter = m_shape_map.find(codepoint);
			if (iter == m_shape_map.end())
			{
				auto loaded = load_glyph(codepoint);
				if (loaded) iter = m_shape_map.find(codepoint);
				else iter = m_shape_map.find(0);
			}
			if (advance_width) *advance_width = iter->second.m_advance_width;
			if (left_side_bearing) *left_side_bearing = iter->second.m_left_side_bearing;
		}
		i32 FontAtlas::get_kern_advance(u32 ch1, u32 ch2)
		{
			auto iter1 = m_shape_map.find(ch1);
			if (iter1 == m_shape_map.end())
			{
				auto loaded = load_glyph(ch1);
				if (loaded)
				{
					iter1 = m_shape_map.find(ch1);
				}
			}
			auto iter2 = m_shape_map.find(ch2);
			if (iter2 == m_shape_map.end())
			{
				auto loaded = load_glyph(ch2);
				if (loaded)
				{
					iter2 = m_shape_map.find(ch2);
				}
			}
			if (iter1 != m_shape_map.end() && iter2 != m_shape_map.end())
			{
				return m_font->get_kern_advance(m_font_index, iter1->second.m_glyph, iter2->second.m_glyph);
			}
			return 0;
		}
		LUNA_VG_API Ref<IFontAtlas> new_font_atlas(Font::IFontFile* font, u32 index)
		{
			Ref<FontAtlas> ret = new_object<FontAtlas>();
			ret->m_shape_atlas = new_shape_atlas();
			ret->set_font(font, index);
			return ret;
		}
	}
}