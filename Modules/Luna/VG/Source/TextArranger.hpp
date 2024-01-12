/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextArranger.hpp
* @author JXMaster
* @date 2022/4/27
*/
#pragma once
#include "../TextArranger.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/Runtime/TSAssert.hpp>
namespace Luna
{
	namespace VG
	{
		struct TextArranger : ITextArranger
		{
			lustruct("VG::TextArranger", "{557EAB99-BFDB-484D-8445-323AC8FC521F}");
			luiimpl();
			lutsassert_lock();

			struct FontState
			{
				Ref<IFontAtlas> m_font;
				u32 m_color;
				f32 m_size;
				f32 m_char_span;
				f32 m_line_span;
			};

			String m_text;
			Vector<Pair<usize, FontState>> m_states;

			FontState m_current_state;
			bool m_state_dirty;

			TextArranger() :
				m_state_dirty(true)
			{
				m_current_state.m_font = nullptr;
				m_current_state.m_color = 0xFFFFFFFF;
				m_current_state.m_size = 18.0f;	// Choose from practice. Suitable for UI text rendering in normal DPI.
				m_current_state.m_char_span = 0.0f;
				m_current_state.m_line_span = 0.0f;
			}

			void reset()
			{
				lutsassert();
				m_text.clear();
				m_states.clear();
				m_state_dirty = true;
				m_current_state.m_font = nullptr;
				m_current_state.m_color = 0xFFFFFFFF;
				m_current_state.m_size = 18.0f;	// Choose from practice. Suitable for UI text rendering in normal DPI.
				m_current_state.m_char_span = 0.0f;
				m_current_state.m_line_span = 0.0f;
			}

			void clear_text_buffer()
			{
				lutsassert();
				m_text.clear();
				m_states.clear();
				m_state_dirty = true;
			}

			IFontAtlas* get_font()
			{
				return m_current_state.m_font;
			}
			void  set_font(IFontAtlas* font)
			{
				lutsassert();
				if (font != m_current_state.m_font)
				{
					m_current_state.m_font = font;
					m_state_dirty = true;
				}
			}
			u32 get_font_color()
			{
				return m_current_state.m_color;
			}
			void  set_font_color(u32 color)
			{
				lutsassert();
				if (color != m_current_state.m_color)
				{
					m_current_state.m_color = color;
					m_state_dirty = true;
				}
			}
			f32 get_font_size()
			{
				return m_current_state.m_size;
			}
			void  set_font_size(f32 size)
			{
				lutsassert();
				if (m_current_state.m_size != size)
				{
					m_current_state.m_size = size;
					m_state_dirty = true;
				}
			}
			f32 get_char_span()
			{
				return m_current_state.m_char_span;
			}
			void set_char_span(f32 span)
			{
				lutsassert();
				if (m_current_state.m_char_span != span)
				{
					m_current_state.m_char_span = span;
					m_state_dirty = true;
				}
			}
			f32 get_line_span()
			{
				return m_current_state.m_line_span;
			}
			void  set_line_span(f32 span)
			{
				lutsassert();
				if (m_current_state.m_line_span != span)
				{
					m_current_state.m_line_span = span;
					m_state_dirty = true;
				}
			}
			void add_text(const c8* text)
			{
				if (m_state_dirty)
				{
					m_states.push_back(make_pair(m_text.size(), m_current_state));
					m_state_dirty = false;
				}
				m_text.append(text);
			}
			void add_text_region(const c8* text, usize text_len)
			{
				if (m_state_dirty)
				{
					m_states.push_back(make_pair(m_text.size(), m_current_state));
					m_state_dirty = false;
				}
				m_text.append(text, text_len);
			}

			TextArrangeResult arrange(const RectF& bounding_rect, 
				TextAlignment line_alignment, TextAlignment glyph_alignment);

			RV commit(const TextArrangeResult& result, IShapeDrawList* draw_list);
		};
	}
}