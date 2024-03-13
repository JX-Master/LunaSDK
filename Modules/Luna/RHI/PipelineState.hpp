/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.hpp
* @author JXMaster
* @date 2019/7/20
*/
#pragma once
#include "Texture.hpp"
#include "PipelineLayout.hpp"
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
namespace Luna
{
    namespace RHI
    {
        //! @addtogroup RHI
        //! @{

        //! Specify shader data format.
        enum class ShaderDataFormat : u8
        {
            //! No shader data. This must be set if @ref ShaderData::data is empty.
            none = 0,
            //! DirectX intermediate language format. Used only for Direct3D 12 format.
            dxil,
            //! SPIR-V format. Used only for Vulkan backend.
            spirv,
            //! Metal shading language source form. Used only for Metal backend.
            msl,
            //! Metal library. Used only for Metal backend.
            metallib
        };
        
        //! Specify one shader data.
        struct ShaderData
        {
            //! The shader data.
            Span<const byte_t> data;
            //! The shader entry point function name for Metal and Vulkan backend. 
            //! This is ignored in D3D12 backend.
            Name entry_point;
            //! The shader data format.
            ShaderDataFormat format = ShaderDataFormat::none;
        };
        
        //! Describes pipeline configurations of one compute pipeline.
        struct ComputePipelineStateDesc
        {
            //! The pipeline layout used with this pipeline state.
            //! @details This is used to check the compatibility between the pipeline layout and
            //! the pipeline state when creating the pipeline state object. Depends the 
            //! implementation, the pipeline state object may or may not stores one refernce
            //! to this pipeline layout. When binding pipeline layouts and pipeline states to
            //! pipelines, any pipeline layout that is compatible to this pipeline state can be used.
            IPipelineLayout* pipeline_layout = nullptr;
            //! The compute shader data.
            ShaderData cs;
            //! The number of threads in one thread group in X dimension for Metal backend.
            //! @details This is used only if the RHI backend is @ref BackendType::metal, since metal shader files
            //! does not include thread group size.
            u32 metal_numthreads_x;
            //! The number of threads in one thread group in Y dimension for Metal backend.
            //! @details This is used only if the RHI backend is @ref BackendType::metal, since metal shader files
            //! does not include thread group size.
            u32 metal_numthreads_y;
            //! The number of threads in one thread group in Z dimension for Metal backend.
            //! @details This is used only if the RHI backend is @ref BackendType::metal, since metal shader files
            //! does not include thread group size.
            u32 metal_numthreads_z;
        };

        //! The input rate for one input attribute (per vertex or per instance).
        enum class InputRate : u8
        {
            //! The input attribute is stepped once per vertex.
            per_vertex = 1,
            //! The input attribute is stepped once per instance.
            per_instance = 2
        };

