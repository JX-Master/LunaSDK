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
#include "Resource.hpp"
#include "PipelineLayout.hpp"
#include <Luna/Runtime/Span.hpp>
namespace Luna
{
	namespace RHI
	{
		struct ComputePipelineStateDesc
		{
			IPipelineLayout* pipeline_layout = nullptr;
			Span<const byte_t> cs;
		};

		enum class InputRate : u8
		{
			per_vertex = 1,
			per_instance = 2
		};

		constexpr u32 APPEND_ALIGNED_ELEMENT = 0xffffffff;

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

		struct InputLayoutDesc
		{
			Span<const InputBindingDesc> bindings;
			Span<const InputAttributeDesc> attributes;
			InputLayoutDesc() = default;
			InputLayoutDesc(
				InitializerList<InputBindingDesc> bindings,
				InitializerList<InputAttributeDesc> attributes
			) : 
				bindings(bindings),
				attributes(attributes) {}
		};

		enum class BlendFactor : u8
		{
			zero,
			one,
			src_color,
			inv_src_color,
			src_alpha,
			inv_src_alpha,
			dst_color,
			inv_dst_color,
			dst_alpha,
			inv_dst_alpha,
			src_alpha_sat,
			blend_factor,
			inv_blend_factor,
			src1_color,
			inv_src1_color,
			src1_alpha,
			inv_src1_alpha
		};
		enum class BlendOp : u8
		{
			add,
			subtract,
			rev_subtract,
			min,
			max
		};
		enum class ColorWriteMask : u8
		{
			none = 0x00,
			red = 1,
			green = 2,
			blue = 4,
			alpha = 8,
			all = red | green | blue | alpha
		};

		struct AttachmentBlendDesc
		{
			bool blend_enable;
			BlendFactor src_blend;
			BlendFactor dst_blend;
			BlendOp blend_op;
			BlendFactor src_blend_alpha;
			BlendFactor dst_blend_alpha;
			BlendOp blend_op_alpha;
			ColorWriteMask render_target_write_mask;

			AttachmentBlendDesc(
				bool blend_enable = false,
				BlendFactor src_blend = BlendFactor::one,
				BlendFactor dst_blend = BlendFactor::zero,
				BlendOp blend_op = BlendOp::add,
				BlendFactor src_blend_alpha = BlendFactor::one,
				BlendFactor dst_blend_alpha = BlendFactor::zero,
				BlendOp blend_op_alpha = BlendOp::add,
				ColorWriteMask render_target_write_mask = ColorWriteMask::all
			) :
				blend_enable(blend_enable),
				src_blend(src_blend),
				dst_blend(dst_blend),
				blend_op(blend_op),
				src_blend_alpha(src_blend_alpha),
				dst_blend_alpha(dst_blend_alpha),
				blend_op_alpha(blend_op_alpha),
				render_target_write_mask(render_target_write_mask) {}
		};

		struct BlendDesc
		{
			bool alpha_to_coverage_enable;
			bool independent_blend_enable;
			AttachmentBlendDesc rt[8];

			BlendDesc(
				InitializerList<AttachmentBlendDesc> rt = {},
				bool alpha_to_coverage_enable = false,
				bool independent_blend_enable = false) :
				alpha_to_coverage_enable(alpha_to_coverage_enable),
				independent_blend_enable(independent_blend_enable)
			{
				u32 i = 0;
				for (auto& it : rt)
				{
					this->rt[i] = it;
					++i;
				}
			}
		};

		enum class FillMode : u8
		{
			wireframe,
			solid
		};

		enum class CullMode : u8
		{
			none,
			front,
			back
		};

		struct RasterizerDesc
		{
			FillMode fill_mode;
			CullMode cull_mode;
			i32 depth_bias;
			f32 depth_bias_clamp;
			f32 slope_scaled_depth_bias;
			u32 forced_sample_count;
			bool front_counter_clockwise;
			bool depth_clip_enable;
			bool multisample_enable;
			bool antialiased_line_enable;
			bool conservative_raster_enabled;

