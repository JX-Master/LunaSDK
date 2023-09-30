/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextArranger.cpp
* @author JXMaster
* @date 2022/4/27
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_VG_API LUNA_EXPORT
#include "TextArranger.hpp"
#include <Luna/Runtime/Unicode.hpp>

namespace Luna
{
	namespace VG
	{
		bool should_rotate_in_vertical_line(u32 codepoint)
		{
			if (
				(codepoint >= 0x3000 && codepoint <= 0x30ff) || // Japanese characters.
				(codepoint >= 0xff00 && codepoint <= 0xffef) || // Japanese katakana (semi).
				(codepoint >= 0xAC00 && codepoint <= 0xD7FF) || // Korean characters.
				(codepoint >= 0x4e00 && codepoint <= 0x9fff) || // Basic Chinese characters / Basic Chinese characters extension.
				(codepoint >= 0x3400 && codepoint <= 0x4DBF) || // Chinese characters extension A.
				(codepoint >= 0x20000 && codepoint <= 0x2A6DF) || // Chinese characters extension B.
				(codepoint >= 0x2A700 && codepoint <= 0x2B738) || // Chinese characters extension C.
				(codepoint >= 0x2B740 && codepoint <= 0x2B81D) || // Chinese characters extension D.
				(codepoint >= 0x2B820 && codepoint <= 0x2CEA1) || // Chinese characters extension E.
				(codepoint >= 0x2CEB0 && codepoint <= 0x2EBE0) || // Chinese characters extension F.
				(codepoint >= 0x30000 && codepoint <= 0x3134A) || // Chinese characters extension G.
				(codepoint >= 0x2F00 && codepoint <= 0x2FD5) ||
				(codepoint >= 0x2E80 && codepoint <= 0x2EF3) ||
				(codepoint >= 0xF900 && codepoint <= 0xFAD9) ||
				(codepoint >= 0x2F800 && codepoint <= 0x2FA1D) ||
				(codepoint >= 0xE815 && codepoint <= 0xE86F) ||
				(codepoint >= 0xE400 && codepoint <= 0xE5E8) ||
				(codepoint >= 0xE600 && codepoint <= 0xE6CF) ||
				(codepoint >= 0x31C0 && codepoint <= 0x31E3) ||
				(codepoint >= 0x2FF0 && codepoint <= 0x2FFB) ||
				(codepoint >= 0x3105 && codepoint <= 0x312F) ||
				(codepoint >= 0x31A0 && codepoint <= 0x31BA) ||
				(codepoint == 0x3007)
				)
			{
				return false;
			}
			return true;
		}

		struct TextStream
		{
			String* m_text;
			Vector<Pair<usize, TextArranger::FontState>>* m_states;
			//! The current text.
			usize m_cursor;
			//! The current state.
			usize m_state_cursor;

			// Current state info. reloaded by `load_current_state`.

			IFontAtlas* m_font_atlas;
			f32 m_font_scale;
			f32 m_ascent;
			f32 m_decent;
			f32 m_line_gap;
			f32 m_font_size;
			f32 m_char_span;
			f32 m_line_span;

			// Current character info. reloaded by `load_current_char`.
			
			//! Current character.
			c32 m_char;
			//! The scaled width from this character's origin point to next character's origin point.
			f32 m_char_advance_length;
			//! The scaled width between the x axis of the origin point to the x of the leftest pixel.
			f32 m_char_left_side_bearing;
			RectF m_char_bounding_rect;

			void load_current_state();
			void load_current_char();
			void next_char()
			{
				m_cursor += utf8_charspan(m_char);
				if (m_state_cursor < (m_states->size() - 1) && m_states->at(m_state_cursor + 1).first <= m_cursor)
				{
					++m_state_cursor;
					load_current_state();
				}
				load_current_char();
			}
			TextStream(String* text, Vector<Pair<usize, TextArranger::FontState>>* states) :
				m_text(text),
				m_states(states),
				m_cursor(0),
				m_state_cursor(0) 
			{
				load_current_state();
				load_current_char();
			}

			IFontAtlas* get_next_char_font_file(f32& font_scale);

		};
		void TextStream::load_current_state()
		{
			if (m_state_cursor < m_states->size())
			{
				m_font_atlas = m_states->at(m_state_cursor).second.m_font;
				m_font_scale = m_font_atlas->scale_for_pixel_height(m_states->at(m_state_cursor).second.m_size);
				i32 a, d, l;
				m_font_atlas->get_vmetrics(&a, &d, &l);
				m_ascent = (f32)a * m_font_scale;
				m_decent = (f32)d * m_font_scale;
				m_line_gap = (f32)l * m_font_scale;
				m_font_size = m_states->at(m_state_cursor).second.m_size;
				m_char_span = m_states->at(m_state_cursor).second.m_char_span;
				m_line_span = m_states->at(m_state_cursor).second.m_line_span;
			}
		}
		void TextStream::load_current_char()
		{
			if (m_cursor < m_text->size())
			{
				m_char = utf8_decode_char(m_text->data() + m_cursor);
				i32 advance_width, left_side_bearing;
				m_font_atlas->get_glyph_hmetrics(m_char, &advance_width, &left_side_bearing);
				m_char_advance_length = (f32)advance_width * m_font_scale;
				m_char_left_side_bearing = (f32)left_side_bearing * m_font_scale;
				RectF rect;
				m_font_atlas->get_glyph(m_char, nullptr, nullptr, &rect);
				m_char_bounding_rect = RectF(rect.offset_x * m_font_scale, rect.offset_y * m_font_scale,
					rect.width * m_font_scale, rect.height * m_font_scale);
			}
		}

