/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Font.hpp
* @author JXMaster
* @date 2019/10/7
*/
#pragma once
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Math/Math.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_FONT_API
#define LUNA_FONT_API
#endif

namespace Luna
{
    namespace Font
    {
        //! @addtogroup Font Font
        //! Font module provides functions to parse font file, extract glyph shape and render glyph bitmap using CPU.
        //! @{
        
        //! The index type of one font glyph.
        using glyph_t = i32;

        //! A special index value that identifies one invalid glyph.
        constexpr glyph_t INVALID_GLYPH = -1;

        //! The command value that starts a new contour, followed by two data points: x, y
        constexpr i16 COMMAND_MOVE_TO = 1;
        //! The command value that draws a line to the specified point, 
        //! followed by two data points: x, y
        constexpr i16 COMMAND_LINE_TO = 2;
        //! The command value that draws a Quadratic Belzier curve to the specified point, 
        //! followed by four data points: cx, cy, x, y
        constexpr i16 COMMAND_CURVE_TO = 3;

        //! @interface IFontFile
        //! Represents a font file that may contain one or multiple fonts.
        struct IFontFile : virtual Interface
        {
            luiid("{989fe385-6d98-480d-89ab-6b7dd3ec5624}");

            //! Gets the data of the font file.
            //! @return Returns the data of the font file.
            virtual Span<const byte_t> get_data() = 0;

            //! Gets the number of fonts in the font file.
            //! @return Returns the number of fonts in the font file.
            virtual u32 get_num_fonts() = 0;

            //! Gets the glyph index of the specified character in specified font.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] codepoint The codepoint of the glyph.
            //! @return Returns the glyph index of the specified character.
            virtual glyph_t find_glyph(u32 font_index, u32 codepoint) = 0;

            //! Computes a scale factor to produce a font whose "height" is `pixels` tall.
            //! @details Height is measured as the distance from the highest ascender to 
            //! the lowest descender; in other words, it's equivalent to calling @ref get_vmetrics
            //! and computing:
            //!       scale = pixels / (ascent - descent)
            //! so if you prefer to measure height by the ascent only, use a similar calculation.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] pixels The new hight of the font glyphs.
            //! @return Returns the scale factor.
            virtual f32 scale_for_pixel_height(u32 font_index, f32 pixels) = 0;

            //! Gets the metrics information for a font in the vertical side.
            //! @details These values are expressed in unscaled coordinates, so you must multiply by
            //! the scale factor for a given size.
            //! @param[in] font_index The index of the font to query.
            //! @param[out] ascent The coordinate above the baseline the font extends.
            //! @param[out] descent The coordinate below the baseline the font extends (i.e. it is typically negative).
            //! @param[out] line_gap The spacing between one row's descent and the next row's ascent,
            //! so you should advance the vertical position by "*ascent - *descent + *line_gap"
            virtual void get_vmetrics(u32 font_index, i32* ascent, i32* descent, i32* line_gap) = 0;

            //! Gets the metrics information for a glyph in the horizontal side.
            //! @details These values are expressed in unscaled coordinates, so you must multiply by
            //! the scale factor for a given size.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] glyph The index of the glyph to query.
            //! @param[out] advance_width The offset from the current horizontal position to the next horizontal position.
            //! @param[out] left_side_bearing The offset from the current horizontal position to the left edge of the character.
            virtual void get_glyph_hmetrics(u32 font_index, glyph_t glyph, i32* advance_width, i32* left_side_bearing) = 0;

            //! Gets an additional amount to add to the 'advance' value between ch1 and ch2.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] glyph The index of the first glyph.
            //! @param[in] glyph The index of the second glyph.
            //! @return Returns the kern advance between two glyphs in unscaled coordinates.
            virtual i32 get_kern_advance(u32 font_index, glyph_t ch1, glyph_t ch2) = 0;

            //! Gets commands in order to draw the specified glyph in unscaled space.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] glyph The index of the glyph to query.
            //! @param[out] out_commands The vector to fetch the returned commands. New commands will be 
            //! added to the back of the vector, and the existing content in the vector will not be modified.
            virtual void get_glyph_shape(u32 font_index, glyph_t glyph, Vector<i16>& out_commands) = 0;

            //! Gets the bounding box of the visible part of the glyph.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] glyph The index of the glyph to query.
            //! @return Returns the bounding box of the visible part of the glyph, in unscaled coordinates.
            virtual RectI get_glyph_bounding_box(u32 font_index, glyph_t glyph) = 0;

            //! Gets the bounding box of the bitmap centered around the glyph origin.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] glyph The index of the glyph to query.
            //! @param[in] scale_x The scale factor in horizontal direction.
            //! @param[in] scale_y The scale factor in vertical direction.
            //! @param[in] shift_x The value to shift bitmap bounding box in horizontal direction in pixels.
            //! @param[in] shift_y The value to shift bitmap bounding box in vertical direction in pixels.
            //! @remark The location to place the bitmap top left is (`left_side_bearing * scale_x + shift_x`, `shift_y`).
            //! Note that the bitmap uses y-increases-down, but the shape uses y-increases-up, so @ref get_glyph_bitmap_box and @ref get_glyph_bounding_box are inverted.
            virtual RectI get_glyph_bitmap_box(u32 font_index, glyph_t glyph, f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y) = 0;

            //! Renders a bitmap of the specified glyph into the buffer specified by the `output`., where `out_w` and `out_h` is the width and height
            //! @details The rendered bitmap is a one-channel bitmap with each pixel take one byte. The value ranges in 0-255.
            //! 
            //! This call uses CPU to rasterize the glyph, since the glyph bitmap is usually very small, this usually does not cause performance issue,
            //! but you should save the render result whenever possible.
            //! @param[in] font_index The index of the font to query.
            //! @param[in] glyph The index of the glyph to query.
            //! @param[out] output The buffer to write rendered data to.
            //! @param[in] out_w The width of the buffer region to be used.
            //! @param[in] out_h The height of the buffer region to be used.
            //! @param[in] out_row_pitch The stride size, in bytes, of one row of data in the buffer.
            //! @param[in] scale_x The scale factor in horizontal direction.
            //! @param[in] scale_y The scale factor in vertical direction.
            //! @param[in] shift_x The value to shift bitmap bounding box in horizontal direction in pixels.
            //! @param[in] shift_y The value to shift bitmap bounding box in vertical direction in pixels.
            virtual void render_glyph_bitmap(u32 font_index, glyph_t glyph, void* output, i32 out_w, i32 out_h, i32 out_row_pitch,
                f32 scale_x, f32 scale_y, f32 shift_x, f32 shift_y) = 0;
        };

        //! Creates a font file object by parsing the provided TTF or TTC font file data.
        //! @details To load font file, you should first creates a blob that owns the file data (.ttf or .ttc), then you pass the 
        //! data to the font system to create a font object for it. The font data will be referenced by font data and should
        //! not be changed during the font lifetime.
        //! @param[in] data The data of the font file.
        //! @param[in] data_size The data size of the font file.
        //! @return Returns the created font file object which is initialized using the font data.
        LUNA_FONT_API R<Ref<IFontFile>> load_ttf_font_file(const byte_t* data, usize data_size);

        //! Gets the default font object, which is embedded into the SDK and only supports for ASCII codepoint range.
        //! @details Currently LunaSDK uses Open Sans Regular as the embedded default font.
        //! @return Returns the default font object.
        LUNA_FONT_API IFontFile* get_default_font();

        //! @}
    }

    struct Module;
    LUNA_FONT_API Module* module_font();
}