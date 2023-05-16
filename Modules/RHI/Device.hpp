/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.hpp
* @author JXMaster
* @date 2019/7/17
*/
#pragma once
#include "ShaderInputLayout.hpp"
#include "PipelineState.hpp"
#include "DescriptorSet.hpp"
#include "CommandBuffer.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "SwapChain.hpp"
#include "Fence.hpp"
#include "QueryHeap.hpp"

#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif

namespace Luna
{
	namespace RHI
	{
		//enum class ResourceCopyOp : u8
		//{
		//	//! Copy data of one buffer resource from resource memory to system memory.
		//	read_buffer,
		//	//! Copy data of one buffer resource from system memory to resource memory.
		//	write_buffer,
		//	//! Copy data of one texture resource from resource memory to system memory.
		//	read_texture,
		//	//! Copy data of one texture resource from system memory to resource memory.
		//	write_texture
		//};
		//struct ResourceCopyReadBufferDesc
		//{
		//	IBuffer* src;
		//	void* dst;
		//	u64 src_offset;
		//	usize size;
		//};
		//struct ResourceCopyWriteBufferDesc
		//{
		//	const void* src;
		//	IBuffer* dst;
		//	u64 dst_offset;
		//	usize size;
		//};
		//struct ResourceCopyReadTextureDesc
		//{
		//	ITexture* src;
		//	void* dst;
		//	SubresourceIndex src_subresource;
		//	u32 dst_row_pitch;
		//	u32 dst_depth_pitch;
		//	BoxU read_box;
		//};
		//struct ResourceCopyWriteTextureDesc
		//{
		//	const void* src;
		//	ITexture* dst;
		//	u32 src_row_pitch;
		//	u32 src_depth_pitch;
		//	SubresourceIndex dst_subresource;
		//	BoxU write_box;
		//};
		//struct ResourceCopyDesc
		//{
		//	ResourceCopyOp op;
		//	union
		//	{
		//		ResourceCopyReadBufferDesc read_buffer;
		//		ResourceCopyWriteBufferDesc write_buffer;
		//		ResourceCopyReadTextureDesc read_texture;
		//		ResourceCopyWriteTextureDesc write_texture;
		//	};
		//	static ResourceCopyDesc as_read_buffer(IBuffer* resource, void* dst, usize size, u64 src_offset)
		//	{
		//		ResourceCopyDesc r;
		//		r.op = ResourceCopyOp::read_buffer;
		//		r.read_buffer.src = resource;
		//		r.read_buffer.dst = dst;
		//		r.read_buffer.size = size;
		//		r.read_buffer.src_offset = src_offset;
		//		return r;
		//	}
		//	static ResourceCopyDesc as_write_buffer(IBuffer* resource, const void* src, usize size, u64 dst_offset)
		//	{
		//		ResourceCopyDesc r;
		//		r.op = ResourceCopyOp::write_buffer;
		//		r.write_buffer.dst = resource;
		//		r.write_buffer.src = src;
		//		r.write_buffer.size = size;
		//		r.write_buffer.dst_offset = dst_offset;
		//		return r;
		//	}
		//	static ResourceCopyDesc as_read_texture(ITexture* resource, void* dst, u32 dst_row_pitch, u32 dst_depth_pitch, const SubresourceIndex& src_subresource, const BoxU& read_box)
		//	{
		//		ResourceCopyDesc r;
		//		r.op = ResourceCopyOp::read_texture;
		//		r.read_texture.src = resource;
		//		r.read_texture.dst = dst;
		//		r.read_texture.dst_row_pitch = dst_row_pitch;
		//		r.read_texture.dst_depth_pitch = dst_depth_pitch;
		//		r.read_texture.src_subresource = src_subresource;
		//		r.read_texture.read_box = read_box;
		//		return r;
		//	}
		//	static ResourceCopyDesc as_write_texture(ITexture* resource, const void* src, u32 src_row_pitch, u32 src_depth_pitch, const SubresourceIndex& dst_subresource, const BoxU& write_box)
		//	{
		//		ResourceCopyDesc r;
		//		r.op = ResourceCopyOp::write_texture;
		//		r.write_texture.dst = resource;
		//		r.write_texture.src = src;
		//		r.write_texture.src_row_pitch = src_row_pitch;
		//		r.write_texture.src_depth_pitch = src_depth_pitch;
		//		r.write_texture.dst_subresource = dst_subresource;
		//		r.write_texture.write_box = write_box;
		//		return r;
		//	}
		//};

		enum class DeviceFeature : u32
		{
			//! `DescriptorSetLayoutFlag::variable_descriptors` is allowed when creating descriptor set layout.
			unbound_descriptor_array,
		};

		enum class CommandQueueType : u8
		{
			//! This command queue can be used for submitting graphics, compute and copy commands.
			graphics = 1,
			//! This command queue can be used for submitting compute and copy commands.
			compute = 2,
			//! This command queue can be used for submitting copy commands.
			copy = 3,
		};

		enum class CommandQueueFlags : u8
		{
			none = 0,
			//! This command queue supports swap chain presenting commands.
			presenting = 0x01,
		};

		struct CommandQueueDesc
		{
			CommandQueueType type;
			CommandQueueFlags flags;

			CommandQueueDesc() = default;
			CommandQueueDesc(CommandQueueType type, CommandQueueFlags flags) :
				type(type),
				flags(flags) {}
		};

		//! Represents one logical graphic device on the platform.
		struct IDevice : virtual Interface
		{
			luiid("{099AB8FA-7239-41EE-B05C-D36B5DCE1ED7}");