        //! Describes one attribute in the input layout.
        struct InputAttributeDesc
        {
            //! The semantic name of this attribute. For exmaple, "COLOR", "TEXCOORD", etc.
            const c8* semantic_name;
            //! The semantic index of this attribute. Use this to differentiate attributes with the same 
            //! semantic name.
            u32 semantic_index;
            //! The location of this input attribute in the shader.
            u32 location;
            //! The belonging binding slot of this attribute.
            u32 binding_slot;
            //! The offset of this attribute from the beginning of the element.
            u32 offset;
            //! The format of this attribute.
            Format format;
            InputAttributeDesc() = default;
            InputAttributeDesc(
                const c8* semantic_name,
                u32 semantic_index,
                u32 location,
                u32 binding_slot,
                u32 offset,
                Format format
            ) :
                semantic_name(semantic_name),
                semantic_index(semantic_index),
                location(location),
                binding_slot(binding_slot),
                offset(offset),
                format(format) {}
        };
        //! Describes one input buffer binding.
        struct InputBindingDesc
        {
            //! The target binding slot. Every binding will take a different slot.
            u32 binding_slot;
            //! The size of one element in the buffer.
            u32 element_size;
            //! The element input rate of the binding.
            InputRate input_rate;
            InputBindingDesc() = default;
            InputBindingDesc(
                u32 binding_slot,
                u32 element_size,
                InputRate input_rate
            ) :
                binding_slot(binding_slot),
                element_size(element_size),
                input_rate(input_rate) {}
        };
        //! Describes the vertex input layout for the graphics pipeline.
        struct InputLayoutDesc
        {
            //! An array of vertex input binding descriptors, each of them describes one
            //! vertex buffer that will be bound to the pipeline.
            Span<const InputBindingDesc> bindings;
            //! An array of vertex input attributes.
            Span<const InputAttributeDesc> attributes;
            InputLayoutDesc() = default;
            InputLayoutDesc(
                Span<const InputBindingDesc> bindings,
                Span<const InputAttributeDesc> attributes
            ) : 
                bindings(bindings),
                attributes(attributes) {}
        };
        //! Specifies the blend factor used for blending.
        enum class BlendFactor : u8
        {
            //! Returns `0.0`.
            zero,
            //! Returns `1.0`.
            one,
            //! Returns `src.rgb`, which is the new generated color of the corresponding attachment.
            src_color,
            //! Returns `1.0 - src.rgb`.
            one_minus_src_color,
            //! Returns `src.a`, which is the new generated alpha of the corresponding attachment.
            src_alpha,
            //! Returns `1.0 - src.a`.
            one_minus_src_alpha,
            //! Returns `dst.rgb`.
            dst_color,
            //! Returns `1.0 - dst.rgb`.
            one_minus_dst_color,
            //! Returns `dst.alpha`.
            dst_alpha,
            //! Returns `1.0 - dst.alpha`.
            one_minus_dst_alpha,
            //! Returns `clamp(src.a, 0.0, 1.0)`.
            src_alpha_saturated,
            //! Returns `blend_factor`.
            blend_factor,
            //! Returns `1.0 - blend_factor`.
            one_minus_blend_factor,
            //! Returns `src1.rgb`, which is the new generated color of the first color attachment.
            src1_color,
            //! Returns `1.0 - src1.rgb`.
            one_minus_src1_color,
            //! Returns `src1.a`, which is the new generated alpha of the first color attachment.
            src1_alpha,
            //! Returns `1.0 - src1.a`.
            one_minus_src1_alpha
        };
        //! Specifies the blend operation.
        enum class BlendOp : u8
        {
            //! Returns `src_blend + dst_blend`.
            add,
            //! Returns `src_blend - dst_blend`.
            subtract,
            //! Returns `dst_blend - src_blend`.
            rev_subtract,
            //! Returns `min(src_blend, dst_blend)`.
            min,
            //! Returns `max(src_blend, dst_blend)`.
            max
        };
        //! Identifies which components of each pixel of a attachment are writable during blending.
        enum class ColorWriteMask : u8
        {
            none = 0x00,
            //! Allow data to be stored in the red component.
            red = 1,
            //! Allow data to be stored in the green component.
            green = 2,
            //! Allow data to be stored in the blue component.
            blue = 4,
            //! Allow data to be stored in the alpha component.
            alpha = 8,
            //! Allow data to be stored in all components.
            all = red | green | blue | alpha
        };
        //! Describes the blending configurations for one attachment.
        struct AttachmentBlendDesc
        {
            //! Whether to enable color blending for this attachment.
            //! If this is `false`, pixels outputted by the graphics pipeline will overwrite
            //! existing pixels directly if they are not discarded in pixel shader.
            bool blend_enable;
            //! The blend factor used for the source color.
            BlendFactor src_blend_color;
            //! The blend factor used for the destination color.
            BlendFactor dst_blend_color;
            //! The blend operation used for color blending.
            BlendOp blend_op_color;
            //! The blend factor used for the source alpha.
            BlendFactor src_blend_alpha;
            //! The blend factor used for the destination alpha.
            BlendFactor dst_blend_alpha;
            //! The blend operation used for alpha blending.
            BlendOp blend_op_alpha;
            //! The color components that can be modified during blending.
            ColorWriteMask color_write_mask;