		IFontAtlas* TextStream::get_next_char_font_file(f32& font_scale)
		{
			usize next_cursor = m_cursor + utf8_charspan(m_char);
			if (m_state_cursor < (m_states->size() - 1) && m_states->at(m_state_cursor + 1).first <= next_cursor)
			{
				usize state_cursor = m_state_cursor + 1;
				auto font = m_states->at(state_cursor).second.m_font;
				font_scale = font->scale_for_pixel_height(m_states->at(state_cursor).second.m_size);
				return font;
			}
			font_scale = m_font_scale;
			return m_font_atlas;
		}

		TextArrangeResult TextArranger::arrange(const RectF& bounding_rect,
			TextAlignment line_alignment, TextAlignment glyph_alignment)
		{
			const f32 max_line_length = bounding_rect.width;
			// Used to clip overflow lines. 
			const f32 max_line_expand = bounding_rect.height;
			f32 line_expand = 0.0f;

			// Pass 1: arrange glyphs.
			Vector<TextLineArrangeResult> lines;
			{
				TextStream s(&m_text, &m_states);
				const f32 initial_glyph_offset = 0.0f;
				f32 glyph_origin = initial_glyph_offset;
				TextLineArrangeResult current_line;
				current_line.ascent = -F32_MAX;
				current_line.decent = F32_MAX;
				current_line.line_gap = -F32_MAX;
				while (s.m_cursor < s.m_text->size())
				{
					// Ignore \r.
					if (s.m_char == '\r')
					{
						s.next_char();
						continue;
					}
					if ((s.m_char == '\n') || (glyph_origin + s.m_char_advance_length > max_line_length))
					{
						// Switch to next line.
						lines.push_back(current_line); // Do copy instead move because we want to reuse the row buffer.
						line_expand += current_line.ascent + current_line.line_gap - current_line.decent;
						if (line_expand > max_line_expand) break;
						current_line.glyphs.clear();
						current_line.ascent = -F32_MAX;
						current_line.decent = F32_MAX;
						current_line.line_gap = -F32_MAX;
						glyph_origin = initial_glyph_offset;
						if (s.m_char == '\n')
						{
							// Skip this char.
							s.next_char();
							continue;
						}
					}
					if (glyph_origin + s.m_char_advance_length > max_line_length)
					{
						// The region width is too small to fit even one character, arrange failed.
						TextArrangeResult res;
						res.bounding_rect = RectF(bounding_rect.offset_x, bounding_rect.offset_y, 0.0f, 0.0f);
						res.overflow = true;
						return res;
					}
					// Pack this character into the current line.
					TextGlyphArrangeResult glyph;
					glyph.origin_offset = glyph_origin;
					glyph.advance_length = s.m_char_advance_length;
					glyph.character = s.m_char;
					glyph.index = (u32)s.m_cursor;
					glyph.bounding_rect = s.m_char_bounding_rect;
					current_line.glyphs.push_back(glyph);
					f32 kern;
					c32 next_char = utf8_decode_char(s.m_text->data() + s.m_cursor + utf8_charspan(s.m_char));
					if (next_char)
					{
						f32 next_font_scale;
						auto next_font_atlas = s.get_next_char_font_file(next_font_scale);
						kern = max((f32)s.m_font_atlas->get_kern_advance(s.m_char, next_char) * s.m_font_scale,
							(f32)next_font_atlas->get_kern_advance(s.m_char, next_char) * next_font_scale);
					}
					else kern = 0.0f;
					kern += s.m_char_span;
					current_line.ascent = max(current_line.ascent, s.m_ascent);
					current_line.decent = min(current_line.decent, s.m_decent);
					current_line.line_gap = max(current_line.line_gap, s.m_line_gap + s.m_line_span);
					glyph_origin += s.m_char_advance_length + kern;
					s.next_char();
				}
				if (!current_line.glyphs.empty())
				{
					lines.push_back(move(current_line));
				}
			}
			
			// Pass 2: arrange lines.
			{
				const f32 initial_line_offset = 0.0f;
				f32 line_baseline = initial_line_offset;
				for (usize i = 0; i < lines.size(); ++i)
				{
					auto& line = lines[i];
					f32 line_gap = line.line_gap;
					if (i < lines.size() - 1)
					{
						line_gap = max(line_gap, lines[i + 1].line_gap);
					}
					line_baseline += line.ascent;
					line.baseline_offset = line_baseline;
					line_baseline += line_gap - line.decent;
				}
			}

			// Pass 3: Calculates the bounding rect for every glyph and every line.
			TextArrangeResult res;
			res.overflow = false;
			{
				res.bounding_rect.width = 0.0f;
				res.bounding_rect.height = 0.0f;

				usize max_line = lines.size();

				// Calculates the size for every line.
				for (usize i = 0; i < lines.size(); ++i)
				{
					auto& line = lines[i];
					line.bounding_rect.width = line.glyphs.empty() ? 0.0f : (line.glyphs.back().origin_offset + line.glyphs.back().advance_length);
					line.bounding_rect.height = line.ascent - line.decent;
					res.bounding_rect.width = max(res.bounding_rect.width, line.bounding_rect.width);
					if (line.baseline_offset - line.decent > bounding_rect.height)
					{
						// overflow.
						max_line = i;
						break;
					}
				}
				// Clips out lines that cannot display.
				if (max_line < lines.size())
				{
					lines.resize(max_line);
					res.overflow = true;
				}
				// Calculate total line size.
				res.bounding_rect.height = lines.empty() ? 0.0f : (lines.back().baseline_offset - lines.back().decent);
				// Arrange the whole rect.
				if (line_alignment == TextAlignment::begin)
				{
					// Align to top.
					res.bounding_rect.offset_y = bounding_rect.offset_y + bounding_rect.height - res.bounding_rect.height;
				}
				else if (line_alignment == TextAlignment::end)
				{
					// Align to bottom.
					res.bounding_rect.offset_y = bounding_rect.offset_y;
				}
				else
				{
					// Align to center.
					res.bounding_rect.offset_y = bounding_rect.offset_y + (bounding_rect.height - res.bounding_rect.height) / 2.0f;
				}
				if (glyph_alignment == TextAlignment::begin)
				{
					// Align to left.
					res.bounding_rect.offset_x = bounding_rect.offset_x;
				}
				else if (glyph_alignment == TextAlignment::center)
				{
					// Align to center.
					res.bounding_rect.offset_x = bounding_rect.offset_x + (bounding_rect.width - res.bounding_rect.width) / 2.0f;
				}
				else
				{
					// Align to right.
					res.bounding_rect.offset_x = bounding_rect.offset_x + bounding_rect.width - res.bounding_rect.width;
				}
				// Arrange each line.
				for (usize i = 0; i < lines.size(); ++i)
				{
					auto& line = lines[i];
					// Top to down.
					line.bounding_rect.offset_y = res.bounding_rect.offset_y + res.bounding_rect.height - line.baseline_offset + line.decent;
					if (glyph_alignment == TextAlignment::begin)
					{
						// left to right.
						line.bounding_rect.offset_x = res.bounding_rect.offset_x;
					}
					else if (glyph_alignment == TextAlignment::end)
					{
						// right to left.
						line.bounding_rect.offset_x = res.bounding_rect.offset_x + res.bounding_rect.width - line.bounding_rect.width;
					}
					else
					{
						// center.
						line.bounding_rect.offset_x = res.bounding_rect.offset_x + (res.bounding_rect.width - line.bounding_rect.width) / 2.0f;
					}
					// Arrange each glyph in the line.
					for (usize j = 0; j < line.glyphs.size(); ++j)
					{
						auto& glyph = line.glyphs[j];
						glyph.bounding_rect.offset_x += line.bounding_rect.offset_x + glyph.origin_offset;
						glyph.bounding_rect.offset_y += line.bounding_rect.offset_y - line.decent;
					}
				}
			}
			res.lines = move(lines);
			return res;
		}

