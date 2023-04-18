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
#include "SwapChain.hpp"
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
		enum class GraphicAdapterType
		{
			//! The adapter type cannot be represented by other adapter types.
			unknwon = 0,
			//! This adapter is a integrated GPU, typically as part of CPU.
			integrated_gpu = 1,
			//! This adapter is a dedicated GPU, typically as video card.
			discrete_gpu = 2,
			//! This adapter is a virtualized GPU in a virtualization environment.
			virtual_gpu = 3,
			//! This adapter is software emulated, typically for testing API features.
			software = 4,
		};

		//! Describes an adapter to system.
		struct GraphicAdapterDesc
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
			GraphicAdapterType type;
		};

		//! Gets the descriptor object of the specified adapter.
		//! @param[in] index The index of the adapter.
		//! @return Returns the descriptor object. Returns `BasicError::not_found` if the adpator at the specified index is not found.
		LUNA_RHI_API R<GraphicAdapterDesc> get_adapter_desc(u32 index);
		
		//! Calculates the subresource index of the specified subresource.
		inline constexpr u32 calc_subresource_index(u32 mip_slice, u32 array_slice, u32 mip_levels)
		{
			return mip_slice + array_slice * mip_levels;
		}

		//! Calculates the mipmap slice and array slice from subresource index.
		inline constexpr void calc_mip_array_slice(u32 subresource, u32 mip_levels, u32& mip_slice, u32& array_slice)
		{
			mip_slice = subresource % mip_levels;
			array_slice = subresource / mip_levels;
		}

		LUNA_RHI_API R<Ref<IDevice>> new_device(u32 adapter_index);

		//! Gets the main graphical device.
		LUNA_RHI_API IDevice* get_main_device();

		//! Creates a swap chain resource and binds it to the specified window.
		//! @param[in] queue The command queue to push the present commands to.
		//! @param[in] window The window this swap chain should be outputted to.
		//! @param[in] desc The descriptor object of the swap chain.
		//! @return Returns the new created swap chain, or `nullptr` if failed to create.
		LUNA_RHI_API R<Ref<ISwapChain>> new_swap_chain(ICommandQueue* queue, Window::IWindow* window, const SwapChainDesc& desc);

		enum class APIType : u8
		{
			d3d12,
			vulkan,
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
	}
}