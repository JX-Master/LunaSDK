/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file RendererImpl.hpp
* @author JXMaster
* @date 2026/7/17
*/
#pragma once
#include "../Renderer.hpp"
#include <Luna/RHI/RHI.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/FontAtlas.hpp>
#include "RendererImpl.generated.hpp"

namespace Luna
{
    namespace GUICore
    {
        enum class RenderBatchType : u8
        {
            vg,
            shadow
        };

        struct RenderBatch
        {
            RenderBatchType type = RenderBatchType::vg;
            u32 first = 0;
            u32 count = 0;
        };

        struct ShadowCall
        {
            RectF draw_rect;
            RectF source_rect;
            RectF clip_rect;
            Float4U color = Float4U(0.0f);
            f32 radius = 0.0f;
            f32 source_radius = 0.0f;
            f32 softness = 0.0f;
            ShadowMode mode = ShadowMode::outer;
        };

        struct [[Luna::struct("{F97FD288-2649-497C-B235-BC39A76F73F5}")]] Renderer : IRenderer
        {
            luiimpl();

            Ref<RHI::IDevice> m_device;
            Ref<VG::IShapeDrawList> m_draw_list;
            Ref<VG::IShapeRenderer> m_shape_renderer;
            Ref<VG::IFontAtlas> m_font_atlas;
            Ref<RHI::IDescriptorSetLayout> m_shadow_descriptor_set_layout;
            Ref<RHI::IPipelineLayout> m_shadow_pipeline_layout;
            Ref<RHI::IPipelineState> m_shadow_pipeline_state;
            Ref<RHI::IBuffer> m_shadow_vertex_buffer;
            Ref<RHI::IBuffer> m_shadow_index_buffer;
            Ref<RHI::IBuffer> m_shadow_constant_buffer;
            Vector<Ref<RHI::IDescriptorSet>> m_shadow_descriptor_sets;
            Vector<RenderBatch> m_render_batches;
            Vector<ShadowCall> m_shadow_calls;
            RendererPerformanceCounters m_counters;
            RHI::Format m_render_target_format = RHI::Format::unknown;
            u32 m_render_target_width = 0;
            u32 m_render_target_height = 0;
            f32 m_screen_width = 0.0f;
            f32 m_screen_height = 0.0f;
            u32 m_shadow_constant_buffer_stride = 0;
            usize m_shadow_constant_buffer_capacity = 0;

            RV init(RHI::IDevice* device);
            RV create_shadow_pipeline(RHI::Format render_target_format);
            RV compile_draw_commands(IContext* context);
            RV prepare_shadow_resources();
            void render_shadow(RHI::ICommandBuffer* cmdbuf, u32 shadow_index);

            virtual RV prepare(IContext* context, RHI::ICommandBuffer* cmdbuf,
                RHI::ITexture* render_target) override;
            virtual void render(RHI::ICommandBuffer* cmdbuf) override;
            virtual RendererPerformanceCounters get_performance_counters() const override;
        };
    }
}
