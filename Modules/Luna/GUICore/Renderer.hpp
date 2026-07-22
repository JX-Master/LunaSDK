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
    namespace GUICore
    {
        //! Describes how one logical GUI surface is projected and rendered.
        struct RenderSurfaceDesc
        {
            //! Whether @ref surface_to_clip replaces the default orthographic projection.
            bool use_custom_transform = false;
            //! Matrix that transforms `(x, y, 0, 1)` positions from GUI surface coordinates to clip space.
            Float4x4U surface_to_clip;
            //! Depth-stencil attachment format of the caller-owned render pass, or @ref RHI::Format::unknown
            //! when the pass has no depth-stencil attachment.
            RHI::Format depth_stencil_format = RHI::Format::unknown;
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

        //! Runtime counters collected by one GUI Core renderer.
        struct RendererPerformanceCounters
        {
            //! Time spent compiling draw commands and preparing renderer resources for the latest frame, in milliseconds.
            f64 prepare_ms = 0.0;
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
        };

        //! @interface IRenderer
        //! Compiles GUI Core primitive draw commands and records their RHI rendering work.
        //! @remark The renderer uses its SDF interpreter for analytic GUI geometry, gradients and shadows. VG remains
        //! the backend for text, images, lines and complex monochrome paths.
        struct [[Luna::interface("{780B8352-8D7E-4401-8C12-77D06818B841}")]] IRenderer : virtual Interface
        {
            //! Compiles one context using the default orthographic screen-space surface.
            //! @param[in] context The GUI Core context to render.
            //! @param[in] cmdbuf The command buffer that receives resource barriers required by renderer resources.
            //! @param[in] render_target The render target used to select the pipeline format and viewport dimensions.
            //! @return Returns success or failure code.
            RV prepare(IContext* context, RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target)
            {
                return prepare(context, cmdbuf, render_target, RenderSurfaceDesc());
            }

            //! Compiles one context's generated draw command stream and prepares referenced resources.
            //! @param[in] context The GUI Core context to render.
            //! @param[in] cmdbuf The command buffer that receives resource barriers required by renderer resources.
            //! @param[in] render_target The render target used to select the pipeline format and viewport dimensions.
            //! @return Returns success or failure code.
            //! @remark The command buffer must not be inside a render pass. The caller remains responsible for
            //! transitioning @p render_target and beginning a compatible render pass before calling @ref render.
            //! @param[in] surface The projection, depth and culling configuration of the logical GUI surface.
            //! @remark Layout, clips and input positions remain in logical surface coordinates even when a custom
            //! transform projects the surface into three-dimensional clip space.
            virtual RV prepare(IContext* context, RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target,
                const RenderSurfaceDesc& surface) = 0;

            //! Records the prepared GUI draw calls into an active render pass.
            //! @param[in] cmdbuf The command buffer whose active render pass receives the draw calls.
            //! @remark Call @ref prepare before this function. This function only binds pipelines, descriptors,
            //! viewports and buffers and submits draw calls. It does not begin or end a render pass.
            virtual void render(RHI::ICommandBuffer* cmdbuf) = 0;

            //! Gets counters for the latest prepared frame.
            //! @return Returns the latest renderer performance counters.
            virtual RendererPerformanceCounters get_performance_counters() const = 0;
        };

        //! Creates one GUI Core renderer.
        //! @param[in] device The RHI device used to allocate rendering resources. Passing `nullptr` uses the main device.
        //! @return Returns the created renderer, or an error code if required resources cannot be created.
        LUNA_GUICORE_API R<Ref<IRenderer>> new_renderer(RHI::IDevice* device = nullptr);
    }
}
