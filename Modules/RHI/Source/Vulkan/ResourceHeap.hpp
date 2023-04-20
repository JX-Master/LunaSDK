/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResourceHeap.hpp
* @author JXMaster
* @date 2023/4/19
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ResourceHeap : IResourceHeap
		{
			lustruct("RHI::ResourceHeap", "{4197FC92-D885-4376-A159-70F9D9EC5EAD}");

			Ref<Device> m_device;
			VkDeviceMemory m_memory = VK_NULL_HANDLE;

			RV init(const ResourceHeapDesc& desc);
			~ResourceHeap();
		};
	}
}