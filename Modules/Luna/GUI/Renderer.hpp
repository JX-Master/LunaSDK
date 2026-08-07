/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Renderer.hpp
* @author JXMaster
* @date 2026/7/17
*/
#pragma once
#include "Context.hpp"
#include <Luna/RHI/CommandBuffer.hpp>
#include <Luna/RHI/PipelineState.hpp>
#include <Luna/Runtime/Math/Matrix.hpp>
#include "Renderer.generated.hpp"

namespace Luna
{
    namespace GUI
    {
        //! Describes how one logical GUI surface is projected and rendered.
        struct RenderSurfaceDesc
        {
            //! Whether @ref surface_to_clip replaces the default orthographic projection.
            bool use_custom_transform = false;
            //! Matrix that transforms `(x, y, 0, 1)` positions from GUI surface coordinates to clip space.
            Float4x4U surface_to_clip;
            //! Whether scene depth testing is enabled.
            bool depth_test_enable = false;
            //! Whether passing GUI fragments write depth.
            //! @remark World-space transparent GUI normally leaves this disabled.
            bool depth_write_enable = false;
            //! Depth comparison function used when @ref depth_test_enable is `true`.
            RHI::CompareFunction depth_compare_function = RHI::CompareFunction::less_equal;
            //! Triangle culling used by the SDF and integrated VG pipelines.
            RHI::CullMode cull_mode = RHI::CullMode::none;

            //! Constructs a screen-space surface description with depth testing disabled.
            RenderSurfaceDesc() :
                surface_to_clip(Float4x4::identity()) {}
        };

        //! Describes the application-owned attachments used by one GUI render recording.
        struct RenderTargetDesc
        {
            //! Required final GUI color texture.
            RHI::ITexture* color_texture = nullptr;
            //! Load operation used by the first GUI-owned color pass.
            RHI::LoadOp color_load_op = RHI::LoadOp::load;
            //! Clear value used when @ref color_load_op is @ref RHI::LoadOp::clear.
            Float4U color_clear_value = Float4U(0.0f);
            //! Color texture state guaranteed when @ref IRenderer::render returns.
            //! @remark @ref RHI::TextureStateFlag::automatic is not a valid final state.
            RHI::TextureStateFlag color_final_state = RHI::TextureStateFlag::shader_read_ps;
            //! Optional scene depth-stencil texture.
            RHI::ITexture* depth_stencil_texture = nullptr;
            //! Depth load operation used by the first GUI-owned pass.
            RHI::LoadOp depth_load_op = RHI::LoadOp::load;
            //! Depth store operation used by the final GUI-owned pass.
            RHI::StoreOp depth_store_op = RHI::StoreOp::store;
            //! Clear value used when @ref depth_load_op is @ref RHI::LoadOp::clear.
            f32 depth_clear_value = 1.0f;
            //! Stencil load operation used by the first GUI-owned pass.
            RHI::LoadOp stencil_load_op = RHI::LoadOp::load;
            //! Stencil store operation used by the final GUI-owned pass.
            RHI::StoreOp stencil_store_op = RHI::StoreOp::store;
            //! Clear value used when @ref stencil_load_op is @ref RHI::LoadOp::clear.
            u8 stencil_clear_value = 0;
            //! Depth-stencil texture state guaranteed when @ref IRenderer::render returns.
            //! @remark This field is ignored when @ref depth_stencil_texture is `nullptr`.
            RHI::TextureStateFlag depth_stencil_final_state =
                RHI::TextureStateFlag::depth_stencil_attachment_read;

            //! Constructs an empty render-target description.
            RenderTargetDesc() = default;
            //! Constructs a color-only render-target description.
            //! @param[in] color_texture The final GUI color texture.
            explicit RenderTargetDesc(RHI::ITexture* color_texture) :
                color_texture(color_texture) {}
        };

