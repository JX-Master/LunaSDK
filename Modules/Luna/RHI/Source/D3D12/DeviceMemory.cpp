/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file DeviceMemory.cpp
* @author JXMaster
* @date 2023/5/16
*/
#include "DeviceMemory.hpp"

namespace Luna
{
	namespace RHI
	{
		RV DeviceMemory::init(MemoryType memory_type, const D3D12MA::ALLOCATION_DESC& allocation_desc, const D3D12_RESOURCE_ALLOCATION_INFO& allocation_info)
		{
			m_memory_type = memory_type;
			auto hr = m_device->m_allocator->AllocateMemory(&allocation_desc, &allocation_info, &m_allocation);
			if(FAILED(hr)) return encode_hresult(hr);
#ifdef LUNA_MEMORY_PROFILER_ENABLED
			memory_profiler_allocate(m_allocation.Get(), get_size());
			memory_profiler_set_memory_domain(m_allocation.Get(), "GPU", 3);
			memory_profiler_set_memory_type(m_allocation.Get(), "Aliasing Memory", 15);
#endif
			return ok;
		}
		DeviceMemory::~DeviceMemory()
		{
#ifdef LUNA_MEMORY_PROFILER_ENABLED
			memory_profiler_deallocate(m_allocation.Get());
#endif
		}
	}
}