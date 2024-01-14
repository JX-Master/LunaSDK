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
		enum class BackendType : u8
		{
			d3d12,
			vulkan,
			metal,
		};

		//! Gets the render backend type.
		LUNA_RHI_API BackendType get_backend_type();
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
	struct Module;
	LUNA_RHI_API Module* module_rhi();
}