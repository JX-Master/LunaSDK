/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file RHI.cpp
* @author JXMaster
* @date 2022/4/14
*/
#include "RHI.hpp"
#include <Runtime/Module.hpp>
#include "../DescriptorSet.hpp"
namespace Luna
{
	namespace RHI
	{
		RV init()
		{
			lutry
			{
				luexp(render_api_init());
			}
			lucatchret;
			return ok;
		}

		void close()
		{
			render_api_close();
		}

		StaticRegisterModule m("RHI", "Window", init, close);
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