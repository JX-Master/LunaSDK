/*!
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
#include "D3D12Common.hpp"
#include "Device.hpp"
#include <Runtime/TSAssert.hpp>

namespace Luna
{
	namespace RHI
	{
		struct Resource : IResource
		{
			lustruct("RHI::Resource", "{dd9486e7-5195-4be3-96a4-b27c2e06bc80}");
			luiimpl();
			lutsassert_lock();

			Ref<Device> m_device;

			ComPtr<ID3D12Resource> m_res;

			ResourceDesc m_desc;

			//! One for each subresource, 0 if this resource does not have a global state.
			Vector<ResourceState> m_states;

			Resource() {}

			u32 count_subresources() const
			{
				if (m_desc.type == ResourceType::texture_3d)
				{
					return m_desc.mip_levels;
				}
				return m_desc.mip_levels * m_desc.depth_or_array_size;
			}
			
			RV init_as_committed(const ResourceDesc& desc, const ClearValue* optimized_clear_value);
			RV init_as_placed(ID3D12Heap* heap, UINT64 heap_offset, const ResourceDesc& desc, const ClearValue* optimized_clear_value);

			RV post_init();

			IDevice* get_device()
			{
				return m_device.as<IDevice>();
			}
			void set_name(const Name& name) { set_object_name(m_res.Get(), name); }

			ResourceDesc get_desc()
			{
				return m_desc;
			}

			/*static ResourceStateFlag global_state(u32 subresource)
			{
				if (m_states.empty())
				{
					switch (m_desc.access_type)
					{
					case EAccessType::gpu_local:
						return ResourceStateFlag::common;
					case EAccessType::upload:
						return ResourceStateFlag::generic_read;
					case EAccessType::readback:
						return ResourceStateFlag::copy_dest;
					default:
						lupanic();
						return ResourceStateFlag::common;
					}
				}
				else
				{
					lucheck(subresource < m_states.size());
					return m_states[subresource];
				}
			}*/

			RV map_subresource(u32 subresource, usize read_begin, usize read_end, void** out_data);
			void unmap_subresource(u32 subresource, usize write_begin, usize write_end);
		};
	}
}