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

#ifdef LUNA_RHI_D3D12

#include "ResourceHeap.hpp"
#include "PipelineState.hpp"
#include "CommandQueue.hpp"
#include "Resource.hpp"
#include "DescriptorSet.hpp"
#include "ShaderInputLayout.hpp"
#include "DescriptorSetLayout.hpp"
#include "RenderTargetView.hpp"
#include "DepthStencilView.hpp"
#include "QueryHeap.hpp"

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
			auto after = m_free_ranges.begin();
			auto before = after;
			++after;
			while (after != m_free_ranges.end())
			{
				if(after->offset >= (offset + count)) break;
				before = after;
				++after;
			}
			if ((before->offset + before->size) == offset)
			{
				// Merge to before.
				before->size += count;
				if (after != m_free_ranges.end() && (before->offset + before->size == after->offset))
				{
					// Merge before to after.
					before->size += after->size;
					m_free_ranges.erase(after);
				}
			}
			else if((offset + count) == after->offset)
			{
				// Merge to after.
				after->offset -= count;
				after->size += count;
			}
			else
			{
				// Cannot merge, insert a new node.
				FreeRange range;
				range.offset = offset;
				range.size = count;
				m_free_ranges.insert(after, range);
			}
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

		RV Device::init(ID3D12Device* dev)
		{
			m_device = dev;
			HRESULT hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_feature_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
			if (FAILED(hr)) return BasicError::bad_platform_call();
			m_architecture.NodeIndex = 0;
			hr = m_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &m_architecture, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE));
			if (FAILED(hr)) return BasicError::bad_platform_call();
			D3D12_COMMAND_QUEUE_DESC desc;
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 0;
			if (FAILED(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_internal_copy_queue))))
			{
				return BasicError::bad_platform_call();
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
			return ok;
		}
		usize Device::get_texture_data_pitch_alignment()
		{
			return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
		}
		usize Device::get_texture_data_placement_alignment()
		{
			return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
		}
		usize Device::get_constant_buffer_data_alignment()
		{
			return 256;
		}
		void Device::get_texture_subresource_buffer_placement(u32 width, u32 height, u32 depth, Format format,
			usize* row_pitch, usize* slice_pitch, usize* res_pitch)
		{
			u64 numBytes = 0;
			u64 rowBytes = 0;
			u64 numRows = 0;

			bool bc = false;
			bool packed = false;
			bool planar = false;
			usize bpe = 0;
			DXGI_FORMAT fmt = encode_pixel_format(format);
			switch (fmt)
			{
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				bc = true;
				bpe = 8;
				break;

			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				bc = true;
				bpe = 16;
				break;

			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_YUY2:
				packed = true;
				bpe = 4;
				break;

			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				packed = true;
				bpe = 8;
				break;

			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
			case DXGI_FORMAT_P208:
				planar = true;
				bpe = 2;
				break;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
				planar = true;
				bpe = 4;
				break;

			default:
				break;
			}

			if (bc)
			{
				u64 numBlocksWide = 0;
				if (width > 0)
				{
					numBlocksWide = max<u64>(1u, (u64(width) + 3u) / 4u);
				}
				u64 numBlocksHigh = 0;
				if (height > 0)
				{
					numBlocksHigh = max<u64>(1u, (u64(height) + 3u) / 4u);
				}
				rowBytes = numBlocksWide * bpe;
				numRows = numBlocksHigh;
				numBytes = rowBytes * numBlocksHigh;
			}
			else if (packed)
			{
				rowBytes = ((u64(width) + 1u) >> 1) * bpe;
				numRows = u64(height);
				numBytes = rowBytes * height;
			}
			else if (fmt == DXGI_FORMAT_NV11)
			{
				rowBytes = ((u64(width) + 3u) >> 2) * 4u;
				numRows = u64(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
				numBytes = rowBytes * numRows;
			}
			else if (planar)
			{
				rowBytes = ((u64(width) + 1u) >> 1) * bpe;
				numBytes = (rowBytes * u64(height)) + ((rowBytes * u64(height) + 1u) >> 1);
				numRows = height + ((u64(height) + 1u) >> 1);
			}
			else
			{
				usize bpp = bits_per_pixel(format);
				if (!bpp)
				{
					if (row_pitch)
					{
						*row_pitch = 0;
					}
					if (slice_pitch)
					{
						*slice_pitch = 0;
					}
					if (res_pitch)
					{
						*res_pitch = 0;
					}
					return;
				}
				rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
				numRows = uint64_t(height);
				numBytes = rowBytes * height;
			}
			if (row_pitch)
			{
				*row_pitch = (usize)rowBytes;
			}
			if (slice_pitch)
			{
				*slice_pitch = (usize)numBytes;
			}
			if (res_pitch)
			{
				*res_pitch = (usize)(numBytes * depth);
			}
		}
		u64 Device::get_resource_size(const ResourceDesc& desc, u64* out_alignment)
		{
			D3D12_RESOURCE_DESC res_desc = encode_resource_desc(desc);
			D3D12_RESOURCE_ALLOCATION_INFO info = m_device->GetResourceAllocationInfo(0, 1, &res_desc);
			if (out_alignment) *out_alignment = info.Alignment;
			return info.SizeInBytes;
		}
		R<Ref<IResource>> Device::new_resource(const ResourceDesc& desc, const ClearValue* optimized_clear_value)
		{
			Ref<Resource> res = new_object<Resource>();
			res->m_device = this;
			RV r = res->init_as_committed(desc, optimized_clear_value);
			if (!r.valid())
			{
				return r.errcode();
			}
			return res;
		}
		R<Ref<IResourceHeap>> Device::new_resource_heap(const ResourceHeapDesc& desc)
		{
			Ref<ResourceHeap> heap = new_object<ResourceHeap>();
			heap->m_device = this;
			RV r = heap->init(desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return heap;
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
		R<Ref<IDescriptorSet>> Device::new_descriptor_set(DescriptorSetDesc& desc)
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
		R<Ref<ICommandQueue>> Device::new_command_queue(CommandQueueType type)
		{
			Ref<CommandQueue> q = new_object<CommandQueue>(this);
			RV r = q->init(type);
			if (!r.valid())
			{
				return r.errcode();
			}
			return q;
		}
		R<Ref<IRenderTargetView>> Device::new_render_target_view(IResource* res, const RenderTargetViewDesc* desc)
		{
			lucheck_msg(res, "\"res\" was nullptr");
			Ref<RenderTargetView> view = new_object<RenderTargetView>();
			view->m_device = this;
			RV r = view->init(res, desc);
			if (!r.valid())
			{
				return r.errcode();
			}
			return view;
		}
		R<Ref<IDepthStencilView>> Device::new_depth_stencil_view(IResource* res, const DepthStencilViewDesc* desc)
		{
			lucheck_msg(res, "\"res\" was nullptr");
			Ref<DepthStencilView> view = new_object<DepthStencilView>();
			view->m_device = this;
			RV r = view->init(res, desc);
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
	}
}

#endif