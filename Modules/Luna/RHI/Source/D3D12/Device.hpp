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
#include "../DXGI/Common.hpp"
#include "D3D12Common.hpp"
#include "d3d12.h"
#include "../../Device.hpp"
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/List.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
	namespace RHI
	{
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

		struct CommandQueue
		{
			CommandQueueDesc m_desc;
			ComPtr<ID3D12CommandQueue> m_command_queue;
			SpinLock m_lock;
		};

		//! @class Device
		struct Device : IDevice
		{
			lustruct("RHI::Device", "{038b4cb4-5e16-41a1-ad6f-7e2a49e2241e}");
			luiimpl();

			ComPtr<IDXGIAdapter> m_adapter;
			ComPtr<ID3D12Device> m_device;

			D3D12_FEATURE_DATA_D3D12_OPTIONS m_feature_options;
			D3D12_FEATURE_DATA_ARCHITECTURE m_architecture;

			//! Global heap for allocating descriptor sets.
			ShaderSourceDescriptorHeap m_cbv_srv_uav_heap;
			ShaderSourceDescriptorHeap m_sampler_heap;
			RenderTargetDescriptorHeap m_rtv_heap;
			RenderTargetDescriptorHeap m_dsv_heap;

			Vector<UniquePtr<CommandQueue>> m_command_queues;

			// Memory Allocator.
			ComPtr<D3D12MA::Allocator> m_allocator;

			~Device();

			R<UniquePtr<CommandQueue>> new_command_queue(const CommandQueueDesc& desc);
			RV init(IDXGIAdapter* adapter);
			
			virtual DeviceFeatureData check_feature(DeviceFeature feature) override;
			virtual void get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch) override;
			virtual R<Ref<IBuffer>> new_buffer(MemoryType memory_type, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_texture(MemoryType memory_type, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
			virtual bool is_resources_aliasing_compatible(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IDeviceMemory>> allocate_memory(MemoryType memory_type, Span<const BufferDesc> buffers, Span<const TextureDesc> textures) override;
			virtual R<Ref<IBuffer>> new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc) override;
			virtual R<Ref<ITexture>> new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value) override;
			virtual R<Ref<IPipelineLayout>> new_pipeline_layout(const PipelineLayoutDesc& desc) override;
			virtual R<Ref<IPipelineState>> new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc) override;
			virtual R<Ref<IPipelineState>> new_compute_pipeline_state(const ComputePipelineStateDesc& desc) override;
			virtual R<Ref<IDescriptorSetLayout>> new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc) override;
			virtual R<Ref<IDescriptorSet>> new_descriptor_set(const DescriptorSetDesc& desc) override;
			virtual u32 get_num_command_queues() override;
			virtual CommandQueueDesc get_command_queue_desc(u32 command_queue_index) override;
			virtual R<Ref<ICommandBuffer>> new_command_buffer(u32 command_queue_index) override;
			virtual R<f64> get_command_queue_timestamp_frequency(u32 command_queue_index) override;
			virtual R<Ref<IQueryHeap>> new_query_heap(const QueryHeapDesc& desc) override;
			virtual R<Ref<IFence>> new_fence() override;
			virtual R<Ref<ISwapChain>> new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc) override;
		};
	}
}