/*!
* This file is a portion of LunaSDK.
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
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
#include "ShapeBuffer.hpp"
#include "ShapeDrawList.generated.hpp"

#ifndef LUNA_VG_API
#define LUNA_VG_API
#endif

namespace Luna
{
    namespace VG
    {
        //! @addtogroup VG
        //! @{
        
        //! Describes one quad instance to be drawn by the shape renderer.
        struct ShapeInstance
        {
            //! The minimum and maximum draw positions encoded as
            //! `{min_x, min_y, max_x, max_y}`.
            Float4U position_bounds;
            //! The minimum and maximum shape coordinates encoded as
            //! `{min_x, min_y, max_x, max_y}`.
            Float4U shapecoord_bounds;
            //! The minimum and maximum texture coordinates encoded as
            //! `{min_x, min_y, max_x, max_y}`.
            Float4U texcoord_bounds;
            //! The shape command range and state index encoded as
            //! `{begin_command, num_commands, state_index, 0}`.
            UInt4U command_range_and_state;
            //! An additional color used to tint the shape.
            Float4U color;
        };

        //! Describes one reusable shape rendering state stored in the state buffer.
        struct ShapeState
        {
            //! The local transform matrix applied to the shape.
            Float4x4U transform;
            //! The clip rectangle encoded as `{offset_x, offset_y, width, height}`.
            //! All zeroes disable rectangular clipping.
            Float4U clip_rect;
            //! The rounded clip rectangle encoded as `{offset_x, offset_y, width, height}`.
            //! All zeroes disable rounded clipping.
            Float4U rounded_clip_rect;
            //! Corner radii of @ref rounded_clip_rect in top-left, top-right, bottom-right and bottom-left order.
            Float4U rounded_clip_radii;
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
            //! The first shape instance to draw for this draw call.
            u32 base_instance;
            //! The number of shape instances to draw for this draw call.
            u32 num_instances;
        };

        //! @interface IShapeDrawList
        //! Represents a draw list that contains shapes to be drawn.
        struct [[Luna::interface("{14F1CA71-7B2D-4072-A2EE-DFD64B62FCD5}")]] IShapeDrawList : virtual Interface
        {
            //! Gets the bounded RHI device.
            //! @return Returns the bounded RHI device.
            virtual RHI::IDevice* get_device() = 0;

            //! Resets the draw list. The call clears all shapes recorded, but retains their memory
            //! and resources, so they can be reused for new shapes.
            virtual void reset() = 0;

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
            virtual void set_shape_buffer(IShapeBuffer* shape_buffer) = 0;

            //! Gets the current set shape buffer. See remarks of @ref set_shape_buffer for details.
            //! @return Returns the current set shape buffer.
            virtual IShapeBuffer* get_shape_buffer() = 0;

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

            //! Sets the transform matrix for the following draw calls.
            //! @details The initial transform matrix is @ref Float4x4::identity when the draw list has been reset.
            //! @param[in] transform The transform matrix to set.
            virtual void set_transform(const Float4x4U& transform) = 0;

            //! Gets the transform matrix for the following draw calls.
            //! @return Returns the transform matrix for the following draw calls.
            virtual Float4x4U get_transform() = 0;

            //! Sets the clip rectangle for the following draw calls.
            //! @param[in] clip_rect The clip rectangle to set.
            //! Set clip rectangle to {0, 0, 0, 0} will disable the clip rectangle.
            virtual void set_clip_rect(const RectF& clip_rect) = 0;

            //! Gets the clip rectangle for the following draw calls.
            //! @return Returns the set clip rectangle.
            virtual RectF get_clip_rect() = 0;

            //! Sets an additional rounded clip rectangle for the following draw calls.
            //! @param[in] clip_rect The rounded clip rectangle to set. A zero-sized rectangle disables rounded clipping.
            //! @param[in] corner_radii Corner radii in top-left, top-right, bottom-right and bottom-left order.
            //! @remark The rounded clip is intersected with the ordinary clip rectangle set by @ref set_clip_rect.
            virtual void set_rounded_clip_rect(const RectF& clip_rect, const Float4U& corner_radii) = 0;

            //! Gets the rounded clip rectangle for the following draw calls.
            //! @return Returns the set rounded clip rectangle.
            virtual RectF get_rounded_clip_rect() = 0;

            //! Gets the rounded clip corner radii for the following draw calls.
            //! @return Returns corner radii in top-left, top-right, bottom-right and bottom-left order.
            virtual Float4U get_rounded_clip_radii() = 0;

            //! Forces geometry recorded after this call into a new draw call.
            //! @remark This is useful when another renderer must insert commands between two ranges of VG geometry
            //! without changing the current VG draw state. The call has no effect until more geometry is recorded.
            virtual void draw_call_barrier() = 0;

            //! Draws one shape. The shape is drawn by adding one draw rect (two triangles) to the list.
            //! @param[in] begin_command The index of the first command point of the glyph to draw in shape buffer.
            //! @param[in] num_commands The number of command points of the glyph to draw.
            //! @param[in] min_position The minimum position of the bounding rect of the shape.
            //! @param[in] max_position The maximum position of the bounding rect of the shape.
            //! @param[in] min_shapecoord The shape coordinate value that maps to the minimum position of the bounding rect of the shape.
            //! @param[in] max_shapecoord The shape coordinate value that maps to the maximum position of the bounding rect of the shape.
            //! @param[in] color The floating-point RGBA color used to tint the shape.
            //! @param[in] min_texcoord The texture coordinate value that maps to the minimum position of the bounding rect of the shape.
            //! @param[in] max_texcoord The texture coordinate value that maps to the maximum position of the bounding rect of the shape.
            virtual void draw_shape(u32 begin_command, u32 num_commands,
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord,
                const Float4U& color = Color::white(),
                const Float2U& min_texcoord = Float2U(0.0f), const Float2U& max_texcoord = Float2U(0.0f)
                ) = 0;

            //! Builds render resources and draw calls that can be used for drawing glyphs.
            virtual RV compile() = 0;

            //! Gets the compiled instance buffer used for rendering shapes in this draw list.
            //! @return Returns the compiled instance buffer.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape draw commands 
            //! take effect.
            virtual RHI::IBuffer* get_instance_buffer() = 0;

            //! Gets the number of instances in the buffer returned by @ref get_instance_buffer.
            //! @return Returns the number of compiled shape instances.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape draw commands 
            //! take effect.
            virtual u32 get_instance_buffer_size() = 0;

            //! Gets the compiled state buffer used by shape instances in this draw list.
            //! @return Returns the compiled state buffer.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape states
            //! take effect.
            virtual RHI::IBuffer* get_state_buffer() = 0;

            //! Gets the number of unique states in the buffer returned by @ref get_state_buffer.
            //! @return Returns the number of compiled unique shape states.
            //! @par Valid Usage
            //! * This function must be called after calling @ref compile in order to let new shape states
            //! take effect.
            virtual u32 get_state_buffer_size() = 0;

            //! Gets an array of draw calls that should be invoked to draw glyphs in this draw list.
            //! @param[out] out_draw_calls Returns the compiled draw calls. Elements will be pushed to the end of the vector,
            //! and existing elements will not be modified.
            virtual Span<const ShapeDrawCall> get_draw_calls() = 0;
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
