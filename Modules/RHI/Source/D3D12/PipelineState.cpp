/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file PipelineState.cpp
* @author JXMaster
* @date 2019/8/15
*/
#include "PipelineState.hpp"

#ifdef LUNA_RHI_D3D12

#include "D3D12Common.hpp"

#include <Runtime/Vector.hpp>

namespace Luna
{
	namespace RHI
	{
		D3D12_BLEND encode_blend_factor(BlendFactor f)
		{
			switch (f)
			{
			case BlendFactor::zero:
				return D3D12_BLEND_ZERO;
			case BlendFactor::one:
				return D3D12_BLEND_ONE;
			case BlendFactor::src_color:
				return D3D12_BLEND_SRC_COLOR;
			case BlendFactor::inv_src_color:
				return D3D12_BLEND_INV_SRC1_COLOR;
			case BlendFactor::src_alpha:
				return D3D12_BLEND_SRC_ALPHA;
			case BlendFactor::inv_src_alpha:
				return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendFactor::dest_alpha:
				return D3D12_BLEND_DEST_ALPHA;
			case BlendFactor::inv_dest_alpha:
				return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendFactor::dest_color:
				return D3D12_BLEND_DEST_COLOR;
			case BlendFactor::inv_dest_color:
				return D3D12_BLEND_INV_DEST_COLOR;
			case BlendFactor::src_alpha_sat:
				return D3D12_BLEND_SRC_ALPHA_SAT;
			case BlendFactor::blend_factor:
				return D3D12_BLEND_BLEND_FACTOR;
			case BlendFactor::inv_blend_factor:
				return D3D12_BLEND_INV_BLEND_FACTOR;
			case BlendFactor::src1_color:
				return D3D12_BLEND_SRC1_COLOR;
			case BlendFactor::inv_src1_color:
				return D3D12_BLEND_INV_SRC1_COLOR;
			case BlendFactor::src1_alpha:
				return D3D12_BLEND_SRC1_ALPHA;
			case BlendFactor::inv_src1_alpha:
			default:
				lupanic();
				return D3D12_BLEND_ONE;
			}
		}

		D3D12_BLEND_OP encode_blend_op(BlendOp o)
		{
			switch (o)
			{
			case BlendOp::add:
				return D3D12_BLEND_OP_ADD;
			case BlendOp::subtract:
				return D3D12_BLEND_OP_SUBTRACT;
			case BlendOp::rev_subtract:
				return D3D12_BLEND_OP_REV_SUBTRACT;
			case BlendOp::min:
				return D3D12_BLEND_OP_MIN;
			case BlendOp::max:
			default:
				lupanic();
				return D3D12_BLEND_OP_MAX;
			}
		}

		D3D12_LOGIC_OP encode_logic_op(LogicOp o)
		{
			switch (o)
			{
			case LogicOp::clear:
				return D3D12_LOGIC_OP_CLEAR;
			case LogicOp::set:
				return D3D12_LOGIC_OP_SET;
			case LogicOp::copy:
				return D3D12_LOGIC_OP_COPY;
			case LogicOp::copy_inverted:
				return D3D12_LOGIC_OP_COPY_INVERTED;
			case LogicOp::noop:
				return D3D12_LOGIC_OP_NOOP;
			case LogicOp::invert:
				return D3D12_LOGIC_OP_INVERT;
			case LogicOp::and :
				return D3D12_LOGIC_OP_AND;
			case LogicOp::nand:
				return D3D12_LOGIC_OP_NAND;
			case LogicOp:: or :
				return D3D12_LOGIC_OP_OR;
			case LogicOp::nor:
				return D3D12_LOGIC_OP_NOR;
			case LogicOp::xor :
				return D3D12_LOGIC_OP_XOR;
			case LogicOp::equiv:
				return D3D12_LOGIC_OP_EQUIV;
			case LogicOp::and_reverse:
				return D3D12_LOGIC_OP_AND_REVERSE;
			case LogicOp::and_inverted:
				return D3D12_LOGIC_OP_AND_INVERTED;
			case LogicOp::or_reverse:
				return D3D12_LOGIC_OP_OR_REVERSE;
			case LogicOp::or_inverted:
			default:
				lupanic();
				return D3D12_LOGIC_OP_OR_INVERTED;
			}
		}

