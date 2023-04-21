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
#include "CommandQueue.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "Fence.hpp"
#include "QueryHeap.hpp"

namespace Luna
{
	namespace RHI
	{
		enum class ResourceCopyOp : u8
		{
			//! Copy data of one buffer resource from resource memory to system memory.
			read_buffer,
			//! Copy data of one buffer resource from system memory to resource memory.
			write_buffer,
			//! Copy data of one texture resource from resource memory to system memory.
			read_texture,
			//! Copy data of one texture resource from system memory to resource memory.
			write_texture
		};
		struct ResourceCopyReadBufferDesc
		{
			void* dest;
			usize size;
			u64 src_offset;
		};
		struct ResourceCopyWriteBufferDesc
		{
			const void* src;
			usize size;
			u64 dest_offset;
		};
		struct ResourceCopyReadTextureDesc
		{
			void* dest;
			u32 dest_row_pitch;
			u32 dest_depth_pitch;
			u32 src_subresource;
			BoxU read_box;
		};
		struct ResourceCopyWriteTextureDesc
		{
			const void* src;
			u32 src_row_pitch;
			u32 src_depth_pitch;
			u32 dest_subresource;
			BoxU write_box;
		};
		struct ResourceCopyDesc
		{
			ResourceCopyOp op;
			IResource* resource;
			union
			{
				ResourceCopyReadBufferDesc read_buffer;
				ResourceCopyWriteBufferDesc write_buffer;
				ResourceCopyReadTextureDesc read_texture;
				ResourceCopyWriteTextureDesc write_texture;
			};
			static ResourceCopyDesc as_read_buffer(IResource* resource, void* dest, usize size, u64 src_offset)
			{
				ResourceCopyDesc r;
				r.op = ResourceCopyOp::read_buffer;
				r.resource = resource;
				r.read_buffer.dest = dest;
				r.read_buffer.size = size;
				r.read_buffer.src_offset = src_offset;
				return r;
			}
			static ResourceCopyDesc as_write_buffer(IResource* resource, const void* src, usize size, u64 dest_offset)
			{
				ResourceCopyDesc r;
				r.op = ResourceCopyOp::write_buffer;
				r.resource = resource;
				r.write_buffer.src = src;
				r.write_buffer.size = size;
				r.write_buffer.dest_offset = dest_offset;
				return r;
			}
			static ResourceCopyDesc as_read_texture(IResource* resource, void* dest, u32 dest_row_pitch, u32 dest_depth_pitch, u32 src_subresource, const BoxU& read_box)
			{
				ResourceCopyDesc r;
				r.op = ResourceCopyOp::read_texture;
				r.resource = resource;
				r.read_texture.dest = dest;
				r.read_texture.dest_row_pitch = dest_row_pitch;
				r.read_texture.dest_depth_pitch = dest_depth_pitch;
				r.read_texture.src_subresource = src_subresource;
				r.read_texture.read_box = read_box;
				return r;
			}
			static ResourceCopyDesc as_write_texture(IResource* resource, const void* src, u32 src_row_pitch, u32 src_depth_pitch, u32 dest_subresource, const BoxU& write_box)
			{
				ResourceCopyDesc r;
				r.op = ResourceCopyOp::write_texture;
				r.resource = resource;
				r.write_texture.src = src;
				r.write_texture.src_row_pitch = src_row_pitch;
				r.write_texture.src_depth_pitch = src_depth_pitch;
				r.write_texture.dest_subresource = dest_subresource;
				r.write_texture.write_box = write_box;
				return r;
			}
		};

		enum class DeviceFeature : u32
		{
			//! `DescriptorSetLayoutFlag::variable_descriptors` is allowed when creating descriptor set layout.
			unbound_descriptor_array,
		};

		//! Represents one logical graphic device on the platform.
		struct IDevice : virtual Interface
		{
			luiid("{099AB8FA-7239-41EE-B05C-D36B5DCE1ED7}");

			//! Checks whether the specified device feature is present.
			virtual bool check_device_feature(DeviceFeature feature) = 0;

			//! Gets the alignment for the buffer data start location and size.
			virtual usize get_constant_buffer_data_alignment() = 0;

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

			//! Creates one new resource and allocates device memory for the resource.
			//! @param[in] desc The descriptor object.
			//! @param[in] optimized_clear_value The optional optimized clear value for a texture resource. Specify `nullptr` if this is a buffer
			//! resource or the resource does not have a optimized clear value.
			virtual R<Ref<IResource>> new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;

			//! Checks whether the given resources can share the same device memory.
			//! @param[in] descs The resource descriptors of resources being examined.
			//! @return Returns `true` if such resources can share the same device memory, returns `false` otherwise.
			virtual bool is_resources_aliasing_compatible(Span<const ResourceDesc> descs) = 0;

			//! Creates one aliasing resource that shares the same device memory with the existing resource.
			//! The user may create multiple aliasing resources with the same device memory, given that only one of them is active at any given time.
			//! The user should use aliasing barrier to switch the active resource between aliasing resources sharing the same device memory.
			virtual R<Ref<IResource>> new_aliasing_resource(IResource* existing_resource, const ResourceDesc& desc, const ClearValue* optimized_clear_value = nullptr) = 0;

			//! Allocates device memory that is capable of storing multiple resources specified, and creating multiple aliasing resources 
			//! that shares the same device memory.
			//! @param[in] descs The resource descriptors of resources that shares the same device memory.
			//! @param[in] optimized_clear_values The optimized clear values for each resource you want to create.
			//! 
			//! Every entry in the span can be `nullptr`, which indicates that the optimized clear value is not specified for that entry.
			//! 
			//! If `optimized_clear_values.size()` is smaller than `descs.size()`, the optimized clear values of first `descs.size()` enties will be specified, other enties 
			//! will have no optimized clear value specified.
			//! @param[out] out_resources Returns the creates resources. 
			//! 
			//! `out_resources.size()` must be greater than or equal to `descs.size()`, and the first `descs.size()` of `out_resources` will be filled.
			//! @remark The user can call `is_resources_aliasing_compatible` before calling `new_aliasing_resources` to check whether the given 
			//! resources can share the same device memory.
			virtual RV new_aliasing_resources(Span<const ResourceDesc> descs, Span<const ClearValue*> optimized_clear_values, Span<Ref<IResource>> out_resources) = 0;

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
			virtual R<Ref<IDescriptorSet>> new_descriptor_set(DescriptorSetDesc& desc) = 0;

			//! Creates one new command queue.
			virtual R<Ref<ICommandQueue>> new_command_queue(const CommandQueueDesc& desc) = 0;

			virtual R<Ref<IRenderTargetView>> new_render_target_view(IResource* resource, const RenderTargetViewDesc* desc = nullptr) = 0;

			virtual R<Ref<IDepthStencilView>> new_depth_stencil_view(IResource* resource, const DepthStencilViewDesc* desc = nullptr) = 0;
		
			virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) = 0;

			virtual R<Ref<IFence>> new_fence() = 0;

			//! Copies resource data between system memory and resource memory.
			//! @param[in] copies An array of resource copy operations to be performed. The user should
			//! batch resource copy operations as much as possible to improve performance.
			virtual RV copy_resource(Span<const ResourceCopyDesc> copies) = 0;
		};
	}
}