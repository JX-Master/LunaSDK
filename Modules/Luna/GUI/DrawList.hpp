/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DrawList.hpp
* @author JXMaster
* @date 2024/7/13
*/
#pragma once
#include "Base.hpp"
#include <Luna/RHI/DescriptorSet.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include "DrawList.generated.hpp"

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! The state required to issue one draw call.
        struct DrawListState
        {
            //! The VG shape buffer to append geometry to.
            VG::IShapeBuffer* shape_buffer = nullptr;
            //! Optional texture sampled by the draw call.
            RHI::ITexture* texture = nullptr;
            //! Sampler state used when @ref texture is not `nullptr`.
            RHI::SamplerDesc sampler = RHI::SamplerDesc(RHI::Filter::linear, RHI::Filter::linear, RHI::Filter::linear,
                    RHI::TextureAddressMode::repeat,
                    RHI::TextureAddressMode::repeat,
                    RHI::TextureAddressMode::repeat);
            //! Clip rectangle for this draw state.
            RectF clip_rect = RectF(0, 0, 0, 0);

            //! Compares two draw list states.
            //! @param[in] rhs The state to compare with.
            //! @return Returns `true` if both states are equal.
            bool operator==(const DrawListState& rhs) const
            {
                return shape_buffer == rhs.shape_buffer && texture == rhs.texture &&
                    sampler == rhs.sampler && clip_rect == rhs.clip_rect;
            }
            //! Compares two draw list states.
            //! @param[in] rhs The state to compare with.
            //! @return Returns `true` if the states are different.
            bool operator!=(const DrawListState& rhs) const
            {
                return !(*this == rhs);
            }
        };

        //! @interface IDrawList
        //! Used to batch GUI draw calls based on layers.
        struct [[Luna::interface("eea861f3-ea7b-4a44-9d20-a94713eb7113")]] IDrawList : virtual Interface
        {
            //! Begins recording GUI drawing into a VG shape draw list.
            //! @param[in] draw_list The target VG draw list.
            virtual void begin(VG::IShapeDrawList* draw_list) = 0;

            //! Gets the current draw state.
            //! @return Returns the current state used by subsequent shape commands.
            virtual DrawListState get_state() = 0;

            //! Pushes a draw state and starts a new batch if necessary.
            //! @param[in] state The state to push.
            //! @param[in] allow_merge Whether compatible adjacent states may be merged.
            //! @return Returns a token that must be passed to @ref pop_state.
            virtual u32 push_state(DrawListState* state, bool allow_merge = true) = 0;

            //! Pops a draw state pushed by @ref push_state.
            //! @param[in] pop_id The token returned by @ref push_state.
            virtual void pop_state(u32 pop_id) = 0;

            //! Gets the current VG shape buffer.
            //! @return Returns the shape buffer used by subsequent shape commands.
            virtual VG::IShapeBuffer* get_shape_buffer() = 0;

            //! Appends raw vertices and indices to the current shape buffer.
            //! @param[in] vertices The vertices to append.
            //! @param[in] indices The indices to append.
            virtual void add_shape_raw(Span<const VG::Vertex> vertices, Span<const u32> indices) = 0;

            //! Appends a VG shape command range as one GUI draw shape.
            //! @param[in] begin_command The first command in the shape buffer.
            //! @param[in] num_commands The number of commands to append.
            //! @param[in] min_position The minimum shape position.
            //! @param[in] max_position The maximum shape position.
            //! @param[in] min_shapecoord The minimum shape coordinate.
            //! @param[in] max_shapecoord The maximum shape coordinate.
            //! @param[in] color The shape color multiplier.
            //! @param[in] min_texcoord The minimum texture coordinate.
            //! @param[in] max_texcoord The maximum texture coordinate.
            virtual void add_shape(u32 begin_command, u32 num_commands,
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord,
                const Float4U& color,
                const Float2U& min_texcoord = Float2U(0.0f), const Float2U& max_texcoord = Float2U(0.0f)) = 0;

            //! Finishes recording and submits accumulated batches to the underlying VG draw list.
            virtual void end() = 0;
        };

        //! Creates a GUI draw list object.
        //! @return Returns the created draw list.
        LUNA_GUI_API Ref<IDrawList> new_draw_list();

        //! @}
    }
}
