/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.cpp
* @author JXMaster
* @date 2022/4/14
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RHI_API LUNA_EXPORT
#include "RHI.hpp"
#include <Luna/Runtime/Module.hpp>
#include "../DescriptorSet.hpp"
namespace Luna
{
	namespace RHI
	{
		struct RHIModule : public Module
		{
			virtual const c8* get_name() override { return "RHI"; }
			virtual RV on_register() override
			{
				return add_dependency_module(this, module_window());
			}
			virtual RV on_init() override
			{
				return render_api_init();
			}
			virtual void on_close() override
			{
				render_api_close();
			}
		};
	}
	LUNA_RHI_API Module* module_rhi()
	{
		static RHI::RHIModule m;
		return &m;
	}
	namespace RHIError
	{
		LUNA_RHI_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("RHIError");
			return e;
		}

		LUNA_RHI_API ErrCode device_hung()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_hung");
			return e;
		}
		LUNA_RHI_API ErrCode device_reset()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_reset");
			return e;
		}
		LUNA_RHI_API ErrCode device_removed()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "device_removed");
			return e;
		}
		LUNA_RHI_API ErrCode driver_internal_error()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "driver_internal_error");
			return e;
		}
		LUNA_RHI_API ErrCode frame_statistics_disjoint()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "frame_statistics_disjoint");
			return e;
		}
		LUNA_RHI_API ErrCode swap_chain_out_of_date()
		{
			static ErrCode e = get_error_code_by_name("RHIError", "swap_chain_out_of_date");
			return e;
		}
	}
}