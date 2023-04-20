/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Resource.hpp
* @author JXMaster
* @date 2023/4/19
*/
#pragma once
#include "ResourceHeap.hpp"

namespace Luna
{
	namespace RHI
	{
		struct BufferResource : IResource
		{
			lustruct("RHI::BufferResource", "{2CE2F6F7-9CCB-4DD5-848A-DBE27F8A8B7A}");

			VkBuffer m_buffer = VK_NULL_HANDLE;
			// If this is not VK_NULL_HANLDE, this resource is
			// created in committed mode. Otherwise, this 
			// resource is created in placed mode.
			VkDeviceMemory m_memory = VK_NULL_HANDLE;

			RV init_as_committed(const ResourceDesc& desc);
			RV init_as_placed(ResourceHeap* heap, const ResourceDesc& desc);
			~BufferResource();
		};

		struct TextureResource : IResource
		{
			lustruct("RHI::TextureResource", "{731F1D3C-2864-44A4-B380-CF03CBB7AFED}");
		
			
		};
	}
}