			RasterizerDesc(
				FillMode fill_mode = FillMode::solid,
				CullMode cull_mode = CullMode::back,
				i32 depth_bias = 0,
				f32 depth_bias_clamp = 0.0f,
				f32 slope_scaled_depth_bias = 0.0f,
				u32 forced_sample_count = 0,
				bool front_counter_clockwise = false,
				bool depth_clip_enable = true,
				bool multisample_enable = false,
				bool antialiased_line_enable = false,
				bool conservative_raster_enabled = false
			) :
				fill_mode(fill_mode),
				cull_mode(cull_mode),
				depth_bias(depth_bias),
				depth_bias_clamp(depth_bias_clamp),
				slope_scaled_depth_bias(slope_scaled_depth_bias),
				forced_sample_count(forced_sample_count),
				front_counter_clockwise(front_counter_clockwise),
				depth_clip_enable(depth_clip_enable),
				multisample_enable(multisample_enable),
				antialiased_line_enable(antialiased_line_enable),
				conservative_raster_enabled(conservative_raster_enabled) {}
		};

		enum class StencilOp : u8
		{
			keep,
			zero,
			replace,
			incr_sat,
			decr_sat,
			invert,
			incr,
			decr
		};

		enum class ComparisonFunc : u8
		{
			never,	// Never pass comparison
			less,
			equal,
			less_equal,
			greater,
			not_equal,
			greater_equal,
			always	// Always pass comparison
		};

		struct DepthStencilOpDesc
		{
			StencilOp stencil_fail_op;
			StencilOp stencil_depth_fail_op;
			StencilOp stencil_pass_op;
			ComparisonFunc stencil_func;

			DepthStencilOpDesc(
				StencilOp stencil_fail_op = StencilOp::keep,
				StencilOp stencil_depth_fail_op = StencilOp::keep,
				StencilOp stencil_pass_op = StencilOp::keep,
				ComparisonFunc stencil_func = ComparisonFunc::always
			) :
				stencil_fail_op(stencil_fail_op),
				stencil_depth_fail_op(stencil_depth_fail_op),
				stencil_pass_op(stencil_pass_op),
				stencil_func(stencil_func) {}
		};

		constexpr u8 DEFAULT_STENCIL_READ_MASK = 0xff;
		constexpr u8 DEFAULT_STENCIL_WRITE_MASK = 0xff;

		struct DepthStencilDesc
		{
			bool depth_test_enable;
			bool depth_write_enable;
			ComparisonFunc depth_func;
			bool stencil_enable;
			u8 stencil_read_mask;
			u8 stencil_write_mask;
			DepthStencilOpDesc front_face;
			DepthStencilOpDesc back_face;

			DepthStencilDesc(
				bool depth_test_enable = true,
				bool depth_write_enable = true,
				ComparisonFunc depth_func = ComparisonFunc::less,
				bool stencil_enable = false,
				u8 stencil_read_mask = DEFAULT_STENCIL_READ_MASK,
				u8 stencil_write_mask = DEFAULT_STENCIL_WRITE_MASK,
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

		enum class IndexBufferStripCutValue : u8
		{
			disabled,
			//! This should be set if the index type is `Format::r16_uint`.
			value_0xffff,
			//! This should be set if the index type is `Format::r32_uint`.
			value_0xffffffff
		};

		enum class PrimitiveTopology : u8
		{
			point_list,
			line_list,
			line_strip,
			triangle_list,
			triangle_strip,
		};

		struct GraphicsPipelineStateDesc
		{
			InputLayoutDesc input_layout;
			IPipelineLayout* pipeline_layout = nullptr;
			Span<const byte_t> vs;
			Span<const byte_t> ps;
			RasterizerDesc rasterizer_state;
			DepthStencilDesc depth_stencil_state;
			BlendDesc blend_state;
			IndexBufferStripCutValue ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			PrimitiveTopology primitive_topology = PrimitiveTopology::triangle_list;
			u8 num_color_attachments = 0;
			//! The pixel format of the render target.
			Format color_formats[8] = { Format::unknown };
			Format depth_stencil_format = Format::unknown;
			//! Specify the sample count, 1 if MSAA is not used.
			u32 sample_count = 1;
			u32 sample_mask = 0xFFFFFFFF;
		};

		//! @interface IPipelineState
		//! @threadsafe
		struct IPipelineState : virtual IDeviceChild
		{
			luiid("{A2AC1B03-5258-464E-9CA4-7497AFB7F443}");


		};
	}
}