		RV TextArranger::commit(const TextArrangeResult& result, IShapeDrawList* draw_list)
		{
			lutry
			{
				usize state_index = 0;
				for (auto& line : result.lines)
				{
					for (auto& glyph : line.glyphs)
					{
						usize cursor = glyph.index;
						while ((state_index < m_states.size() - 1) && (m_states[state_index + 1].first <= cursor))
						{
							++state_index;
						}
						// Draw this glyph.
						usize size;
						RectF shape_coord;
						usize offset;
						m_states[state_index].second.m_font->get_glyph(glyph.character, &offset, &size, &shape_coord);
						if (glyph.bounding_rect.width != 0.0f && glyph.bounding_rect.height != 0.0f)
						{
							lulet(shape_buffer, m_states[state_index].second.m_font->get_shape_buffer());
							draw_list->set_shape_buffer(shape_buffer);
							draw_list->draw_shape((u32)offset, (u32)size,
								Float2U(glyph.bounding_rect.offset_x, glyph.bounding_rect.offset_y),
								Float2U(glyph.bounding_rect.offset_x + glyph.bounding_rect.width, glyph.bounding_rect.offset_y + glyph.bounding_rect.height),
								Float2U(shape_coord.offset_x, shape_coord.offset_y),
								Float2U(shape_coord.offset_x + shape_coord.width, shape_coord.offset_y + shape_coord.height),
								m_states[state_index].second.m_color
							);
						}
					}
				}
			}
			lucatchret;
			return ok;
		}

		LUNA_VG_API Ref<ITextArranger> new_text_arranger(IFontAtlas* initial_font)
		{
			auto ret = new_object<TextArranger>();
			ret->set_font(initial_font);
			return ret;
		}
	}
}