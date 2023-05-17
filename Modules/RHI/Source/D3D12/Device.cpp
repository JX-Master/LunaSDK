/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Device.cpp
* @author JXMaster
* @date 2019/7/17
*/
#include "Device.hpp"
#include "PipelineState.hpp"
#include "Resource.hpp"
#include "DescriptorSet.hpp"
#include "ShaderInputLayout.hpp"
#include "DescriptorSetLayout.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "ResolveTargetView.hpp"
#include "QueryHeap.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "SwapChain.hpp"

namespace Luna
{
	namespace RHI
	{
		RV ShaderSourceDescriptorHeap::init(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
		{
			m_type = desc.Type;
			m_heap_size = desc.NumDescriptors;
			m_mutex = new_mutex();
			if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap))))
			{
				return BasicError::bad_platform_call();
			}
			m_cpu_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
			m_gpu_handle = m_heap->GetGPUDescriptorHandleForHeapStart();
			m_descriptor_size = device->GetDescriptorHandleIncrementSize(m_type);
			FreeRange range;
			range.offset = 0;
			range.size = m_heap_size;
			m_free_ranges.push_back(range);
			return ok;
		}
		void ShaderSourceDescriptorHeap::internal_free_descs(u32 offset, u32 count)
		{
			// Insert before this iterator. May be `end`.
			auto after = m_free_ranges.begin();
			while (after != m_free_ranges.end())
			{
				if (after->offset >= (offset + count)) break;
				++after;
			}
			if(after != m_free_ranges.begin())
			{
				// Merge to before.
				auto before = after;
				--before;
				if ((before->offset + before->size) == offset)
				{
					before->size += count;
					if (after != m_free_ranges.end() && (before->offset + before->size == after->offset))
					{
						// Merge before to after.
						before->size += after->size;
						m_free_ranges.erase(after);
					}
					return;
				}
			}
			if ((after != m_free_ranges.end()) && ((offset + count) == after->offset))
			{
				// Merge to after.
				after->offset = offset;
				after->size += count;
				return;
			}
			// Cannot merge, insert a new node.
			FreeRange range;
			range.offset = offset;
			range.size = count;
			m_free_ranges.insert(after, range);
		}
		u32 ShaderSourceDescriptorHeap::allocate_descs(u32 count)
		{
			luassert(count);
			MutexGuard guard(m_mutex);
			for (auto iter = m_free_ranges.begin(); iter != m_free_ranges.end(); ++iter)
			{
				if (iter->size >= count)
				{
					u32 ret = iter->offset;
					if (iter->size > count)
					{
						iter->size -= count;
						iter->offset += count;
					}
					else
					{
						m_free_ranges.erase(iter);
					}
					return ret;
				}
			}
			lupanic_msg("Out of descriptors.");
			return U32_MAX;
		}
		void ShaderSourceDescriptorHeap::free_descs(u32 offset, u32 count)
		{
			MutexGuard guard(m_mutex);
			internal_free_descs(offset, count);
		}
		void RenderTargetDescriptorHeap::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
		{
			m_device = device;
			m_type = type;
			m_mutex = new_mutex();
			m_descriptor_size = device->GetDescriptorHandleIncrementSize(type);
		}
		R<ComPtr<ID3D12DescriptorHeap>> RenderTargetDescriptorHeap::allocate_view()
		{
			MutexGuard guard(m_mutex);
			if (m_free_views.empty())
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc;
				desc.Type = m_type;
				desc.NodeMask = 0;
				desc.NumDescriptors = 1;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				ComPtr<ID3D12DescriptorHeap> ret;
				if (FAILED(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ret))))
				{
					return BasicError::bad_platform_call();
				}
				return ret;
			}
			else
			{
				ComPtr<ID3D12DescriptorHeap> ret = m_free_views.front();
				m_free_views.pop_front();
				return ret;
			}
		}
		void RenderTargetDescriptorHeap::free_view(ID3D12DescriptorHeap* view)
		{
			MutexGuard guard(m_mutex);
			ComPtr<ID3D12DescriptorHeap> v{ view };
			m_free_views.push_back(move(v));
		}
		Device::~Device()
		{
#ifdef LUNA_DEBUG_LEVEL_DEBUG
			ID3D12DebugDevice* pDebugDevice = nullptr;
			HRESULT hr = m_device->QueryInterface(__uuidof(ID3D12DebugDevice), reinterpret_cast<void**>(&pDebugDevice));
			if (SUCCEEDED(hr))
			{
				hr = pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
			}
			if (pDebugDevice != nullptr)
			{
				pDebugDevice->Release();
			}
#endif
		}
		inline bool is_render_target_or_depth_stencil_texture(const TextureDesc& desc)
		{
			return test_flags(desc.usages, TextureUsageFlag::render_target) || test_flags(desc.usages, TextureUsageFlag::depth_stencil);
		}
		R<UniquePtr<CommandQueue>> Device::new_command_queue(const CommandQueueDesc& desc)
		{
			UniquePtr<CommandQueue> ret(memnew<CommandQueue>());
			lutry
			{
				ret->m_desc = desc;
				D3D12_COMMAND_QUEUE_DESC d{};
				d.Type = encode_command_queue_type(desc.type);
				d.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				d.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				d.NodeMask = 0;
				luexp(encode_hresult(m_device->CreateCommandQueue(&d, IID_PPV_ARGS(&ret->m_command_queue))));
			}
			lucatchret;
			return ret;
		}
		RV Device::init(IDXGIAdapter* adapter)
		{
			lutry
			{
				m_adapter = adapter;
				luexp(encode_hresult(::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))));
				D3D12MA::ALLOCATOR_DESC allocator_desc{};
				allocator_desc.pDevice = m_device.Get();
				allocator_desc.pAdapter = adapter;
				luexp(encode_hresult(D3D12MA::CreateAllocator(&allocator_desc, &m_allocator)));

				luexp(encode_hresult(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_feature_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS))));
				m_architecture.NodeIndex = 0;
				luexp(encode_hresult(m_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &m_architecture, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE))));
				{
					// Create 1 direct queue, 2 compute queues and 2 copy queues.
					CommandQueueDesc desc;
					desc.type = CommandQueueType::graphics;
					desc.flags = CommandQueueFlags::presenting;
					UniquePtr<CommandQueue> queue;
					luset(queue, new_command_queue(desc));
					m_command_queues.push_back(move(queue));
					desc.type = CommandQueueType::compute;
					desc.flags = CommandQueueFlags::none;
					luset(queue, new_command_queue(desc));
					m_command_queues.push_back(move(queue));
					luset(queue, new_command_queue(desc));
					m_command_queues.push_back(move(queue));
					desc.type = CommandQueueType::copy;
					luset(queue, new_command_queue(desc));
					m_command_queues.push_back(move(queue));
					luset(queue, new_command_queue(desc));
					m_command_queues.push_back(move(queue));
				}
				{
					D3D12_DESCRIPTOR_HEAP_DESC desc;
					desc.NodeMask = 0;
					desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
					desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
					desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
					auto r = m_cbv_srv_uav_heap.init(m_device.Get(), desc);
					if (failed(r))
					{
						return r.errcode();
					}
					desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
					desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
					r = m_sampler_heap.init(m_device.Get(), desc);
					if (failed(r))
					{
						return r.errcode();
					}
					m_rtv_heap.init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
					m_dsv_heap.init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
				}

			}
			lucatchret;
			return ok;
		}
		bool Device::check_device_feature(DeviceFeature feature)
		{
			switch (feature)
			{
			case DeviceFeature::unbound_descriptor_array: return true;
			}
			return false;
		}
		usize Device::get_uniform_buffer_data_alignment()
		{
			return 256;
		}
		void Device::get_texture_data_placement_info(u32 width, u32 height, u32 depth, Format format,
				u64* size, u64* alignment, u64* row_pitch, u64* slice_pitch)
		{
			u64 bpp = bits_per_pixel(format);
			u64 row = align_upper(bpp * width / 8, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			if(row_pitch) *row_pitch = row;
			u64 slice = row * height;
			if(slice_pitch) *slice_pitch = slice;
			u64 sz = align_upper(slice * depth, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
			if(size) *size = sz;
			if(alignment) *alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
		}
		R<Ref<IBuffer>> Device::new_buffer(const BufferDesc& desc)
		{
			Ref<BufferResource> res = new_object<BufferResource>();
			res->m_device = this;
			RV r = res->init_as_committed(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return res;
		}
		R<Ref<ITexture>> Device::new_texture(const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<TextureResource> res = new_object<TextureResource>();
			res->m_device = this;
			RV r = res->init_as_committed(desc, optimized_clear_value);
			if (!r.valid())
			{
				return r.errcode();
			}
			return res;
		}
		bool Device::is_resources_aliasing_compatible(Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
		{
			usize num_descs = buffers.size() + textures.size();
			if (num_descs <= 1) return true;
			ResourceHeapType heap_type = buffers.empty() ? textures[0].heap_type : buffers[0].heap_type;
			for (auto& desc : buffers)
			{
				if (desc.heap_type != heap_type) return false;
			}
			for (auto& desc : textures)
			{
				if (desc.heap_type != heap_type) return false;
			}
			if (m_feature_options.ResourceHeapTier >= D3D12_RESOURCE_HEAP_TIER_2)
			{
				// heaps can support resources from all three categories 
				// (Buffers, Non-render target & non-depth stencil textures, Non-render target & non-depth stencil textures).
				return true;
			}
			else
			{
				// heaps can only support resources from a single resource category.
				if (!buffers.empty() && !textures.empty())
				{
					return false;
				}
				// Buffers can be created in the same heap.
				if (!buffers.empty()) return true;
				bool non_rt_texture_present = false;
				bool rt_texture_present = false;
				for (auto& desc : textures)
				{
					if (is_render_target_or_depth_stencil_texture(desc))
					{
						rt_texture_present = true;
					}
					else
					{
						non_rt_texture_present = true;
					}
				}
				return !(rt_texture_present && non_rt_texture_present);
			}
		}
		R<Ref<IDeviceMemory>> Device::allocate_memory(Span<const BufferDesc> buffers, Span<const TextureDesc> textures)
		{
			Ref<IDeviceMemory> ret;
			lutry
			{
				if (buffers.empty() && textures.empty()) return BasicError::bad_arguments();
				if (!is_resources_aliasing_compatible(buffers, textures)) return BasicError::not_supported();
				D3D12MA::ALLOCATION_DESC allocation_desc{};
				allocation_desc.HeapType = encode_heap_type(buffers.empty() ? textures[0].heap_type : buffers[0].heap_type);
				allocation_desc.ExtraHeapFlags = D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
				D3D12_RESOURCE_DESC* descs = (D3D12_RESOURCE_DESC*)alloca(sizeof(D3D12_RESOURCE_DESC) * (buffers.size() + textures.size()));
				u32 i = 0;
				for (auto& buffer : buffers)
				{
					descs[i] = encode_buffer_desc(buffer);
					allocation_desc.ExtraHeapFlags &= ~D3D12_HEAP_FLAG_DENY_BUFFERS;
					++i;
				}
				for (auto& texture : textures)
				{
					descs[i] = encode_texture_desc(texture);
					if (is_render_target_or_depth_stencil_texture(texture))
					{
						allocation_desc.ExtraHeapFlags &= ~D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
					}
					else
					{
						allocation_desc.ExtraHeapFlags &= ~D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
					}
					++i;
				}
				D3D12_RESOURCE_ALLOCATION_INFO allocation_info = m_device->GetResourceAllocationInfo(0, i, descs);
				auto memory = new_object<DeviceMemory>();
				memory->m_device = this;
				luexp(memory->init(allocation_desc, allocation_info));
			}
			lucatchret;
			return ret;
		}
		R<Ref<IBuffer>> Device::new_aliasing_buffer(IDeviceMemory* device_memory, const BufferDesc& desc)
		{
			Ref<IBuffer> ret;
			lutry
			{
				DeviceMemory* memory = cast_object<DeviceMemory>(device_memory->get_object());
				Ref<BufferResource> res = new_object<BufferResource>();
				res->m_device = this;
				luexp(res->init_as_aliasing(desc, memory));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<ITexture>> Device::new_aliasing_texture(IDeviceMemory* device_memory, const TextureDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<ITexture> ret;
			lutry
			{
				DeviceMemory * memory = cast_object<DeviceMemory>(device_memory->get_object());
				Ref<TextureResource> res = new_object<TextureResource>();
				res->m_device = this;
				luexp(res->init_as_aliasing(desc, memory, optimized_clear_value));
				ret = res;
			}
			lucatchret;
			return ret;
		}
		R<Ref<IShaderInputLayout>> Device::new_shader_input_layout(const ShaderInputLayoutDesc& desc)
		{
			Ref<ShaderInputLayout> slayout = new_object<ShaderInputLayout>();
			slayout->m_device = this;
			lutry
			{
				luexp(slayout->init(desc));
			}
			lucatchret;
			return slayout;
		}
		R<Ref<IPipelineState>> Device::new_graphics_pipeline_state(const GraphicsPipelineStateDesc& desc)
		{
			Ref<PipelineState> s = new_object<PipelineState>(this);
			if (!s->init_graphic(desc))
			{
				return BasicError::failure();
			}
			return s;
		}
		R<Ref<IPipelineState>> Device::new_compute_pipeline_state(const ComputePipelineStateDesc& desc)
		{
			Ref<PipelineState> s = new_object<PipelineState>(this);
			if (!s->init_compute(desc))
			{
				return BasicError::failure();
			}
			return s;
		}
		R<Ref<IDescriptorSetLayout>> Device::new_descriptor_set_layout(const DescriptorSetLayoutDesc& desc)
		{
			Ref<DescriptorSetLayout> ret = new_object<DescriptorSetLayout>();
			ret->m_device = this;
			ret->init(desc);
			return ret;
		}
		R<Ref<IDescriptorSet>> Device::new_descriptor_set(const DescriptorSetDesc& desc)
		{
			Ref<DescriptorSet> ds = new_object<DescriptorSet>();
			ds->m_device = this;
			RV r = ds->init(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return ds;
		}
		u32 Device::get_num_command_queues()
		{
			return (u32)m_command_queues.size();
		}
		CommandQueueDesc Device::get_command_queue_desc(u32 command_queue_index)
		{
			return m_command_queues[command_queue_index]->m_desc;
		}
		R<Ref<ICommandBuffer>> Device::new_command_buffer(u32 command_queue_index)
		{
			Ref<CommandBuffer> buffer = new_object<CommandBuffer>();
			lutry
			{
				buffer->m_device = this;
				buffer->m_queue = command_queue_index;
				luexp(buffer->init());
			}
			lucatchret;
			return buffer;
		}
		R<f64> Device::get_command_queue_timestamp_frequency(u32 command_queue_index)
		{
			UINT64 t;
			HRESULT hr = m_command_queues[command_queue_index]->m_command_queue->GetTimestampFrequency(&t);
			if (FAILED(hr)) return encode_hresult(hr).errcode();
			return (f64)t;
		}
		R<Ref<IRenderTargetView>> Device::new_render_target_view(ITexture* texture, const RenderTargetViewDesc* desc)
		{
			lucheck_msg(texture, "\"texture\" was nullptr");
			Ref<RenderTargetView> view = new_object<RenderTargetView>();
			view->m_device = this;
			RV r = view->init(texture, desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return view;
		}
		R<Ref<IDepthStencilView>> Device::new_depth_stencil_view(ITexture* texture, const DepthStencilViewDesc* desc)
		{
			lucheck_msg(texture, "\"texture\" was nullptr");
			Ref<DepthStencilView> view = new_object<DepthStencilView>();
			view->m_device = this;
			RV r = view->init(texture, desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return view;
		}
		R<Ref<IResolveTargetView>> Device::new_resolve_target_view(ITexture* texture, const ResolveTargetViewDesc* desc)
		{
			lucheck_msg(texture, "\"texture\" was nullptr");
			Ref<ResolveTargetView> view = new_object<ResolveTargetView>();
			view->m_device = this;
			RV r = view->init(texture, desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return view;
		}
		R<Ref<IQueryHeap>> Device::new_query_heap(const QueryHeapDesc& desc)
		{
			Ref<QueryHeap> heap = new_object<QueryHeap>();
			heap->m_device = this;
			RV r = heap->init(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return heap;
		}
		R<Ref<IFence>> Device::new_fence()
		{
			Ref<Fence> fence = new_object<Fence>();
			fence->m_device = this;
			RV r = fence->init();
			if (!r.valid())
			{
				return r.errcode();
			}
			return fence;
		}
		R<Ref<ISwapChain>> Device::new_swap_chain(u32 command_queue_index, Window::IWindow* window, const SwapChainDesc& desc)
		{
			Ref<SwapChain> r = new_object<SwapChain>();
			lutry
			{
				r->m_device = this;
				luexp(r->init(command_queue_index, window, desc));
			}
			lucatchret;
			return r;
		}
	}
}