		D3D12_STENCIL_OP encode_stencil_op(StencilOp op)
		{
			switch (op)
			{
			case StencilOp::keep:
				return D3D12_STENCIL_OP_KEEP;
			case StencilOp::zero:
				return D3D12_STENCIL_OP_ZERO;
			case StencilOp::replace:
				return D3D12_STENCIL_OP_REPLACE;
			case StencilOp::incr_sat:
				return D3D12_STENCIL_OP_INCR_SAT;
			case StencilOp::decr_sat:
				return D3D12_STENCIL_OP_DECR;
			case StencilOp::invert:
				return D3D12_STENCIL_OP_INVERT;
			case StencilOp::incr:
				return D3D12_STENCIL_OP_INCR;
			case StencilOp::decr:
			default:
				lupanic();
				return D3D12_STENCIL_OP_KEEP;
			}
		}

		inline void fill_shader_data(D3D12_SHADER_BYTECODE& dest, const Span<const byte_t>& src)
		{
			dest.BytecodeLength = src.size();
			dest.pShaderBytecode = src.data();
		}

		bool PipelineState::init_graphic(const GraphicsPipelineStateDesc& desc)
		{
			m_is_graphics = true;
			ShaderInputLayout* slayout = static_cast<ShaderInputLayout*>(desc.shader_input_layout->get_object());
			D3D12_GRAPHICS_PIPELINE_STATE_DESC d;
			d.pRootSignature = slayout->m_rs.Get();

			fill_shader_data(d.VS, desc.vs);
			fill_shader_data(d.PS, desc.ps);
			fill_shader_data(d.DS, desc.ds);
			fill_shader_data(d.HS, desc.hs);
			fill_shader_data(d.GS, desc.gs);

			Vector<D3D12_SO_DECLARATION_ENTRY> stream_entries;
			d.StreamOutput.NumEntries = (UINT)desc.stream_output.entries.size();
			d.StreamOutput.NumStrides = (UINT)desc.stream_output.buffer_strides.size();
			d.StreamOutput.pBufferStrides = desc.stream_output.buffer_strides.data();
			d.StreamOutput.RasterizedStream = desc.stream_output.rasterized_stream;

			if (!desc.stream_output.entries.empty())
			{
				stream_entries.resize(desc.stream_output.entries.size());
				d.StreamOutput.pSODeclaration = stream_entries.data();
				for (usize i = 0; i < stream_entries.size(); ++i)
				{
					D3D12_SO_DECLARATION_ENTRY& dst = stream_entries[i];
					const StreamOutputDeclarationEntry& src = desc.stream_output.entries[i];
					dst.ComponentCount = src.component_count;
					dst.OutputSlot = src.output_slot;
					dst.SemanticIndex = src.semantic_index;
					dst.SemanticName = src.semantic_name.c_str();
					dst.StartComponent = src.start_component;
					dst.Stream = src.stream;
				}
			}
			else
			{
				d.StreamOutput.pSODeclaration = nullptr;
			}

			{
				d.BlendState.AlphaToCoverageEnable = desc.blend_state.alpha_to_coverage_enable ? TRUE : FALSE;
				d.BlendState.IndependentBlendEnable = desc.blend_state.independent_blend_enable ? TRUE : FALSE;
				for (u32 i = 0; i < 8; ++i)
				{
					D3D12_RENDER_TARGET_BLEND_DESC& rt = d.BlendState.RenderTarget[i];
					const RenderTargetBlendDesc& srt = desc.blend_state.rt[i];
					rt.BlendEnable = srt.blend_enable ? TRUE : FALSE;
					rt.LogicOpEnable = srt.logic_op_enable ? TRUE : FALSE;
					rt.SrcBlend = encode_blend_factor(srt.src_blend);
					rt.DestBlend = encode_blend_factor(srt.dest_blend);
					rt.BlendOp = encode_blend_op(srt.blend_op);
					rt.SrcBlendAlpha = encode_blend_factor(srt.src_blend_alpha);
					rt.DestBlendAlpha = encode_blend_factor(srt.dest_blend_alpha);
					rt.BlendOpAlpha = encode_blend_op(srt.blend_op_alpha);
					rt.LogicOp = encode_logic_op(srt.logic_op);
					rt.RenderTargetWriteMask = 0;
					if ((srt.render_target_write_mask & ColorWriteMask::red) != ColorWriteMask::none)
					{
						rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_RED;
					}
					if ((srt.render_target_write_mask & ColorWriteMask::green) != ColorWriteMask::none)
					{
						rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_GREEN;
					}
					if ((srt.render_target_write_mask & ColorWriteMask::blue) != ColorWriteMask::none)
					{
						rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_BLUE;
					}
					if ((srt.render_target_write_mask & ColorWriteMask::alpha) != ColorWriteMask::none)
					{
						rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_ALPHA;
					}
				}
				d.SampleMask = desc.sample_mask;
			}

			{
				switch (desc.rasterizer_state.fill_mode)
				{
				case FillMode::solid:
					d.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
					break;
				case FillMode::wireframe:
					d.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
					break;
				}
				switch (desc.rasterizer_state.cull_mode)
				{
				case CullMode::back:
					d.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
					break;
				case CullMode::front:
					d.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
					break;
				case CullMode::none:
					d.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
					break;
				}
				d.RasterizerState.ForcedSampleCount = desc.rasterizer_state.forced_sample_count;
				d.RasterizerState.FrontCounterClockwise = desc.rasterizer_state.front_counter_clockwise ? TRUE : FALSE;
				d.RasterizerState.DepthBias = desc.rasterizer_state.depth_bias;
				d.RasterizerState.DepthBiasClamp = desc.rasterizer_state.depth_bias_clamp;
				d.RasterizerState.SlopeScaledDepthBias = desc.rasterizer_state.slope_scaled_depth_bias;
				d.RasterizerState.DepthClipEnable = desc.rasterizer_state.depth_clip_enable ? TRUE : FALSE;
				d.RasterizerState.MultisampleEnable = desc.rasterizer_state.multisample_enable ? TRUE : FALSE;
				d.RasterizerState.AntialiasedLineEnable = desc.rasterizer_state.antialiased_line_enable ? TRUE : FALSE;
				d.RasterizerState.ConservativeRaster = desc.rasterizer_state.conservative_raster_enabled ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			}

			{
				d.DepthStencilState.DepthEnable = desc.depth_stencil_state.depth_test_enable ? TRUE : FALSE;
				d.DepthStencilState.DepthWriteMask = desc.depth_stencil_state.depth_write_enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
				d.DepthStencilState.DepthFunc = encode_comparison_func(desc.depth_stencil_state.depth_func);
				d.DepthStencilState.StencilEnable = desc.depth_stencil_state.stencil_enable ? TRUE : FALSE;
				d.DepthStencilState.StencilReadMask = desc.depth_stencil_state.stencil_read_mask;
				d.DepthStencilState.StencilWriteMask = desc.depth_stencil_state.stencil_write_mask;
				d.DepthStencilState.FrontFace.StencilFailOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_fail_op);
				d.DepthStencilState.FrontFace.StencilDepthFailOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_depth_fail_op);
				d.DepthStencilState.FrontFace.StencilPassOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_pass_op);
				d.DepthStencilState.FrontFace.StencilFunc = encode_comparison_func(desc.depth_stencil_state.front_face.stencil_func);
				d.DepthStencilState.BackFace.StencilFailOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_fail_op);
				d.DepthStencilState.BackFace.StencilDepthFailOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_depth_fail_op);
				d.DepthStencilState.BackFace.StencilPassOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_pass_op);
				d.DepthStencilState.BackFace.StencilFunc = encode_comparison_func(desc.depth_stencil_state.back_face.stencil_func);
			}

			Vector<D3D12_INPUT_ELEMENT_DESC> input_elements;
			if (desc.input_layout.input_elements.size())
			{
				u32 num_elements = (u32)desc.input_layout.input_elements.size();
				input_elements.resize(num_elements);
				d.InputLayout.NumElements = num_elements;
				d.InputLayout.pInputElementDescs = input_elements.data();
				for (u32 i = 0; i < num_elements; ++i)
				{
					D3D12_INPUT_ELEMENT_DESC& e = input_elements[i];
					const InputElementDesc& se = desc.input_layout.input_elements[i];
					e.SemanticName = se.semantic_name;
					e.SemanticIndex = se.semantic_index;
					e.Format = encode_pixel_format(se.format);
					e.InputSlot = se.input_slot;
					e.AlignedByteOffset = se.aligned_byte_offset;
					switch (se.input_slot_class)
					{
					case InputClassification::per_vertex:
						e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
						break;
					case InputClassification::per_instance:
						e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
						break;
					}
					e.InstanceDataStepRate = se.instance_data_step_rate;
				}
			}
			else
			{
				d.InputLayout.NumElements = 0;
				d.InputLayout.pInputElementDescs = nullptr;
			}

			switch (desc.ib_strip_cut_value)
			{
			case IndexBufferStripCutValue::disabled:
				d.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
				break;
			case IndexBufferStripCutValue::value_0xffff:
				d.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
				break;
			case IndexBufferStripCutValue::value_0xffffffff:
				d.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
				break;
			}

			switch (desc.primitive_topology_type)
			{
			case PrimitiveTopologyType::line:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				break;
			case PrimitiveTopologyType::patch:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
				break;
			case PrimitiveTopologyType::point:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
				break;
			case PrimitiveTopologyType::triangle:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				break;
			case PrimitiveTopologyType::undefined:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
				break;
			}

			d.NumRenderTargets = desc.num_render_targets;
			for (u32 i = 0; i < 8; ++i)
			{
				d.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}
			for (u32 i = 0; i < d.NumRenderTargets; ++i)
			{
				d.RTVFormats[i] = encode_pixel_format(desc.rtv_formats[i]);
			}
			d.DSVFormat = encode_pixel_format(desc.dsv_format);
			d.SampleDesc.Count = desc.sample_count;
			d.SampleDesc.Quality = desc.sample_quality;
			d.NodeMask = 0;
			d.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			d.CachedPSO.CachedBlobSizeInBytes = 0;
			d.CachedPSO.pCachedBlob = nullptr;

			if (FAILED(m_device->m_device->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&m_pso))))
			{
				return false;
			}
			return true;
		}
		bool PipelineState::init_compute(const ComputePipelineStateDesc& desc)
		{
			m_is_graphics = false;
			ShaderInputLayout* slayout = static_cast<ShaderInputLayout*>(desc.shader_input_layout->get_object());
			D3D12_COMPUTE_PIPELINE_STATE_DESC d;
			d.pRootSignature = slayout->m_rs.Get();
			d.CachedPSO.CachedBlobSizeInBytes = 0;
			d.CachedPSO.pCachedBlob = nullptr;
			fill_shader_data(d.CS, desc.cs);
			d.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			d.NodeMask = 0;
			if (FAILED(m_device->m_device->CreateComputePipelineState(&d, IID_PPV_ARGS(&m_pso))))
			{
				return false;
			}
			return true;
		}
	}
}

#endif