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
#include "ShapeBuffer.hpp"

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! @addtogroup VG
        //! @{

        //! @interface IFontAtlas
        //! Represents one font glyph packer that packs font glyph data to one shape buffer.
        struct IFontAtlas : virtual Interface
        {
            luiid("{FCDB9053-448B-4E7D-BC94-B67A7E81081A}");

            //! Removes all glyphs in the font atlas, but keeps the internal storage, so that
            //! they can be reused to store new glyphs.
            virtual void clear() = 0;

            //! Gets the font file data bound to this font atlas.
            //! @param[out] index If not `nullptr`, returns the index of the font used by this font atlas.
            //! @return Returns the font file data bound to this font atlas.
            virtual Font::IFontFile* get_font(u32* index) = 0;

            //! Sets the font bound to this font atlas.
            //! @param[in] font The font file data used by this font atlas.
            //! @param[in] inedx The index of the font used by this font atlas.
            virtual void set_font(Font::IFontFile* font, u32 index) = 0;

            //! Gets the shape buffer that stores the glyph contour commands. 
            //! @remark Note that the shape buffer data is managed by font atlas, the user should not modify
            //! the data directly, but can bind the shape buffer to any shape draw list.
            //! @return Returns the shape buffer.
            virtual IShapeBuffer* get_shape_buffer() = 0;

            //! Queries the information of the specified glyph, and optionally packs the glyph to this atlas if it is not packed yet.
            //! @param[in] codepoint The codepoint of the glyph. This is the Unicode of the glyph in most font files.
            //! @param[out] first_shape_point If not `nullptr`, returns the offset of the first point of the shape in the shape buffer.
            //! @param[out] num_shape_points If not `nullptr`, returns the number of points of the shape data.
            //! @param[out] bounding_rect If not `nullptr`, returns the bounding rect of the glyph.
            virtual void get_glyph(u32 codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect) = 0;
        };

        //! Creates one new font atlas.
        //! @return Returns the created font atlas.
        LUNA_VG_API Ref<IFontAtlas> new_font_atlas();

         //! Gets one font glyph shape.
        //! @param[in] font The font to query shape.
        //! @param[in] font_index The font index to query shape.
        //! @param[in] codepoint The codepoint of the glyph. This is the Unicode of the glyph in most font files.
        //! @param[out] out_shape_points If not `nullptr`, returns the glyph shape commands. Commands will be appended to the back of this vector.
        //! @param[out] out_bounding_rect If not `nullptr`, returns the bounding rect of the glyph.
        LUNA_VG_API RV get_font_glyph_shape(Font::IFontFile* font, u32 font_index, u32 codepoint, Vector<f32>* out_shape_points, RectF* out_bounding_rect);


        //! @}
    }
}