            AttachmentBlendDesc(
                bool blend_enable = false,
                BlendFactor src_blend_color = BlendFactor::one,
                BlendFactor dst_blend_color = BlendFactor::zero,
                BlendOp blend_op_color = BlendOp::add,
                BlendFactor src_blend_alpha = BlendFactor::one,
                BlendFactor dst_blend_alpha = BlendFactor::zero,
                BlendOp blend_op_alpha = BlendOp::add,
                ColorWriteMask color_write_mask = ColorWriteMask::all
            ) :
                blend_enable(blend_enable),
                src_blend_color(src_blend_color),
                dst_blend_color(dst_blend_color),
                blend_op_color(blend_op_color),
                src_blend_alpha(src_blend_alpha),
                dst_blend_alpha(dst_blend_alpha),
                blend_op_alpha(blend_op_alpha),
                color_write_mask(color_write_mask) {}
        };

        //! Describes blending configurations for one graphics pipeline.
        struct BlendDesc
        {
            //! Whether to use the alpha value outputted from pixel shader
            //! as the coverage value to compute sample coverage mask
            //! in MSAA pipelines.
            bool alpha_to_coverage_enable;
            //! Whether to use independent blending configurations for every
            //! attachment.
            //! 
            //! If this is `false`, only the first element in `attachments` 
            //! will be used, and its values will be applied to all attachments.
            bool independent_blend_enable;
            //! The blending configurations for each attachment.
            AttachmentBlendDesc attachments[8];

            BlendDesc(
                InitializerList<AttachmentBlendDesc> attachments = {},
                bool alpha_to_coverage_enable = false,
                bool independent_blend_enable = false) :
                alpha_to_coverage_enable(alpha_to_coverage_enable),
                independent_blend_enable(independent_blend_enable)
            {
                u32 i = 0;
                for (auto& it : attachments)
                {
                    this->attachments[i] = it;
                    ++i;
                }
            }
        };

        //! Describes triangle fill mode of the rasterizer.
        enum class FillMode : u8
        {
            //! Only generate fragments for pixels near the border of the triangle.
            wireframe,
            //! Generate fragments for all pixels covered by the triangle.
            solid
        };

        //! Describes cull mode of the rasterizer.
        enum class CullMode : u8
        {
            //! Generate fragments for all pixels covered by triangles.
            none,
            //! Only generate fragments for pixels covered by front-facing triangles.
            front,
            //! Only generate fragments for pixels covered by back-facing triangles.
            back
        };
        
        //! Describes rasterizer configurations for one graphics pipeline.
        struct RasterizerDesc
        {
            //! The constant depth bias value added to the depth value generated by the vertex shader 
            //! in depth bias step.
            i32 depth_bias;
            //! The slope-scaled depth bias value added to the depth value generated by the vertex shader 
            //! in depth bias step.
            f32 slope_scaled_depth_bias;
            //! The bias value range ([`-depth_bias_clamp`, `depth_bias_clamp`]) allowed to be added to the depth value generated by the vertex shader 
            //! in depth bias step.
            f32 depth_bias_clamp;
            //! The fill mode of the rasterizer.
            FillMode fill_mode;
            //! The cull mode of the rasterizer.
            CullMode cull_mode;
            //! If this is `true`, triangles will be regarded as front-facing if its three vertices are wound 
            //! counter-clockwisly from view of camera.
            //! 
            //! If this is `false`, triangles will be regarded as front-facing if its three vertices are wound 
            //! clockwisly order from view of camera.
            bool front_counter_clockwise;
            //! Whether to discard fragments outside of the allowed depth range (0~1).
            //! If this is `false`, outside fragments will have their depth values clamped to [`0`, `1`].
            bool depth_clip_enable;

            RasterizerDesc(
                FillMode fill_mode = FillMode::solid,
                CullMode cull_mode = CullMode::back,
                i32 depth_bias = 0,
                f32 slope_scaled_depth_bias = 0.0f,
                f32 depth_bias_clamp = 0.0f,
                bool front_counter_clockwise = false,
                bool depth_clip_enable = true
            ) :
                 depth_bias(depth_bias),
                slope_scaled_depth_bias(slope_scaled_depth_bias),
                depth_bias_clamp(depth_bias_clamp),
                fill_mode(fill_mode),
                cull_mode(cull_mode),
                front_counter_clockwise(front_counter_clockwise),
                depth_clip_enable(depth_clip_enable) {}
        };

