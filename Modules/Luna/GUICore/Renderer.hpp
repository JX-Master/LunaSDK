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
#include "Renderer.generated.hpp"

namespace Luna
{
    namespace GUICore
    {
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
            //! Number of bytes uploaded for SDF instances, shape programs and color programs.
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
            //! Compiles one context's generated draw command stream and prepares referenced resources.
            //! @param[in] context The GUI Core context to render.
            //! @param[in] cmdbuf The command buffer that receives resource barriers required by renderer resources.
            //! @param[in] render_target The render target used to select the pipeline format and viewport dimensions.
            //! @return Returns success or failure code.
            //! @remark The command buffer must not be inside a render pass. The caller remains responsible for
            //! transitioning @p render_target and beginning a compatible render pass before calling @ref render.
            virtual RV prepare(IContext* context, RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) = 0;

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
