/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShapeRenderer.hpp
* @author JXMaster
* @date 2022/4/25
*/
#pragma once
#include "../ShapeRenderer.hpp"
#include <Luna/Runtime/TSAssert.hpp>

namespace Luna
{
    namespace VG
    {
        RV init_render_resources();
        void deinit_render_resources();

        struct DrawCommand
        {
            Ref<RHI::IBuffer> vertex_buffer;
            Ref<RHI::IBuffer> index_buffer;
            usize num_draw_calls;
            Float4x4U transform_matrix;
        };

        struct FillShapeRenderer : IShapeRenderer
        {
            lustruct("RHI::FillShapeRenderer", "{3E50DDB9-C896-4B87-9000-BA8E5C7632BE}");
            luiimpl();
            lutsassert_lock();

            Ref<RHI::ITexture> m_render_target;
            u32 m_screen_width;
            u32 m_screen_height;

            Ref<RHI::IPipelineState> m_fill_pso;
            RHI::Format m_rt_format;

            Vector<DrawCommand> m_draw_commands; // One per `draw()`.
            Vector<ShapeDrawCall> m_draw_calls;

            Vector<Ref<RHI::IDescriptorSet>> m_desc_sets;
            Ref<RHI::IBuffer> m_cbs_resource;
            usize m_cbs_capacity;

            FillShapeRenderer() :
                m_screen_width(0),
                m_screen_height(0),
                m_cbs_capacity(0),
                m_rt_format(RHI::Format::unknown) {}

            RV create_pso(RHI::Format rt_format);

            virtual RV begin(RHI::ITexture* render_target) override;
            virtual void draw(
                RHI::IBuffer* vertex_buffer,
                RHI::IBuffer* index_buffer,
                Span<const ShapeDrawCall> draw_calls,
                Float4x4U* transform_matrix
            ) override;
            virtual RV end() override;
            virtual void submit(RHI::ICommandBuffer* cmdbuf) override;
        };
    }
}