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
            sdf
        };

        struct RenderBatch
        {
            RenderBatchType type = RenderBatchType::vg;
            u32 first = 0;
            u32 count = 0;
            u32 resource_index = 0;
        };

        struct SDFInstance
        {
            Float4U draw_rect;
            Float2U evaluation_origin;
            UInt4U program_data;
        };

        struct SDFState
        {
            Float4U clip_rect;
            Float4U rounded_clip_rect;
            Float4U rounded_clip_radii;
        };

        struct SDFProgramPage
        {
            Ref<RHI::IBuffer> buffer;
            u32 num_floats = 0;
        };

        struct SDFPagePair
        {
            u32 shape_page = 0;
            u32 color_page = 0;
            Ref<RHI::IDescriptorSet> descriptor_set;
        };

        struct [[Luna::struct("{F97FD288-2649-497C-B235-BC39A76F73F5}")]] Renderer : IRenderer
        {
            luiimpl();

            Ref<RHI::IDevice> m_device;
            Ref<VG::IShapeDrawList> m_draw_list;
            Ref<VG::IShapeRenderer> m_shape_renderer;
            Ref<VG::IFontAtlas> m_font_atlas;
            Ref<RHI::IDescriptorSetLayout> m_sdf_descriptor_set_layout;
            Ref<RHI::IPipelineLayout> m_sdf_pipeline_layout;
            Ref<RHI::IPipelineState> m_sdf_pipeline_state;
            Ref<RHI::IBuffer> m_sdf_vertex_buffer;
            Ref<RHI::IBuffer> m_sdf_index_buffer;
            Ref<RHI::IBuffer> m_sdf_frame_buffer;
            Ref<RHI::IBuffer> m_sdf_instance_buffer;
            Ref<RHI::IBuffer> m_sdf_state_buffer;
            Vector<RenderBatch> m_render_batches;
            Vector<SDFInstance> m_sdf_instances;
            Vector<SDFState> m_sdf_states;
            Vector<f32> m_compiled_sdf_shape_floats;
            Vector<f32> m_compiled_sdf_color_floats;
            Vector<SDFProgramPage> m_sdf_shape_pages;
            Vector<SDFProgramPage> m_sdf_color_pages;
            Vector<SDFPagePair> m_sdf_page_pairs;
            RendererPerformanceCounters m_counters;
            RHI::Format m_render_target_format = RHI::Format::unknown;
            u32 m_render_target_width = 0;
            u32 m_render_target_height = 0;
            f32 m_screen_width = 0.0f;
            f32 m_screen_height = 0.0f;
            usize m_sdf_instance_capacity = 0;
            usize m_sdf_state_capacity = 0;

            RV init(RHI::IDevice* device);
            RV create_sdf_pipeline(RHI::Format render_target_format);
            RV compile_draw_commands(IContext* context);
            RV prepare_sdf_resources();
            void render_sdf(RHI::ICommandBuffer* cmdbuf, const RenderBatch& batch);

            virtual RV prepare(IContext* context, RHI::ICommandBuffer* cmdbuf,
                RHI::ITexture* render_target) override;
            virtual void render(RHI::ICommandBuffer* cmdbuf) override;
            virtual RendererPerformanceCounters get_performance_counters() const override;
        };
    }
}
