/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Resource.hpp
* @author JXMaster
* @date 2019/8/10
* @brief D3D12 implementation of Resource Object
*/
#pragma once

#ifdef LUNA_RHI_D3D12
#include "D3D12Common.hpp"
#include "Device.hpp"
#include <Runtime/TSAssert.hpp>

namespace Luna
{
	namespace RHI
	{
		struct Resource : IResource
		{
			lustruct("RHI::D3D12::Resource", "{dd9486e7-5195-4be3-96a4-b27c2e06bc80}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;

			ComPtr<ID3D12Resource> m_res;

			struct AffiliateResource
			{
				ComPtr<ID3D12Resource> m_res;
				void* m_mapped;
				u64 m_size;
				u64 m_width;
				u32 m_height;
				u32 m_depth;
				u32 m_ref_count;
				AffiliateResource() : 
					m_mapped(nullptr),
					m_size(0),
					m_width(1),
					m_height(1),
					m_depth(1),
					m_ref_count(0) {}
			};

			//! Used for CPU mapping for shared and shared_upload typed resource, 
			//! when the UMA is not supportted.
			//! One affiliarte resource for every subresource, so they can be mapped independently.
			Vector<AffiliateResource> m_affiliate_resources;

			// The following resources are created and shared if m_affiliate_resources is not empty.
			ComPtr<ID3D12CommandAllocator> m_copy_ca;
			ComPtr<ID3D12Fence> m_copy_fence;
			HANDLE m_copy_event;
			u64 m_copy_event_value;

			ResourceDesc m_desc;

			//! One for each subresource, 0 if this resource does not have a global state.
			Vector<ResourceState> m_states;

			Resource() :
				m_copy_event(NULL) {}

			~Resource()
			{
				if (m_copy_event)
				{
					::CloseHandle(m_copy_event);
					m_copy_event = NULL;
				}
			}

			u32 count_subresources() const
			{
				if (m_desc.type == ResourceType::texture_3d)
				{
					return m_desc.mip_levels;
				}
				return m_desc.mip_levels * m_desc.depth_or_array_size;
			}

			void set_desc(const ResourceDesc& desc);
			
			RV init_as_committed(const ResourceDesc& desc, const ClearValue* optimized_clear_value);
			RV init_as_placed(ID3D12Heap* heap, UINT64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value);

			RV post_init();

			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}

			ResourceDesc get_desc()
			{
				return m_desc;
			}

			/*static ResourceState global_state(u32 subresource)
			{
				if (m_states.empty())
				{
					switch (m_desc.access_type)
					{
					case EAccessType::gpu_local:
						return ResourceState::common;
					case EAccessType::upload:
						return ResourceState::generic_read;
					case EAccessType::readback:
						return ResourceState::copy_dest;
					default:
						lupanic();
						return ResourceState::common;
					}
				}
				else
				{
					lucheck(subresource < m_states.size());
					return m_states[subresource];
				}
			}*/

			RV map_subresource(u32 subresource, bool load_data, void** out_data);
			void unmap_subresource(u32 subresource, bool store_data);
			RV read_subresource(void* dest, u32 dest_row_pitch, u32 dest_depth_pitch, u32 subresource, const BoxU& read_box);
			RV write_subresource(u32 subresource, const void* src, u32 src_row_pitch, u32 src_depth_pitch, const BoxU& write_box);
		};
	}
}

#endif