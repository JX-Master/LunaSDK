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
		RV DeviceMemory::init(const D3D12MA::ALLOCATION_DESC& allocation_desc, const D3D12_RESOURCE_ALLOCATION_INFO& allocation_info)
		{
			return encode_hresult(m_device->m_allocator->AllocateMemory(&allocation_desc, &allocation_info, &m_allocation));
		}
	}
}