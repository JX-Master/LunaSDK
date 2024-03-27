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
        //! @addtogroup VG
        //! @{

        //! @interface IFontAtlas
        //! Represents one font glyph packer that packs font glyph data to one 
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

            //! Sets the font bound to this font atlas. The will reset the font atlas.
            //! @param[in] font The font file data used by this font atlas.
            //! @param[in] inedx The index of the font used by this font atlas.
            virtual void set_font(Font::IFontFile* font, u32 index) = 0;

            //! Gets the shape buffer that stores the glyph contour commands. 
            //! @details This call will copy shape command points to the shape buffer using GPU if shape point data
            //! is modified after last call to @ref get_shape_buffer (or if @ref get_shape_buffer is called for the first time after
            //! @ref clear), so the user should call this function only if all glyph shapes are packed to the atlas to avoid data copy overhead.
            //! @return Returns the shape buffer.
            virtual R<RHI::IBuffer*> get_shape_buffer() = 0;

            //! Gets the shape points data.
            //! @return Returns the shape point data. The returned data span is valid until a new glyph is packed to the atlas.
            virtual Span<const f32> get_shape_points() = 0;

            //! Queries the information of the specified glyph, and optionally packs the glyph to this atlas if it is not packed yet.
            //! @param[in] codepoint The codepoint of the glyph. This is the Unicode of the glyph in most font files.
            //! @param[out] first_shape_point If not `nullptr`, returns the offset of the first point of the shape in the shape buffer.
            //! @param[out] num_shape_points If not `nullptr`, returns the number of points of the shape data.
            //! @param[out] bounding_rect If not `nullptr`, returns the bounding rect of the glyph.
            virtual void get_glyph(usize codepoint, usize* first_shape_point, usize* num_shape_points, RectF* bounding_rect) = 0;
        };

        //! Creates one new font atlas.
        //! @param[in] font The font file data used to pack font glyph.
        //! @param[in] index The index of the font to use in font file data.
        //! @param[in] device The RHI device bound to the font atlas. This is used to create 
        //! RHI buffers used by the draw list.
        //! 
        //! If this is `nullptr`, the main device (device fetched from @ref RHI::get_main_device) will be used.
        //! @return Returns the created font atlas.
        LUNA_VG_API Ref<IFontAtlas> new_font_atlas(Font::IFontFile* font, u32 index, RHI::IDevice* device = nullptr);

        //! @}
    }
}