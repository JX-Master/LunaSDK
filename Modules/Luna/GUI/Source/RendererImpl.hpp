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
    namespace GUI
    {
        enum class RenderBatchType : u8
        {
            vg,
            sdf,
            backdrop_capture
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

        struct BackdropBlurParams
        {
            UInt2U destination_size;
            Float2U source_uv_origin;
            Float2U source_uv_size;
            Float2U sample_step;
            f32 gaussian_center_weight = 1.0f;
            u32 gaussian_pair_count = 0;
            u32 filter_mode = 0;
            u32 padding = 0;
            Float4U gaussian_pair_0 = Float4U(0.0f);
            Float4U gaussian_pair_1 = Float4U(0.0f);
            Float4U gaussian_pair_2 = Float4U(0.0f);
            Float4U gaussian_pair_3 = Float4U(0.0f);
            Float4U gaussian_pair_4 = Float4U(0.0f);
            Float4U gaussian_pair_5 = Float4U(0.0f);
            Float4U gaussian_pair_6 = Float4U(0.0f);
            Float4U gaussian_pair_7 = Float4U(0.0f);
            Float4U gaussian_pair_8 = Float4U(0.0f);
            Float4U gaussian_pair_9 = Float4U(0.0f);
            Float4U gaussian_pair_10 = Float4U(0.0f);
            Float4U gaussian_pair_11 = Float4U(0.0f);
            Float4U gaussian_pair_12 = Float4U(0.0f);
        };

        inline constexpr u32 MAX_BACKDROP_DOWNSAMPLE_PASSES = 4;
        inline constexpr u32 MAX_BACKDROP_GAUSSIAN_PAIRS = 13;
        inline constexpr f32 MAX_BACKDROP_WORKING_SIGMA = 10.0f;
        inline constexpr f32 BACKDROP_GAUSSIAN_SUPPORT = 2.5f;
        inline constexpr f32 BACKDROP_GAUSSIAN_SIGMA_QUANTIZATION = 0.5f;
        inline constexpr f32 BACKDROP_GAUSSIAN_FAST_PATH_SIGMA = 0.5f;
        inline constexpr u32 MAX_BACKDROP_FILTER_PASSES =
            MAX_BACKDROP_DOWNSAMPLE_PASSES + 2;
        static_assert(sizeof(Float4U) == 16);
        static_assert(offsetof(BackdropBlurParams, gaussian_pair_0) == 48);
        static_assert(offsetof(BackdropBlurParams, gaussian_pair_12) == 240);
        static_assert(sizeof(BackdropBlurParams) == 256,
            "BackdropBlurParams must match the compute shader constant-buffer layout.");

        struct BackdropFilterPass
        {
            RHI::ITexture* source = nullptr;
            RHI::ITexture* destination = nullptr;
            u32 source_mip = 0;
            u32 destination_mip = 0;
            UInt2U destination_size;
        };

        struct BackdropCapture
        {
            u32 element = INVALID_ELEMENT;
            BackdropBlurCaptureDesc desc;
            RectF consumer_bounds;
            RectF source_rect;
            bool used = false;
            u32 downsample_pass_count = 0;
            u32 filter_pass_count = 0;
            bool horizontal_blur = false;
            bool vertical_blur = false;
            Ref<RHI::ITexture> downsample_texture;
            Ref<RHI::ITexture> blur_textures[2];
            Ref<RHI::IBuffer> parameter_buffers[MAX_BACKDROP_FILTER_PASSES];
            Ref<RHI::IDescriptorSet> descriptor_sets[MAX_BACKDROP_FILTER_PASSES];
            BackdropFilterPass filter_passes[MAX_BACKDROP_FILTER_PASSES];
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
            Ref<RHI::IDescriptorSetLayout> m_backdrop_descriptor_set_layout;
            Ref<RHI::IPipelineLayout> m_backdrop_pipeline_layout;
            Ref<RHI::IPipelineState> m_backdrop_pipeline_state;
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
            Vector<BackdropCapture> m_backdrop_captures;
            u32 m_num_backdrop_captures = 0;
            RendererPerformanceCounters m_counters;
            RHI::Format m_render_target_format = RHI::Format::unknown;
            RHI::Format m_depth_stencil_format = RHI::Format::unknown;
            RHI::CompareFunction m_depth_compare_function = RHI::CompareFunction::less_equal;
            RHI::CullMode m_cull_mode = RHI::CullMode::none;
            bool m_depth_test_enable = false;
            bool m_depth_write_enable = false;
            u32 m_render_target_width = 0;
            u32 m_render_target_height = 0;
            f32 m_logical_width = 0.0f;
            f32 m_logical_height = 0.0f;
            Float4x4U m_surface_to_clip = Float4x4::identity();
            usize m_sdf_instance_capacity = 0;
            usize m_sdf_state_capacity = 0;

            RV init(RHI::IDevice* device);
            RV create_sdf_pipeline(RHI::Format render_target_format, RHI::Format depth_stencil_format,
                const RenderSurfaceDesc& surface);
            RV compile_draw_commands(IContext* context, RHI::ITexture* render_target);
            RV prepare_backdrop_capture(BackdropCapture& capture, RHI::ITexture* render_target);
            RV prepare_sdf_resources();
            void render_sdf(RHI::ICommandBuffer* cmdbuf, const RenderBatch& batch);
            void submit_batch(RHI::ICommandBuffer* cmdbuf, const RenderBatch& batch);
            void record_backdrop_capture(RHI::ICommandBuffer* cmdbuf,
                const BackdropCapture& capture, RHI::ITexture* render_target);

            virtual RV render(IContext* context, RHI::ICommandBuffer* cmdbuf,
                const RenderTargetDesc& target, const RenderSurfaceDesc& surface) override;
            virtual RendererPerformanceCounters get_performance_counters() const override;
        };
    }
}
