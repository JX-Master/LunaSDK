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
#include "D3D12Common.hpp"

#include <Luna/Runtime/Vector.hpp>

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
			case BlendFactor::one_minus_src_color:
				return D3D12_BLEND_INV_SRC_COLOR;
			case BlendFactor::src_alpha:
				return D3D12_BLEND_SRC_ALPHA;
			case BlendFactor::one_minus_src_alpha:
				return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendFactor::dst_alpha:
				return D3D12_BLEND_DEST_ALPHA;
			case BlendFactor::one_minus_dst_alpha:
				return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendFactor::dst_color:
				return D3D12_BLEND_DEST_COLOR;
			case BlendFactor::one_minus_dst_color:
				return D3D12_BLEND_INV_DEST_COLOR;
			case BlendFactor::src_alpha_saturated:
				return D3D12_BLEND_SRC_ALPHA_SAT;
			case BlendFactor::blend_factor:
				return D3D12_BLEND_BLEND_FACTOR;
			case BlendFactor::one_minus_blend_factor:
				return D3D12_BLEND_INV_BLEND_FACTOR;
			case BlendFactor::src1_color:
				return D3D12_BLEND_SRC1_COLOR;
			case BlendFactor::one_minus_src1_color:
				return D3D12_BLEND_INV_SRC1_COLOR;
			case BlendFactor::src1_alpha:
				return D3D12_BLEND_SRC1_ALPHA;
			case BlendFactor::one_minus_src1_alpha:
				return D3D12_BLEND_INV_SRC1_ALPHA;
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
			case StencilOp::increment_saturated:
				return D3D12_STENCIL_OP_INCR_SAT;
			case StencilOp::decrement_saturated:
				return D3D12_STENCIL_OP_DECR;
			case StencilOp::invert:
				return D3D12_STENCIL_OP_INVERT;
			case StencilOp::increment:
				return D3D12_STENCIL_OP_INCR;
			case StencilOp::decrement:
			default:
				lupanic();
				return D3D12_STENCIL_OP_KEEP;
			}
		}

		inline void fill_shader_data(D3D12_SHADER_BYTECODE& dst, const Span<const byte_t>& src)
		{
			dst.BytecodeLength = src.size();
			dst.pShaderBytecode = src.data();
		}

		inline void encode_target_blend_desc(D3D12_RENDER_TARGET_BLEND_DESC& rt, const AttachmentBlendDesc& src)
		{
			rt.BlendEnable = src.blend_enable ? TRUE : FALSE;
			rt.LogicOpEnable = FALSE;
			rt.SrcBlend = encode_blend_factor(src.src_blend_color);
			rt.DestBlend = encode_blend_factor(src.dst_blend_color);
			rt.BlendOp = encode_blend_op(src.blend_op_color);
			rt.SrcBlendAlpha = encode_blend_factor(src.src_blend_alpha);
			rt.DestBlendAlpha = encode_blend_factor(src.dst_blend_alpha);
			rt.BlendOpAlpha = encode_blend_op(src.blend_op_alpha);
			rt.LogicOp = D3D12_LOGIC_OP_NOOP;
			rt.RenderTargetWriteMask = 0;
			if ((src.color_write_mask & ColorWriteMask::red) != ColorWriteMask::none)
			{
				rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_RED;
			}
			if ((src.color_write_mask & ColorWriteMask::green) != ColorWriteMask::none)
			{
				rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_GREEN;
			}
			if ((src.color_write_mask & ColorWriteMask::blue) != ColorWriteMask::none)
			{
				rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_BLUE;
			}
			if ((src.color_write_mask & ColorWriteMask::alpha) != ColorWriteMask::none)
			{
				rt.RenderTargetWriteMask = rt.RenderTargetWriteMask | D3D12_COLOR_WRITE_ENABLE_ALPHA;
			}
		}

		RV PipelineState::init_graphic(const GraphicsPipelineStateDesc& desc)
		{
			m_is_graphics = true;
			PipelineLayout* playout = static_cast<PipelineLayout*>(desc.pipeline_layout->get_object());
			D3D12_GRAPHICS_PIPELINE_STATE_DESC d;
			d.pRootSignature = playout->m_rs.Get();

			fill_shader_data(d.VS, desc.vs);
			fill_shader_data(d.PS, desc.ps);
			fill_shader_data(d.DS, {});
			fill_shader_data(d.HS, {});
			fill_shader_data(d.GS, {});

			d.StreamOutput.NumEntries = 0;
			d.StreamOutput.NumStrides = 0;
			d.StreamOutput.pBufferStrides = nullptr;
			d.StreamOutput.RasterizedStream = 0;
			d.StreamOutput.pSODeclaration = nullptr;

			{
				d.BlendState.AlphaToCoverageEnable = desc.blend_state.alpha_to_coverage_enable ? TRUE : FALSE;
				d.BlendState.IndependentBlendEnable = desc.blend_state.independent_blend_enable ? TRUE : FALSE;
				for (u32 i = 0; i < 8; ++i)
				{
					if (d.BlendState.IndependentBlendEnable)
					{
						encode_target_blend_desc(d.BlendState.RenderTarget[i], desc.blend_state.attachments[i]);
					}
					else
					{
						encode_target_blend_desc(d.BlendState.RenderTarget[i], desc.blend_state.attachments[0]);
					}
				}
				d.SampleMask = 0xFFFFFFFF;
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
				d.RasterizerState.ForcedSampleCount = 0;
				d.RasterizerState.FrontCounterClockwise = desc.rasterizer_state.front_counter_clockwise ? TRUE : FALSE;
				d.RasterizerState.DepthBias = desc.rasterizer_state.depth_bias;
				d.RasterizerState.DepthBiasClamp = desc.rasterizer_state.depth_bias_clamp;
				d.RasterizerState.SlopeScaledDepthBias = desc.rasterizer_state.slope_scaled_depth_bias;
				d.RasterizerState.DepthClipEnable = desc.rasterizer_state.depth_clip_enable ? TRUE : FALSE;
				d.RasterizerState.MultisampleEnable = desc.sample_count == 1 ? FALSE : TRUE;
				d.RasterizerState.AntialiasedLineEnable = TRUE;
				d.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			}

			{
				d.DepthStencilState.DepthEnable = desc.depth_stencil_state.depth_test_enable ? TRUE : FALSE;
				d.DepthStencilState.DepthWriteMask = desc.depth_stencil_state.depth_write_enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
				d.DepthStencilState.DepthFunc = encode_compare_function(desc.depth_stencil_state.depth_func);
				d.DepthStencilState.StencilEnable = desc.depth_stencil_state.stencil_enable ? TRUE : FALSE;
				d.DepthStencilState.StencilReadMask = desc.depth_stencil_state.stencil_read_mask;
				d.DepthStencilState.StencilWriteMask = desc.depth_stencil_state.stencil_write_mask;
				d.DepthStencilState.FrontFace.StencilFailOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_fail_op);
				d.DepthStencilState.FrontFace.StencilDepthFailOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_depth_fail_op);
				d.DepthStencilState.FrontFace.StencilPassOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_pass_op);
				d.DepthStencilState.FrontFace.StencilFunc = encode_compare_function(desc.depth_stencil_state.front_face.stencil_func);
				d.DepthStencilState.BackFace.StencilFailOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_fail_op);
				d.DepthStencilState.BackFace.StencilDepthFailOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_depth_fail_op);
				d.DepthStencilState.BackFace.StencilPassOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_pass_op);
				d.DepthStencilState.BackFace.StencilFunc = encode_compare_function(desc.depth_stencil_state.back_face.stencil_func);
			}

			Vector<D3D12_INPUT_ELEMENT_DESC> input_elements;
			if (desc.input_layout.attributes.size())
			{
				u32 num_elements = (u32)desc.input_layout.attributes.size();
				input_elements.resize(num_elements);
				d.InputLayout.NumElements = num_elements;
				d.InputLayout.pInputElementDescs = input_elements.data();
				for (u32 i = 0; i < num_elements; ++i)
				{
					D3D12_INPUT_ELEMENT_DESC& e = input_elements[i];
					auto& se = desc.input_layout.attributes[i];
					e.SemanticName = se.semantic_name;
					e.SemanticIndex = se.semantic_index;
					e.Format = encode_format(se.format);
					e.InputSlot = se.binding_slot;
					e.AlignedByteOffset = se.offset;
					e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
					for (usize i = 0; i < desc.input_layout.bindings.size(); ++i)
					{
						if (se.binding_slot == desc.input_layout.bindings[i].binding_slot)
						{
							switch (desc.input_layout.bindings[i].input_rate)
							{
							case InputRate::per_vertex:
								e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
								break;
							case InputRate::per_instance:
								e.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
								break;
							}
						}
					}
					e.InstanceDataStepRate = (e.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) ? 0 : 1;
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

			m_primitive_topology = desc.primitive_topology;
			switch (desc.primitive_topology)
			{
			case PrimitiveTopology::line_list:
			case PrimitiveTopology::line_strip:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				break;
			case PrimitiveTopology::point_list:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
				break;
			case PrimitiveTopology::triangle_list:
			case PrimitiveTopology::triangle_strip:
				d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				break;
			}

			d.NumRenderTargets = desc.num_color_attachments;
			for (u32 i = 0; i < 8; ++i)
			{
				d.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}
			for (u32 i = 0; i < d.NumRenderTargets; ++i)
			{
				d.RTVFormats[i] = encode_format(desc.color_formats[i]);
			}
			d.DSVFormat = encode_format(desc.depth_stencil_format);
			d.SampleDesc.Count = desc.sample_count;
			d.SampleDesc.Quality = desc.sample_count == 1 ? 0 : 1;
			d.NodeMask = 0;
			d.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			d.CachedPSO.CachedBlobSizeInBytes = 0;
			d.CachedPSO.pCachedBlob = nullptr;
			HRESULT hr = m_device->m_device->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&m_pso));
			if (FAILED(hr))
			{
				return encode_hresult(hr);
			}
			return ok;
		}
		RV PipelineState::init_compute(const ComputePipelineStateDesc& desc)
		{
			m_is_graphics = false;
			PipelineLayout* playout = static_cast<PipelineLayout*>(desc.pipeline_layout->get_object());
			D3D12_COMPUTE_PIPELINE_STATE_DESC d;
			d.pRootSignature = playout->m_rs.Get();
			d.CachedPSO.CachedBlobSizeInBytes = 0;
			d.CachedPSO.pCachedBlob = nullptr;
			fill_shader_data(d.CS, desc.cs);
			d.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			d.NodeMask = 0;
			HRESULT hr = m_device->m_device->CreateComputePipelineState(&d, IID_PPV_ARGS(&m_pso));
			if (FAILED(hr))
			{
				return encode_hresult(hr);
			}
			return ok;
		}
	}
}