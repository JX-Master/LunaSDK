/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeDrawList.hpp
* @author JXMaster
* @date 2023/9/27
*/
#pragma once
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/RHI/DescriptorSet.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! @addtogroup VG
        //! @{
        
        //! Describes one vertex to be drawn for the glyph.
        struct Vertex
        {
            //! The position of the vertex.
            Float2U position;
            //! The shape coordinate of the vertex for mapping the shape
            //! commands.
            Float2U shapecoord;
            //! The texture coordinate of the vertex for sampling
            //! the attached resources.
            Float2U texcoord;
            //! An additional color that can be used to tint the vertex.
            u32 color;
            //! The offset of the first command for this shape in the shape buffer.
            u32 begin_command;
            //! The number of commands (f32 values) used for this shape.
            u32 num_commands;
        };

        //! Describes one shape draw call.
        struct ShapeDrawCall
        {
            //! The shape buffer bind to this draw call.
            Ref<RHI::IBuffer> shape_buffer;
            //! The texture bind to this draw call. May be `nullptr`.
            Ref<RHI::ITexture> texture;
            //! The attached sampler for this draw call.
            RHI::SamplerDesc sampler;
            //! The fist index to draw for this draw call.
            u32 base_index;
            //! The number of indices to draw for this draw call.
            u32 num_indices;
            //! The origin point for this draw call.
            Float2U origin_point;
            //! The rotation for this draw call.
            f32 rotation;
        };

        //! @interface IShapeDrawList
        //! Represents a draw list that contains shapes to be drawn.
        struct IShapeDrawList : virtual Interface
        {
            luiid("{14F1CA71-7B2D-4072-A2EE-DFD64B62FCD5}");

            //! Resets the draw list. The call clears all shapes recorded, but retains their memory
            //! and resources, so they can be reused for new shapes.
            virtual void reset() = 0;

            //! Gets shape points that are recorded in internal shape buffer.
            //! @return Returns one vector that contains the recorded shape points.
            //! The user may add points to this vector manually.
            virtual Vector<f32>& get_shape_points() = 0;

            //! Sets the shape buffer used for the following draw calls.
            //! @param[in] shape_buffer The shape buffer to set. If this is `nullptr`, the internal shape buffer will be set.
            //! @remark The shape buffer is similar to "glyph atlas texture" in VG. However, instead of storing bitmaps of glyphs 
            //! like traditional atlas texture, the shape buffer stores command points that describe contours of glyphs. Command points 
            //! are stored as an array of 32-bit floating-point values and can be read by GPU shader to generate high-resolution graphics 
            //! by performing scanline testing against contours directly.
            //! 
            //! One shape buffer may contain contours of multiple glyphs, each glyph takes one continuous range of command points in 
            //! the shape buffer. The shape buffer can be pre-generated and bind to one shape draw list directly by calling @ref set_shape_buffer.
            //! This is useful if the user wish to draw static glyphs like font characters, since such glyphs never change during run-time, the 
            //! user can pack all needed glyphs to one shape buffer, and use that shape buffer to draw glyphs directly. If the user does not want 
            //! to create shape buffer herself, she can also pass `nullptr` to @ref set_shape_buffer to use the shape draw list's internal shape 
            //! buffer. The internal shape buffer is designed to draw contours that may change every frame, like the GUI widget that are generated 
            //! at runtime, and the data of the internal shape buffer will be cleared every time @ref reset is called.
            virtual void set_shape_buffer(RHI::IBuffer* shape_buffer) = 0;

            //! Gets the current set shape buffer. See remarks of @ref set_shape_buffer for details.
            //! @return Returns the current set shape buffer. Returns `nullptr` if the internal shape buffer is used.
            virtual RHI::IBuffer* get_shape_buffer() = 0;

            //! Sets the texture to be sampled when rendering the succeeding shapes.
            //! @param[in] tex The texture to set. Specify `nullptr` is allowed, which behaves the same as applying one white texture with all components set to
            //! 1.0f.
            //! @remark The draw list only stores the texture and its state as-is and provides it to the renderer 
            //! when the draw list is processed by the renderer. It does not do any validation to the texture and 
            //! its states. It is the user and renderer's responsibility to validate the texture and its state.
            //! 
            //! The draw list has texture being set to `nullptr` after reset.
            virtual void set_texture(RHI::ITexture* tex) = 0;

            //! Gets the currently set texture.
            //! @return Returns the currently set texture, returns `nullptr` if no texture is set.
            virtual RHI::ITexture* get_texture() = 0;

            //! Sets the sampler state to be used when sampling bound textures.
            //! @param[in] desc The sampler state descriptor. Specify `nullptr` to reset the sampler state to initial settings.
            virtual void set_sampler(const RHI::SamplerDesc* desc) = 0;

            //! Gets the sampler state currently set.
            //! @return Returns the currently set sampler state.
            virtual RHI::SamplerDesc get_sampler() = 0;

            //! Sets the origin point for the following draw calls.
            //! @details The origin point is relative to the origin point of the canvas. 
            //! The origin point of the canvas is at the bottom-left corner. The x axis points to right and the y axis points to up.
            //! 
            //! The initial origin point is (0,0) when the draw list has been reset.
            //! @param[in] origin The origin point position to set.
            virtual void set_origin(const Float2& origin) = 0;

            //! Gets the origin point for the following draw calls.
            //! @return Returns the origin point for the following draw calls.
            virtual Float2 get_origin() = 0;

            //! Sets the rotation for the following draw calls.
            //! @details The rotation is relative to the set origin point.
            //! @param[in] degrees The rotation to set in clockwise degrees.
            virtual void set_rotation(f32 degrees) = 0;

            //! Gets the rotation for the following draw calls.
            //! @return Returns the rotation for the following draw calls in clockwise degrees.
            virtual f32 get_rotation() = 0;

            //! Draws one shape by submitting vertices and indices directly.
            //! @param[in] vertices The draw vertices.
            //! @param[in] indices The draw indices. Valid index range is [`0`, `vertices.size()`).
            virtual void draw_shape_raw(Span<const Vertex> vertices, Span<const u32> indices) = 0;

            //! Draws one shape. The shape is drawn by adding one draw rect (two triangles) to the list.
            //! @param[in] begin_command The index of the first command point of the glyph to draw in shape buffer.
            //! @param[in] num_commands The number of command points of the glyph to draw.
            //! @param[in] min_position The minimum position of the bounding rect of the shape.
            //! @param[in] max_position The maximum position of the bounding rect of the shape.
            //! @param[in] min_shapecoord The shape coordinate value that maps to the minimum position of the bounding rect of the shape.
            //! @param[in] max_shapecoord The shape coordinate value that maps to the maximum position of the bounding rect of the shape.
            //! @param[in] color The color to tint the shape in RGBA8 form.
            //! @param[in] min_texcoord The texture coordinate value that maps to the minimum position of the bounding rect of the shape.
            //! @param[in] max_shapecoord The texture coordinate value that maps to the maximum position of the bounding rect of the shape.
            virtual void draw_shape(u32 begin_command, u32 num_commands,
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord,
                u32 color = 0xFFFFFFFF,
                const Float2U& min_texcoord = Float2U(0.0f), const Float2U& max_texcoord = Float2U(0.0f)
                ) = 0;

            //! Builds render resources and draw calls that can be used for drawing glyphs.
            virtual RV compile() = 0;

            //! Gets the compiled vertex buffer used for rendering glyphs in this draw list.
            //! @return Returns the compiled vertex buffer.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape draw commands 
            //! take effect.
            virtual RHI::IBuffer* get_vertex_buffer() = 0;

            //! Gets the number of vertices in the vertex buffer returned by @ref get_vertex_buffer.
            //! @return Returns the number of vertices in the vertex buffer.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape draw commands 
            //! take effect.
            virtual u32 get_vertex_buffer_size() = 0;

            //! Gets the compiled index buffer used for rendering glyphs in this draw list.
            //! @return Returns the compiled index buffer.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape draw commands 
            //! take effect.
            virtual RHI::IBuffer* get_index_buffer() = 0;

            //! Gets the number of indices in the index buffer returned by @ref get_index_buffer.
            //! @return Returns the number of indices in the index buffer.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape draw commands 
            //! take effect.
            virtual u32 get_index_buffer_size() = 0;

            //! Gets an array of draw calls that should be invoked to draw glyphs in this draw list.
            //! @param[out] out_draw_calls Returns the compiled draw calls. Elements will be pushed to the end of the vector,
            //! and existing elements will not be modified.
            virtual void get_draw_calls(Vector<ShapeDrawCall>& out_draw_calls) = 0;
        };

        //! Creates a new shape draw list.
        //! @param[in] device The device used to render to the draw list. This is used to create 
        //! RHI buffers used by the draw list.
        //! 
        //! If this is `nullptr`, the main device (device fetched from @ref RHI::get_main_device) will be used.
        //! @return Returns the created shape draw list.
        LUNA_VG_API Ref<IShapeDrawList> new_shape_draw_list(RHI::IDevice* device = nullptr);

        //! @}
    }
}