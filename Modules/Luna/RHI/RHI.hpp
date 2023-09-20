/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.hpp
* @author JXMaster
* @date 2022/4/12
* @brief Render Hardware Interface for Luna SDK.
*/
#pragma once
#include "Device.hpp"
#ifndef LUNA_RHI_API
#define LUNA_RHI_API
#endif
namespace Luna
{
    namespace RHI
    {
		//! The graphic adapter type. The adapter type is used only as a hint when choosing adpaters, it
		//! does not guarantee any GPU architecture and feature. The user should check feature support
		//! manually.
		enum class AdapterType
		{
			//! The adapter type cannot be represented by other adapter types.
			unknwon = 0,
			//! This adapter is a integrated GPU, typically as part of CPU.
			integrated_gpu = 1,
			//! This adapter is a discrete GPU, typically as video card.
			discrete_gpu = 2,
			//! This adapter is a virtualized GPU in a virtualization environment.
			virtual_gpu = 3,
			//! This adapter is software emulated, typically for testing API features.
			software = 4,
		};

		//! Describes an adapter to system.
		struct AdapterDesc
		{
			//! The name of the adapter.
			char name[256];
			//! The amount of GPU local (dedicated) memory installed to this adapter. 
			//! The GPU dedicated memory may be allocated from video memory for dedicated video card,
			//! or part of system memory for embedded GPU. The memory can only be read/write by GPU,
			//! CPU does not have access to it.
			u64 local_memory;
			//! The amount of memory shared from system main memory for this adapter. The memory is allocated
			//! in system memory, and can be accessed by both CPU and GPU. GPU accesses this memory usually
			//! through PCIes and with cache enabled so it is much slower than accessing local memory.
			u64 shared_memory;
			//! The type of the adapter.
			AdapterType type;
		};

		//! Gets the number of adapters in the current platform.
		LUNA_RHI_API u32 get_num_adapters();

		//! Gets the description of the specified adapter.
		//! @param[in] adapter_index The index of the adapter. The adapter index must in range [0, get_num_adapters()).
		//! @return Returns the descriptor object.
		LUNA_RHI_API AdapterDesc get_adapter_desc(u32 adapter_index);

		enum class PowerPreference
		{
			//! No particular power preference. This may result in an adapter that the 
			//! highest priority defined by system settings.
			undefined,
			//! Prefers a power saving adapter instead of a high-performance one. This 
			//! may result in an integrated GPU in a multi-GPU system.
			low_power,
			//! Prefers a high-performance adapter instead of a power saving one. This 
			//! may result in an dedicated GPU in a multi-GPU system.
			high_performance,
		};

		//! Gets the adapter index of the adapter that is preferred for creating the device on the current platform.
		//! @param[in] power_preference The power preference of the adapter being chosen.
		//LUNA_RHI_API u32 get_preferred_adapter(PowerPreference power_preference = PowerPreference::undefined);

		//! Creates one device using the specified adapter.
		//! @param[in] adapter_index The adapter index chosed for the device.
		LUNA_RHI_API R<Ref<IDevice>> new_device(u32 adapter_index);

		//! Gets the main device.
		//! If the main device is not set by `set_main_device`, this will create one device based on the adapters
		//! returned by `get_preferred_device_adapter`, set the device as the main device and return the device.
		LUNA_RHI_API IDevice* get_main_device();

		//! Sets the main device. This call replaces the prior main device.
		//LUNA_RHI_API void set_main_device(IDevice* device);

		enum class APIType : u8
		{
			d3d12,
			vulkan,
			metal,
		};

		//! Gets the underlying graphic API type.
		LUNA_RHI_API APIType get_current_platform_api_type();
    }

	namespace RHIError
	{
		LUNA_RHI_API errcat_t errtype();

		LUNA_RHI_API ErrCode device_hung();
		LUNA_RHI_API ErrCode device_reset();
		LUNA_RHI_API ErrCode device_removed();
		LUNA_RHI_API ErrCode driver_internal_error();
		LUNA_RHI_API ErrCode frame_statistics_disjoint();
		//! The swap chain is no longer compatible with the surface and should be reset.
		LUNA_RHI_API ErrCode swap_chain_out_of_date();
	}
}