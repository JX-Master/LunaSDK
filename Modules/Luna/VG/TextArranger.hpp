
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
        //! @addtogroup VG
        //! @{
        
        //! Specifies the test alignment side.
        enum class TextAlignment : u8
        {
            //! Aligns text to the beginning side of the text region.
            begin = 1,
            //! Aligns text to the center of the text region.
            center = 2,
            //! Aligns text to the ending side of the text region.
            end = 3,
        };

        //! Describes the arrange result of one text glyph.
        struct TextGlyphArrangeResult
        {
            //! The bounding rectangle of the glyph.
            RectF bounding_rect;

            //! The orgin point offset of this glyph relative to the beginning of the current
            //! line.
            f32 origin_offset;

            //! The advance length of the glyph.
            //! This is not always equal to `bounding_rect.width`, because some characters may take more spaces than necessary for paddings.
            f32 advance_length;

            //! The Unicode codepoint of the glyph.
            c32 character;

            //! The index of this glyph in the shape buffer of the font atlas.
            u32 index;
        };

        //! Describes the arrange result of one text line that contains multiple glyphs.
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

            //! The arrange result of one text glyphs in this line.
            Vector<TextGlyphArrangeResult> glyphs;
        };

        //! Describes text arrange result returned by @ref ITextArranger::arrange.
        struct TextArrangeResult
        {
            //! The real bounding rect occupied by the text. This may be smaller than 
            //! the bounding rect specified.
            RectF bounding_rect;

            //! True if the bounding rect is too small to hold all text specified.
            bool overflow;

            //! The arrange result of one text lines.
            Vector<TextLineArrangeResult> lines;
        };

        //! Describes parameters used to arrange one text section.
        struct TextArrangeSection
        {
            //! The font atlas used to record glyphs in this section.
            IFontAtlas* font_atlas = nullptr;
            //! The number of @ref c8 characters in this section.
            usize num_chars = 0;
            //! The font color in RGBA8 format.
            u32 color = 0xFFFFFFFF;
            //! The font size.
            f32 font_size = 18.0f;
            //! The space to add between two adjacent characters.
            f32 char_span = 0.0f;
            //! The space to add between two adjacent lines.
            f32 line_span = 0.0f;
        };
        
        //! Arranges glyphs in the specified bounding rectangle.
        //! @param[in] text The UTF-8 text to arrange.
        //! @param[in] text_len The length of `text. If this is 
        //! @ref USIZE_MAX, the text length is determined by `strlen(text)`.
        //! @param[in] sections The text arrange sections. Every section may
        //! use a different set of text arrange parameters.
        //! @param[in] bounding_rect The bounding rectangle to arrange text in.
        //! @param[in] vertical_alignment The vertical alignment for text lines.
        //! @param[in] horizontal_alignment The horizontal alignment for text lines.
        //! @return Returns the text arrange result.
        LUNA_VG_API TextArrangeResult arrange_text(
            const c8* text, usize text_len,
            Span<const TextArrangeSection> sections,
            const RectF& bounding_rect,
            TextAlignment vertical_alignment, 
            TextAlignment horizontal_alignment
        );

        //! Commits the text arrange result to the specicied draw list for rendering.
        //! @param[in] result The text arrange result.
        //! @param[in] sections The text arrange sections. This must be the same sections
        //! passed to @ref arrange_text when arranging texts.
        //! @param[in] draw_list The draw list to commit text arrange result to.
        LUNA_VG_API RV commit_text_arrange_result(
            const TextArrangeResult& result,
            Span<const TextArrangeSection> sections,
            IShapeDrawList* draw_list
        );

        //! @}
    }
}