
/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TextArranger.hpp
* @author JXMaster
* @date 2023/9/27
*/
#pragma once
#include "FontAtlas.hpp"
#include "ShapeDrawList.hpp"
#include <Luna/Runtime/Math/Color.hpp>

namespace Luna
{
    namespace VG
    {
        enum class TextAlignment : u8
		{
			begin = 1,
			center = 2,
			end = 3,
		};

		struct TextGlyphArrangeResult
		{
			RectF bounding_rect;

			//! The orgin point offset of this glyph relative to the beginning of the current
			//! line.
			f32 origin_offset;

			//! The advance length of the glyph.
			//! This is not equal to `bounding_rect.width` (or `bounding_rect.height` in vertical 
			//! line), because some characters may take more spaces than necessary for paddings.
			f32 advance_length;

			//! The UTF-32 character codepoint of the glyph.
			c32 character;

			//! The index of this glyph in the text buffer.
			u32 index;
		};

		struct TextLineArrangeResult
		{
			//! The bounding rect of the line.
			RectF bounding_rect;

			//! The offset of the baseline of this line. The offset is relative to the starting edge of the
			//! text's bounding box.
			f32 baseline_offset;

			//! The ascent value (units from baseline to the top of the character) of this line.
			f32 ascent;

			//! The decent value (units from baseline to the bottom of the character, typically negative) of this line.
			f32 decent;

			//! The line gap of this line. The final line gap is determined by the greater line_gap
			//! value of two adjacent lines.
			f32 line_gap;

			Vector<TextGlyphArrangeResult> glyphs;
		};

		struct TextArrangeResult
		{
			//! The real bounding rect occupied by the text. This may be smaller than 
			//! the bounding rect specified.
			RectF bounding_rect;

			//! True if the bounding rect is too small to hold all text specified.
			bool overflow;

			Vector<TextLineArrangeResult> lines;
		};

		struct ITextArranger : virtual Interface
		{
			luiid("{EB049D67-134C-4F84-A912-99A8AC406847}");

			virtual void reset() = 0;

			virtual void clear_text_buffer() = 0;

			virtual IFontAtlas* get_font() = 0;

			virtual void set_font(IFontAtlas* font) = 0;

			virtual u32 get_font_color() = 0;

			virtual void set_font_color(u32 color) = 0;

			virtual f32 get_font_size() = 0;

			virtual void set_font_size(f32 size) = 0;

			virtual f32 get_char_span() = 0;

			//! Sets the spen between the last character and the next character. The character span value takes effect 
			//! until it is changed again.
			virtual void set_char_span(f32 span) = 0;

			virtual f32 get_line_span() = 0;

			//! Sets the line span between the current line and the next line. The line span value takes effect until it is 
			//! changed again.
			virtual void set_line_span(f32 span) = 0;

			virtual void add_text(const c8* text) = 0;

			virtual void add_text_region(const c8* text, usize text_len) = 0;

			virtual TextArrangeResult arrange(const RectF& bounding_rect,
				TextAlignment line_alignment, TextAlignment glyph_alignment) = 0;

			virtual RV commit(const TextArrangeResult& result, IShapeDrawList* draw_list) = 0;
		};

		LUNA_VG_API Ref<ITextArranger> new_text_arranger(IFontAtlas* initial_font);
    }
}