        //! Specifies the stencil operation to perform when stencil test passed or failed.
        enum class StencilOp : u8
        {
            //! Keeps the original stencil data.
            keep,
            //! Clears the stencil data to 0.
            zero,
            //! Replaces the stencil data with the data set by @ref ICommandBuffer::set_stencil_ref.
            replace,
            //! Increases the stencil data by one, and clamps the result so that it will not overflow and goes to 0.
            increment_saturated,
            //! Decreases the stencil data by one, and clamps the result so that it will not underflow and goes to the maximum representable value.
            decrement_saturated,
            //! Inverts the stencil data. This will invert every bit of the stencil data.
            invert,
            //! Increases the stencil data by one. If the data overflows, it will be reset to 0.
            increment,
            //! Decreases the stencil data by one. If the data underflows, it will be reset to the maximum representable value.
            decrement
        };

        //! Specifies the compare function used for comparison.
        enum class CompareFunction : u8
        {
            //! Never passes the comparison.
            never,
            //! Passes the comparison if A < B.
            less,
            //! Passes the comparison if A == B.
            equal,
            //! Passes the comparison if A <= B.
            less_equal,
            //! Passes the comparison if A > B.
            greater,
            //! Passes the comparison if A != B.
            not_equal,
            //! Passes the comparison if A >= B.
            greater_equal,
            //! Always passes the comparison.
            always
        };
        
        //! Describes the depth stencil operation.
        struct DepthStencilOpDesc
        {
            //! The operation to perform when stencil comparison failed.
            StencilOp stencil_fail_op;
            //! The operation to perform when stencil comparison passed, but depth test failed.
            StencilOp stencil_depth_fail_op;
            //! The operation to perform when both stencil comparison and depth test passed.
            StencilOp stencil_pass_op;
            //! The compare function used for stencil comparison.
            //! The comparison is performed between the stencil reference value set by @ref ICommandBuffer::set_stencil_ref
            //! and the stencil value in the stencil attachment.
            CompareFunction stencil_func;

            DepthStencilOpDesc(
                StencilOp stencil_fail_op = StencilOp::keep,
                StencilOp stencil_depth_fail_op = StencilOp::keep,
                StencilOp stencil_pass_op = StencilOp::keep,
                CompareFunction stencil_func = CompareFunction::always
            ) :
                stencil_fail_op(stencil_fail_op),
                stencil_depth_fail_op(stencil_depth_fail_op),
                stencil_pass_op(stencil_pass_op),
                stencil_func(stencil_func) {}
        };

        //! Describes depth stencil stage configurations of one graphics pipeline.
        struct DepthStencilDesc
        {
            //! Whether depth testing is enabled. If this is `false`, all pixels from pixel shader will pass 
            //! depth testing.
            bool depth_test_enable;
            //! Whether to write the pixel's depth value to depth buffer if one pixel passes depth testing.
            bool depth_write_enable;
            //! The compare function used for depth comparison in depth testing.
            CompareFunction depth_func;
            //! Whether stencil testing is enabled. If this is `false`, all pixels from pixel shader will pass 
            //! stencil testing.
            bool stencil_enable;
            //! The mask used to specify bits that will be loaded from stencil buffer for stencil testing.
            //! All bits that are not specified in the mask (with mask bit value 0) will be set to 0 before stencil testing 
            //! is performed.
            u8 stencil_read_mask;
            //! The mask used to specify bits that are allowed to be modified in stencil testing.
            u8 stencil_write_mask;
            //! The depth stencil operation performed for the front face of one triangle.
            DepthStencilOpDesc front_face;
            //! The depth stencil operation performed for the back face of one triangle.
            DepthStencilOpDesc back_face;

            DepthStencilDesc(
                bool depth_test_enable = true,
                bool depth_write_enable = true,
                CompareFunction depth_func = CompareFunction::less,
                bool stencil_enable = false,
                u8 stencil_read_mask = 0xff,
                u8 stencil_write_mask = 0xff,
                const DepthStencilOpDesc& front_face = DepthStencilOpDesc(),
                const DepthStencilOpDesc& back_face = DepthStencilOpDesc()
            ) :
                depth_test_enable(depth_test_enable),
                depth_write_enable(depth_write_enable),
                depth_func(depth_func),
                stencil_enable(stencil_enable),
                stencil_read_mask(stencil_read_mask),
                stencil_write_mask(stencil_write_mask),
                front_face(front_face),
                back_face(back_face) {}
        };

