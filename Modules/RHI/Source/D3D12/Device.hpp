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

#ifdef LUNA_RHI_D3D12

#include "../DXGI/Common.hpp"
#include "d3d12.h"
#include "../../Device.hpp"
#include <Runtime/Mutex.hpp>
#include <Runtime/RingDeque.hpp>
#include <Runtime/List.hpp>
#include <Runtime/SpinLock.hpp>

namespace Luna
{
	namespace RHI
	{
		class ResourceHeap;
		class ViewTableHeap;
		struct ViewTableHeapDesc;
		class PipelineState;
		class CommandAllocator;
		class CommandList;
		class CommandQueue;
		class Fence;
		class CommittedResource;
		class UploadBuffer;

		struct ShaderSourceDescriptorHeap
		{
			ComPtr<ID3D12DescriptorHeap> m_heap;
			Ref<IMutex> m_mutex;
			D3D12_DESCRIPTOR_HEAP_TYPE m_type;
			u32 m_heap_size;
			D3D12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;
			D3D12_GPU_DESCRIPTOR_HANDLE m_gpu_handle;
			u32 m_descriptor_size;

			struct FreeRange
			{
				u32 offset;
				u32 size;
			};

			List<FreeRange> m_free_ranges;

			RV init(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

			void internal_free_descs(u32 offset, u32 count);

			u32 allocate_descs(u32 count);
			void free_descs(u32 offset, u32 count);
		};

		struct RenderTargetDescriptorHeap
		{
			ID3D12Device* m_device;
			D3D12_DESCRIPTOR_HEAP_TYPE m_type;
			Ref<IMutex> m_mutex;
			RingDeque<ComPtr<ID3D12DescriptorHeap>> m_free_views;
			u32 m_descriptor_size;

			void init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type);
			R<ComPtr<ID3D12DescriptorHeap>> allocate_view();
			void free_view(ID3D12DescriptorHeap* view);
		};

		//! @class Device
		struct Device : IDevice
		{
			lustruct("RHI::Device", "{038b4cb4-5e16-41a1-ad6f-7e2a49e2241e}");
			luiimpl()

			ComPtr<ID3D12Device> m_device;

			//! The queue for data upload/readback for shared resources.
			ComPtr<ID3D12CommandQueue> m_internal_copy_queue;

			D3D12_FEATURE_DATA_D3D12_OPTIONS m_feature_options;
			D3D12_FEATURE_DATA_ARCHITECTURE m_architecture;

			//! Global heap for allocating descriptor sets.
			ShaderSourceDescriptorHeap m_cbv_srv_uav_heap;
			ShaderSourceDescriptorHeap m_sampler_heap;
			RenderTargetDescriptorHeap m_rtv_heap;
			RenderTargetDescriptorHeap m_dsv_heap;

			SpinLock m_swap_chain_shared_resource_lock;
			bool m_swap_chain_shared_resource_initialized;
			ComPtr<ID3D12Resource> m_swap_chain_vert_buf;
			ComPtr<ID3DBlob> m_swap_chain_root_signature_data;

			Device() :
				m_swap_chain_shared_resource_initialized(false) {}
			~Device();

			RV init(ID3D12Device* dev);

			usize get_texture_data_pitch_alignment();
			usize get_texture_data_placement_alignment();
			usize get_constant_buffer_data_alignment();
			void  get_texture_subresource_buffer_placement(u32 width, u32 height, u32 depth, Format format,
				usize* row_pitch, usize* slice_pitch, usize* res_pitch);
			usize get_resource_size(const ResourceDesc& desc, usize* out_alignment);
			R<Ref<IResource>> new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value);
			R<Ref<IResourceHeap>> new_resource_heap(const ResourceHeapDesc& desc);
			R<Ref<IShaderInputLayout>> new_shader_input_layout(const ShaderInputLayoutDesc& desc);
			R<Ref<IPipelineState>> new_graphic_pipeline_state(const GraphicPipelineStateDesc& desc);
			R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc);
			R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc);
			R<Ref<IDescriptorSet>> new_descriptor_set(DescriptorSetDesc& desc);
			R<Ref<ICommandQueue>> new_command_queue(CommandQueueType type);
			R<Ref<IRenderTargetView>> new_render_target_view(IResource* resource, const RenderTargetViewDesc* desc);
			R<Ref<IDepthStencilView>> new_depth_stencil_view(IResource* resource, const DepthStencilViewDesc* desc);
			R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc);
		};

		inline D3D12_HEAP_PROPERTIES encode_heap_properties(Device* device, ResourceHeapType heap_type)
		{
			D3D12_HEAP_PROPERTIES hp;
			hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			hp.CreationNodeMask = 0;
			hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			switch (heap_type)
			{
			case ResourceHeapType::local:
				hp.Type = D3D12_HEAP_TYPE_DEFAULT;
				break;
			case ResourceHeapType::shared:
				if (device->m_architecture.UMA)
				{
					hp.Type = D3D12_HEAP_TYPE_CUSTOM;
					D3D12_HEAP_PROPERTIES heap_properties = device->m_device->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_READBACK);
					hp.CPUPageProperty = heap_properties.CPUPageProperty;
					hp.MemoryPoolPreference = heap_properties.MemoryPoolPreference;
				}
				else
				{
					hp.Type = D3D12_HEAP_TYPE_DEFAULT;
				}
				break;
			case ResourceHeapType::shared_upload:
				if (device->m_architecture.UMA)
				{
					hp.Type = D3D12_HEAP_TYPE_CUSTOM;
					D3D12_HEAP_PROPERTIES heap_properties = device->m_device->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_UPLOAD);
					hp.CPUPageProperty = heap_properties.CPUPageProperty;
					hp.MemoryPoolPreference = heap_properties.MemoryPoolPreference;
				}
				else
				{
					hp.Type = D3D12_HEAP_TYPE_DEFAULT;
				}
				break;
			case ResourceHeapType::readback:
				hp.Type = D3D12_HEAP_TYPE_READBACK;
				break;
			case ResourceHeapType::upload:
				hp.Type = D3D12_HEAP_TYPE_UPLOAD;
				break;
			default:
				lupanic();
				break;
			}
			hp.VisibleNodeMask = 0;
			return hp;
		}
	}
}

#endif