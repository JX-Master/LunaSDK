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
#include <Luna/Runtime/Interface.hpp>
#include <Luna/VG/FontAtlas.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/RHI/Texture.hpp>
#include <Luna/RHI/DescriptorSet.hpp>
#include <Luna/VG/ShapeDrawList.hpp>

#ifndef LUNA_GUI_API
#define LUNA_GUI_API
#endif

namespace Luna
{
    namespace GUI
    {
        //! The state required to issue one draw call.
        struct DrawListState
        {
            VG::IShapeBuffer* shape_buffer = nullptr;
            RHI::ITexture* texture = nullptr;
            RHI::SamplerDesc sampler = RHI::SamplerDesc(RHI::Filter::linear, RHI::Filter::linear, RHI::Filter::linear,
                    RHI::TextureAddressMode::repeat,
                    RHI::TextureAddressMode::repeat,
                    RHI::TextureAddressMode::repeat);
            RectF clip_rect = RectF(0, 0, 0, 0);

            bool operator==(const DrawListState& rhs) const
            {
                return shape_buffer == rhs.shape_buffer && texture == rhs.texture && 
                    sampler == rhs.sampler && clip_rect == rhs.clip_rect;
            }
            bool operator!=(const DrawListState& rhs) const
            {
                return !(*this == rhs);
            }
        };

        //! @interface IDrawList
        //! Used to batch GUI draw calls based on layers.
        struct IDrawList : virtual Interface
        {
            luiid("eea861f3-ea7b-4a44-9d20-a94713eb7113");

            virtual void begin(VG::IShapeDrawList* draw_list) = 0;

            virtual DrawListState get_state() = 0;

            virtual u32 push_state(DrawListState* state, bool allow_merge = true) = 0;

            virtual void pop_state(u32 pop_id) = 0;

            virtual VG::IShapeBuffer* get_shape_buffer() = 0;

            virtual void add_shape_raw(Span<const VG::Vertex> vertices, Span<const u32> indices) = 0;

            virtual void add_shape(u32 begin_command, u32 num_commands, 
                const Float2U& min_position, const Float2U& max_position,
                const Float2U& min_shapecoord, const Float2U& max_shapecoord,
                const Float4U& color,
                const Float2U& min_texcoord = Float2U(0.0f), const Float2U& max_texcoord = Float2U(0.0f)) = 0;

            virtual void end() = 0;
        };

        LUNA_GUI_API Ref<IDrawList> new_draw_list();
    }
}