        //! The value used to finish an existing line or triangle strip and start a new one when 
        //! @ref GraphicsPipelineStateDesc::primitive_topology is set to @ref PrimitiveTopology::line_strip or
        //! @ref PrimitiveTopology::triangle_strip.
        enum class IndexBufferStripCutValue : u8
        {
            //! This should be set if @ref GraphicsPipelineStateDesc::primitive_topology is not 
            //! @ref PrimitiveTopology::line_strip or @ref PrimitiveTopology::triangle_strip.
            disabled,
            //! Use 0xFFFF as index buffer strip cut value.
            //! This can only be set if the index buffer format is @ref Format::r16_uint.
            value_0xffff,
            //! Use 0xFFFFFFFF as index buffer strip cut value.
            //! This can only be set if the index buffer format is @ref Format::r32_uint.
            value_0xffffffff
        };

        //! Specifies the primitive type for the graphics pipeline to draw.
        enum class PrimitiveTopology : u8
        {
            //! Draws point list, where every vertex in the vertex buffer specifies one point.
            point_list,
            //! Draws line list, where every two continious vertices in the vertex buffer specify
            //! two points of one line.
            line_list,
            //! Draws line strip, where every vertex and its prior vertex in the vertex buffer specify 
            //! two points of one line.
            //!  
            //! If any of the two vertices is not valid (out of valid vertex draw range or indexed by
            //! strip cut value), the current line will not be drawn.
            line_strip,
            //! Draws triangle list, where every three continious vertices in the vertex buffer specify
            //! three points of one triangle.
            triangle_list,
            //! Draws line strip, where every vertex and its prior two vertices in the vertex buffer 
            //! specify three points of one triangle.
            //!  
            //! If any of the three vertices is not valid (out of valid vertex draw range or indexed by
            //! strip cut value), the current triangle will not be drawn.
            triangle_strip,
        };

        //! Describes pipeline configurations of one compute pipeline.
        struct GraphicsPipelineStateDesc
        {
            //! The input layout configurations.
            InputLayoutDesc input_layout;
            //! The compatible pipeline layout configurations.
            IPipelineLayout* pipeline_layout = nullptr;
            //! The vertex shader data.
            ShaderData vs;
            //! The pixel shader data.
            ShaderData ps;
            //! The rasterizer configurations.
            RasterizerDesc rasterizer_state;
            //! The configurations of depth stencil stage.
            DepthStencilDesc depth_stencil_state;
            //! The configurations of blend stage.
            BlendDesc blend_state;
            //! The index buffer strip cut value. This must match the format of the index buffer, 
            //! see @ref IndexBufferStripCutValue for details.
            IndexBufferStripCutValue ib_strip_cut_value = IndexBufferStripCutValue::disabled;
            //! The primitive topology of primitives to be drawn.
            PrimitiveTopology primitive_topology = PrimitiveTopology::triangle_list;
            //! The number of attachments that can be set. This must be a value between [`1`, `8`].
            u8 num_color_attachments = 0;
            //! The color attachment formats.
            //! Only [`0`, `num_color_attachments`) elements in this array will be used, other 
            //! elements will be ignored.
            Format color_formats[8] = { Format::unknown };
            //! The depth stencil attachment format.
            //! This must be @ref Format::unknown if depth stencil attachment is not used.
            Format depth_stencil_format = Format::unknown;
            //! Specify the sample count. This must be `1` if MSAA is not used.
            u32 sample_count = 1;
        };

        //! @interface IPipelineState
        //! Represents one pipeline state object that stores pipeline configurations that can be 
        //! applied to one pipeline in one call.
        struct IPipelineState : virtual IDeviceChild
        {
            luiid("{A2AC1B03-5258-464E-9CA4-7497AFB7F443}");


        };

        //! @}
    }
}
