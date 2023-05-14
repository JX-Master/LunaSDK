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
#include "ShaderInputLayout.hpp"
#include <Runtime/Span.hpp>
namespace Luna
{
	namespace RHI
	{
		struct ComputePipelineStateDesc
		{
			IShaderInputLayout* shader_input_layout = nullptr;
			Span<const byte_t> cs;
		};

		enum class InputRate : u8
		{
			per_vertex = 1,
			per_instance = 2
		};

		constexpr u32 APPEND_ALIGNED_ELEMENT = 0xffffffff;

		struct InputPropertyDesc
		{
			//! The semantic name of this property.
			const c8* semantic_name;
			//! The semantic index of this property.
			u32 semantic_index;
			//! The index of the instance in `InputLayoutDesc::instances` this property belongs to.
			u32 instance_index;
			//! The location of this input property in the shader.
			u32 location;
			//! The offset of this property from the beginning of the belonging instance.
			u32 offset;
			//! The format of this property.
			Format format;

			InputPropertyDesc(
				const c8* semantic_name,
				u32 semantic_index,
				u32 instance_index,
				u32 location,
				u32 offset,
				Format format
			) :
				semantic_name(semantic_name),
				semantic_index(semantic_index),
				instance_index(instance_index),
				location(location),
				offset(offset),
				format(format) {}
		};

		//! Describes one vertex buffer bound to the pipeline. 
		struct InputInstanceDesc
		{
			u32 binding_slot;
			u32 instance_size;
			InputRate input_rate;

			InputInstanceDesc(
				u32 binding_slot,
				u32 instance_size,
				InputRate input_rate
			) :
				binding_slot(binding_slot),
				instance_size(instance_size),
				input_rate(input_rate) {}
		};

		struct InputLayoutDesc
		{
			Span<const InputInstanceDesc> input_instances;
			Span<const InputPropertyDesc> input_properties;

			InputLayoutDesc() = default;
			InputLayoutDesc(
				InitializerList<const InputInstanceDesc> input_instances,
				InitializerList<const InputPropertyDesc> input_properties
			) : 
				input_instances(input_instances),
				input_properties(input_properties) {}
		};

		enum class BlendFactor : u8
		{
			zero,
			one,
			src_color,
			inv_src_color,
			src_alpha,
			inv_src_alpha,
			dest_color,
			inv_dest_color,
			dest_alpha,
			inv_dest_alpha,
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
		enum class LogicOp : u8
		{
			clear,
			set,
			copy,
			copy_inverted,
			invert,
			and,
			nand,
			or ,
			nor,
			xor,
			equiv,
			and_reverse,
			and_inverted,
			or_reverse,
			or_inverted
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
			BlendFactor dest_blend;
			BlendOp blend_op;
			BlendFactor src_blend_alpha;
			BlendFactor dest_blend_alpha;
			BlendOp blend_op_alpha;
			ColorWriteMask render_target_write_mask;

			AttachmentBlendDesc(
				bool blend_enable = false,
				BlendFactor src_blend = BlendFactor::one,
				BlendFactor dest_blend = BlendFactor::zero,
				BlendOp blend_op = BlendOp::add,
				BlendFactor src_blend_alpha = BlendFactor::one,
				BlendFactor dest_blend_alpha = BlendFactor::zero,
				BlendOp blend_op_alpha = BlendOp::add,
				ColorWriteMask render_target_write_mask = ColorWriteMask::all
			) :
				blend_enable(blend_enable),
				src_blend(src_blend),
				dest_blend(dest_blend),
				blend_op(blend_op),
				src_blend_alpha(src_blend_alpha),
				dest_blend_alpha(dest_blend_alpha),
				blend_op_alpha(blend_op_alpha),
				render_target_write_mask(render_target_write_mask) {}
		};

		struct BlendDesc
		{
			bool alpha_to_coverage_enable;
			bool logic_op_enable;
			LogicOp logic_op;
			AttachmentBlendDesc rt[8];

			BlendDesc(
				std::initializer_list<AttachmentBlendDesc> rt = {},
				bool alpha_to_coverage_enable = false,
				bool logic_op_enable = false,
				LogicOp logic_op = LogicOp::clear) :
				alpha_to_coverage_enable(alpha_to_coverage_enable),
				logic_op_enable(logic_op_enable),
				logic_op(logic_op)
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
			IShaderInputLayout* shader_input_layout = nullptr;
			Span<const byte_t> vs;
			Span<const byte_t> ps;
			RasterizerDesc rasterizer_state;
			DepthStencilDesc depth_stencil_state;
			BlendDesc blend_state;
			IndexBufferStripCutValue ib_strip_cut_value = IndexBufferStripCutValue::disabled;
			PrimitiveTopology primitive_topology = PrimitiveTopology::triangle_list;
			u8 num_render_targets = 0;
			//! The pixel format of the render target.
			Format rtv_formats[8] = { Format::unknown };
			Format dsv_format = Format::unknown;
			//! Specify the sample count, 1 if MSAA is not used.
			u32 sample_count = 1;
			u32 sample_mask = 0xFFFFFFFF;
			u32 sample_quality = 0;
		};

		//! @interface IPipelineState
		//! @threadsafe
		struct IPipelineState : virtual IDeviceChild
		{
			luiid("{A2AC1B03-5258-464E-9CA4-7497AFB7F443}");


		};
	}
}