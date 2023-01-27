/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include "ResourceHeap.hpp"
#include "ShaderInputLayout.hpp"
#include "PipelineState.hpp"
#include "DescriptorSet.hpp"
#include "CommandQueue.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"

namespace Luna
{
	namespace RHI
	{
		//! Represents one logical graphic device on the platform.
		struct IDevice : virtual Interface
		{
			luiid("{099AB8FA-7239-41EE-B05C-D36B5DCE1ED7}");

			//! Gets the alignment for the size of one row texture data to follow when placing texture data in buffers.
			virtual usize get_texture_data_pitch_alignment() = 0;

			//! Gets the alignment for the start offset of texture data to follow when placing texture data in buffers.
			virtual usize get_texture_data_placement_alignment() = 0;

			//! Gets the alignment for the buffer data start location and size.
			virtual usize get_constant_buffer_data_alignment() = 0;

			//! Calculates the placement information for the specified texture subresource. The texture data is arranged in row-major order.
			//! @param[in] width The width of the subresource.
			//! @param[in] height The height of the subresource.
			//! @param[in] depth The depth of the subresource.
			//! @param[in] format The format of the subresource.
			//! @param[out] row_pitch The row pitch of the subresource. Specify `nullptr` if this is not needed.
			//! @param[out] slice_pitch The slice pitch (size) of the subresource. Specify `nullptr` if this is not needed.
			//! @param[out] res_pitch The pitch of the whole subresource, which is the size of the subresource. Specify `nullptr` if this is not needed.
			virtual void calc_texture_subresource_buffer_placement(u32 width, u32 height, u32 depth, Format format,
				usize* row_pitch, usize* slice_pitch, usize* res_pitch) = 0;

			//! Calculates the resource size by its descriptor. The size can be used to create buffers for uploading/reading data of the resource.
			virtual usize calc_resource_size(const ResourceDesc& desc, usize* out_alignment = nullptr) = 0;

			//! Creates one new resource.
			//! @param[in] desc The descriptor object.
			//! @param[in] optimized_clear_value The optional optimized clear value for a texture resource. Specify `nullptr` if this is a buffer
			//! resource or the resource does not have a optimized clear value.
			virtual R<Ref<IResource>> new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;
		
			//! Creates one new resource heap.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IResourceHeap>> new_resource_heap(const ResourceHeapDesc& desc) = 0;

			//! Creates one new shader input layout.
			virtual R<Ref<IShaderInputLayout>> new_shader_input_layout(const ShaderInputLayoutDesc& desc) = 0;

			//! Creates one new graphic pipeline state.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IPipelineState>> new_graphic_pipeline_state(const GraphicPipelineStateDesc& desc) = 0;

			//! Creates one compute pipeline state.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) = 0;

			//! Creates one new descriptor set layout object that can be used to create descriptor sets.
			virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) = 0;

			//! Creates one new descriptor set object that describes resources that are bound to the pipeline.
			//! @param[in] target_input_layout The shader input layout object this view set is created for.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IDescriptorSet>> new_descriptor_set(DescriptorSetDesc& desc) = 0;

			//! Creates one new command queue.
			virtual R<Ref<ICommandQueue>> new_command_queue(CommandQueueType type) = 0;

			virtual R<Ref<IRenderTargetView>> new_render_target_view(IResource* resource, const RenderTargetViewDesc* desc = nullptr) = 0;

			virtual R<Ref<IDepthStencilView>> new_depth_stencil_view(IResource* resource, const DepthStencilViewDesc* desc = nullptr) = 0;
		};
	}
}