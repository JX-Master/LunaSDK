/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Memory.hpp
* @author JXMaster
* @date 2023/4/20
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct DeviceMemory : IDeviceMemory
		{
			lustruct("RHI::DeviceMemory", "{F99F86B6-3339-4C28-A82A-13B63ADAFBBC}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			MemoryType m_memory_type;
			VmaAllocation m_allocation = VK_NULL_HANDLE;
			VmaAllocationInfo m_allocation_info;
			u32 m_alignment;

			RV init(MemoryType memory_type, bool allow_aliasing, const VkMemoryRequirements& pVkMemoryRequirements);
			~DeviceMemory();

			virtual IDevice* get_device() override { return m_device; }
			virtual void set_name(const c8* name) override { m_name = name; }
			virtual MemoryType get_memory_type() override { return m_memory_type; }
			virtual u64 get_size() override { return m_allocation_info.size; }
		};
	}
}