        //! Runtime counters collected by one GUI renderer.
        struct RendererPerformanceCounters
        {
            //! CPU time spent compiling and recording the latest GUI render plan, in milliseconds.
            f64 render_ms = 0.0;
            //! Number of VG draw calls produced by the latest compilation.
            u32 vg_draw_call_count = 0;
            //! Number of instanced SDF draw calls produced by the latest compilation.
            u32 sdf_draw_call_count = 0;
            //! Number of SDF shape instances produced by the latest compilation.
            u32 sdf_instance_count = 0;
            //! Number of distinct SDF raster states referenced by the latest compilation.
            u32 sdf_state_count = 0;
            //! Number of scalar floats uploaded for SDF shape programs.
            u32 sdf_shape_float_count = 0;
            //! Number of scalar floats uploaded for SDF color programs.
            u32 sdf_color_float_count = 0;
            //! Number of shape and color structured-buffer pages referenced by the latest compilation.
            u32 sdf_program_page_count = 0;
            //! Number of distinct shape-page and color-page descriptor pairs referenced by SDF batches.
            u32 sdf_page_pair_count = 0;
            //! Number of transitions between VG and SDF batches required by painter order.
            u32 backend_switch_count = 0;
            //! Number of bytes uploaded for SDF instance data.
            usize sdf_instance_upload_bytes = 0;
            //! Number of bytes uploaded for deduplicated SDF raster states.
            usize sdf_state_upload_bytes = 0;
            //! Number of bytes uploaded for SDF instances, states, shape programs and color programs.
            usize sdf_upload_bytes = 0;
            //! Number of contiguous renderer batches needed to preserve painter order.
            u32 render_batch_count = 0;
            //! Number of live backdrop captures materialized by the latest rendering.
            u32 backdrop_capture_count = 0;
            //! Number of graphics passes recorded by the latest rendering.
            u32 render_pass_count = 0;
            //! Number of pixels processed across backdrop filtering dispatches.
            u64 backdrop_filtered_pixel_count = 0;
            //! Number of compute dispatches used for backdrop filtering.
            u32 backdrop_blur_dispatch_count = 0;
            //! Approximate bytes occupied by live backdrop filtering textures.
            usize backdrop_temporary_texture_bytes = 0;
        };

        //! @interface IRenderer
        //! Compiles GUI primitive draw commands and records their RHI rendering work.
        //! @remark The renderer uses its SDF interpreter for analytic GUI geometry, gradients and shadows. VG remains
        //! the backend for text, images, lines and complex monochrome paths.
        struct [[Luna::interface("{780B8352-8D7E-4401-8C12-77D06818B841}")]] IRenderer : virtual Interface
        {
            //! Records one context using the default orthographic screen-space surface.
            //! @param[in] context The GUI context to render.
            //! @param[in] cmdbuf The recording graphics command buffer that receives the complete GUI render plan.
            //! @param[in] target The required color and optional depth-stencil attachments.
            //! @return Returns success or failure code.
            RV render(IContext* context, RHI::ICommandBuffer* cmdbuf, const RenderTargetDesc& target)
            {
                return render(context, cmdbuf, target, RenderSurfaceDesc());
            }

            //! Compiles and records one context's complete GUI render plan.
            //! @param[in] context The GUI context to render.
            //! @param[in] cmdbuf The recording graphics command buffer that receives all resource barriers and passes.
            //! @param[in] target The required color and optional depth-stencil attachments.
            //! @param[in] surface The projection, depth and culling configuration of the logical GUI surface.
            //! @return Returns success or failure code.
            //! @remark The command buffer must not be inside any pass when this call begins or returns. GUI records
            //! every attachment transition and render/compute pass, but does not submit, wait or reset the command buffer.
            //! @remark Layout, clips and input positions remain in logical surface coordinates even when a custom
            //! transform projects the surface into three-dimensional clip space.
            virtual RV render(IContext* context, RHI::ICommandBuffer* cmdbuf, const RenderTargetDesc& target,
                const RenderSurfaceDesc& surface) = 0;

            //! Gets counters for the latest recorded frame.
            //! @return Returns the latest renderer performance counters.
            virtual RendererPerformanceCounters get_performance_counters() const = 0;
        };

        //! Creates one GUI renderer.
        //! @param[in] device The RHI device used to allocate rendering resources. Passing `nullptr` uses the main device.
        //! @return Returns the created renderer, or an error code if required resources cannot be created.
        LUNA_GUI_API R<Ref<IRenderer>> new_renderer(RHI::IDevice* device = nullptr);
    }
}
