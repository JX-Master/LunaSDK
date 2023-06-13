/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FontFileTTF.cpp
* @author JXMaster
* @date 2019/10/7
*/
#define STB_TRUETYPE_IMPLEMENTATION
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include "FontFileTTF.hpp"

namespace Luna
{
	namespace Font
	{
		RV FontFileTTF::init(const byte_t* data, usize data_size)
		{
			i32 i = stbtt_GetNumberOfFonts((const unsigned char*)data);
			if (i <= 0)
			{
				return set_error(BasicError::bad_arguments(), "Create TTF font file failed because there is no font in the specified font file data.");
			}
			m_data = Blob(data, data_size);
			m_infos.reserve((u32)i);
			for (u32 j = 0; j < (u32)i; ++j)
			{
				stbtt_fontinfo info;
				stbtt_InitFont(&info, (const unsigned char*)m_data.data(),
					stbtt_GetFontOffsetForIndex((const unsigned char*)m_data.data(), j));
				info.userdata = nullptr;
				m_infos.push_back(info);
			}
			return ok;
		}
		u32 FontFileTTF::count_fonts()
		{
			return u32(m_infos.size());
		}
		glyph_t FontFileTTF::find_glyph(u32 font_index, u32 codepoint)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			int glyph = stbtt_FindGlyphIndex(&m_infos[font_index], codepoint);
			return glyph ? glyph : INVALID_GLYPH;
		}
		f32 FontFileTTF::scale_for_pixel_height(u32 font_index, f32 pixels)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			return stbtt_ScaleForPixelHeight(&m_infos[font_index], pixels);
		}
		void FontFileTTF::get_vmetrics(u32 font_index, i32* ascent, i32* descent, i32* lineGap)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			stbtt_GetFontVMetrics(&m_infos[font_index], ascent, descent, lineGap);
		}
		void FontFileTTF::get_glyph_hmetrics(u32 font_index, glyph_t glyph, i32* advance_width, i32* left_side_bearing)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			if (glyph == INVALID_GLYPH)
			{
				glyph = 0;
			}
			stbtt_GetGlyphHMetrics(&m_infos[font_index], glyph, advance_width, left_side_bearing);
		}
		i32 FontFileTTF::get_kern_advance(u32 font_index, glyph_t ch1, glyph_t ch2)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			return stbtt_GetGlyphKernAdvance(&m_infos[font_index], ch1, ch2);
		}
		Vector<i16> FontFileTTF::get_glyph_shape(u32 font_index, glyph_t glyph)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			if (glyph == INVALID_GLYPH)
			{
				return Vector<i16>();
			}
			Vector<i16> ret;
			stbtt_vertex* vertices;
			int num_vertices = stbtt_GetGlyphShape(&m_infos[font_index], glyph, &vertices);
			for (int i = 0; i < num_vertices; ++i)
			{
				const auto& v = vertices[i];
				switch (v.type)
				{
				case STBTT_vmove:
					ret.insert(ret.end(), { COMMAND_MOVE_TO, v.x, v.y }); break;
				case STBTT_vline:
					ret.insert(ret.end(), { COMMAND_LINE_TO, v.x, v.y }); break;
				case STBTT_vcurve:
					ret.insert(ret.end(), { COMMAND_CURVE_TO, v.cx, v.cy, v.x, v.y }); break;
				case STBTT_vcubic:
					lupanic(); break;
				}
			}
			if(vertices) stbtt_FreeShape(&m_infos[font_index], vertices);
			return ret;
		}
		RectI FontFileTTF::get_glyph_bounding_box(u32 font_index, glyph_t glyph)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			if (glyph == INVALID_GLYPH)
			{
				glyph = 0;
			}
			i32 x0, y0, x1, y1;
			int r = stbtt_GetGlyphBox(&m_infos[font_index], glyph, &x0, &y0, &x1, &y1);
			if (!r) return RectI(0, 0, 0, 0);
			return RectI(x0, y0, x1 - x0, y1 - y0);
		}
		RectI FontFileTTF::get_glyph_bitmap_box(u32 font_index, glyph_t glyph, f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			if (glyph == INVALID_GLYPH)
			{
				glyph = 0;
			}
			i32 ix0, iy0, ix1, iy1;
			stbtt_GetGlyphBitmapBoxSubpixel(&m_infos[font_index], glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, &ix1, &iy1);
			return RectI(ix0, iy0, ix1 - ix0, iy1 - iy0);
		}
		void FontFileTTF::render_glyph_bitmap(u32 font_index, glyph_t glyph, void* output, i32 out_w, i32 out_h, i32 out_row_pitch, f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y)
		{
			lucheck_msg(font_index < m_infos.size(), "Invalid font index.");
			if (glyph == INVALID_GLYPH)
			{
				glyph = 0;
			}
			stbtt_MakeGlyphBitmapSubpixel(&m_infos[font_index], (unsigned char*)output, out_w, out_h, out_row_pitch, scale_x, scale_y, shift_x, shift_y, glyph);
		}
	}
}