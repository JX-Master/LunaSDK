/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PipelineState.cpp
* @author JXMaster
* @date 2023/4/23
*/
#include "PipelineState.hpp"
#include "ShaderInputLayout.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ShaderModule
		{
			Device* device = nullptr;
			VkShaderModule shader_module = VK_NULL_HANDLE;

			RV init(Device* device, Span<const byte_t> code)
			{
				this->device = device;
				VkShaderModuleCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				create_info.codeSize = code.size();
				create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
				create_info.flags = 0;
				return encode_vk_result(device->m_funcs.vkCreateShaderModule(device->m_device, &create_info, nullptr, &shader_module));
			}
			~ShaderModule()
			{
				if (shader_module != VK_NULL_HANDLE)
				{
					device->m_funcs.vkDestroyShaderModule(device->m_device, shader_module, nullptr);
					shader_module = nullptr;
				}
			}
		};

		RV PipelineState::init_as_graphics(const GraphicsPipelineStateDesc& desc)
		{
			lutry
			{
				VkGraphicsPipelineCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				create_info.flags = 0;
				// vs, hs, ds, gs, ps.
				VkPipelineShaderStageCreateInfo stages[5] = { {} };
				VkShaderModule shader_modles[5] = { VK_NULL_HANDLE };
				u32 num_stages = 0;
				ShaderModule vs;
				ShaderModule ps;
				if (!desc.vs.empty())
				{
					luexp(vs.init(m_device, desc.vs));
					shader_modles[num_stages] = vs.shader_module;
					auto& dest = stages[num_stages];
					dest.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					dest.stage = VK_SHADER_STAGE_VERTEX_BIT;
					dest.module = shader_modles[num_stages];
					dest.pName = "main";
					dest.flags = 0;
					dest.pSpecializationInfo = nullptr;
					++num_stages;
				}
				if (!desc.ps.empty())
				{
					luexp(ps.init(m_device, desc.ps));
					shader_modles[num_stages] = ps.shader_module;
					auto& dest = stages[num_stages];
					dest.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					dest.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					dest.module = shader_modles[num_stages];
					dest.pName = "main";
					dest.flags = 0;
					dest.pSpecializationInfo = nullptr;
					++num_stages;
				}
				create_info.pStages = stages;
				create_info.stageCount = num_stages;
				// Vertex input.
				VkPipelineVertexInputStateCreateInfo vertex_input{};
				vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				if (desc.input_layout.bindings.empty())
				{
					vertex_input.vertexBindingDescriptionCount = 0;
					vertex_input.pVertexBindingDescriptions = nullptr;
				}
				else
				{
					VkVertexInputBindingDescription* descs = (VkVertexInputBindingDescription*)alloca(
						sizeof(VkVertexInputBindingDescription) * desc.input_layout.bindings.size());
					memzero(descs, sizeof(VkVertexInputBindingDescription) * desc.input_layout.bindings.size());
					for (usize i = 0; i < desc.input_layout.bindings.size(); ++i)
					{
						auto& dest = descs[i];
						auto& src = desc.input_layout.bindings[i];
						dest.binding = src.binding_slot;
						dest.stride = src.element_size;
						switch (src.input_rate)
						{
						case InputRate::per_vertex:
							dest.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; break;
						case InputRate::per_instance:
							dest.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE; break;
						}
					}
					vertex_input.vertexBindingDescriptionCount = (u32)desc.input_layout.bindings.size();
					vertex_input.pVertexBindingDescriptions = descs;
				}
				if (desc.input_layout.attributes.empty())
				{
					vertex_input.vertexAttributeDescriptionCount = 0;
					vertex_input.pVertexAttributeDescriptions = nullptr;
				}
				else
				{
					VkVertexInputAttributeDescription* descs = (VkVertexInputAttributeDescription*)alloca(
						sizeof(VkVertexInputAttributeDescription) * desc.input_layout.attributes.size());
					memzero(descs, sizeof(VkVertexInputAttributeDescription) * desc.input_layout.attributes.size());
					for (usize i = 0; i < desc.input_layout.attributes.size(); ++i)
					{
						auto& dest = descs[i];
						auto& src = desc.input_layout.attributes[i];
						dest.location = src.location;
						dest.binding = src.binding_slot;
						dest.format = encode_format(src.format);
						dest.offset = src.offset;
					}
					vertex_input.vertexAttributeDescriptionCount = (u32)desc.input_layout.attributes.size();
					vertex_input.pVertexAttributeDescriptions = descs;
				}
				create_info.pVertexInputState = &vertex_input;
				// input assembly.
				VkPipelineInputAssemblyStateCreateInfo input_assembly{};
				input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				input_assembly.topology = encode_primitive_topology(desc.primitive_topology);
				input_assembly.primitiveRestartEnable = desc.ib_strip_cut_value == IndexBufferStripCutValue::disabled ? VK_FALSE : VK_TRUE;
				create_info.pInputAssemblyState = &input_assembly;
				// tessllation
				create_info.pTessellationState = nullptr;
				// viewports.
				VkPipelineViewportStateCreateInfo viewport{};
				viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewport.viewportCount = m_device->m_physical_device_properties.limits.maxViewports;
				viewport.scissorCount = m_device->m_physical_device_properties.limits.maxViewports;
				create_info.pViewportState = &viewport;
				// rasterization
				VkPipelineRasterizationStateCreateInfo rasterizer{};
				rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizer.depthClampEnable = desc.rasterizer_state.depth_clip_enable ? VK_FALSE : VK_TRUE; // Enabling depth clamp will disable depth clip.
				rasterizer.rasterizerDiscardEnable = VK_FALSE;
				switch (desc.rasterizer_state.fill_mode)
				{
				case FillMode::solid: rasterizer.polygonMode = VK_POLYGON_MODE_FILL; break;
				case FillMode::wireframe: rasterizer.polygonMode = VK_POLYGON_MODE_LINE; break;
				}
				switch (desc.rasterizer_state.cull_mode)
				{
				case CullMode::none: rasterizer.cullMode = VK_CULL_MODE_NONE; break;
				case CullMode::front: rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; break;
				case CullMode::back: rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; break;
				}
				rasterizer.frontFace = desc.rasterizer_state.front_counter_clockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
				rasterizer.depthBiasEnable = desc.rasterizer_state.depth_bias == 0 ? VK_FALSE : VK_TRUE;
				rasterizer.depthBiasConstantFactor = (float)desc.rasterizer_state.depth_bias;
				rasterizer.depthBiasClamp = desc.rasterizer_state.depth_bias_clamp;
				rasterizer.depthBiasSlopeFactor = desc.rasterizer_state.slope_scaled_depth_bias;
				rasterizer.lineWidth = 1.0f;
				create_info.pRasterizationState = &rasterizer;
				// multisample.
				VkPipelineMultisampleStateCreateInfo multisampling{};
				multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampling.rasterizationSamples = encode_sample_count((u8)desc.sample_count);
				multisampling.sampleShadingEnable = VK_FALSE;
				multisampling.minSampleShading = 0.0f;
				multisampling.pSampleMask = &desc.sample_mask;
				multisampling.alphaToCoverageEnable = desc.blend_state.alpha_to_coverage_enable ? VK_TRUE : VK_FALSE;
				multisampling.alphaToOneEnable = VK_FALSE;
				create_info.pMultisampleState = &multisampling;
				// depth stencil.
				VkPipelineDepthStencilStateCreateInfo depth_stencil{};
				depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				depth_stencil.depthTestEnable = desc.depth_stencil_state.depth_test_enable ? VK_TRUE : VK_FALSE;
				depth_stencil.depthWriteEnable = desc.depth_stencil_state.depth_write_enable ? VK_TRUE : VK_FALSE;
				depth_stencil.depthCompareOp = encode_compare_op(desc.depth_stencil_state.depth_func);
				depth_stencil.depthBoundsTestEnable = VK_FALSE;
				depth_stencil.stencilTestEnable = desc.depth_stencil_state.stencil_enable ? VK_TRUE : VK_FALSE;
				depth_stencil.front.failOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_fail_op);
				depth_stencil.front.passOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_pass_op);
				depth_stencil.front.depthFailOp = encode_stencil_op(desc.depth_stencil_state.front_face.stencil_depth_fail_op);
				depth_stencil.front.compareOp = encode_compare_op(desc.depth_stencil_state.front_face.stencil_func);
				depth_stencil.front.compareMask = desc.depth_stencil_state.stencil_read_mask;
				depth_stencil.front.writeMask = desc.depth_stencil_state.stencil_write_mask;
				depth_stencil.front.reference = 0;
				depth_stencil.back.failOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_fail_op);
				depth_stencil.back.passOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_pass_op);
				depth_stencil.back.depthFailOp = encode_stencil_op(desc.depth_stencil_state.back_face.stencil_depth_fail_op);
				depth_stencil.back.compareOp = encode_compare_op(desc.depth_stencil_state.back_face.stencil_func);
				depth_stencil.back.compareMask = desc.depth_stencil_state.stencil_read_mask;
				depth_stencil.back.writeMask = desc.depth_stencil_state.stencil_write_mask;
				depth_stencil.back.reference = 0;
				depth_stencil.minDepthBounds = 0.0f;
				depth_stencil.maxDepthBounds = 1.0f;
				create_info.pDepthStencilState = &depth_stencil;
				// blend state.
				VkPipelineColorBlendStateCreateInfo blend{};
				blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				blend.logicOpEnable = desc.blend_state.logic_op_enable ? VK_TRUE : VK_FALSE;
				blend.logicOp = encode_logic_op(desc.blend_state.logic_op);
				blend.attachmentCount = desc.num_render_targets;
				VkPipelineColorBlendAttachmentState attachments[8] = { {} };
				for (usize i = 0; i < desc.num_render_targets; ++i)
				{
					auto& dest = attachments[i];
					auto& src = desc.blend_state.rt[i];
					dest.blendEnable = src.blend_enable ? VK_TRUE : VK_FALSE;
					dest.srcColorBlendFactor = encode_blend_factor(src.src_blend);
					dest.dstColorBlendFactor = encode_blend_factor(src.dest_blend);
					dest.colorBlendOp = encode_blend_op(src.blend_op);
					dest.srcAlphaBlendFactor = encode_blend_factor(src.src_blend_alpha);
					dest.dstAlphaBlendFactor = encode_blend_factor(src.dest_blend_alpha);
					dest.alphaBlendOp = encode_blend_op(src.blend_op_alpha);
					dest.colorWriteMask = encode_color_component_flags(src.render_target_write_mask);
				}
				blend.pAttachments = attachments;
				create_info.pColorBlendState = &blend;
				// dynamic states.
				VkDynamicState dynamic_states[] = {
					VK_DYNAMIC_STATE_VIEWPORT,
					VK_DYNAMIC_STATE_SCISSOR,
					VK_DYNAMIC_STATE_BLEND_CONSTANTS,
					VK_DYNAMIC_STATE_STENCIL_REFERENCE
				};
				VkPipelineDynamicStateCreateInfo synamic_state_create_info{};
				synamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				synamic_state_create_info.pDynamicStates = dynamic_states;
				synamic_state_create_info.dynamicStateCount = 4;
				create_info.pDynamicState = &synamic_state_create_info;
				// pipeline layout.
				ShaderInputLayout* slayout = (ShaderInputLayout*)desc.shader_input_layout->get_object();
				create_info.layout = slayout->m_pipeline_layout;
				// render pass.
				RenderPassKey render_pass;
				for (usize i = 0; i < desc.num_render_targets; ++i)
				{
					render_pass.color_formats[i] = desc.rtv_formats[i];
					render_pass.color_store_ops[i] = StoreOp::store;
					if (desc.sample_count > 1)
					{
						render_pass.resolve_formats[i] = desc.rtv_formats[i];
					}
				}
				render_pass.depth_stencil_format = desc.dsv_format;
				if (render_pass.depth_stencil_format != Format::unknown)
				{
					render_pass.depth_load_op = LoadOp::load;
					render_pass.depth_store_op = StoreOp::store;
				}
				render_pass.sample_count = (u8)desc.sample_count;
				LockGuard guard(m_device->m_render_pass_pool_lock);
				luset(create_info.renderPass, m_device->m_render_pass_pool.get_render_pass(render_pass));
				guard.unlock();
				create_info.subpass = 0;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateGraphicsPipelines(m_device->m_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &m_pipeline)));
			}	
			lucatchret;
			return ok;
		}
		RV PipelineState::init_as_compute(const ComputePipelineStateDesc& desc)
		{
			lutry
			{
				VkComputePipelineCreateInfo create_info{};
				create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
				// compute stage.
				ShaderModule cs;
				VkShaderModule shader_modle = { VK_NULL_HANDLE };
				luexp(cs.init(m_device, desc.cs));
				create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				create_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
				create_info.stage.module = cs.shader_module;
				create_info.stage.pName = "main";
				create_info.stage.flags = 0;
				create_info.stage.pSpecializationInfo = nullptr;
				// pipeline layout.
				ShaderInputLayout* slayout = (ShaderInputLayout*)desc.shader_input_layout->get_object();
				create_info.layout = slayout->m_pipeline_layout;
				luexp(encode_vk_result(m_device->m_funcs.vkCreateComputePipelines(m_device->m_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &m_pipeline)));
			}
			lucatchret;
			return ok;
		}
		PipelineState::~PipelineState()
		{
			if (m_pipeline != VK_NULL_HANDLE)
			{
				m_device->m_funcs.vkDestroyPipeline(m_device->m_device, m_pipeline, nullptr);
				m_pipeline = VK_NULL_HANDLE;
			}
		}
	}
}