			//! Checks whether the specified device feature is present.
			virtual bool check_device_feature(DeviceFeature feature) = 0;

			//! Gets the alignment for the buffer data start location and size.
			virtual usize get_uniform_buffer_data_alignment() = 0;

			//! Gets the texture data placement information when storing texture data in a buffer. 
			//! The texture data is arranged in row-major order.
			//! @param[in] width The width of the texture data.
			//! @param[in] height The height of the texture data.
			//! @param[in] depth The depth of the texture data.
			//! @param[in] format The format of the texture data.
			//! @param[out] size The size of the texture data in the buffer. Specify `nullptr` if this is not needed.
			//! @param[out] alignment The alignment requirement of the texture data. Specify `nullptr` if this is not needed.
			//! @param[out] row_pitch The row pitch of the texture data. Specify `nullptr` if this is not needed.
			//! @param[out] slice_pitch The slice (row * column) pitch of the texture data. Specify `nullptr` if this is not needed.
			virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size = nullptr, u64* alignment = nullptr, u64* row_pitch = nullptr, u64* slice_pitch = nullptr) = 0;

			//! Creates one new buffer resource and allocates device memory for the resource.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IBuffer>> new_buffer(const BufferDesc& desc) = 0;

			//! Creates one new texture resource and allocates device memory for the resource.
			//! @param[in] desc The descriptor object.
			//! @param[in] optimized_clear_value The optional optimized clear value for a texture resource. Specify `nullptr` if 
			//! the resource does not have a optimized clear value.
			virtual R<Ref<ITexture>> new_texture(const TextureDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;

			//! Checks whether the given resources can share the same device memory.
			//! @param[in] buffers The buffer descriptors of resources being examined.
			//! @param[in] textures The texture descriptors of resources being examined.
			//! @return Returns `true` if such resources can share the same device memory, returns `false` otherwise.
			virtual bool is_resources_aliasing_compatible(Span<const BufferDesc> buffers, Span<const TextureDesc> textures) = 0;

			//! Allocates device memory that is capable of storing multiple resources specified.
			virtual R<Ref<IDeviceMemory>> allocate_memory(Span<const BufferDesc> buffers, Span<const TextureDesc> textures) = 0;

			//! Creates one aliasing buffer that shares the same device memory with the existing resource.
			//! The user may create multiple aliasing resources with the same device memory, given that only one of them is active at any given time.
			//! The user should use aliasing barrier to switch the active resource between aliasing resources sharing the same device memory.
			virtual R<Ref<IBuffer>> new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc) = 0;

			//! Creates one aliasing texture that shares the same device memory with the existing resource.
			//! The user may create multiple aliasing resources with the same device memory, given that only one of them is active at any given time.
			//! The user should use aliasing barrier to switch the active resource between aliasing resources sharing the same device memory.
			virtual R<Ref<ITexture>> new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;

			//! Creates one new shader input layout.
			virtual R<Ref<IShaderInputLayout>> new_shader_input_layout(const ShaderInputLayoutDesc& desc) = 0;

			//! Creates one new graphic pipeline state.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IPipelineState>> new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc) = 0;

			//! Creates one compute pipeline state.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) = 0;

			//! Creates one new descriptor set layout object that can be used to create descriptor sets.
			virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) = 0;

			//! Creates one new descriptor set object that describes resources that are bound to the pipeline.
			//! @param[in] target_input_layout The shader input layout object this view set is created for.
			//! @param[in] desc The descriptor object.
			virtual R<Ref<IDescriptorSet>> new_descriptor_set(const DescriptorSetDesc& desc) = 0;

			//! Gets the number of command queues of the device.
			virtual u32 get_num_command_queues() = 0;

			//! Gets the command queue description of the specified command queue.
			//! @param[in] command_queue_index The index of the command queue to check.
			//! The index must be in range [0, get_num_command_queues()).
			virtual CommandQueueDesc get_command_queue_desc(u32 command_queue_index) = 0;

			//! Creates one command buffer.
			//! @param[in] command_queue_index The index of the command queue that will be attached to the 
			//! command buffer. The command buffer can only submit commands to its attached command queue.
			virtual R<Ref<ICommandBuffer>> new_command_buffer(u32 command_queue_index) = 0;

			//! Gets the GPU timestamp frequency of the command queue. 
			//! The timestamp frequency is measured in ticks per second.
			virtual R<f64> get_command_queue_timestamp_frequency(u32 command_queue_index) = 0;

			virtual R<Ref<IRenderTargetView>> new_render_target_view(ITexture* resource, const RenderTargetViewDesc* desc = nullptr) = 0;

			virtual R<Ref<IDepthStencilView>> new_depth_stencil_view(ITexture* resource, const DepthStencilViewDesc* desc = nullptr) = 0;

			virtual R<Ref<IResolveTargetView>> new_resolve_target_view(ITexture* resource, const ResolveTargetViewDesc* desc = nullptr) = 0;
		
			virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) = 0;

			virtual R<Ref<IFence>> new_fence() = 0;

			//! Creates a swap chain resource and binds it to the specified window.
			//! @param[in] command_queue_index The command queue attached to the swap chain. Present commands will only be 
			//! submitted to the command queue attached with the swap chain.
			//! @param[in] window The window this swap chain should be outputted to.
			//! @param[in] desc The swap chain description.
			//! @return Returns the new created swap chain, or `nullptr` if failed to create.
			virtual R<Ref<ISwapChain>> new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc) = 0;
		};
	}
}